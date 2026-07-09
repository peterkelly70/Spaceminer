/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

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

#define INSTANTIATE
#include "sdl_video.hxx"

int gl_b[MAXCOLOUR] = { 0x00, 0xC0, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0xC0,
		 	0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
		 	0x00, 0x00, 0x80, 0xA0, 0x41};
int gl_r[MAXCOLOUR] = { 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0xC0, 0xC0,
		 	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
		 	0xC0, 0xFF, 0x80, 0xA0, 0x42 };
int gl_g[MAXCOLOUR] = { 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0,
		 	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
 		 	0x60, 0x80, 0x80, 0xA0, 0x43 };

/* The colour with b=41 r=42 g=43 (ZX_TRANS) is used as a colour key when 
 * drawing sprites with transparent backgrounds. It's a neutral enough
 * colour that most things should show up against it if it does get 
 * displayed. */

SdlVideoSurface::SdlVideoSurface(SDL_Surface *s)
{
	m_surface = s;
	setupPalette();
}

SdlVideoSurface::~SdlVideoSurface()
{
	if (m_surface) SDL_FreeSurface(m_surface);
}


void SdlVideoSurface::setupPalette(void)
{
	SDL_Color colours[MAXCOLOUR];
	int n;

	for (n = 0; n < MAXCOLOUR; n++)
	{
		colours[n].r = gl_r[n];
		colours[n].g = gl_g[n];
		colours[n].b = gl_b[n];
	} 

	SDL_SetColors(m_surface, colours, 0, MAXCOLOUR);
}


/** Internal routine to plot a single point. Screen surface must be locked
    and pixel values must have been calculated */

int  SdlVideoSurface::lock(void)
{
	if ( SDL_MUSTLOCK(m_surface) )
	{
		if ( SDL_LockSurface(m_surface) < 0 ) 
		{
			jswLog("JSWED Video: Failed to lock video surface: %s\n", SDL_GetError());
			return -1;
		}
	}
	gl_sdl_locked++;
	return 0;
}

void SdlVideoSurface::unlock(void)
{
	if (gl_sdl_locked) --gl_sdl_locked;
	if ( SDL_MUSTLOCK(m_surface) )
	{
		SDL_UnlockSurface(m_surface);
	}
}

void SdlVideoSurface::plot(Uint8 *bits, Uint32 pixel, int bpp)
{
	switch(bpp)
	{
		case 1:
		*((Uint8 *)(bits)) = (Uint8)pixel;
		break;

		case 2:
		*((Uint16 *)(bits)) = (Uint16)pixel;
		break;

		case 3:
		{ /* Format/endian independent */
			Uint8 r, g, b;

			r = (pixel>>m_surface->format->Rshift)&0xFF;
			g = (pixel>>m_surface->format->Gshift)&0xFF;
			b = (pixel>>m_surface->format->Bshift)&0xFF;
			*((bits)+m_surface->format->Rshift/8) = r;
			*((bits)+m_surface->format->Gshift/8) = g;
			*((bits)+m_surface->format->Bshift/8) = b;
		}
		break;

		case 4:
		*((Uint32 *)(bits)) = (Uint32)pixel;
		break;
	}
}


unsigned int SdlVideoSurface::pointZx(int x, int y)
{
	Uint32 pixel = point(x, y);
/* 	Uint8 r, g,b;
	SDL_GetRGB(pixel, m_surface->format, &r, &g, &b);

	for (int n = 0; n < MAXCOLOUR;n++)
	{
		if (gl_r[n] == r && gl_g[n] == g && gl_b[n] == b) return n;
	}
	return 0;	
*/	return pixel;	// I'd have thought you'd need to reverse the 
			// SDL_MapRGB(), but apparently not. Strange...
}

