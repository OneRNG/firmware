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
//	This code handles the extra anti-tamper feature of RNG
//	essentially it works this way:
//
//	1) OneRNG device flash is protected so we can't read it from the debug interface
//	   but we can erase (all of) it
//
//	2) when we program it we put a known random value in the first 96 bytes of
//		the last 128 bytes of it 16 bytes 'ID, 16 bytes 'K', 64 bytes 'S' 
//
//	3) user asks device for a its 'ID' from bytes 0:15 of this area 'cmdI'
//		returns '!!iiiiii....iiii!' where iiii...iiii is 32 hex digits
//
//	4) user provides 'ID' to the OneRNG web site
//		a) it generates a 16 byte token 'T'
//		b) caches the tuple ('ID'/'T') - TTL 2 minutes
//		c) and returns 'T' to the user
//
//	5) when asked for a confirmation a OneRNG is given the token 'T' 'cmdRtttt....tttt'
//			(ttt....tttt is 32 hex digits) and:
//		a) use the bytes 16:31 of the region as an AES128 key 'K'
//		b) encrypt a 2 block nonce of random data 'R' (32 bytes) -> 'ER'
//		c) encrypt (chained) 2 blocks of token 'T' (32 bytes) -> 'ET'
//		d) encrypt (chained) the final 32:95 bytes of the region 'S' (64 bytes) -> 'ES'
//		e) offer the resulting 128 bytes from the encryption to the host 'ER'+'ET'+'ES'
//	
//	6) user presents bytes 0:15 of the user data 'ID' and bytes 0:63 of the encrypted data 'ER'+'ET' to OneRNG's web page
//
//	7) OneRNG's server:
//		a) wait a random time (to hide error cause info)
//		b) inspects its database of manufactured devices using the key from bytes 16-31 'ID'
//		c) if 'ID' isn't in the DB return an error
//		d) retrieves bytes 0-95 'K'+'ID'+'S' that were programmed into the device 
//		e) decrypts 'ER' using 'K' giving nonce 'R' (discarded)
//		f) decrypts (chained) 'ET' using 'K' giving 'T'
//		g) if 'T' doesn't match returns an error
//		h) reencrypts 'R'+'T'+'S' using 'K' giving 'ER'+'ET'+'ES'
//		i) returns 'ES' to user
//		j) tells user how many times it has offered up results for this id (to record
//		   potential replay attacks)
//
//	8) User compares 'ES' from user's OneRNG with value that OneRNG delivered in step 5d)
//		gets worried if they are not the same
//
//	(we allow this to be done manually but we also provide a python script)
//
//	The basic idea is that we have a shared secret K+S (ID is essentially a secret id)
//	we tell the server we know the key by encrypting something it does know with K (T) and return
//	the encrypted second secret S - the user compares the encrypted second secret (ES) with the
//	one the OneRNG gave it 
//
//	We also use T to stop replay attacks (where someone records a good transaction from another
//	OneRNG and replays it from a compromised one)
//
//	In other words:
//
//      step               OneRNG web      host/user      User's OneRNG
//
//      3)                                         'ID'<-'ID'
//
//      4)                          'ID'<-'ID'
//                                  'T'->'T'
//
//      5)                                         'T'->'T'
//                                     'ER'+'ET'+'ESa'<-'R'+'T'+'S'
//
//      6)                'ID'+'ER'+'ET'<-'ID'+''ER'+'ET'
//
//	7)     'ID'+'ER'+'ET'->'R'+'T'->'ESb'
//				          'ESa'=='ESb'
//

#include <mcs51reg.h>
#include <string.h>
#include "task.h"
#include "interface.h"

#define AES_ENCRYPT     0x00
#define AES_DECRYPT     0x02
#define AES_LOAD_KEY    0x04
#define AES_LOAD_IV     0x06
#define AES_MODE_CBC    0x00

unsigned char __xdata token[16];
static __bit tmpb;
extern __bit no_avalanche;
extern __xdata unsigned char *__data  pool_out;
extern unsigned char __xdata pool[];
extern unsigned int __data  pool_count;
static void out_hex();

