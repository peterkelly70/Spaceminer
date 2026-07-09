/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004  John Elliott <jce@seasip.demon.co.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*************************************************************************/


#include "jswed.hxx"

static jswByte gl_hexChecksum;
static char gl_errBuf[50];
static jswByte gl_hexLine;


char *getHex(FILE *fp, jswByte *b) 
{
	register int    c;
	jswByte   x;

	c = fgetc(fp);
	if ('0' <= c && c <= '9') 	x = c - '0';
	else if ('A' <= c && c <= 'F')	x = c - 'A' + 10;
	else 
	{
		sprintf (gl_errBuf, "A Funny hex letter %c\n", c);
		fclose(fp);
		return gl_errBuf;
	}

	x <<= 4;
	c = fgetc(fp);
	if ('0' <= c && c <= '9') 	x |= c - '0';
	else if ('A' <= c && c <= 'F')	x |= c - 'A' + 10;
	else 
	{
		sprintf (gl_errBuf, "A Funny hex letter %c\n", c);
		fclose(fp);
		return gl_errBuf;
	}
	gl_hexChecksum += x;
	*b = x;
	return NULL;
}



char *SpectrumMemory::loadHexPatch(char *path)
{
	gl_hexChecksum = 0;
	gl_hexLine = 0;
	jswByte n, ah, al, type, val;
	char *boo;
	int c, i;
	unsigned naddr;
	FILE *fpHex;

	if (path == NULL) return "F HEX file not found";
	fpHex = fopen(path, "rb");
	if (!fpHex) return "F HEX file not found";
	do {
/* Skip until the first hex record starts. Some .HEX files have a line
 * explaining what they are. */
		do {
	 		c = fgetc(fpHex);
	    		if (c == EOF) 
			{
				fclose(fpHex);
				return "R Premature end of HEX file";
	    		}
		} while (c != ':');

		++gl_hexLine;
		gl_hexChecksum = 0;
		boo = ::getHex(fpHex, &n ); if (boo) return boo;
		boo = ::getHex(fpHex, &ah); if (boo) return boo;
		boo = ::getHex(fpHex, &al); if (boo) return boo;
		boo = ::getHex(fpHex, &type); if (boo) return boo;


		switch (type)
		{
		    case 0:
			if (!n)	/* MAC uses a line with no bytes as EOF */
			{
				type = 1;
				break;
			}
			naddr = (ah << 8) | al;

			for (i = 0; i < n; i++)
			{
				boo = getHex(fpHex, &val);
				if (boo) return boo;
//				printf("poke %04x, %02x\n", naddr + i, val);
				poke(naddr + i, val);
			}

			case 1: break; 
			default:
				fprintf (stderr, "B Funny hex record type %d\n", type);
				exit (1);
		}

		boo = getHex(fpHex, &val); if (boo) return boo;	// Checksum byte
		if (gl_hexChecksum != 0) 
		{
			fclose(fpHex);
			return "R Hex Checksum error";
		}
	
	} while (type != 1);
	fclose(fpHex);
	return NULL;
}