unsigned int SdlVideoSurface::point(Uint8 *bits, int bpp)
{
	switch(bpp)
	{
		case 1:
		return *((Uint8 *)(bits));
		break;

		case 2:
		return *((Uint16 *)(bits));
		break;

		case 3:
		{ /* Format/endian independent */
			Uint8 r, g, b;

			r = *((bits)+m_surface->format->Rshift/8);
			g = *((bits)+m_surface->format->Gshift/8);
			b = *((bits)+m_surface->format->Bshift/8);
			return (r << m_surface->format->Rshift) |
			   (g << m_surface->format->Gshift) |
			   (b << m_surface->format->Bshift);
		}
		break;

		case 4:
		return *((Uint32 *)(bits));
		break;
	}
	return 0;
}


void SdlVideoSurface::plotReplace(Uint8 *bits, Uint32 p1, Uint32 p2, int bpp)
{
	switch(bpp)
	{
		case 1:
		if (*((Uint8 *)(bits)) == (Uint8)p1)  *((Uint8 *)(bits)) = (Uint8)p2;
		break;

		case 2:
		if (*((Uint16 *)(bits)) == (Uint16)p1) *((Uint16 *)(bits)) = (Uint16)p2;
		break;

		case 3:
		{ /* Format/endian independent */
			Uint8 r1, g1, b1, r2, g2, b2;

			r1 = (p1>>m_surface->format->Rshift)&0xFF;
			g1 = (p1>>m_surface->format->Gshift)&0xFF;
			b1 = (p1>>m_surface->format->Bshift)&0xFF;
			r2 = (p2>>m_surface->format->Rshift)&0xFF;
			g2 = (p2>>m_surface->format->Gshift)&0xFF;
			b2 = (p2>>m_surface->format->Bshift)&0xFF;

			if (*((bits)+m_surface->format->Rshift/8) == r1 &&
			*((bits)+m_surface->format->Gshift/8) == g1 &&
			*((bits)+m_surface->format->Bshift/8) == b1)
			{
				*((bits)+m_surface->format->Rshift/8) = r2;
		   			*((bits)+m_surface->format->Gshift/8) = g2;
				*((bits)+m_surface->format->Bshift/8) = b2;
			}
		}
		break;

		case 4:
		if (*((Uint32 *)(bits)) == (Uint32)p1) *((Uint32 *)(bits)) = (Uint32)p2;
		break;
	}
}



/** Draw a filled box */
void SdlVideoSurface::fillBox(int x, int y, int w, int h, int colour)
{
	Uint32 pixel;
	SDL_Rect  rect;

/* 2.2.4: Make sure we're not drawing objects outside the surface */
        if (x < 0 || y < 0 || x >= m_surface->w || y >= m_surface->h) 
		return;

	if (x + w > m_surface->w) w = m_surface->w - x;
	if (y + h > m_surface->h) h = m_surface->h - y;

	pixel = SDL_MapRGB(m_surface->format, gl_r[colour], 
					      gl_g[colour], 
					      gl_b[colour]);

	if (lock()) return;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_FillRect(m_surface, &rect, pixel);

	unlock();

	SDL_UpdateRect(m_surface, x, y, w, h);

}



/** Draw an unfilled box */
void SdlVideoSurface::box(int x, int y, int w, int h, int colour)
{
	Uint32 pixel;
	SDL_Rect rect;

	pixel = SDL_MapRGB(m_surface->format, gl_r[colour], 
					      gl_g[colour], 
					      gl_b[colour]);
	if (lock()) return;  
	rect.x = x;
	rect.y = y;
	rect.w = 1; 
	rect.h = h;
	SDL_FillRect(m_surface, &rect, pixel);
	rect.x = x + w - 1;
	SDL_FillRect(m_surface, &rect, pixel);
	rect.x = x;
	rect.w = w;
	rect.h = 1;
	SDL_FillRect(m_surface, &rect, pixel);
	rect.y = y + h - 1;
	SDL_FillRect(m_surface, &rect, pixel);
	unlock();
	SDL_UpdateRect(m_surface, x, y, w, h);

}