void
start_encrypt() __naked
{
        __asm;                  //      u8 i;
        mov	_FMAP, #7				// ptr = 0x
	mov	dptr, #0xff80+16
        mov     _ENCCS, #AES_LOAD_KEY|AES_MODE_CBC	// ENCCS = ES_LOAD_KEY|AES_MODE_CBC
        orl     _ENCCS, #1      //      ENCCS |= 0x01;                  // start
        mov     r0, #16         //      for (i=0; i<16; i++) 
0002$:          clr     a
                movc    a, @a+dptr//            ENCDI = *key++;
                inc     dptr
                mov     _ENCDI, a
                djnz    r0, 0002$
0001$:  mov     a, _ENCCS       //      aes_done();
        jnb     _ACC_3, 0001$

	mov	dptr, #0xff80
        mov     _ENCCS, #AES_LOAD_IV|AES_MODE_CBC	// ENCCS = ES_LOAD_IV|AES_MODE_CBC
        orl     _ENCCS, #1      //      ENCCS |= 0x01;                  // start
        mov     r0, #16         //      for (i=0; i<16; i++) 
0004$:          clr     a
                movc    a, @a+dptr//            ENCDI = *key++;
                inc     dptr
                mov     _ENCDI, a
                djnz    r0, 0004$
0003$:  mov     a, _ENCCS       //      aes_done();
        jnb     _ACC_3, 0003$
	ret
	__endasm;
}

void
sync_header() __naked
{
	__asm;
	mov	dptr, #_USBF5		// start with '@@@@@@@@'
	mov	a, #'@'
	mov	r0, #8			// for (i = 0; i < 8; i++)
0044$:		movx	@dptr, a	//	USBF5 = '@'
               	djnz    r0, 0044$
	ret
	__endasm;
}

void
send_id_block() __naked
{
	__asm;
	mov	dptr, #_USBF5	
	inc	_DPS
	mov	dptr, #0xff80
	mov	r0, #16			// for (i = 0; i < 16; i++)
0004$:          clr     a
                movc    a, @a+dptr	//       USBF5 = *id++;
                inc     dptr
		dec	_DPS
		lcall	_out_hex	
		inc	_DPS
                djnz    r0, 0004$
	
	dec 	_DPS
	ret
	__endasm;
}

void
encrypt_random_block() __naked
{
	__asm;
	mov	dptr, #_USBF5		// start with '@@@@@@@@'
	mov	a, #'@'
	mov	r0, #8			// for (i = 0; i < 8; i++)
0044$:		movx	@dptr, a	//      USBF5 = '@'
               	djnz    r0, 0044$
	mov	c, _no_avalanche	//	tmpb = no_avalanche;
	mov	_tmpb, c
	clr	_no_avalanche		//	no_avalanche = 0;
0002$:	mov	a, _pool_count+1	//	while (pool_count < 32) 
	jnz	0001$
	mov	a, _pool_count
	anl	a, #0xe0		
	jnz	0001$
		lcall	_idle_pool	//		idle_pool();
		sjmp	0002$
0001$:
	mov	c, _tmpb		//	idle_pool = tmpb;
	mov	_no_avalanche, c

	mov	r1, #2			//	for (j = 0 j < 2; j++) {
0077$:
        	mov     dpl, _pool_out         
        	mov     dph, _pool_out+1

        	mov     _ENCCS, #AES_MODE_CBC|AES_ENCRYPT	// ENCCS = AES_MODE_CBC|AES_ENCRYPT
        	orl     _ENCCS, #1      	//      ENCCS |= 0x01;                  // start
        	mov     r0, #16        		//      for (i=0; i<16; i++) 
0004$:        		movx    a, @dptr	//            ENCDI = *pool_out++;
                	mov     _ENCDI, a
                	inc     dptr
                        mov     a, dpl          //            if (pool_out == &pool[POOL_SIZE])
                        cjne    a, #_pool+POOL_SIZE, 0011$
                        mov     a, dph          //            		pool_out = &pool[0];
                        cjne    a, #(_pool+POOL_SIZE)>>8, 0011$
                                mov     dptr, #_pool
0011$:                
                	djnz    r0, 0004$
        	mov     _pool_out, dpl        
        	mov     _pool_out+1, dph
		
                clr     c
                mov     a, _pool_count          //              pool_count -= 16;
                subb    a, #16
                mov     _pool_count, a
                mov     a, _pool_count+1
                subb    a, #0
                mov     _pool_count+1, a

        	mov     r0, #32			//      wait_us(1);
0111$:  		djnz    r0, 0111$

		mov	dptr, #_USBF5
        	mov     r0, #16         	//      for (i=0; i<16; i++) 
0014$:        		mov     a, _ENCDO	// 		*USBF5 = hex(*ENCDO)
			lcall	_out_hex	
                	djnz    r0, 0014$
0013$:  	mov     a, _ENCCS       	//      aes_done();
        		jnb     _ACC_3, 0013$
	djnz	r1, 0077$		//	}

	ret
	__endasm;
}

