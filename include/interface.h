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

#ifndef __interface_h
#define __interface_h

#include <mcs51reg.h>
#include <cc2530.h>
#include "task.h"

#ifndef THIS_ARCH
#define THIS_ARCH	1	// initial CC2533	// 0 unavailable as an extension
#endif
#ifndef THIS_CODE_BASE
#define THIS_CODE_BASE	0	// initial CC2533
#endif
typedef struct code_hdr {	// caution: these offsets are hard coded in suota assembly, fixcrc and packet_interface.cpp
        unsigned char    crc[4];         
        unsigned char    len[2];
	unsigned char    arch;
        unsigned char    code_base;
        unsigned char    version[2];
        unsigned char    data[1];
} code_hdr;


extern unsigned char (* __data x_app) (u8 v);
#define APP_INIT		0
#define APP_GET_MAC 		1
#define APP_GET_KEY 		2
#define APP_GET_SUOTA_KEY 	3
#define APP_RCV_PACKET		4
#define APP_LOW_LEVEL_INIT	0xff
#define APP_WAKE		5
#define APP_KEY			6
#define APP_SUOTA_START		7
#define	APP_SUOTA_DONE		8
__bit __at (0x06) sys_active;	
#define		KEY_X		0
#define		KEY_O		1
#define		KEY_LEFT	2
#define		KEY_RIGHT	3

// call backs
#ifdef DRV_LEDS
extern void leds_rgb(unsigned char * __xdata);
extern void leds_off();
#endif
extern void uart_init();
extern void putchar(char c);
extern void putstr(char __code *cp);
extern void puthex(unsigned char v);


extern void (* __pdata uart_rx_0_vect) ();
extern void (* __pdata uart_rx_1_vect) ();
extern void (* __pdata uart_tx_0_vect) ();
extern void (* __pdata uart_tx_1_vect) ();
extern void (* __pdata p0_vect) ();
extern void (* __pdata p1_vect) ();
extern void (* __pdata p2_vect) ();
extern void (* __pdata t2_vect) ();
extern void (* __pdata t3_vect) ();
extern void (* __pdata t4_vect) ();
extern void (* __pdata adc_vect) ();
extern void (* __pdata aec_vect) ();
extern void (* __pdata dma_vect) ();

//
// Simple Arduino compatability macros
//
//	instead of pin numbers we use P(port,bit)
//	P(1,2) refers to pin  p1.2 
//
//	"digitalWrite(P(1,2), HIGH);"
//
#ifndef LOW
#define HIGH 1
#define LOW 0
#define P(p, b) p , b
#define __XP(p,b) P##p##_##b
#define __Xp(p,b) p
#define __Xb(p,b) b
#define digitalWrite(p , v) __XP(p)=(v)
#define digitalRead(p) __XP(p)
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define __XPINP(p) P##p##INP
#define __XPDIR(p) P##p##DIR
#define __PINP(p) __XPINP(p)
#define __PDIR(p) __XPDIR(p)
#define pinMode(p, m) if ((m)==OUTPUT) {__PDIR(__Xp(p)) |= (1<<__Xb(p)); __PINP(__Xp(p)) |= (1<<__Xb(p));} else { __PDIR(__Xp(p)) &= ~(1<<__Xb(p)); if ((m)==INPUT_PULLUP) {__PINP(__Xp(p)) &= ~(1<<__Xb(p)); } else {__PINP(__Xp(p)) |= (1<<__Xb(p));} }
#endif
#endif