/** Draw a box with a dotted outline */
void SdlVideoSurface::dottedBox(int x1, int y1, int w, int h, int colour)
{
	int x, y, bpp;
	Uint32 pixel;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	pixel = SDL_MapRGB(m_surface->format, gl_r[colour], 
					      gl_g[colour], 
					      gl_b[colour]);
	if (x1 < 0) x1 = 0; 
	if (y1 < 0) y1 = 0;
	if (x1 + w >= VIDEO_W) w = VIDEO_W - x1 - 1;
	if (y1 + h >= VIDEO_H) h = VIDEO_H - y1 - 1;
					  
	if (lock()) return;  
	for (y = 0; y < h; y+=2)
	{
		bits = ((Uint8 *)m_surface->pixels)+ 
			((y+y1) * m_surface->pitch) + (x1 * bpp);
		plot(bits, pixel, bpp);
		bits += (w-1) * bpp;
		plot(bits, pixel, bpp);
	}

	for (x = 0; x < w; x+=2)
	{
		bits = ((Uint8 *)m_surface->pixels)+ 
			(y1 * m_surface->pitch) + ((x+x1) * bpp);
		plot(bits, pixel, bpp);
		bits += (h-1) * m_surface->pitch;
		plot(bits, pixel, bpp);
	}


	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}



/** Rotate a row of pixels to the left by one */
void SdlVideoSurface::rotateL(int x1, int y1, int w, int h, int count)
{
	int x, y, n, bpp;
	Uint32 pixel, pix2;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	if (lock()) return;  

	for (n = 0; n < count; n++) for (y = 0; y < h; y++)
	{
		bits = ((Uint8 *)m_surface->pixels)+
			 ((y+y1) * m_surface->pitch) + (x1 * bpp);
		pixel = point(bits, bpp);

		bits += (w - 1) * bpp;

		for (x = (w - 1); x >= 0; x--)
		{
			pix2 = point(bits, bpp);
			plot(bits, pixel, bpp);
			pixel = pix2;
			bits -= bpp;
		}
	}

	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}

void SdlVideoSurface::rotateR(int x1, int y1, int w, int h, int count)
{
	int x, y, n, bpp;
	Uint32 pixel, pix2;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	if (lock()) return;  

	for (n = 0; n < count; n++) for (y = 0; y < h; y++)
	{
		bits = ((Uint8 *)m_surface->pixels)+
			 ((y+y1) * m_surface->pitch) + ((x1+w-1) * bpp);
		pixel = point(bits, bpp);

		for (x = 0; x < (w-1); x++)
		{
			pix2 = point(bits - bpp, bpp);
			plot(bits, pix2, bpp);
			bits -= bpp;
		}
		plot(bits, pixel, bpp);
	}

	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}




/** Draw a box with every other pixel set */
void SdlVideoSurface::shadedBox(int x1, int y1, int w, int h, int c1, int c2)
{
	int x, y, bpp;
	Uint32 p1, p2;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	p1 = SDL_MapRGB(m_surface->format, gl_r[c1], 
					   gl_g[c1], 
					   gl_b[c1]);
	p2 = SDL_MapRGB(m_surface->format, gl_r[c2], 
					   gl_g[c2], 
					   gl_b[c2]);
	if (lock()) return;  
	for (y = 0; y < h; y++)
	{
		bits = ((Uint8 *)m_surface->pixels)+ 
			((y+y1) * m_surface->pitch) + (x1 * bpp);
		for (x = 0; x < w; x++)
		{
			if (((x ^ y) & 3)) plot(bits, p1, bpp);
			else		   plot(bits, p2, bpp);
			bits += bpp;
		}
	}
	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}




