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

VideoCheckbox::VideoCheckbox(int x, int y): VideoControl(x,y, CHAR_W, CHAR_H)
{
	m_tick = 0;
	m_nstates = 2;
	m_listener = NULL;
}

VideoCheckbox::~VideoCheckbox()
{
}

#include "tick.xbm"
#include "shaded.xbm"


void VideoCheckbox::redrawCtrl(void)
{
	unsigned char *ptr;

	if (!m_visible) return;

	videoScreen->fillBox(m_x + 1, m_y + 1, m_w - 2, m_h - 2, ZX_BRWHITE);
	if (m_tick)
	{
		ptr = tick_bits;
		if (m_tick == 2) ptr = shaded_bits;
		VideoBitmap *bm = bitmapFromXbm(tick_width, tick_height, 
						ptr, 0, ZX_BRBLUE);
		bm->toScreen(m_x + 2, m_y + 2);
		delete bm;
	}
}

void VideoCheckbox::redraw(void)
{
	if (!m_visible) return;
	videoScreen->box     (m_x,     m_y,     m_w,      m_h, ZX_BLACK);
	redrawCtrl();
}

void VideoCheckbox::drawFocus(int focused)
{
	VideoControl::drawFocus(focused);
}

void VideoCheckbox::setCheck(int i)
{
	if (i < m_nstates) m_tick = i;
	else m_tick = 1;

	redrawCtrl();
}

int VideoCheckbox::getCheck(void)
{
	return m_tick;
}


void VideoCheckbox::setStates(int i)
{
	m_nstates = i;
}

int VideoCheckbox::getStates(void)
{
	return m_nstates;
}



int VideoCheckbox::updateCheck(int n)
{
	int rv = ZXE_OK;
	if (m_listener) rv = m_listener->onCheckboxSelect(this, n);

	if (rv != ZXE_OK) return rv;
	m_tick = n;
	return ZXE_OK;
}

int VideoCheckbox::toggleCheck()
{	
	int rv = ZXE_CANCEL;
	int ot, nt;

	if (m_nstates == 2) return updateCheck(!m_tick);

	ot = m_tick;
	nt = ((m_tick + 1) % (m_nstates));

	while (nt != ot)
	{
		rv = updateCheck(nt);

		if (rv == ZXE_OK || rv >= ZXE_QUIT) return rv;

		nt = (nt + 1) % (m_nstates);
	}
	return rv;
}


int VideoCheckbox::onKeyDown(int keysym)
{
	int rv;

	switch(keysym)
	{
		case ZXK_ENTER:	return ZXE_OK;
		case ZXK_BREAK:	return ZXE_CANCEL;
		case ' ': 	rv = toggleCheck();
				if (rv != ZXE_CANCEL) redrawCtrl();
				if (rv >= ZXE_QUIT)   return rv;
				return ZXE_CONTINUE;

		case 'y': case 'Y': case '1': case '+': case '=':
				rv = updateCheck(1);	
				if (rv != ZXE_CANCEL) redrawCtrl();
				if (rv >= ZXE_QUIT) return rv;
				return ZXE_CONTINUE;
		case 'n': case 'N': case '0': case '-':
                                rv = updateCheck(0);
                                if (rv != ZXE_CANCEL) redrawCtrl();
                                if (rv >= ZXE_QUIT) return rv;                                                  return ZXE_CONTINUE;

	}
	return VideoControl::onKeyDown(keysym);
}


int VideoCheckbox::onButtonDown(int x, int y, int button)
{
	int rv = toggleCheck();
	if (rv != ZXE_CANCEL) redrawCtrl();
	if (rv >= ZXE_QUIT) return rv;
	return ZXE_CONTINUE;	
}


