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

VideoControl::VideoControl(int x, int y, int w, int h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_focused = 1;
	m_visible = 0;
}

VideoControl::~VideoControl()
{
}

int VideoControl::hitTest(int x, int y)
{
	if (x < m_x || y < m_y || x >= (m_x + m_w) || y >= (m_y + m_h)) 
		return 0;
	return 1;
}


int VideoControl::canFocus()
{
	return 1;
}

void VideoControl::drawFocus(int focused)
{
	m_focused = focused;
	if (m_visible) videoScreen->dottedBox(m_x - 2, m_y - 2, m_w + 4, m_h + 4,
			focused ? ZX_BRBLUE : ZX_WHITE);	
}


void VideoControl::setVisible(int visible, int r)
{
	m_visible = visible;
	if (m_visible && r) redraw();
}


int  VideoControl::getVisible(void)
{
	return m_visible;
}