/** Draw a box, in two different colours */
void SdlVideoSurface::stripe(int x1, int y1, int w, int h, int c1, int c2)
{
	int x, y, dy, bpp;
	Uint32 pixel1, pixel2;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	pixel1 = SDL_MapRGB(m_surface->format, gl_r[c1], 
					       gl_g[c1], 
					       gl_b[c1]);
					    
	pixel2 = SDL_MapRGB(m_surface->format, gl_r[c2], 
					       gl_g[c2], 
					       gl_b[c2]);
					    
	if (lock()) return;  
	for (dy=h, y = 0; y < h; y++, dy--)
	{
		bits = ((Uint8 *)m_surface->pixels)+ 
			((y+y1) * m_surface->pitch) + (x1 * bpp);
		for (x = 0; x < w; x++) 
		{       
			plot(bits, (dy > x) ? pixel1 : pixel2, bpp);
			bits += bpp;
		}
	}
	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}

/** In a box, replace one colour with another */
void SdlVideoSurface::fillReplace(int x1, int y1, int w, int h, int c1, int c2)
{
	int x, y, bpp;
	Uint32 pixel1, pixel2;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	pixel1 = SDL_MapRGB(m_surface->format, gl_r[c1], 
					       gl_g[c1], 
					       gl_b[c1]);
					    
	pixel2 = SDL_MapRGB(m_surface->format, gl_r[c2], 
					       gl_g[c2], 
					       gl_b[c2]);
					    
	if (lock()) return;  
	for (y = 0; y < h; y++)
	{
		if ((y + y1) < 0 || (y + y1) >= m_surface->h) continue;
		bits = ((Uint8 *)m_surface->pixels)+ 
			((y+y1) * m_surface->pitch) + (x1 * bpp);
		for (x = 0; x < w; x++) 
		{       
			if ((x + x1) >= 0 && (x + x1) < m_surface->w)
				plotReplace(bits, pixel1, pixel2, bpp);
			bits += bpp;
		}
	}
	unlock();
	SDL_UpdateRect(m_surface, x1, y1, w, h);

}



/** Draw text */
void SdlVideoSurface::drawText(int x1, int y1, const char *str, int fg, int bg)
{
	int x0, x, y, bpp, mask, ym;
	Uint32 pixel_fg, pixel_bg;
	Uint8 *bits, *src, val;

	bpp = m_surface->format->BytesPerPixel;

	pixel_fg = SDL_MapRGB(m_surface->format, gl_r[fg], 
						 gl_g[fg], 
						 gl_b[fg]);
	pixel_bg = SDL_MapRGB(m_surface->format, gl_r[bg], 
						 gl_g[bg], 
						 gl_b[bg]);
	x0 = x1;
	if (lock()) return;  
	while(*str)
	{
		if (x1 < 0) continue;
		if (x1 >= VIDEO_W) break;

		src = videoFont + 8 * ((jswByte)(*str));
		for (y = y1, ym = 0; y < (CHAR_H + y1); y++, ym++)
		{
			if (y < 0) continue;	  	
			if (y >= VIDEO_H) break;
			bits = ((Uint8 *)m_surface->pixels)+ 
				(y * m_surface->pitch) + (x1 * bpp);
			val = src[ym / 2];
			mask = 0x8080;
			for (x = x1; x < (CHAR_W + x1); x++) 
			{	   
				if (val & mask & 0xFF) 
					plot(bits, pixel_fg, bpp);
				else	plot(bits, pixel_bg, bpp);
				bits += bpp;
				if (mask & 0x8000) { mask &= 0x7FFF; }
				else	{ mask = (mask >> 1) | 0x8000; }
			}
		}
		x1 += CHAR_W;
		++str;
	}
	unlock();
	SDL_UpdateRect(m_surface, x0, y1, x1 - x0, CHAR_H);

}



