#include <mcs51reg.h>
#include <cc2530.h>
#include "usb.h"
#include "interface.h"

// (c) Copyright Paul Campbell paul@taniwha.com 2013
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by Paul Campbell , who shall
// act as a proxy defined in Section 6 of version 3 of the license.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

unsigned char __data eventMaskIn;
unsigned char __data eventMaskOut;
static unsigned char __data eventMaskCtrl;
static unsigned char __pdata irqMaskCtrl;
static unsigned char __pdata irqMaskIn;
static unsigned char __pdata irqMaskOut;

__bit inSuspend;
extern __bit pool_busy;
static void usb_handler(struct task __xdata *p);
static struct task __xdata usb_task = {usb_handler, 0, 0, 0};

struct sh __pdata setup_header;

unsigned char __xdata * __data pData;
unsigned int __data bytes_left;
unsigned char __pdata usbState;             ///< USB device state
unsigned char __xdata configurationValue;   ///< Current configuration value
unsigned char __xdata pAlternateSetting[5];  ///< Current alternate settings
unsigned char __pdata ep0Status;             ///< Endpoint 0 status
unsigned char __pdata pEpInStatus[5];        ///< Endpoint 1-5 IN status
unsigned char __pdata pEpOutStatus[5];
unsigned char __pdata remoteWakeup;  

#ifndef NULL
#define NULL	0
#endif

static void (* __pdata ProcessFunc)();


void usb_intr(void);
void
usb_init()
{
    	eventMaskIn = 0x00;
    	eventMaskOut = 0x00;
    	eventMaskCtrl = 0x00;
    	inSuspend = 0;
    	irqMaskCtrl = 0xff;
    	irqMaskIn = 0xff;
    	irqMaskOut = 0x3e;
	
    	USBCIE = 0xff;
    	USBIIE = 0xff;
    	USBOIE = 0x3E;
	
    	P2IFG = 0;
	P2IF = 0;
	P2IEN |= 0x20;
	IEN2 |= 0x02; 

	p2_vect = usb_intr;

	USBCTRL= USBCTRL_PLL_EN|USBCTRL_USB_EN;
        while (!(USBCTRL&USBCTRL_PLL_LOCKED))
		; 
	USBCIE |= USBCIE_RESUMEIE;
}

void
usb_intr(void) __naked
{
	__asm;
	push	PSW
	push	ACC
	push	_DPS
	push	dpl
	push	dph
	push	_DPL1
	push	_DPH1
	push	ar0
	push	ar1
	push	ar2
	push	ar3
	push	ar4
	push	ar5
	push	ar6
	push	ar7
	mov	_DPS, #0
0001$:		mov	a, _SLEEPSTA	//	while (!(SLEEPSTA & SLEEP_XOSC_STB_BM));
		jnb	a.6, 0001$

	mov	dptr, #_USBCIF
   	movx	a, @dptr 		//	u = USBCIF;


	jnb	a.0, 0003$		//	if (u&USBCIF_SUSPENDIF) 
		setb _inSuspend		//		inSuspend = 1;
		clr  p1.1
0003$:
	jb 	a.2, 0004$		//	if (u&(USBCIF_RSTIF|USBCIF_RESUMEIF)) 
	jnb	a.1, 0005$
0004$:
		clr	_inSuspend	//		inSuspend = 0;
		mov	c, _pool_busy
		mov	p1.1, c
0005$:

	jnb	a.2, 0002$		//	 if (u&USBCIF_RSTIF) {
		mov	r1, #_irqMaskCtrl	//	 
		movx	a, @r1		//		USBCIE = irqMask;
		mov	dptr, #_USBCIE
		movx	@dptr, a
		
		mov	r1, #_irqMaskIn	//	 
		movx	a, @r1		//		USBIIE = irqMask;
		mov	dptr, #_USBIIE
		movx	@dptr, a

		mov	r1, #_irqMaskOut	//	 
		movx	a, @r1		//		USBOIE = irqMask;
		mov	dptr, #_USBOIE
		movx	@dptr, a

		mov	dptr, #_USBPOW	//		USBPOW |= USBPOW_SUSPEND_EN;
		movx	a, @dptr
		orl	a, #USBPOW_SUSPEND_EN
		movx	@dptr, a
   					//	}
0002$:
	anl	a, #0x0f
	orl	a, _eventMaskCtrl	//	eventMask |= USN;
	mov	_eventMaskCtrl, a

	mov	dptr, #_USBIIF		//	 eventMask |= USBIIF;
	movx	a, @dptr
	orl	a, _eventMaskIn
	mov	_eventMaskIn, a
	
	mov	dptr, #_USBOIF		//	eventMask |= USBOIF << 9;
	movx	a, @dptr
	orl	a, _eventMaskOut
	mov	_eventMaskOut, a


					//	if (P2IFG&P2IFG_DPIF) {
	mov	a, _P2IFG
	jnb	a.5, 0010$
		anl	_P2IFG, #~(1<<5)//		P2IFG = ~P2IFG_DPIF;
		clr	_inSuspend	//		inSuspend = 0;	
		mov	c, _pool_busy
		mov	p1.1, c
					//	}
0010$:
	mov	_P2IFG, #0		//	P2IFG = 0;
	clr	_P2IF			//	P2IF = 0; 

	mov	dptr, #_usb_task
	lcall	_queue_task_0

	pop	ar7
	pop	ar6
	pop	ar5
	pop	ar4
	pop	ar3
	pop	ar2
	pop	ar1
	pop	ar0
	pop	_DPH1
	pop	_DPL1
	pop	dph
	pop	dpl
	pop	_DPS
	pop	ACC
	pop	PSW
	reti
	__endasm;
}

