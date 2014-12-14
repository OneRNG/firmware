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

#include "interface.h"
static xxxx() __naked {
	__asm;
	.area GSINIT0    (CODE)
	.globl	_CODE_HEADER
_CODE_HEADER:
	.db	0, 0, 0, 0	// CRC will go here
	.db	0, 0		// len from here to end of code 
	.db	THIS_ARCH
	.db	THIS_CODE_BASE
	.db	0, 0		// version (little endian)
	.globl	_my_app
	mov	a, dpl
	cjne	a, #APP_LOW_LEVEL_INIT, 0001$
		sjmp	0002$
0001$:		ljmp	_my_app	
0002$:	mov	r0, #s_DSEG	// clear data seg
	mov	a, #l_DSEG
	jz	0004$
	mov	r1, a
	clr	a
0003$:		mov	@r0, a
		inc	r0
0004$:	mov	r0, #s_PSEG	// clear pseg
	mov	a, #l_PSEG
	jz	0006$
	mov	r1, a
	clr	a
0005$:		movx	@r0, a
		inc	r0
		djnz	r1, 0005$
0006$:	mov	dptr, #s_XSEG	// clear xseg
	mov	r1, #l_XSEG
	mov	r2, #l_XSEG>>8
	cjne	r1, #0, 0007$
	cjne	r2, #0, 0007$
		sjmp	0011$
0007$:
	inc	r2
0010$:		movx	@dptr, a
		inc	dptr
		djnz	r1, 0010$
		djnz	r2, 0010$
0011$:
        mov     r1, #l_XINIT
        mov     a, r1
        orl     a, #(l_XINIT >> 8)
        jz      0023$
        	mov     r2, #((l_XINIT+255) >> 8)
        	mov     _DPS, #0x01
        	mov     dptr, #s_XINIT
        	dec     _DPS         
        	mov     dptr, #s_XISEG
0021$:			clr     a
        		inc     _DPS         
        		movc    a, @a+dptr
        		inc     dptr
        		dec     _DPS        
        		movx    @dptr, a
        		inc     dptr
        		djnz    r1, 0021$
        		djnz    r2, 0021$
0023$:	ret
	.area XSEG    (XDATA)
	.globl	_xseg_end
_xseg_end:
	.ds	1
	__endasm;
}

