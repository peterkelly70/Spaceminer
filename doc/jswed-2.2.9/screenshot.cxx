/************************************************************************

    JSWED 2.2.4 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005  John Elliott <jce@seasip.demon.co.uk>

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
#include "png.h"

extern "C" {

#include <SDL.h>
#include <SDL_image.h>

};

// Define png_jumpbuf() in case we are using a pre-1.0.6 version of libpng 
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) png_ptr->jmpbuf
#endif

static void se_png_error(png_structp png_ptr, png_const_charp error_msg)
{
	alert((char *)error_msg, "Cancel");
	longjmp(png_ptr->jmpbuf, 1);
}

static void se_png_warn(png_structp png_ptr, png_const_charp warning_msg)
{
	fprintf(stderr, "LibPNG warning: %s\n", warning_msg);
}


extern int gl_b[], gl_r[], gl_g[];


int VideoSurface::savePng(int ix, int iy, int w, int h, 
		const char *filename, int scale)
{
        int rv;
	int x, y, n;
	png_byte  rows[128 * 192];
	png_bytep rowPtr[192];
	jswByte mask;
	png_color pngPalette[16];
	FILE *fp;

	memset(rows, 0, sizeof(rows));
	for (y = 0; y < h; y++)
	{
		rowPtr[y] = &rows[128 * y];
	}
	for (n = 0; n < 16; n++)
	{
		pngPalette[n].red   = gl_r[n];
		pngPalette[n].green = gl_g[n];
		pngPalette[n].blue  = gl_b[n];
	}

	png_structp pPng  = NULL;
      	png_infop   pInfo = NULL; 

	pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp)this, se_png_error, se_png_warn);
	if (pPng)
	{
		pInfo = png_create_info_struct(pPng);
	}
	if (!pPng || !pInfo)
	{
		if (pPng) png_destroy_write_struct(&pPng, NULL);
		rv = alert("Out of memory", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;	
	}
	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_write_struct(&pPng, &pInfo);
		return ZXE_CONTINUE;	
	}
	fp = fopen(filename, "wb");
	if (!fp)
	{
		png_destroy_write_struct(&pPng, &pInfo);
		rv = alert("File create failed", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;	
	}
	// Let's go.
	png_init_io(pPng, fp);
	png_set_IHDR(pPng, pInfo, w, h, 4, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(pPng, pInfo, pngPalette, 16);

	// Generate the image.
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			int pix = pointZx(ix + x * scale, iy + y * scale);
			if (x & 1) 
			{
				rowPtr[y][x/2] |= (pix & 0x0F);
			}
			else
			{
				rowPtr[y][x/2] |= ((pix & 0x0F) << 4);
			}
		}
	}
	png_set_rows(pPng, pInfo, rowPtr);
	png_write_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&pPng, &pInfo);
	fclose(fp);

	return ZXE_CONTINUE;
}

