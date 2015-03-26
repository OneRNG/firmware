// (c) Copyright Paul Campbell paul@taniwha.com 2015
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

//
//	Manufacturing test
//
//	this file generates a stream of bits on P2.1 (the debug data pin) describing
//		the results of self test - (valid CRC, active RNG)
//		this provide no more useful information to an outside snooper
//		other than "the device is operating" (basically what the LED displays)
//

#include <mcs51reg.h>
#include <cc2530.h>
#include "interface.h"

static void manufacturing_thread(task __xdata*t);
__xdata task manufacturing_task = {manufacturing_thread,0,0,0};
static __data unsigned char manufacturing_state=0;
static __bit crc_ok;		// should be 1
extern __bit pool_busy;		// should be 1
extern unsigned char __data error_state; // should be 0

static void check_manufacturing_crc_16() __naked
{
	__asm;
	mov	r6, _RNDL
	mov	r7, _RNDH
	mov	_RNDL, #0xff			// crc = 0xff
	mov	_RNDL, #0xff
	
	mov	_FMAP, #0x00			// for (ptr = 0; ptr < (top-of-mem-32); ptr++) 
0001$:
		mov	dptr, #0x8000
0002$:
			clr	a
			movc	a, @a+dptr	//	crc = crc16(crc, *ptr);
			mov	_RNDH, a
			inc	dptr
			mov	a, dph
			jnz	0002$
		mov	a, _FMAP
		inc	a
		mov	_FMAP, a
		cjne	a, #7, 0001$

		mov	dptr, #0x8000
0003$:
			clr	a
			movc	a, @a+dptr
			mov	_RNDH, a
			inc	dptr
			mov	a, dph
			cjne	a, #0xff, 0003$
0004$:
			clr	a
			movc	a, @a+dptr
			mov	_RNDH, a
			inc	dptr
			mov	a, dpl
			cjne	a, #0xe0, 0004$
	clr	a	
	movc	a, @a+dptr	// lsb CRC	// crc_ok = crc == *(u16*)ptr;
	mov	r0, a
	inc	dptr
	clr	a
	movc	a, @a+dptr	// msb CRC
	mov	r1, a
	mov	a, _RNDL
	cjne	a, ar0, 0005$
	mov	a, _RNDH
	cjne	a, ar1, 0005$
		setb	_crc_ok
		sjmp	0006$
0005$:		clr	_crc_ok
0006$:
	mov	_RNDL, r7
	mov	_RNDL, r6
	mov	_ADCCON1, #0x00	// go
	ret
	__endasm;
}

static void
manufacturing_thread(task __xdata*t)
{
	switch (manufacturing_state) {	
	case 0: check_manufacturing_crc_16();	// fall thru
		P2DIR |= 1<<1;
                P2INP |= 1<<1;
		P2_1 = 0;
	case 1: manufacturing_state = 2;	// frame 00110101
		P2_1 = 0;
		break;
	case 2: manufacturing_state = 3;	// frame 00110101
		P2_1 = 0;
		break;
	case 3: manufacturing_state = 4;	// frame 00110101
		P2_1 = 1;
		break;
	case 4: manufacturing_state = 5;	// frame 00110101
		P2_1 = 1;
		break;
	case 5: manufacturing_state = 6;	// frame 00110101
		P2_1 = 0;
		break;
	case 6: manufacturing_state = 7;	// frame 00110101
		P2_1 = 1;
		break;
	case 7: manufacturing_state = 8;	// frame 00110101
		P2_1 = 0;
		break;
	case 8: manufacturing_state = 9;	// frame 00110101
		P2_1 = 1;
		break;
	case 9: manufacturing_state = 10;	// CRC - should be 1
		P2_1 = crc_ok;			
		break;
	case 10:manufacturing_state = 11;	// no error - should be 1
		if (error_state == 0) {
			P2_1 = 1;
		} else {
			P2_1 = 0;
		}
		break;
	case 11:manufacturing_state = 1;	// no error - should be 1
		P2_1 = pool_busy;
		break;
	default:
		manufacturing_state = 1;
		break;
	}
	queue_task(&manufacturing_task, HZ/10);
}
