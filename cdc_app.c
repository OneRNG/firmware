#define VERSION 2
__code char version[] = "\r\nVersion 2\r\n";
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

#define NORM_LED 1	// disable when debugging

#include <mcs51reg.h>
#include <cc2530.h>
#include "interface.h"
#include "string.h"
#include "packet_interface.h"
#include "usb.h"

#define RX_SIZE 250
#define TX_SIZE 250

//
//	UART access USB<->uart
//

#define POOL_SIZE (7*1024)

unsigned char __xdata pool[POOL_SIZE];
__xdata unsigned char *__data  pool_in = &pool[0];
__xdata unsigned char *__data  pool_out = &pool[0];
__xdata unsigned char *__data  pool_update = &pool[0];
unsigned int __data  pool_count=0;
__bit pool_busy=0;
__bit no_crc=0;
__bit no_avalanche=0;
__bit rf_source=0;
__bit rf_running=0;
__bit dumping=0;
__bit versioning=0;
static unsigned char __pdata time_rf_dwell = 0;
unsigned char __data vind;
#define MAX_00_S	8
#define MAX_FF_S	8

unsigned char __data num_zeros=0;
unsigned char __data num_ffs=0;
unsigned char __data error_state=0;
unsigned char __data dump_state;
unsigned int  __data dump_addr;
unsigned char  __data dump_page;

extern __bit cdcRTS;
__bit pause=1;
extern unsigned char __data eventMaskIn;
extern unsigned char __data eventMaskOut;

static void pool_rcv_thread(task __xdata*t);
static __xdata task pool_rcv_task = {pool_rcv_thread,0,0,0};

static void pool_cmd_thread(task __xdata*t);
static __xdata task pool_cmd_task = {pool_cmd_thread,0,0,0};

static void error_thread(task __xdata*t);
static __xdata task error_task = {error_thread,0,0,0};

static void rf_thread(task __xdata*t);
static __xdata task rf_task = {rf_thread,0,0,0};

static void
cpl() __naked
{
	__asm;
	cpl	p1.1
	ret
	__endasm;
}

static void
error_thread(task __xdata*t)
{
	if (num_zeros == MAX_00_S) {
		if (error_state == 1) {		// too many 00s? flash 1 on 3 off - stuck at 0
#ifdef NORM_LED
			P1_1 = 0;
#endif
			error_state = 0;
			queue_task(&error_task, 3*HZ);
		} else {
			P1_1 = 1;
			error_state = 1;
			queue_task(&error_task, 1*HZ);
		}
	} else
	if (num_ffs == MAX_FF_S) {		// too many FFs?  flash 1 on 1 off 1 on 3 off - stuck at 1
		if (error_state >= 3) {
#ifdef NORM_LED
			P1_1 = 0;
#endif
			error_state = 0;
			queue_task(&error_task, 3*HZ);
		} else
		if (error_state == 1) {
#ifdef NORM_LED
			P1_1 = 0;
#endif
			error_state = 2;
			queue_task(&error_task, 1*HZ);
		} else {
#ifdef NORM_LED
			P1_1 = 1;
#endif
			error_state++;
			queue_task(&error_task, 1*HZ);
		}
	} else {
		error_state = 0;		// normal state - just poll for an error state
//		P1_1 = pool_busy;
		queue_task(&error_task, 1*HZ);
	}
}

