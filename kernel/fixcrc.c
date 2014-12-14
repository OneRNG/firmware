//
// Copyright 2013 Paul Campbell paul@taniwha.com
//
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int mn=0x10000, mx=0;
unsigned char m[65536];


int
hex(FILE *f)
{
        char c1, c2;
        int r;
	c1 = fgetc(f);
        if (c1 < 0)
                return -1;
	c2 = fgetc(f);
        if (c2 < 0)
                return -1;
        if (c1 >= '0' && c1 <= '9') {
                r = c1-'0';
        } else
        if (c1 >= 'A' && c1 <= 'F') {
                r = c1-'A'+10;
        } else
        if (c1 >= 'a' && c1 <= 'f') {
                r = c1-'a'+10;
        } else return -1;
        r=r<<4;
        if (c2 >= '0' && c2 <= '9') {
                r += c2-'0';
        } else
        if (c2 >= 'A' && c2 <= 'F') {
                r += c2-'A'+10;
        } else
        if (c2 >= 'a' && c2 <= 'f') {
                r += c2-'a'+10;
        } else return -1;
        return r;
}

int
upload(FILE *f)
{
	memset(m, 0xff, sizeof(m));
	for(;;) {
	    	int c;
            	int i;
            	int sum=0, len, addr;
		c = fgetc(f);
		if (c < 0)
			break;
            	if (c == '\n')
                    continue;
            	if (c != ':') {
			fprintf(stderr, "bad file format\n");
fail2:
                        return 0;
            	}
            	len = hex(f);
            	if (len < 0) {
			fprintf(stderr, "bad file format\n");
                    	goto fail2;
		}
            	sum += len;
            	addr = hex(f);
            	if (addr < 0) {
			fprintf(stderr, "bad file format\n");
                    	goto fail2;
		}
            	sum += addr;
            	i = hex(f);
            	if (i < 0) {
			fprintf(stderr, "bad file format\n");
                    	goto fail2;
		}
            	sum += i;
            	addr = (addr<<8)+i;
            	int type = hex(f);
            	if (type < 0) {
			fprintf(stderr, "bad file format\n");
                    	goto fail2;
		}
            	sum += type;
            	if (type == 0x01)
                    	break;
		if (mn > addr)
			mn = addr;
            	for (i = 0; i < len; i++) {
                	int v = hex(f);
                	if (v < 0)
                        	break;
                	m[(addr+i)&0xffff] = v;
                	sum += v;
            	}
            	i = hex(f);
            	if (i < 0) {
                	goto fail2;
		}
            	if (i < 0 || i != ((0x100-(sum&0xff))&0xff)) {
			fprintf(stderr, "bad checksum\n");
			goto fail2;
		}
            	addr += len;
            	if (addr > mx)
                    	mx = addr;
        }
	return  1;
} 

unsigned long 
crc(unsigned long c, unsigned char *p, int len)
{
	while (len--) {
		unsigned char v = *p++;
		int i;

		c ^= (v<<24);
		for (i = 0; i < 8; i++) {
			if (c&0x80000000) {
				c = (c<<1)^0x04c11db7;
			} else {
				c = (c<<1);
			}
		}
	}
	return c;
}


int
main(int argc, char **argv)
{
	unsigned char h[4];
	unsigned char k[16];
	int len;
	unsigned long c;
	int version = -1;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0 && i < argc) {
			i++;
			version = strtol(argv[i], 0, 0);
		} else 
		if (strcmp(argv[i], "-k") == 0 && i < argc) {
			char *cp;
			i++;
			cp = argv[i];
			for (i = 0; i < 16; i++) {
				int v;

				if (*cp >= '0' && *cp <= '9') {
					v = (*cp-'0');
				} else
				if (*cp >= 'a' && *cp <= 'f') {
					v = (*cp-'a')+10;
				} else
				if (*cp >= 'A' && *cp <= 'F') {
					v = (*cp-'A')+10;
				} else {
					fprintf(stderr, "%s: bad key '%s'\n", argv[0], argv[i]);
					exit(9);
				}
				cp++;
				v = v<<4;
				if (*cp >= '0' && *cp <= '9') {
					v |= (*cp-'0');
				} else
				if (*cp >= 'a' && *cp <= 'f') {
					v |= (*cp-'a')+10;
				} else
				if (*cp >= 'A' && *cp <= 'F') {
					v |= (*cp-'A')+10;
				} else {
					fprintf(stderr, "%s: bad key '%s'\n", argv[0], argv[i]);
					exit(9);
				}
				cp++;
				k[i] = v;
			}
			if (*cp) {
				fprintf(stderr, "%s: bad key '%s' - too long\n", argv[0], argv[i]);
				exit(9);
			}
		} else {
			fprintf(stderr, "%s: unknown flag '%s'\n", argv[0], argv[i]);
			exit(9);
		}
	}
	if (!upload(stdin) || mx <= mn) {
		fprintf(stderr, "unpack failed mx=%d mn=%d\n", mx, mn);
		exit(9);
	}
	len = mx-mn;
	m[mn+4] = (len-4);
	m[mn+5] = (len-4)>>8;
	if (version >= 0) {
		m[mn+8] = version;
		m[mn+9] = version>>8;
	}
	c = crc(0xffffffff, &m[mn+4], len-4);
	m[mn+0] = c;
	m[mn+1] = c>>8;
	m[mn+2] = c>>16;
	m[mn+3] = c>>24;
	h[0] = mn;	// base addr
	h[1] = mn>>8;
	h[2] = len;	// len
	h[3] = len>>8;
	fwrite(&h[0], 4, 1, stdout);
	fwrite(&k[0], 16, 1, stdout);
	fwrite(&m[mn], len, 1, stdout);
	exit(0);
}
