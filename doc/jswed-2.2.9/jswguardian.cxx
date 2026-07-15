/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "jswguardian.hxx"


JswGuardian::JswGuardian(jswByte *g, jswByte x, SpectrumMemory *mem, int bg)
{
        memcpy(m_guard, g, 8);
        m_guard[2] = x;
        m_frame = (x & 0xE0);
        m_x = (x & 0x1F);
        m_y = m_guard[3] & 0xFE;
        m_mem = mem;
        m_bmp      = newVideoBitmap(256, 32);
        m_bmpSmall = newVideoBitmap(128, 16);
        m_bgcol = bg;
	memcpy(m_orig, m_guard, 8);
}


JswGuardian::~JswGuardian()
{
	delete m_bmp;
	delete m_bmpSmall;
}



void JswGuardian::drawRope(VideoSurface *s)
{
	int y = 0;
	int iy = 0;
	int guard_x = m_guard[1];
	int x = 8 * m_guard[2];	// Initial X
	unsigned ropeTable = m_mem->peek(0x931D) * 256;

	do
	{
		s->fillBox(x * 2, iy, 2, 2, ZX_WHITE);

		int p = (y + guard_x) & 0x7F;	
		int dy = m_mem->peek(ropeTable + 128 + p);
		int dx = m_mem->peek(ropeTable + p);

		iy += dy;

		if (guard_x & 0x80) x -= dx;
		else	 	    x += dx;		
	}
	while ((y++) < m_guard[4]); 

}

void JswGuardian::moveRope(void)
{
	int a;

	if (m_guard[0] & 0x80)  // Going left
        {
		a = m_guard[1];
		if (a >= 0x80)  // left-hand bit
		{
		        a -= 2;
		        if (a <  0x94) a -= 2;
		        if (a == 0x80) a = 0;
		}
		else            // right-hand bit
		{
		        a += 2;
		        if (a < 0x12) a += 2;
		}
	}
	else            // going right
	{		
		a = m_guard[1];
		if (a & 0x80)   // left-hand bit
		{
			a += 2;
			if (a < 0x92) a += 2;
		}
		else            // right-hand bit
		{
			a -= 2;
			if (a < 20) a -= 2;
			if (!a)     a = 0x80;
		}
	}
	m_guard[1] = (a & 0xFF);
	if (m_guard[7] == (m_guard[1] & 0x7F))
		m_guard[0] ^= 0x80;
}


jswByte JswGuardian::getSpritePage(void) { return m_guard[5]; }

void JswGuardian::setSpritePage(jswByte page)
{
        m_guard[5] = page;
        constructSprites();
}

jswByte JswGuardian::getInk(void) { return m_guard[1] & 0x0F; }

void JswGuardian::setInk(jswByte i)
{
        i &= 0x0F;
        m_guard[1] &= 0xF0; m_guard[1] |= i;
        constructSprites();
}


jswByte JswGuardian::getY() { return m_guard[3]; }
void JswGuardian::setRopeLen(jswByte len) { m_guard[4] = len; }
jswByte JswGuardian::getRopeLen(void) { return m_guard[4]; }
void JswGuardian::setRopeSwing(jswByte swing) { m_guard[1] = m_guard[7] = swing; }
jswByte JswGuardian::getRopeSwing(void)     { return m_guard[7]; }
jswByte JswGuardian::getStep(void)          { return m_guard[4]; }
jswByte JswGuardian::getAnim() { return m_guard[1] >> 5; }

void JswGuardian::setAnim(jswByte b)
{
        b &= 7;
        m_guard[1] &= 0x1F;
        m_guard[1] |= (b << 5);
        constructSprites();
}

