/************************************************************************

    JSWED 2.2.2 - Editor for Jet Set Willy and derivatives

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
#include "spritelist.hxx"
#include "spriteform.hxx"
#include "png.h"

extern "C" {

#include <SDL.h>
#include <SDL_image.h>

};


SpriteForm::SpriteForm(int mode, SpectrumMemory *mem, SpriteList *list) :
	VideoForm(10 * CHAR_W, 9 * CHAR_H, 20 * CHAR_W, 13 * CHAR_H)
{
	m_list = list;
	m_mode = mode;
	m_mem  = mem;
	m_png  = 0;
	m_iobuf = NULL;
	m_nsprites = 0;

	m_ok = new VideoButton(ZXE_OK, m_x + CHAR_W, m_y + m_h - 2 * CHAR_H,
			"  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 * CHAR_W, 
			m_y + m_h - 2 * CHAR_H, "Cancel");
	m_first = new VideoButton(-1, m_x + CHAR_W, m_y + 3 * CHAR_SH," From ");
	m_last  = new VideoButton(-1, m_x + CHAR_W, m_y + 4 * CHAR_H, "  To  ");
	m_prev  = new VideoButton(-1, m_x + CHAR_W, m_y + 13 * CHAR_SH, "Preview");
	m_type  = new ScrollingList(m_x + CHAR_W, m_y + 13 * CHAR_SH,
				m_w - 2 * CHAR_W, 4 * CHAR_H);

	m_first->setListener(this);
	m_last->setListener(this);
	m_prev->setListener(this);
	m_type->setListener(this);
	m_type->setSelectAction(ZXE_CONTINUE);
	m_type->setTitle("Format");
	m_type->addString("+3DOS");
	m_type->addString("PNG");

	int max = m_list->getItemCount();

	m_addr0 = m_list->getItem(0).zxAddress;
	m_addr1 = m_list->getItem(max-1).zxAddress;
	m_bmp0 = NULL;
	m_bmp1 = NULL;
	makeBitmaps();
}

SpriteForm::~SpriteForm()
{
	if (m_iobuf) delete m_iobuf;
	delete m_ok;
	delete m_cancel;
	delete m_first;
	delete m_last;
	delete m_prev;
	delete m_bmp0;
	delete m_bmp1;	
}

void SpriteForm::makeBitmaps()
{
	if (!m_mode)
	{
		int n, max, inside;

		m_addr1 = m_addr0;

		// Calculate addr1 from addr0.
		// We do this by finding the entry with address addr0,
		// then moving (m_nsprites-1) steps along the list.
		max = m_list->getItemCount();
		for (n = 0; n < max; n++)
		{
			int addr = m_list->getItem(n).zxAddress;
			if (addr == m_addr0) 
			{
				inside = m_nsprites;
			}
			if (inside)
			{
				m_addr1 = addr;
				--inside;
			}
		}	
	}
	if (m_bmp0) delete m_bmp0;
	if (m_bmp1) delete m_bmp1;

	m_bmp0 = bitmapFromSprite(16, 16, m_mem->memoryAt(m_addr0), 1, ZX_BLACK, ZX_WHITE);
	m_bmp1 = bitmapFromSprite(16, 16, m_mem->memoryAt(m_addr1), 1, ZX_BLACK, ZX_WHITE);

}

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




int SpriteForm::exportPNG()
{
	char filename[PATH_MAX];
	JswFileForm ff;
	int rv;
	int x, y, n;
	png_byte  *rows = new png_byte[m_nsprites * 2 * 16];
	png_bytep rowPtr[16];
	jswByte mask;
	png_color pngPalette[2];
	memset(rows, 0, sizeof(rows));
	for (y = 0; y < 16; y++)
	{
		rowPtr[y] = &rows[2 * m_nsprites * y];
	}
	pngPalette[0].red   = 255;
	pngPalette[0].green = 255;
	pngPalette[0].blue  = 255;
	pngPalette[1].red   = 0;
	pngPalette[1].green = 0;
	pngPalette[1].blue  = 0;

	png_structp pPng  = NULL;
	png_infop   pInfo = NULL;

	pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				(png_voidp)NULL, se_png_error, se_png_warn);
	if (pPng)
	{
		pInfo = png_create_info_struct(pPng);
	}
	if (!pPng || !pInfo)
	{
		delete rows;
		if (pPng) png_destroy_write_struct(&pPng, NULL);
		rv = alert("Out of memory", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;
	}
	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_write_struct(&pPng, &pInfo);
		delete rows;
		return ZXE_CONTINUE;
	}
	filename[0] = 0;
	rv = ff.doModal(filename);
	if (rv != ZXE_OK)
	{
		png_destroy_write_struct(&pPng, &pInfo);
		delete rows;
		return rv;
	}
	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

		if ( (rv = vm.doModal()) != ZXE_OK)
		{
			png_destroy_write_struct(&pPng, &pInfo);
			delete rows;
			return ZXE_CONTINUE;
		}
		if (vm.getSelected())
		{
			png_destroy_write_struct(&pPng, &pInfo);
			delete rows;
			return ZXE_CONTINUE;
		}
	}
	fp = fopen(filename, "wb");
	if (!fp)
	{
		png_destroy_write_struct(&pPng, &pInfo);
		delete rows;
		rv = alert("File create failed", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;
	}
	png_init_io(pPng, fp);
	png_set_IHDR(pPng, pInfo, 16 * m_nsprites, 16, 1, PNG_COLOR_TYPE_PALETTE,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(pPng, pInfo, pngPalette,2);
	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 2 * m_nsprites; x++)
		{
			jswByte *mem;
			mem = m_iobuf + (32 * (x/2)) + (2*y) + (x&1);

			rowPtr[y][x] = mem[0];
		}
	}
	png_set_rows(pPng, pInfo, rowPtr);
	png_write_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&pPng, &pInfo);
	fclose(fp);

	delete rows;
	return ZXE_CONTINUE;	
}

static jswByte header[128] = 
{
	 'P',  'L',  'U',  'S',  '3',  'D',  'O',  'S',
	0x1A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
};



int SpriteForm::exportBin()
{
	char filename[PATH_MAX];
	JswFileForm ff;
	int rv;
	int x, y, n;
	jswByte mask;

	filename[0] = 0;
	rv = ff.doModal(filename);
	if (rv != ZXE_OK) return rv;

	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

		if ( (rv = vm.doModal()) != ZXE_OK) return ZXE_CONTINUE;
		if (vm.getSelected()) return ZXE_CONTINUE;
	}
	fp = fopen(filename, "wb");
	if (!fp)
	{
		rv = alert("File create failed", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;
	}
	header[11] = ((m_nsprites * 32 + 128)) & 0xFF;
	header[12] = ((m_nsprites * 32 + 128) >>  8) & 0xFF;
	header[13] = ((m_nsprites * 32 + 128) >> 16) & 0xFF;
	header[14] = ((m_nsprites * 32 + 128) >> 24) & 0xFF;
	header[16] = ((m_nsprites * 32)) & 0xFF;
	header[17] = ((m_nsprites * 32) >>  8) & 0xFF;
	header[21] = m_nsprites & 0xFF;
	header[22] = 16;
	header[23] = 'S';

	header[127] = 0;
	for (n = 0; n < 127; n++)
	{
		header[127] += header[n];
	}
	fwrite(header,    1,  128, fp);
	fwrite(m_iobuf, 32, m_nsprites, fp);
	fclose(fp);

	return ZXE_CONTINUE;
}



int SpriteForm::importBin(FILE *fp)
{
	long pos,max, size;

	pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	max = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	size = max - pos;

	if (m_iobuf) delete m_iobuf;
	m_iobuf = new jswByte[size];

	fread(m_iobuf, 1, size, fp);
	fclose(fp);

	m_nsprites = size / 32;

	return ZXE_CONTINUE;
}

int SpriteForm::importImage(const char *filename)
{
	SDL_Surface *s0 = IMG_Load(filename);
	int rv;
	int x, y;
	Uint8 r, g, b;
	jswByte mask;

	if (!s0 || (s0->w < 16))
	{
		rv = alert("Failed to load image", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CANCEL;    
	}
	int imgw = s0->w;
	int imgh = s0->h;
	m_nsprites = imgw / 16;
	if (m_iobuf) delete m_iobuf;
	m_iobuf = new jswByte[m_nsprites * 32];	
	memset(m_iobuf, 0, m_nsprites * 32);
	if (imgh > 16) imgh = 16;
	for (y = 0; y < imgh; y++)
	{
		mask = 0x80;
		for (x = 0; x < 16 * m_nsprites; x++)
		{
			jswByte *dest = m_iobuf;
		       	dest +=	(x/16) * 32;	// Offset to sprite
			dest += 2 * y;		// Y within sprite
			dest += (x/8) & 1;	// X within sprite

			jswByte *pixel = (jswByte *)(s0->pixels);
			pixel += (y * s0->pitch);
			pixel += (x * s0->format->BytesPerPixel);
			Uint32 pixval = 0;
			switch(s0->format->BitsPerPixel)
			{
				case 8:  pixval = *(Uint8  *)pixel; break;
				case 16: pixval = *(Uint16 *)pixel; break;
				// XXX 24-bit pixels are nassty
				case 24: pixval = *(Uint32 *)pixel; break;
				case 32: pixval = *(Uint32 *)pixel; break;
			}
			SDL_GetRGB(pixval, s0->format, &r, &g, &b);
			double intensity = 0.2125*r + 0.7154*g + 0.0721*b;
			if (intensity < 128.0)
			{
				*dest |= mask;	
			}
			mask = mask >> 1;
			if (mask == 0) mask = 0x80;
		}
	}
	SDL_FreeSurface(s0);
	return ZXE_CONTINUE;
}



int SpriteForm::doModal()
{
	int n, inside;
	int r;
	int addr, max;

	if (!m_mode)
	{
		char filename[PATH_MAX]; 
 		JswFileForm ff;		 
		int n;
		FILE *fp;
		jswByte magic[128], sum;

		filename[0] = 0;
		r = ff.doModal(filename);
		if (r >= ZXE_QUIT) return r;
		if (r != ZXE_OK)   return ZXE_CONTINUE;

		fp = fopen(filename, "rb");
		if (!fp)
		{
			r = alert("Could not open file", "Cancel");
			if (r >= ZXE_QUIT) return r;
			return ZXE_CONTINUE;
		}
		// Work out what we have here
		memset(magic, 0, sizeof(magic));
		fread(magic, 1, sizeof(magic), fp);
		if (!png_sig_cmp(magic, 0, 8))  // Importing PNG
		{
			 fclose(fp);
			 r = importImage(filename);
		}
		else if (!memcmp(magic, "BM", 2))    // Importing BMP
		{
			 fclose(fp);
			 r = importImage(filename);
		}
		else if (!memcmp(magic, "\377\330\377", 3)) // Importing JPG
		{
			fclose(fp);
			r = importImage(filename);
		}
		else if (!memcmp(magic, "PLUS3DOS\032", 9))
		{
			sum = 0;
			for (n = 0; n < 127; n++)
			{
				sum += magic[n];
			}
			if (sum == magic[n]) /* PLUS3DOS magic found */
			{
				r = importBin(fp);
			}
		}
		else
		{
			// No magic number. Treat as raw Spectrum screen.
			fseek(fp, 0, SEEK_SET);
			r = importBin(fp);
		}
		if (r != ZXE_OK && r != ZXE_CONTINUE) return r;
		makeBitmaps();
	}


	r = VideoForm::doModal();
	if (r == ZXE_OK)
	{

		if (m_mode)	// Export
		{
			max = m_list->getItemCount();
			for (n = inside = m_nsprites = 0; n < max; n++)
			{
				addr = m_list->getItem(n).zxAddress;
				if (addr == m_addr0) inside = 1;
				if (inside) ++m_nsprites;
				if (addr == m_addr1) inside = 0;
			}
			if (!m_nsprites)
			{
				alert("No sprites selected", "Cancel");
				return ZXE_CANCEL;
			}
			if (m_iobuf) delete m_iobuf;
			m_iobuf= new jswByte[32 * m_nsprites];
			jswByte *ptr = m_iobuf;
			for (n = inside = 0; n < max; n++)
			{
				addr = m_list->getItem(n).zxAddress;
				if (addr == m_addr0) inside = 1;
				if (inside)
				{
					memcpy(ptr, m_list->getItem(n).bitmap,
							32);
					ptr += 32;
				}
				if (addr == m_addr1) inside = 0;
			}
			// m_iobuf now holds image to export.
			if (m_png) r = exportPNG();
			else	   r = exportBin();
			return r;
		}
		else	// m_mode == 0, ie import
		{		
			jswByte *iobuf = m_iobuf;
			max = m_list->getItemCount();
			inside = 0;
			for (n = 0; n < max; n++)
			{
				addr = m_list->getItem(n).zxAddress;
				jswByte *dest = m_list->getItem(n).bitmap;
				if (addr == m_addr0) 
				{
					inside = m_nsprites;
				}
				if (inside)
				{
					memcpy(dest, iobuf, 32);
					iobuf += 32;
					--inside;
				}
			}
		}	
	}
	return r;
}