/** Draw small (8x8) text */
void SdlVideoSurface::drawSmallText(int x1, int y1, const char *str, int fg, int bg)
{
	int x0, x, y, bpp, mask, ym;
	Uint32 pixel_fg, pixel_bg;
	Uint8 *bits, *src, val;

	bpp = m_surface->format->BytesPerPixel;

	pixel_fg = SDL_MapRGB(m_surface->format, gl_r[fg], 
						 gl_g[fg], 
						 gl_b[fg]);
	pixel_bg = SDL_MapRGB(m_surface->format, gl_r[bg], 
						 gl_g[bg], 
						 gl_b[bg]);
	x0 = x1;
	if (lock()) return;  
	while(*str)
	{
	    if (x1 >= VIDEO_W) break;

	    src = videoFont + 8 * ((jswByte)(*str));
	    for (y = y1, ym = 0; y < CHAR_SH + y1; y++, ym++)
	    {
		bits = ((Uint8 *)m_surface->pixels)+ 
			(y * m_surface->pitch) + (x1 * bpp);
		val = src[ym];
		mask = 0x80;
		for (x = x1; x < CHAR_SW + x1; x++) 
		{       
			if (val & mask) plot(bits, pixel_fg, bpp);
			else		plot(bits, pixel_bg, bpp);
			bits += bpp;
			mask = (mask >> 1);
		}
	    }
	    x1 += CHAR_SW;
	    ++str;
	}
	unlock();
	SDL_UpdateRect(m_surface, x0, y1, x1 - x0, CHAR_SH);

}



/** Set a single pixel */
void SdlVideoSurface::plot(int x, int y, int colour)
{
	int bpp;
	Uint32 pixel;
	Uint8 *bits;

	bpp = m_surface->format->BytesPerPixel;

	pixel = SDL_MapRGB(m_surface->format, gl_r[colour], 
					      gl_g[colour], 
					      gl_b[colour]);
					    
	if (lock()) return;  
	bits = ((Uint8 *)m_surface->pixels)+ (y * m_surface->pitch) + (x * bpp);
	plot(bits, pixel, bpp);
	unlock();
	SDL_UpdateRect(m_surface, x, y, 1, 1);
}


unsigned int SdlVideoSurface::point(int x, int y)
{
	int bpp;
	Uint32 pixel;
	Uint8 *bits;
	Uint8 r,g,b;
	int n;

	bpp = m_surface->format->BytesPerPixel;

	if (lock()) return 0;
	bits = ((Uint8 *)m_surface->pixels)+ (y * m_surface->pitch) + (x * bpp);
	pixel = point(bits, bpp);

	SDL_GetRGB(pixel, m_surface->format, &r, &g, &b);	
	unlock();

	for (n = 0; n < 16; n++) 
	{
		if (gl_r[n] == r && gl_g[n] == g && gl_b[n] == b) return n;
	}
	return 0;
}


/** Initialise video */
int videoInit(char *iconName, char *caption, char *iconCaption)
{
	atexit(videoDeInit);
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 ) 
		jswDiewith(1, "Couldn't initialize SDL: %s\n", SDL_GetError());

	// Any screen size acceptable; 8bpp preferred
	// [2.3.0] Ask for 24bpp instead.
	videoScreen = new SdlVideoSurface(SDL_SetVideoMode
		(VIDEO_W, VIDEO_H, 24, SDL_SWSURFACE | SDL_ANYFORMAT));

	videoIcon = new SdlVideoBitmap(iconName);
	if (videoIcon)
	{
		SdlVideoSurface *svs = (SdlVideoSurface *)videoIcon->getSurface();
		if (svs && svs->getSurface())
		{
			SDL_WM_SetIcon(svs->getSurface(), NULL);	
		}
	}
	SDL_WM_SetCaption(caption, iconCaption);

	videoScreen->cls();
	return 0;
}


void videoDeInit(void)
{
	if (gl_colours) 
	{
		delete gl_colours;
		gl_colours = NULL;
	}
	if (videoIcon) delete videoIcon;
	if (videoScreen) delete videoScreen;
	SDL_Quit();
}



