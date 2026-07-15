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

VideoColourChooser::VideoColourChooser(int x, int y): VideoControl(x,y,
8*CHAR_W + 2, CHAR_H + 2)
{
	m_ink = 0;
	m_listener = NULL;
}

VideoColourChooser::~VideoColourChooser()
{
}


void VideoColourChooser::redrawCtrl(void)
{
	int n, ctrst;

	if (!m_visible) return;
	for (n = 0; n < 8; n++)
	{
		char c = n + '0';

		ctrst = (n < 4) ? ZX_WHITE : ZX_BLACK;
		
		if (n == m_ink)
		{
			videoScreen->drawText(1 + m_x + CHAR_W * n, 1 + m_y, c,
				ctrst | ZX_BRIGHT, n | ZX_BRIGHT);
			videoScreen->box(1 + m_x + CHAR_W * n, 1 + m_y,
				         CHAR_W, CHAR_H, ZX_BRCYAN);
					 	
		}
		else
		{
	                videoScreen->drawText(1 + m_x + CHAR_W * n, 1 + m_y, c, 
				ctrst, n);
		}
	} 
}

void VideoColourChooser::redraw(void)
{
	if (!m_visible) return;
	videoScreen->box     (m_x,     m_y,     m_w,      m_h, ZX_BLACK);
	redrawCtrl();
}

void VideoColourChooser::drawFocus(int focused)
{
	VideoControl::drawFocus(focused);
}

void VideoColourChooser::setColour(int i)
{
	m_ink = i & 7;
	redrawCtrl();
}

int VideoColourChooser::getColour(void)
{
	return m_ink;
}



int VideoColourChooser::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_ENTER:	return ZXE_OK;
		case ZXK_BREAK:	return ZXE_CANCEL;
		case ZXK_RIGHT:
			++m_ink;
			m_ink &= 7;
			if (m_listener) m_listener->onChooseColour(this);
			redrawCtrl();
			return ZXE_CONTINUE;

		case ZXK_LEFT:
			--m_ink;
			m_ink &= 7;
                        if (m_listener) m_listener->onChooseColour(this); 
			redrawCtrl();
			return ZXE_CONTINUE;
	}
	return VideoControl::onKeyDown(keysym);
}


int VideoColourChooser::onButtonDown(int x, int y, int button)
{
	x -= (m_x + 1); if (x < 0) return ZXE_CONTINUE;
	x /= CHAR_W; if (x > 7) return ZXE_CONTINUE;
	m_ink = x;
        if (m_listener) m_listener->onChooseColour(this);
	redrawCtrl();
	return ZXE_CONTINUE;	
}