void
queue_pool_data(unsigned char x) __naked
{
	__asm;
	mov	_RNDH, dpl			//	RNDH = x;		// through the crc16
	jb	_no_crc, 0200$			//	if (!no_crc) {		// whiten the data
		mov	dpl, _RNDH		//		x = RNDH;	// hardware
						//	}
0200$:	clr	EA
	mov	a, _pool_count			//	if (pool_count == POOL_SIZE) {
	cjne	a, #POOL_SIZE, 0001$
	mov	a, _pool_count+1
	cjne	a, #POOL_SIZE>>8, 0001$		//		// the pool is full, keep stiring it
		mov	r0, dpl
		mov	dpl, _pool_update	//		*pool_update++ ^= x;
		mov	dph, _pool_update+1
		movx	a, @dptr
		xrl	a, r0
		movx	@dptr, a
		inc	dptr
		mov	a, dpl
		cjne	a, #_pool+POOL_SIZE, 0002$//		if (pool_update == &pool[POOL_SIZE])
		mov	a, dph			//			pool_update = &pool[0];
		cjne	a, #(_pool+POOL_SIZE)>>8, 0002$
			mov	dptr, #_pool
0002$:		mov	_pool_update, dpl
		mov	_pool_update+1, dph
		setb	EA
		ret
		
0001$:						//	} else {
		mov	a, dpl
		mov	dpl, _pool_in		//		*pool_in++ = x;
		mov	dph, _pool_in+1
		movx	@dptr, a
		inc	dptr
		mov	a, dpl			//		if (pool_in == &pool[POOL_SIZE]) 
		cjne	a, #_pool+POOL_SIZE, 0003$//			pool_in = &pool[0];
		mov	a, dph
		cjne	a, #(_pool+POOL_SIZE)>>8, 0003$
			mov	dptr, #_pool
0003$:		mov	_pool_in, dpl
		mov	_pool_in+1, dph
		inc	_pool_count		//		pool_count++;
		mov	a, _pool_count
		jnz	0004$
			inc	_pool_count+1
0004$:
		jb	_pool_busy, 0005$	//		if (!pool_busy && pool_count > 64) {
			mov	a, _pool_count+1
			jnz	0007$
			mov	a, _pool_count
			jb	a.7, 0007$
			jnb	a.6, 0005$
0007$:
			setb	_pool_busy	//			pool_busy = 1;
#ifdef NORM_LED
  			setb	p1.1		//			p1.1 = 1;
#endif
			mov	dptr, #_pool_rcv_task//			queue_task(&pool_rcv_task, 0_'
			lcall	_queue_task_0
0005$:						//		}
		setb	EA			//	}
		ret
	__endasm;
}

void
idle_pool() __naked
{
	__asm;						//	for (;; ) {
0001$:		jb	_no_avalanche, 0200$		//		if (!no_avalanche) {
		mov	r0, #8				//			for (i = 0; i < 8 i++)
0003$:							//				v = (v<<1)|p1.7;
			mov	c, p1.7
			rlc	a
			djnz	r0, 0003$
		mov	dpl, 	a
		jnz	0300$				//			if (v == 0) {
			mov	_num_ffs, #0		//				num_ffs = 0;
			mov	a, _num_zeros		//				if (num_zeros == MAX_00_S)
			cjne	a, #MAX_00_S, 0310$	//					goto retry
				sjmp	0200$
0310$:			inc	_num_zeros		//				num_zeros++;
			sjmp	0302$			//			} else
0300$:		cjne	a, #0xff, 0301$			//			if (v == 0xff) {
			mov	_num_zeros, #0		//				num_zeros = 0;
			mov	a, _num_ffs		//				if (num_ffs == MAX_FF_S)
			cjne	a, #MAX_FF_S, 0311$	//					goto retry;
				sjmp	0200$

0311$:			inc	_num_ffs		//				num_ffs++;
			sjmp	0302$			//			} else {
	
0301$:			mov	_num_zeros, #0		//				num_zeros = 0;
			mov	_num_ffs, #0		//				num_ffs = 0;
0302$:							//			}
		mov	a, _enter_sleep_mod_flag	//			if (enter_sleep_mod_flag) break;
		jz	0002$
		lcall	_queue_pool_data		//			queue_pool_data(v);
0200$:		mov	a, _enter_sleep_mod_flag	//retry:		if (enter_sleep_mod_flag) break;
		jz	0002$
							//		}
		jnb	_rf_running, 0001$		//		if (rf_running) {


			mov	a, _enter_sleep_mod_flag	//retry:	if (enter_sleep_mod_flag) break;
			jz	0002$
			mov	r0, #8			//			for (i = 0; i < 8;i++)
			mov	dptr, #_RFRND
			mov	r2, #0
0500$:				movx	a, @dptr	//				v = (v<<1)|RFRND&1;

				mov	c, a.0	// - note must wait 8 clocks between samples
				mov	a, r2
				rlc	a
				mov	r2, a
				nop
				djnz	r0, 0500$
			mov	dpl, r2
			mov	a, _enter_sleep_mod_flag//			if (enter_sleep_mod_flag) break;
			jz	0002$
			lcall	_queue_pool_data	//			queue_pool_data(v);
							//		}
0201$:		mov	a, _enter_sleep_mod_flag	//		if (enter_sleep_mod_flag) break;
		jnz	0001$
0002$:							//	}
	ret
	__endasm;
}

static unsigned char
get_5(void) __naked
{
	__asm;
	mov	dptr, #_USBF5
	movx	a, @dptr
	mov	dpl, a
	ret
	__endasm;
}

static unsigned char __data in_state=0;