static unsigned char __code * __data pDesc;
unsigned char __code*
find_next(unsigned char type, unsigned char halt) __naked
{
	__asm;
//	unsigned char __code *p;

	mov	r2, dpl			//	p = NULL;
	mov	r0, #_find_next_PARM_2
	movx	a, @r0
	mov	r3, a
	mov	dpl, _pDesc
	mov	dph, _pDesc+1
0001$:	
		clr	a		//	while (pDesc[0] != 0) {
		movc	a, @a+dptr
		jz	0002$
		mov	r4, a

		mov	a, #1		//		if (pDesc[1] == type) {
		movc	a, @a+dptr
		cjne	a, ar2, 0003$

					//			p = pDesc;
			mov	a, r4	//			pDesc += pDesc[0];
			add	a, dpl
			mov	_pDesc, a
			clr	a
			addc	a, dph
			mov	_pDesc+1, a
			ret		//			break;
					//		} else
0003$:		
		cjne	a, ar3, 0004$	//		if (halt && pDesc[1] == halt) 
		mov	a, r3		//			break;
		jnz	0002$
0004$:
		mov	a, r4		//		pDesc += pDesc[0];
		add	a, dpl
		mov	dpl, a
		clr	a
		addc	a, dph
		mov	dph, a
		sjmp	0001$		//	}
0002$:
	mov     _pDesc, dpl
	mov     _pDesc+1, dph

	mov	dptr, #0		//	return p;
	ret
	__endasm;
} 

static unsigned char __xdata temp_desc[256];
static unsigned char __xdata *
copy_to_xdata(unsigned char __code *p, unsigned char l) __naked
{
	__asm;
	clr	_DPS
	mov	r0, #_copy_to_xdata_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_temp_desc
	mov	_DPH1, #_temp_desc>>8
0001$:		clr	a
		movc	a, @a+dptr
		inc	dptr
		inc	_DPS
		movx	@dptr, a
		inc	dptr
		dec	_DPS
		djnz	r0, 0001$
	mov	dptr, #_temp_desc
	ret
	__endasm;
}


unsigned char __xdata *
get_device_desc()
{
	unsigned char __code *p;
	pDesc = &descs[0];
	p = find_next(DESC_TYPE_DEVICE, 0);
	return copy_to_xdata(p, p[0]);
}

unsigned char __xdata *
get_config_desc(unsigned char val, unsigned char ind)
{
	unsigned char __code *p;
	pDesc = &descs[0];
	while (p =  find_next(DESC_TYPE_CONFIG, 0)) {
		if (val) {
			if (val == ((struct configuration_desc __code *)p)->bConfigurationValue) 
				return copy_to_xdata(p, p[2]);
		} else {
			if (!ind)
				return copy_to_xdata(p, p[2]);
			ind--;
		}
	}
	return 0;
}

