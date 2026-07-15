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
#include "jswdirbox.hxx"

JswDirCheckbox::JswDirCheckbox(int x, int y): VideoCheckbox(x, y)
{
	m_vertical = 0;
	m_listener = NULL;
}

JswDirCheckbox::~JswDirCheckbox()
{
}

#include "left.xbm"
#include "right.xbm"
#include "up.xbm"
#include "down.xbm"

void JswDirCheckbox::redrawCtrl(void)
{
	unsigned char *ptr;

	if (!m_visible) return;

	videoScreen->fillBox(m_x + 1, m_y + 1, m_w - 2, m_h - 2, ZX_BRWHITE);
	if      (m_tick && m_vertical) ptr = down_bits;
	else if (m_tick)               ptr = right_bits;
	else if (m_vertical)           ptr = up_bits;
	else                           ptr = left_bits;

	VideoBitmap *bm = bitmapFromXbm(left_width, left_height, ptr, 0,
					ZX_BRBLUE);
	bm->toScreen(m_x + 2, m_y + 2);
	delete bm;
}

void JswDirCheckbox::setVertical(int v)
{
	m_vertical = v;
	redrawCtrl();	
}



JswDiagCheckbox::JswDiagCheckbox(int x, int y): VideoCheckbox(x, y)
{
	m_nstates = 4;
	m_listener = NULL;
}

JswDiagCheckbox::~JswDiagCheckbox()
{
}

#include "diag0.xbm"
#include "diag1.xbm"
#include "diag2.xbm"
#include "diag3.xbm"

void JswDiagCheckbox::redrawCtrl(void)
{
	unsigned char *ptr = NULL;

	if (!m_visible) return;

	videoScreen->fillBox(m_x + 1, m_y + 1, m_w - 2, m_h - 2, ZX_BRWHITE);
	switch(m_tick)
	{
		case 0: ptr = diag0_bits; break;
		case 1: ptr = diag1_bits; break;
		case 2: ptr = diag2_bits; break;
		case 3: ptr = diag3_bits; break;
	}
	if (!ptr) return;
	VideoBitmap *bm = bitmapFromXbm(diag0_width, diag0_height, ptr, 0,
					ZX_BRBLUE);
	bm->toScreen(m_x + 2, m_y + 2);
	delete bm;
}