static void
pool_cmd_thread(task __xdata*t) // woken when uart buffers have room new data or USB has data to send
{
	for (;;) {
		unsigned char c, a;
		unsigned char x;
		unsigned char x2;

		EA=0;
		c = USBINDEX;
		USBINDEX = 5;
		if (!(USBCSOL&USBCSOL_OUTPKT_RDY)) {
			USBINDEX = c;
			EA=1;
			return;
		}
		x = USBCNTL;
		x2 = USBCNTH;
		a = get_5();			// get a character in from USBA
		if (in_state == 0) {
			if (a == 'c')
				in_state=1;
		} else
		if (in_state == 1) {
			 if (a == 'm') {
				in_state=2;
			} else {
				in_state=0;
			}
		} else
		if (in_state == 2) {
			 if (a == 'd') {
				in_state=3;
			} else {
				in_state=0;
			}
		} else
		if (in_state == 3) { 
			in_state=0;
			if (a >= '0' && a <= '9') {
				a -= '0';
set:				// a is 4 bit mask
				//	 - bit 0 - disables CRC whitening
				//	 - bit 1 - enables RF source
				//	 - bit 2 - disables avalanche source
				if (a&1) {
					no_crc = 1;
				} else {
					no_crc = 0;
				}
				if (a&2) {
					if (!rf_source) {
						rf_source = 1;
						time_rf_dwell = 0;
						cancel_task(&rf_task);
						queue_task(&rf_task, 0);
					}
				} else {
					rf_source = 0;
					rf_running = 0;
					RFST = 0xef;
					RFST = 0xec;
				}
				if (a&4) {
					no_avalanche = 1;
				} else {
					no_avalanche = 0;
				}
			} else
			if (a >= 'a' && a <= 'f') {
				a = a-'a'+10;
				goto set;
			} else
			if (a >= 'A' && a <= 'F') {
				a = a-'A'+10;
				goto set;
			} else
			if (a == 'X') {	
				dumping = 1;
				dump_state = 0;
				dump_addr = 0x8000;
				dump_page = 0;
			} else 
			if (a == 'O') {
				pause = 0;	// run
                        	queue_task_0(&pool_rcv_task);

			} else 
			if (a == 'v') {
				vind = 0;	// version
				versioning = 1;
			} else 
			if (a == 'o') {
				pause = 1;	// pause
			} else 
			if (a == 'w') {		// flush
				pool_in = &pool[0];
				pool_out = &pool[0];
				pool_update = &pool[0];
				pool_count=0;
				pool_busy=0;
			}
		} else {
			in_state = 0;
		}
		if (x==1 && !x2) {
			USBINDEX = 5;
			USBCSOL = 0;
		}
		USBINDEX = c;
		EA=1;
	}
}
			