unsigned char __code *
get_interface_desc(unsigned char val, unsigned char num, unsigned char alt)
{
	unsigned char __code *p;
	pDesc = &descs[0];
	get_config_desc(val, 0);
	while (p = find_next(DESC_TYPE_INTERFACE, DESC_TYPE_CONFIG)) {
		if (num == ((struct interface_desc __code *)p)->bInterfaceNumber && 
		    alt == ((struct interface_desc __code *)p)->bAlternateSetting) 
			return p;
	}
	return 0;
}

unsigned char __xdata *
get_string_desc(unsigned char ind)
{
	unsigned char __code *p;
	pDesc = &descs[0];
	for (;;) {
		p = find_next(DESC_TYPE_STRING, 0);
		if (!p)
			return 0;
		if (ind == 0)
			return copy_to_xdata(p, p[0]);
		ind--;
	}
}

void
usb_read_fifo_0_p(unsigned char __pdata *pd, unsigned char count)
{
	__asm;
	mov	r0, #_usb_read_fifo_0_p_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	r1, dpl
	mov	dptr, #_USBF0
0001$:		movx	a, @dptr
		movx	@r1, a
		inc	r1
		djnz	r0, 0001$
	ret
	__endasm;
}
void
usb_read_fifo_0(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_0_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF0
	mov	_DPH1, #_USBF0>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_read_fifo_5(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_5_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF5
	mov	_DPH1, #_USBF5>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_read_fifo_4(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_4_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF4
	mov	_DPH1, #_USBF4>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_read_fifo_3(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_3_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF3
	mov	_DPH1, #_USBF3>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_read_fifo_2(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_2_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF2
	mov	_DPH1, #_USBF2>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_read_fifo_1(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	clr	_DPS
	mov	r0, #_usb_read_fifo_1_PARM_2
	movx	a, @r0
	mov	r0, a
	mov	_DPL1, #_USBF1
	mov	_DPH1, #_USBF1>>8
0001$:		inc	_DPS
		movx	a, @dptr
		dec	_DPS
		movx	@dptr, a
		inc	dptr
		djnz	r0, 0001$
	ret
	__endasm;
}

void
usb_write_fifo_0(unsigned char __xdata *pd, unsigned char count)
{
	__asm;
	mov	r0, #_usb_write_fifo_0_PARM_2
	movx	a, @r0
	jz	0002$
	mov	r0, a
	mov	_DPL1, #_USBF0
	mov	_DPH1, #_USBF0>>8
0001$:		movx	a, @dptr
		inc	dptr
		inc	_DPS
		movx	@dptr, a
		dec	_DPS
		djnz	r0, 0001$
0002$:
	ret
	__endasm;
}

static unsigned char __xdata xtmp[32];

static void usb_has_data_1()
{
	for (;;) {
		unsigned char c;
		unsigned char l, x, x2;

		EA=0;
		c = USBINDEX;
		USBINDEX = 1;
		if (!(USBCSOL&USBCSOL_OUTPKT_RDY)) {
			USBINDEX = c;
			EA=1;
			return;
		}
		x = USBCNTL;
		x2 = USBCNTH;
		if (x2 || x > sizeof(xtmp)) {
			l = sizeof(xtmp);
		} else {
			l = x;
		}
		usb_read_fifo_1(&xtmp[0], l);
		if (l == x && !x2) {
			USBINDEX = 1;
			USBCSOL = 0;
		}
		EA=1;
	}
}
static void usb_has_data_2()
{

	for (;;) {
		unsigned char c;
		unsigned char l, x, x2;

		EA=0;
		c = USBINDEX;
		USBINDEX = 2;
		if (!(USBCSOL&USBCSOL_OUTPKT_RDY)) {
			USBINDEX = c;
			EA=1;
			return;
		}
		x = USBCNTL;
		x2 = USBCNTH;
		if (x2 || x > sizeof(xtmp)) {
			l = sizeof(xtmp);
		} else {
			l = x;
		}
		usb_read_fifo_2(&xtmp[0], l);
		if (l == x && !x2) {
			USBINDEX = 2;
			USBCSOL = 0;
		}
		EA=1;
	}
		
}
static void usb_has_data_3()
{

	for (;;) {
		unsigned char c;
		unsigned char l, x, x2;

		EA=0;
		c = USBINDEX;
		USBINDEX = 3;
		if (!(USBCSOL&USBCSOL_OUTPKT_RDY)) {
			USBINDEX = c;
			EA=1;
			return;
		}
		x = USBCNTL;
		x2 = USBCNTH;
		if (x2 || x > sizeof(xtmp)) {
			l = sizeof(xtmp);
		} else {
			l = x;
		}
		usb_read_fifo_3(&xtmp[0], l);
		if (l == x && !x2) {
			USBINDEX = 3;
			USBCSOL = 0;
		}
		EA=1;
	}
		
}
static void usb_has_data_4()
{

	for (;;) {
		unsigned char c;
		unsigned char l, x, x2;

		EA=0;
		c = USBINDEX;
		USBINDEX = 4;
		if (!(USBCSOL&USBCSOL_OUTPKT_RDY)) {
			USBINDEX = c;
			EA=1;
			return;
		}
		x = USBCNTL;
		x2 = USBCNTH;
		if (x2 || x > sizeof(xtmp)) {
			l = sizeof(xtmp);
		} else {
			l = x;
		}
		usb_read_fifo_4(&xtmp[0], l);
		if (l == x && !x2) {
			USBINDEX = 4;
			USBCSOL = 0;
		}
		EA=1;
	}
		
}