void SpriteForm::redraw()
{
	char txt[20];

	VideoForm::redraw();

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
	if (m_mode) videoScreen->drawText(m_x, m_y, "Export sprites      "
					, ZX_BRWHITE, ZX_BLACK);
	else	    videoScreen->drawText(m_x, m_y, "Import sprites      "
					, ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);

	sprintf(txt, "%04x", m_addr0);
	videoScreen->drawText(m_first->getX() + 10 * CHAR_W, 
			m_first->getY(), txt, ZX_BLACK, ZX_WHITE);
	sprintf(txt, "%04x", m_addr1);
	videoScreen->drawText(m_last->getX() + 10 * CHAR_W, 
			m_last->getY(), txt, ZX_BLACK, ZX_WHITE);
	if (!m_mode)
	{
		videoScreen->drawText(m_last->getX(), m_last->getY(), "  To",
				ZX_BLACK, ZX_WHITE);
	}

	m_bmp0->toScreen(m_first->getX() + m_first->getW() + CHAR_SW,
			m_first->getY());
     	m_bmp1->toScreen(m_last->getX() + m_last->getW() + CHAR_SW,
			m_last->getY());

}

int SpriteForm::onButtonSelect(VideoButton *b)
{
	int rv;

	if (b == m_first)
	{
		SpriteList sl(m_list,
				2, b->getY() + b->getH(), VIDEO_W - 4, 
				CHAR_H + CHAR_SH + 40);
		sl.setSelected(m_addr0);
		sl.setTitle("First sprite");
		rv = sl.doModal();
		if (rv >= ZXE_QUIT) return rv;
		if (rv == ZXE_OK)
		{
			m_addr0 = sl.getSelected(NULL);
			makeBitmaps();
			redraw();
		}
		return ZXE_CONTINUE;
	}

	if (b == m_last)
	{
		SpriteList sl(m_list,
				2, b->getY() + b->getH(), VIDEO_W - 4, 
				CHAR_H + CHAR_SH + 40);
		sl.setSelected(m_addr1);
		sl.setTitle("Last sprite");
		rv = sl.doModal();
		if (rv >= ZXE_QUIT) return rv;
		if (rv == ZXE_OK)
		{
			m_addr1 = sl.getSelected(NULL);
			makeBitmaps();
			redraw();
		}
	}
	if (b == m_prev)
	{
		SpriteList sl(m_list,
				2, b->getY() + b->getH(), VIDEO_W - 4, 
				CHAR_H + CHAR_SH + 40);
		sl.emptyBuffer();
		for (int n = 0; n < m_nsprites; n++)
		{
			sl.addSprite(n, m_iobuf + 32 * n);
		}
		rv = sl.doModal();
		if (rv >= ZXE_QUIT) return rv;
	}
	return ZXE_CONTINUE;
}


void SpriteForm::hideChildren(void)
{
	VideoForm::hideChildren();
	removeChild(m_first);	
	if (m_mode) 
	{
		removeChild(m_last);	
		removeChild(m_type);
	}
	else	removeChild(m_prev);
	removeChild(m_ok);	
	removeChild(m_cancel);	
}


void SpriteForm::showChildren(int redraw)
{
	addChild(m_first);	
	if (m_mode) 
	{
		addChild(m_last);	
		addChild(m_type);
	}
	else	addChild(m_prev);
	addChild(m_ok);	
	addChild(m_cancel);	
	VideoForm::showChildren(redraw);
}


void SpriteForm::onSelect(ScrollingList *sl, int n)
{
	m_png = n;	
}