static void
pool_rcv_thread(task __xdata*t)	__naked	// woken when pool has new data or USB can send data
{
	__asm;
	mov	_DPS, #0
0001$:						//	for (;;) {
						//		unsigned int len;
						//		unsigned char ep, xlen;
		mov	a, _eventMaskIn		//		if (eventMaskIn&USBIIE_EP0IE)
		jb	a.0, 0020$		//			break;
		mov	a, _eventMaskOut
		jb	a.5, 0020$		//		if (eventMaskOut&USBOIE_OUTEP5IE)
						//			break
		jb	_pause, 0020$		//		if (pause) 
						//			return;
		jnb	_cdcRTS, 0020$		//		if (!cdcRTS) 
						//			return;
		clr	EA			//		EA = 0;
		jb	_dumping, 0002$		//		if (dumping) goto do_dump;
		jb	_versioning, 0002$	//		if (versioning) goto do_dump;
		mov	a, _pool_count+1	//		if (pool_count < 64) {
		jnz	0002$
		mov	a, _pool_count
		jb	a.7, 0002$
		jb	a.6, 0002$
#ifdef NORM_LED
  			clr	p1.1
#endif
			clr	_pool_busy	//			pool_busy = 0;
			setb	EA		//			EA = 1;
0020$:			ret			//			return;
0555$:		
			ljmp	0510$		//		}
0002$:
		mov	dptr, #_USBINDEX	//		ep = USBINDEX;
		movx	a, @dptr
		mov	r7, a
		mov	a, #5			//		USBINDEX = 5;
		movx	@dptr, a
		mov	dptr, #_USBCSIL		//		if (USBCSIL&USBCSIL_INPKT_RDY) {
		movx	a, @dptr
		jnb	a.0, 0003$
			mov	dptr, #_USBINDEX//			USBINDEX = ep;
			mov	a, r7
			movx	@dptr, a
			mov	a, _eventMaskIn	//			eventMaskIn &= ~USBIIE_INEP5IE;
			clr	a.5
			mov	_eventMaskIn, a
			setb	EA		//			EA = 1;
			ret			//			return;

1201$:			ljmp 	1200$
0003$:						//		}
		jb	_versioning, 1201$	//		if (versioning) goto do_version;
		jb	_dumping, 0555$		//		if (dumping) goto do_dump;
		mov	r2, #64			//		len = 64;
		clr	c
		mov	a, _pool_count		//		pool_count -= 64;
		subb	a, r2
		mov	_pool_count, a
		mov 	a, _pool_count+1
		subb	a, #0
		mov	_pool_count+1, a
		mov	_DPL1, #_USBF5		//		// usb_write_fifo_5(pool_out, len);
		mov	_DPH1, #_USBF5>>8
		mov	dpl, _pool_out		//		for (i = 0;i < 64; i++) {
		mov	dph, _pool_out+1
0010$:			movx	a, @dptr	//			*USBF5 = *pool_out;
			inc	_DPS
			movx	@dptr, a
			dec	_DPS
			clr	a		//			*pool_out++ = 0;	// delete data once it's sent
			movx	@dptr, a
			inc	dptr
			mov	a, dpl		//			if (pool_out == &pool[POOL_SIZE])
			cjne	a, #_pool+POOL_SIZE, 0011$
			mov	a, dph		//				pool_out = &pool[0];
			cjne	a, #(_pool+POOL_SIZE)>>8, 0011$
				mov	dptr, #_pool
0011$:			djnz	r2, 0010$	//		}
		mov	_pool_out, dpl
		mov	_pool_out+1, dph

0501$:						//send_usb:
		mov	dptr, #_USBINDEX	//		USBINDEX = 5;
		mov	a, #5
		movx	@dptr, a
		mov	a, #USBCSIL_INPKT_RDY	//		USBCSIL = USBCSIL_INPKT_RDY;
		mov	dptr, #_USBCSIL
		movx	@dptr, a
		mov	dptr, #_USBINDEX	//		USBINDEX = ep;
		mov	a, r7
		movx	@dptr, a
		setb	EA			//		EA = 1;
	ljmp	0001$				//	}

	//
	//	code to push rom image
	//
	//	looks like:
	//		fe ed be ef 20 14	- magic number
	//		00 00 04 		- len (little endian)
	//		00 00			- version (little endian)
	//		image			- len bytes of image 
	//
0510$:
	clr	_DPS
	mov	_DPL1, #_USBF5		
	mov	_DPH1, #_USBF5>>8
	mov	r2, #64					//	for (i = 0; i < 64; i++) {
	
0500$:
	mov	a, _dump_state				//	switch (dump_state) {
	cjne	a, #13, 0600$				//	case 11:
		mov	a, _dump_page			//		v = *{dump_page,dump_addr};
		mov	_FMAP, a				
		mov	dpl, _dump_addr
		mov	dph, _dump_addr+1
		clr	a
		movc	a, @a+dptr
		mov	r0, a
		inc	dptr				//		dump_addr++;
		mov	_dump_addr, dpl
		mov	a, dph
		mov	_dump_addr+1, a			//		if (dump_addr == 0) {
		jnz	0570$
			mov	_dump_addr+1, #0x80	//			dump_addr = 0x800;
			inc	_dump_page		//			dump_page++;
			mov	a, _dump_page		//			if (dump_page==8)
			cjne	a, #8, 0570$		//				dump_state++;
				inc	_dump_state	// last byte	}
0570$:		mov	a, r0				//		break;

0700$:		inc	_DPS				//		// at end of case statement
		movx	@dptr, a			//	*USBF5 = v;
		dec 	_DPS
	djnz	r2, 0500$
	ljmp	0501$					//	goto send_usb;


0600$:	inc	_dump_state				//	case 0:	dump_state++;
	cjne	a, #0, 0601$
		mov	a, #0xfe			//		v = 0xfe;
		sjmp	0700$				//		break;
0601$:	cjne	a, #1, 0602$				//	case 1: dump_state++;
		mov	a, #0xed			//		v = 0xed;
		sjmp	0700$				//		break;
0602$:	cjne	a, #2, 0603$				//	case 2: dump_state++;
		mov	a, #0xbe			//		v = 0xbe;
		sjmp	0700$				//		break;
0603$:	cjne	a, #3, 0604$				//	case 3: dump_state++;
		mov	a, #0xef			//		v = 0xef;
		sjmp	0700$				//		break;
0604$:	cjne	a, #4, 0605$				//	case 4: dump_state++;
		mov	a, #0x20			//		v = 0x20;
		sjmp	0700$				//		break;
0605$:	cjne	a, #5, 0606$				//	case 5: dump_state++;
		mov	a, #0x14			//		v = 0x14;
		sjmp	0700$				//		break;
0606$:	cjne	a, #6, 0607$				//	case 6: dump_state++;
		mov	a, #0x00			//		v = 0x00;
		sjmp	0700$				//		break;
0607$:	cjne	a, #7, 0608$				//	case 7: dump_state++;
		mov	a, #0x00			//		v = 0x00;
		sjmp	0700$				//		break;
0608$:	cjne	a, #8, 0609$				//	case 8: dump_state++;
		mov	a, #0x04			//		v = 0x04;
		sjmp	0700$				//		break;
0609$:	cjne	a, #9, 0610$				//	case 9: dump_state++;
		mov	a, #VERSION			//		v = VERSION;
		sjmp	0700$				//		break;
0610$:	cjne	a, #10, 0611$				//	case 10: dump_state++;
		mov	a, #VERSION>>8			//		v = VERSION>>8;
		sjmp	0700$				//		break;
0611$:	cjne	a, #11, 0612$				//	case 11: dump_state++;
		.globl s_CODE
		.globl l_CODE
		mov	a, #s_CODE
		add	a, #l_CODE			//		v = text size;
		sjmp	0700$				//		break;
0612$:	cjne	a, #12, 0613$				//	case 12: dump_state++;
		mov	a, #s_CODE
		add	a, #l_CODE
		mov	a, #s_CODE>>8			
		addc	a, #l_CODE>>8			//		v = (text size)>>8;
		sjmp	0700$				//		break;

0613$:	clr	_dumping				//	default:dumping = 0; 
	ljmp	0501$					//		goto send_usb;
							//	}
							// do_version
1200$:	
	clr	_DPS
	mov	_DPL1, #_USBF5		
	mov	_DPH1, #_USBF5>>8
	mov	dptr, #_version
1203$:							//	for (;;) {
		mov	a, _vind			//		v = version[vind++];
		inc	_vind				//
		movc	a, @a+dptr			//
		jz	1202$				//		if (!v)
							//			break;
		inc	_DPS				//
		movx	@dptr, a			//		*USBF5 = v;
		dec 	_DPS
		sjmp	1203$				//	}
1202$:
	clr	_versioning
	ljmp	0501$					//	goto send_usb;
	__endasm;
}