static unsigned char
change_feature(unsigned char set)
{
	unsigned char endpoint;

	if (setup_header.length || usbState != DEV_CONFIGURED && (setup_header.index != 0)) {
		ep0Status = EP_STALL;
   	} else {
		switch (setup_header.requestType&RT_MASK_RECIP) {
      		case RT_RECIP_DEV:
         		if ((setup_header.value&0xff) != DEVICE_REMOTE_WAKEUP) 
            			return 0;
            		remoteWakeup = set;
         		break;
		case RT_RECIP_IF:
			return 0;
		case RT_RECIP_EP:
			endpoint = setup_header.index&0x7F;
         		if ((setup_header.value&0xff) != ENDPOINT_HALT) 
            			return 0;
         		if (endpoint > 5) {
            			ep0Status = EP_STALL;
         		} else {
            			USBINDEX = endpoint;
            			if (setup_header.index&0x80) {
               				USBCSIL = set ? USBCSIL_SEND_STALL : USBCSIL_CLR_DATA_TOG;
               				pEpInStatus[endpoint - 1] = set ? EP_HALT : EP_IDLE;
            			} else {
               				USBCSOL = set ? USBCSOL_SEND_STALL : USBCSOL_CLR_DATA_TOG;
               				pEpOutStatus[endpoint - 1] = set ? EP_HALT : EP_IDLE;
            			}
            			USBINDEX = 0;
			}
         		break;
      		default:
         		ep0Status = EP_STALL;
         		break;
      		}
   	}
   	return 1;
}

static void
config_endpoints(struct interface_desc __code *pi)
{
	unsigned char n;

	for (n = 0; n < pi->bNumEndpoints; n++) {
		struct endpoint_desc __code *pe;
		if (pe = (struct endpoint_desc __code *)find_next(DESC_TYPE_ENDPOINT, 0)) {
			unsigned char endpoint;
			unsigned int maxpRegValue;
			unsigned char csRegValue;

			endpoint = pe->bEndpointAddress & 0x0F;
			USBINDEX = endpoint;
			csRegValue = 0x00;
			maxpRegValue = (pe->wMaxPacketSize + 7) >> 3;

			if (pe->bEndpointAddress&0x80) {	// in
				USBCSIL = USBCSIL_CLR_DATA_TOG | USBCSIL_FLUSH_PACKET;
				USBCSIL = USBCSIL_FLUSH_PACKET;
				if ((pe->bmAttributes&EP_ATTR_TYPE_BM) == EP_ATTR_ISO)
					csRegValue |= USBCSIH_ISO;  // ISO flag
				if (endpoint >= 4)
					csRegValue |= USBCSIH_IN_DBL_BUF;          // Double buffering
				USBCSIH = csRegValue;
				USBMAXI = maxpRegValue;
				pEpInStatus[endpoint - 1] = EP_IDLE;
			} else {
				USBCSOL = USBCSOL_CLR_DATA_TOG | USBCSOL_FLUSH_PACKET;
				USBCSOL = USBCSOL_FLUSH_PACKET;
				if ((pe->bmAttributes&EP_ATTR_TYPE_BM) == EP_ATTR_ISO)
					csRegValue |= USBCSOH_ISO;  // ISO flag
				if (endpoint >= 4)
					csRegValue |= USBCSOH_OUT_DBL_BUF;        // Double buffering
				USBCSOH = csRegValue;
				USBMAXO = maxpRegValue;
				pEpOutStatus[endpoint - 1] = EP_IDLE;
			}
			USBINDEX = 0;
		}
	}
} 


