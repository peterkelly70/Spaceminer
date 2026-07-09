/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2004  John Elliott <jce@seasip.demon.co.uk>

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
#include "sdl_video.hxx"

////////////////////////////////////////////////////////////////////////
// VideoBitmap class
////////////////////////////////////////////////////////////////////////


SdlVideoBitmap::SdlVideoBitmap(const char *image) : VideoBitmap(0, 0)
{
	SDL_Surface *s0 = IMG_Load(image);
	SDL_Surface *s = NULL;

	if (s0) s = SDL_DisplayFormat(s0);
	if (!s) s = s0;
	else	SDL_FreeSurface(s0);

	if (s)
	{
		m_width  = s->w;
		m_height = s->h;	
		m_surface = new SdlVideoSurface(s);
/* Debugging: dump out the bitmap we got 
		int bypp = s->format->BytesPerPixel;	
		int bpp = s->format->BitsPerPixel;	
		printf("bypp=%d bpp=%d\n", bypp, bpp);
		unsigned char *pix = (unsigned char *)(s->pixels);
		for (int y = 0; y < m_height; y++)
			for (int x = 0; x < m_width; x++)
		{
			printf("x=%d y=%d pix=%02x %02x %02x %02x\n",
				x, y, 
				pix[(y * s->pitch) +     (x * bypp)],	
				pix[(y * s->pitch) + 1 + (x * bypp)],	
				pix[(y * s->pitch) + 2 + (x * bypp)],	
				pix[(y * s->pitch) + 3 + (x * bypp)]);
		}
*/
		
	}
	else
        {
		m_surface = NULL;
                jswLog("JSWED Video: Failed to load %s: %s\n",
                        image, SDL_GetError());

        }

}

SdlVideoBitmap::SdlVideoBitmap(int w, int h, Uint32 flags) : VideoBitmap(w,h)
{
	SDL_Surface *screen = ((SdlVideoSurface *)videoScreen)->getSurface();

	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE | flags, w, h,
			screen->format->BitsPerPixel,
			screen->format->Rmask,
			screen->format->Gmask,
			screen->format->Bmask,
			screen->format->Amask);
	if (!s)
	{
		m_surface = NULL;
                jswLog("JSWED Video: Failed to create surface: %s\n",
                        SDL_GetError());

	}
	else m_surface = new SdlVideoSurface(s);
}

SdlVideoBitmap::~SdlVideoBitmap()
{
	if (m_surface) delete m_surface;
}

VideoSurface *SdlVideoBitmap::getSurface()
{
	return m_surface;
}

void SdlVideoBitmap::toScreen(int x, int y)
{
	toSurface(videoScreen, x, y);
}

void SdlVideoBitmap::toSurface(VideoSurface *s, int x, int y)
{
	SDL_Rect sr, dr;
        SDL_Surface *surface = ((SdlVideoSurface *)s)->getSurface();

	if (!m_surface) return;
	sr.x = sr.y = 0;
	dr.x = x;
	dr.y = y;
	sr.w = dr.w = m_width;
	sr.h = dr.h = m_height;

	/* Try to clip out-of-range blits */
	if (dr.x >= surface->w || dr.y >= surface->h) return;
	if (dr.x + dr.w >= surface->w) dr.w = surface->w - dr.x;
	if (dr.y + dr.h >= surface->h) dr.h = surface->h - dr.y;

	if (SDL_BlitSurface(m_surface->getSurface(), &sr, surface, &dr) < 0)
		jswLog("JSWED Video: Failed to blit to surface: %s\n",
			SDL_GetError());
        SDL_UpdateRect(surface, dr.x, dr.y, dr.w, dr.h);
}


void SdlVideoBitmap::toScreen(int x, int y, int xs, int ys, int ws, int hs)
{
	toSurface(videoScreen, x, y, xs, ys, ws, hs);
}


void SdlVideoBitmap::toSurface(VideoSurface *s, int x, int y, 
				int xs, int ys, int ws, int hs)
{
        SDL_Rect sr, dr;
        SDL_Surface *screen = ((SdlVideoSurface *)s)->getSurface();

        if (!m_surface) return;
        sr.x = xs;
	sr.y = ys;
        dr.x = x;
        dr.y = y;
        sr.w = dr.w = ws; 
        sr.h = dr.h = hs;
	
	/* Try to clip out-of-range blits */
	if (dr.x >= screen->w || dr.y >= screen->h) return;
	if (dr.x + dr.w >= screen->w) dr.w = screen->w - dr.x;
	if (dr.y + dr.h >= screen->h) dr.h = screen->h - dr.y;

        if (SDL_BlitSurface(m_surface->getSurface(), &sr, screen, &dr) < 0)
                jswLog("JSWED Video: Failed to blit to screen: %s\n",
                        SDL_GetError());
        SDL_UpdateRect(screen, dr.x, dr.y, dr.w, dr.h);

}