void
usb_can_send_5()
{
	queue_task(&pool_rcv_task, 0);
}

void
flush_in_5()
{
}

void
usb_has_data_5()
{
	queue_task(&pool_cmd_task, 0);
}

static void
uart_setup()
{
	EA = 0;
	cdc_init(9600);
	EA = 1;
}

void
change_uart_mode()
{
}


static void rf_thread(task __xdata*t)
{
	if (!rf_source) {	// idle .... poll
		queue_task(&rf_task, HZ/5);
		rf_running = 0;
		time_rf_dwell = 0;
		return;
	}
	if (time_rf_dwell == 0) {
		FREQCTRL = RNDH&0x3f;
		time_rf_dwell = (RNDL&0x1f)+80;
		FRMCTRL0 = (FRMCTRL0&(3<<2))|(1<<2);	// infinite receive mode
		RFST = 0xED;	// flush 
		RFST = 0xED;
		RFST = 0xec;
		RFST = 0xe3;
		rf_running = 0;
		queue_task(&rf_task, 2);
		return;
	}
	if (!(RSSISTAT&1)) {
		queue_task(&rf_task, 2);
		rf_running = 0;
		time_rf_dwell--;
		return;
	}
	rf_running = 1;
	time_rf_dwell--;
	queue_task(&rf_task, HZ/30);
}



unsigned char my_app(unsigned char op) 
{
	switch (op) {
	case APP_INIT:
		RNDL = 1;
		RNDL = 1;
		sys_active = 1;
		uart_setup();
    		P1DIR |= 1<<1;
    		P1INP |= 1<<1;
#ifdef NORM_LED
    		P1_1 = 1;
#else
		P1_1 = 0;
#endif
    		P1DIR &= ~(1<<7);
    		P1INP |= (1<<7);
		RFIRQM0 = 0;	// disable RF subsection interrupts
		RFIRQM1 = 0;
		FRMCTRL0 = (FRMCTRL0&(3<<2))|(1<<2);	// infinite receive mode
		queue_task(&error_task, 1*HZ);
		queue_task(&rf_task, HZ/2);
		break;
	case APP_GET_MAC:
		return 0;
	case APP_GET_KEY:
		break;
	case APP_GET_SUOTA_KEY:
		break;
	case APP_RCV_PACKET:
		break;
	}
	return 0;
}
