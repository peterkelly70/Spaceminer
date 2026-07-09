/************************************************************************

    JSWED 2.00 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001  John Elliott <jce@seasip.demon.co.uk>

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

VideoSurface::VideoSurface()
{

}

VideoSurface::~VideoSurface()
{

}

/*** Video routines which are not dependent on the graphics library in use */

void VideoSurface::zxLogo(int x, int y)
{
	stripe(x,         y,CHAR_W,CHAR_H,ZX_BLACK,    ZX_BRRED);
        stripe(x+  CHAR_W,y,CHAR_W,CHAR_H,ZX_BRRED,    ZX_BRYELLOW);
        stripe(x+2*CHAR_W,y,CHAR_W,CHAR_H,ZX_BRYELLOW, ZX_BRGREEN);
        stripe(x+3*CHAR_W,y,CHAR_W,CHAR_H,ZX_BRGREEN,  ZX_BRCYAN);
        stripe(x+4*CHAR_W,y,CHAR_W,CHAR_H,ZX_BRCYAN,   ZX_BLACK);
}

void VideoSurface::smallZxLogo(int x, int y)
{
        stripe(x,          y,CHAR_SW,CHAR_SH,ZX_BLACK,    ZX_BRRED);
        stripe(x+  CHAR_SW,y,CHAR_SW,CHAR_SH,ZX_BRRED,    ZX_BRYELLOW);
        stripe(x+2*CHAR_SW,y,CHAR_SW,CHAR_SH,ZX_BRYELLOW, ZX_BRGREEN);
        stripe(x+3*CHAR_SW,y,CHAR_SW,CHAR_SH,ZX_BRGREEN,  ZX_BRCYAN);
        stripe(x+4*CHAR_SW,y,CHAR_SW,CHAR_SH,ZX_BRCYAN,   ZX_BLACK);
}


void VideoSurface::drawText(int x1, int y1, char c, int fg, int bg)
{
	char s[2];

	if (c == 0)
	{
		fillBox(x1, y1, CHAR_W, CHAR_H, bg);
		return;		
	}
	s[0] = c;
	s[1] = 0;
	drawText(x1, y1, s, fg, bg);
}



void VideoSurface::drawSmallText(int x1, int y1, char c, int fg, int bg)
{
	char s[2];

	if (c == 0)
	{
		fillBox(x1, y1, CHAR_SW, CHAR_SH, bg);
		return;		
	}
	s[0] = c;
	s[1] = 0;
	drawSmallText(x1, y1, s, fg, bg);
}

void VideoSurface::bottomBar(char *s)
{
	fillBox (0, VIDEO_H - 3 * CHAR_H, VIDEO_W, CHAR_H, ZX_BLACK);
	drawText(0, VIDEO_H - 3 * CHAR_H, s, ZX_BRWHITE, ZX_BLACK);
	zxLogo(VIDEO_W - 6 * CHAR_W, VIDEO_H - 3 * CHAR_H);
}



/** Clear screen */
void VideoSurface::cls(void)
{
        fillBox(0, 0, VIDEO_W, VIDEO_H, ZX_WHITE);
}