static void
out_hex() __naked	// dptr is dest, a is value, r7 is free
{
	__asm;
	mov	r7, a
	swap	a
	anl	a, #0x0f
	cjne	a, #9, 1$
2$:		add	a, #'0'
		sjmp	3$
1$:	jc	2$
		add	a, #('A'-10)
3$: 	movx	@dptr, a

	mov	a, r7
	anl	a, #0x0f
	cjne	a, #9, 11$
12$:		add	a, #'0'
		sjmp	13$
11$:	jc	12$
		add	a, #('A'-10)
13$: 	movx	@dptr, a
	ret
	__endasm;
}

void
encrypt_token() __naked
{
	__asm;
        mov     dptr, #_token      
        mov     _ENCCS, #AES_MODE_CBC|AES_ENCRYPT	// ENCCS = AES_MODE_CBC|AES_ENCRYPT
        orl     _ENCCS, #1      	//      ENCCS |= 0x01;                  // start
        mov     r0, #16        		//      for (i=0; i<16; i++) 
0004$:      	movx    a, @dptr	//            ENCDI = *pool_out++;
                mov     _ENCDI, a
                inc     dptr
                djnz    r0, 0004$

        mov     r0, #32			//      wait_us(1);
0011$:  	djnz    r0, 0011$

	mov	dptr, #_USBF5
        mov     r0, #16         	//      for (i=0; i<16; i++) 
0014$:        	mov     a, _ENCDO	// 		*USBF5 = hex(*ENCDO)
		lcall	_out_hex	
               	djnz    r0, 0014$
0013$:  mov     a, _ENCCS       	//      aes_done();
        	jnb     _ACC_3, 0013$
					//

	ret
	__endasm;
}


void
encrypt_secret_blocks() __naked
{
	__asm;
        mov	_FMAP, #7				// ptr = 0x
	//mov	dptr, #0xff80+32			// in dptr at call time
	mov	_DPL, #_USBF5
	mov	_DPH, #_USBF5>>8

	mov	r1, #2			//	for (j = 0 j < 4; j++) {
0077$:

        	mov     _ENCCS, #AES_MODE_CBC|AES_ENCRYPT// ENCCS = AES_MODE_CBC|AES_ENCRYPT
        	orl     _ENCCS, #1      	//      ENCCS |= 0x01;                  // start
        	mov     r0, #16        		//      for (i=0; i<16; i++) 
0004$:        		clr	a
			movc    a, @a+dptr	//            ENCDI = *secret++;
                	mov     _ENCDI, a
                	inc     dptr
                	djnz    r0, 0004$

        	mov     r0, #32			//      wait_us(1);
0011$:  		djnz    r0, 0011$

		inc	_DPS
        	mov     r0, #16         	//      for (i=0; i<16; i++) 
0014$:        		mov     a, _ENCDO	// 		*USBF5 = hex(*ENCDO)
			lcall	_out_hex	
                	djnz    r0, 0014$
		dec	_DPS
0013$:  	mov     a, _ENCCS       	//      aes_done();
        		jnb     _ACC_3, 0013$
	djnz	r1, 0077$		//	}

	ret
	__endasm;
}