static unsigned int __xdata status;

static void
usb_handler(struct task __xdata *p)
{

	for (;;) {
		EA = 0;
    		if (eventMaskCtrl&USBCIF_RSTIF) {
			u8 i;
        		eventMaskCtrl &= ~USBCIF_RSTIF;
			EA = 1;
  			usbState = DEV_DEFAULT;
   			configurationValue = 0;

   			ep0Status = EP_IDLE;
   			for (i = 0; i < sizeof(pEpInStatus); i++)
       				pEpInStatus[i] = EP_HALT;
   			for (i = 0; i < sizeof(pEpOutStatus); i++)
       				pEpOutStatus[i] = EP_HALT;
   			ProcessFunc = NULL;
			continue;
		}

		if (eventMaskIn&USBIIE_EP0IE) {
  			unsigned char controlReg;
   			unsigned char bytesNow;
   			unsigned char oldEndpoint;
        		eventMaskIn &= ~USBIIE_EP0IE;
			EA = 1;

   			oldEndpoint = USBINDEX;
   			USBINDEX = 0;
   			controlReg = USBCS0;
   			if (controlReg&USBCS0_SETUP_END) {
      				USBCS0 = USBCS0_CLR_SETUP_END;
      				ep0Status = EP_CANCEL;
      				if (ProcessFunc)
					ProcessFunc();
      				ep0Status = EP_IDLE;
   			}

			if (controlReg&USBCS0_SENT_STALL) {
				USBCS0 = 0x00;
				ep0Status = EP_IDLE;
			}

   			if (ep0Status == EP_RX) {
      				bytesNow = USBCNT0;
      				usb_read_fifo_0(pData, bytesNow);
      				bytes_left -= bytesNow;
      				pData += bytesNow;

      				USBCS0 = bytes_left ? USBCS0_CLR_OUTPKT_RDY : (USBCS0_CLR_OUTPKT_RDY | USBCS0_DATA_END);
      				if (bytes_left == 0) {
         				if (ProcessFunc)
						ProcessFunc();
         				ep0Status = EP_IDLE;
      				}
      				USBINDEX = oldEndpoint;
				EA=0;
				goto do_next;
   			} else
   			if (ep0Status == EP_IDLE) {
      				if (controlReg&USBCS0_OUTPKT_RDY) {
         				usb_read_fifo_0_p((unsigned char __pdata *)&setup_header, 8);
         				ProcessFunc = NULL;
         				switch (setup_header.requestType & (RT_MASK_TYPE | RT_MASK_DIR)) {
         				case RT_STD_OUT:
            					switch (setup_header.request) {
            					case SET_ADDRESS:       
   							if (setup_header.index || setup_header.length || setup_header.value&0xff80) {
      								ep0Status = EP_STALL;
								break;
							}
							{
								unsigned char a = setup_header.value;
      								USBADDR = a;
      								if (a != 0) {
         								if (usbState == DEV_DEFAULT)
										usbState = DEV_ADDRESS;
      								} else {
         								if (usbState == DEV_ADDRESS)
										usbState = DEV_DEFAULT;
   								}
							}
							break;
            					case SET_FEATURE:  
							if (change_feature(1)) 
								ep0Status = EP_STALL;
							break;
            					case CLEAR_FEATURE:
							if (change_feature(0)) 
								ep0Status = EP_STALL;
							break;
            					case SET_CONFIGURATION:
   							if (usbState == DEV_DEFAULT || setup_header.index || setup_header.length || setup_header.value&0xff00) {
      								ep0Status = EP_STALL;
								break;
   							}
      							if (setup_header.value&0xff) {
   								struct configuration_desc __xdata *pConfiguration;

  								pConfiguration = (struct configuration_desc __xdata *)get_config_desc(setup_header.value&0xff, 0);
         							if (pConfiguration) {
   									unsigned char n;
            								usbState = DEV_CONFIGURED;
            								configurationValue = setup_header.value&0xff;
            								for (n = 0; n < pConfiguration->bNumInterfaces; n++) {
   										struct interface_desc __code *pi;
               									pAlternateSetting[n] = 0x00;
               									do {
                  									pi = (struct interface_desc __code *)find_next(DESC_TYPE_INTERFACE, 0);
               									} while (pi->bAlternateSetting != pAlternateSetting[n]);
               									config_endpoints(pi);
									}
         							} else {
            								ep0Status = EP_STALL;
         							}
      							} else {
								u8	i;
         							configurationValue = setup_header.value;
         							usbState = DEV_ADDRESS;
   								for (i = 0; i < sizeof(pEpInStatus); i++)
       									pEpInStatus[i] = EP_HALT;
   								for (i = 0; i < sizeof(pEpOutStatus); i++)
       									pEpOutStatus[i] = EP_HALT;
      							}
							break;
            					case SET_INTERFACE:   
							if (usbState != DEV_CONFIGURED || setup_header.requestType != RT_OUT_INTERFACE || setup_header.length) {
								ep0Status = EP_STALL;
								break;
							}
							{
								struct interface_desc __code *pi;
								if (pi = (struct interface_desc __code *)get_interface_desc(configurationValue, setup_header.index, setup_header.value)) {
									pAlternateSetting[setup_header.index] = setup_header.value;
									config_endpoints(pi);
								} else {
									ep0Status = EP_STALL;
								}
							} 
			 				break;
            					case SET_DESCRIPTOR:    
            					default:
							ep0Status = EP_STALL;
							break;
            					}
            					break;
         				case RT_STD_IN:
            					switch (setup_header.request) {
            					case GET_STATUS: 
   							if (setup_header.value || setup_header.index&0xff00 || setup_header.length != 2) {
      								ep0Status = EP_STALL;
								break;
   							} 
      							switch (setup_header.requestType) {
      							case RT_IN_DEVICE:
         							if (setup_header.index&0xff) {
            								ep0Status = EP_STALL;
	    								break;
         							} 
            							status = selfPowered ? 0x0001 : 0x0000;
            							if (remoteWakeup)
									status |= 0x0002;
         							break;
      							case RT_IN_INTERFACE:
         							if (usbState != DEV_CONFIGURED) {
            								ep0Status = EP_STALL;
         							} else {
            								status = 0x0000;
         							}
         							break;
      							case RT_IN_ENDPOINT:
								{
   									unsigned char endpoint;
         								endpoint = setup_header.index&0x7F;
         								if (usbState != DEV_CONFIGURED || endpoint > 5) {
            									ep0Status = EP_STALL;
										break;
         								} 
            								if (setup_header.index&0x80) {
               									status = (pEpInStatus[endpoint-1] == EP_HALT) ? 0x0001 : 0x0000;
            								} else {
               									status = (pEpOutStatus[endpoint-1] == EP_HALT) ? 0x0001 : 0x0000;
            								}
								}
         							break;
      							default:
         							ep0Status = EP_STALL;
         							break;
      							}
      							if (ep0Status != EP_STALL) {
         							pData = (unsigned char __xdata *)&status;
         							bytes_left = 2;
         							ep0Status = EP_TX;
      							}
							break;
            					case GET_DESCRIPTOR:    
							switch (setup_header.value>>8) {
							case DESC_TYPE_DEVICE:
								pData = get_device_desc();
								bytes_left = pData[0];
								break;
							case DESC_TYPE_CONFIG:
								pData = get_config_desc(0, setup_header.value);
								bytes_left = pData[2];	// never more than 255
								break;
							case DESC_TYPE_STRING:
								pData = get_string_desc(setup_header.value);
								bytes_left = pData[0];
								break;
							default:
								{
									pData = NULL;
									bytes_left = 0;
								}
							}
							if (pData == NULL)
								ep0Status = EP_STALL;
							if (ep0Status != EP_STALL) {
								if (bytes_left > setup_header.length) 
									bytes_left = setup_header.length;
								ep0Status = EP_TX;
							}
							break;
            					case GET_CONFIGURATION:
  							if (setup_header.value || setup_header.index || setup_header.length != 1) {
      								ep0Status = EP_STALL;
								break;
							}
      							pData = &configurationValue;
      							bytes_left = 1;
      							ep0Status = EP_TX;
							break;
            					case GET_INTERFACE:     
							if (usbState != DEV_CONFIGURED || setup_header.requestType != RT_IN_INTERFACE || setup_header.value || (setup_header.length != 1)) {
      								ep0Status = EP_STALL;
								break;
							}
							pData = (unsigned char __xdata *)&pAlternateSetting[setup_header.index];
							bytes_left = 1;
							ep0Status = EP_TX;
							break;
            					case SYNCH_FRAME:    
            					default:      	
							ep0Status = EP_STALL;
							break;
            					}
            					break;
         				case RT_VEND_OUT:
            					ep0Status = EP_STALL;
            					break;
       					case RT_VEND_IN:
            					ep0Status = EP_STALL;
            					break;
         				case RT_CLASS_OUT:
            					ProcessFunc = class_out;
						class_out();
            					break;
         				case RT_CLASS_IN:
            					ProcessFunc = class_in;
						class_in();
            					break;
         				default:
            					ep0Status = EP_STALL;
            					break;
         				}
         				USBCS0 = (ep0Status == EP_STALL) ? (USBCS0_CLR_OUTPKT_RDY | USBCS0_SEND_STALL) : USBCS0_CLR_OUTPKT_RDY;
      				}
    			}
   			if (ep0Status == EP_TX) {
      				controlReg = USBCS0_INPKT_RDY;
      				if (bytes_left < EP0_PACKET_SIZE) {
         				bytesNow = bytes_left;
         				controlReg |= USBCS0_DATA_END;
      				} else {
         				bytesNow = EP0_PACKET_SIZE;
      				}
      				usb_write_fifo_0(pData, bytesNow);
      				pData += bytesNow;
      				bytes_left -= bytesNow;

      				USBCS0 = controlReg;
	
      				if (bytesNow < EP0_PACKET_SIZE) {
         				if (ProcessFunc)
						ProcessFunc();
         				ep0Status = EP_IDLE;
      				}
	
   			} 
   			USBINDEX = oldEndpoint;
			EA=0;
		}
do_next:
		if (eventMaskIn&USBIIE_INEP5IE) {
			USBINDEX = 5;
			if (!(USBCSIL&USBCSIL_INPKT_RDY)) {
				EA = 1;
				usb_can_send_5();
				EA=0;
			}
		}
		if (eventMaskOut&USBOIE_OUTEP5IE) {
        		eventMaskOut &= ~USBOIE_OUTEP5IE;
			USBINDEX = 5;
			if (USBCSOL&USBCSOL_OUTPKT_RDY) {
				EA = 1;
				usb_has_data_5();
				EA=0;
			}
		}
		if (eventMaskOut&USBOIE_OUTEP4IE) {
        		eventMaskOut &= ~USBOIE_OUTEP4IE;
		}
		if (eventMaskOut&USBOIE_OUTEP3IE) {
        		eventMaskOut &= ~USBOIE_OUTEP3IE;
		}
		if (eventMaskIn&USBIIE_INEP4IE) {
        		eventMaskIn &= ~USBIIE_INEP4IE;
			USBINDEX = 4;
			if (USBCSOL&USBCSOL_OUTPKT_RDY) {
				EA = 1;
				usb_has_data_4();
				EA=0;
			}
		}
		if (eventMaskIn&USBIIE_INEP3IE) {
        		eventMaskIn &= ~USBIIE_INEP3IE;
			USBINDEX = 3;
			if (USBCSOL&USBCSOL_OUTPKT_RDY) {
				EA = 1;
				usb_has_data_3();
				EA=0;
			}
		}
		if (eventMaskIn&USBIIE_INEP2IE) {
        		eventMaskIn &= ~USBIIE_INEP2IE;
			USBINDEX = 2;
			if (USBCSOL&USBCSOL_OUTPKT_RDY) {
				EA = 1;
				usb_has_data_2();
				EA=0;
			}
		}
		if (eventMaskIn&USBIIE_INEP1IE) {
        		eventMaskIn &= ~USBIIE_INEP1IE;
			USBINDEX = 1;
			if (USBCSOL&USBCSOL_OUTPKT_RDY) {
				EA = 1;
				usb_has_data_1();
				EA=0;
			}
		}
    		if (eventMaskCtrl&USBCIF_SUSPENDIF) {
			eventMaskCtrl &= ~USBCIF_SUSPENDIF;
			EA=1;

        		// Take the chip into PM1 until a USB resume is detected.
        		//usbsuspEnter();

			EA=0;
			eventMaskCtrl &= ~USBCIF_RESUMEIF;
			EA=1;
			continue;
    		}
		EA=1;
		break;
    	}
}