void SdlVideoBitmap::fromScreen(int x, int y)
{
        SDL_Rect sr, dr;
        SDL_Surface *screen = ((SdlVideoSurface *)videoScreen)->getSurface();

	if (!m_surface) return;
        sr.x = sr.y = 0;
        dr.x = x;
        dr.y = y;
        sr.w = dr.w = m_width;
        sr.h = dr.h = m_height;

        if (SDL_BlitSurface(screen, &dr, m_surface->getSurface(), &sr) < 0)
                jswLog("JSWED Video: Failed to blit to screen: %s\n",
                        SDL_GetError());
}

void SdlVideoBitmap::fromSprite(const jswByte *sprite, int doubled, int fg, int bg,
	int format)
{
	int x, y;
	int xp, yp;
	int d, ph;
	int mask;
	VideoSurface *s = getSurface();

/* Convert a 1-bit bitmap to an SDL bitmap. The "format" flag is:
 *
 * 0: Sprite  - simple raster rows, each byte starts with most significant bit
 * 1: XBM     - as Sprite, but each byte starts with the least significant bit
 * 2: Screen$ - Always 256 bytes wide, rows are in Spectrum screen order
 */

	if (doubled)	ph = (m_width+14) / 16;
	else		ph = (m_width+7 ) / 8;
	xp = yp = 0;
	d = 0;
	for (y = 0; y < m_height; y++)
	{
		if (format == 1) mask = 1; else mask = 0x80;
		if (format == 2)
		{
			int y1 = y; 

			if (doubled) y1 /= 2;

			yp = (y1 & 0x07) * 256 +	
                             (y1 & 0x38) *   4 +
                             (y1 >>   6) * 2048;
		}
		xp = yp;
		for (x = 0; x < m_width; x++)
		{
			if (sprite[xp] & mask)s->plot(x, y, fg);
			else		      s->plot(x, y, bg);	
			if (!doubled || (d & 1))
			{
				if (format == 1) mask = mask << 1;
				else             mask = mask >> 1;
				if (mask == 0x000) { mask = 0x80; ++xp; }
				if (mask == 0x100) { mask = 0x01; ++xp; }
			}
			if (doubled) d = d ^ 1;
		}
		if ((!doubled) || (d & 2))
		{
			if (format != 2) yp += ph;
		}
                if (doubled) d = d ^ 2;

	}
}

/* Make areas of this bitmap that were painted with the TRANSPARENT colour,
 * really transparent. */
void SdlVideoBitmap::makeTransparent()
{
	SDL_SetColorKey(m_surface->getSurface(), 
		SDL_SRCCOLORKEY | SDL_RLEACCEL,
		SDL_MapRGB(m_surface->getSurface()->format, 
		gl_r[ZX_TRANS], gl_g[ZX_TRANS], gl_b[ZX_TRANS]));
}


VideoBitmap *newVideoBitmap(const char *filename)
{
	return new SdlVideoBitmap(filename);
}

VideoBitmap *newVideoBitmap(int w, int h)
{
	return new SdlVideoBitmap(w, h);
}

VideoBitmap *newVideoBitmap(int x, int y, int w, int h)
{
	SdlVideoBitmap *bmp = new SdlVideoBitmap(w, h);
	if (bmp) bmp->fromScreen(x,y);
	return bmp;
}

VideoBitmap *bitmapFromSprite(int w, int h, const jswByte *sprite, int doubled,
		int fg, int bg)
{
	SdlVideoBitmap *bmp;
	Uint32 flags = 0;

	if (doubled)    bmp = new SdlVideoBitmap(w * 2, h * 2, flags);
	else		bmp = new SdlVideoBitmap(w, h, flags);
	if (bmp) bmp->fromSprite(sprite, doubled, fg, bg);
	return bmp;	
}

VideoBitmap *bitmapFromXbm(int w, int h, const jswByte *xbm, int doubled,
                int fg, int bg)
{
        SdlVideoBitmap *bmp;
	Uint32 flags = 0;

        if (doubled)    bmp = new SdlVideoBitmap(w * 2, h * 2, flags);
        else            bmp = new SdlVideoBitmap(w, h, flags);
        if (bmp) bmp->fromXbm(xbm, doubled, fg, bg);
        return bmp;
}

VideoBitmap *bitmapFromScreen(int h, const jswByte *scr, int doubled, int fg, int bg)
{
        SdlVideoBitmap *bmp;

        if (doubled)    bmp = new SdlVideoBitmap(512, h * 2);
        else            bmp = new SdlVideoBitmap(256, h);
        if (bmp) bmp->fromScreen(scr, doubled, fg, bg);
        return bmp;
}



