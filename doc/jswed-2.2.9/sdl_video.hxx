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

extern "C" {

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_image.h>

};

#ifdef INSTANTIATE
# define EXT
# define EQ(x) = x
#else
# define EXT extern
# define EQ(x)
#endif

EXT SDL_Color *gl_colours;
EXT int gl_sdl_locked EQ(0);

#define MAXCOLOUR 21

extern int gl_b[MAXCOLOUR];
extern int gl_r[MAXCOLOUR];
extern int gl_g[MAXCOLOUR];

class SdlVideoSurface : public VideoSurface
{
private:
	SDL_Surface *m_surface;
public:
        SdlVideoSurface(SDL_Surface *s);
        virtual ~SdlVideoSurface();

	inline SDL_Surface *getSurface(void) { return m_surface; }

	void setupPalette(void);

        virtual void box(int x, int y, int w, int h, int colour);
        virtual void dottedBox(int x, int y, int w, int h, int colour);
        virtual void fillBox(int x, int y, int w, int h, int colour);
	virtual void shadedBox(int x, int y, int w, int h, int c1, int c2);
        virtual void fillReplace(int x, int y, int w, int h, int c1, int c2);
        virtual void plot(int x, int y, int colour);
	virtual unsigned int point(int x, int y);
	virtual unsigned int pointZx(int x, int y);
        virtual void stripe(int x, int y, int w, int h, int colour1, int colour2);
        virtual void drawSmallText(int x1, int y1, const char *str, int fg, int bg);
        virtual void drawText(int x1, int y1, const char *str, int fg, int bg);
        virtual void rotateL(int x, int y, int w, int h, int count = 1);
        virtual void rotateR(int x, int y, int w, int h, int count = 1);

protected:
        Uint32 point(Uint8 *bits, int bpp);
	void plot(Uint8 *bits, Uint32 pixel, int bpp);
	void plotReplace(Uint8 *bits, Uint32 p1, Uint32 p2, int bpp);
	int lock(void);
	void unlock(void);	
};

EXT VideoSurface *videoScreen;
EXT VideoBitmap *videoIcon;

#include "sdl_bitmap.hxx"
