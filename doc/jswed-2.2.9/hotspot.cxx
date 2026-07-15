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

#include "hotspot.hxx"

Hotspot::Hotspot(int id, int x, int y, int w, int h, int ks1, int ks2)
{
	m_id = id;
	m_x  = x;
	m_y  = y;
	m_w  = w;
	m_h  = h;
	m_ks1 = ks1;
	m_ks2 = ks2;
}


int Hotspot::hitTest(int x, int y)
{
	if (x >= m_x && y >= m_y && x < (m_x + m_w) && y < (m_y + m_h))
		return m_id;
	return -1;
}

int Hotspot::keyTest(int ks)
{
	if (ks == m_ks1 || ks == m_ks2) return m_id;
	else return -1;
}


