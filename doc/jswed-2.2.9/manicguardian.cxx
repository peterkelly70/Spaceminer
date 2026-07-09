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
#include "manicguardian.hxx"


ManicGuardian::ManicGuardian(jswByte *r, jswByte *g, jswByte *s)
{
	m_room = r;
	memcpy(m_guard, g, 7);
        m_bmp           = newVideoBitmap(256, 32);
        m_bmpFlash      = newVideoBitmap(256, 32);
        m_bmpSmall      = newVideoBitmap(128, 16);
        m_bmpSmallFlash = newVideoBitmap(128, 16);
	memcpy(m_orig, m_guard, 7);
	m_sprites = s;
	m_ticker = 0;
	constructSprites();
}

ManicHGuardian::ManicHGuardian(bool b, jswByte *r, jswByte *g) : ManicGuardian(r,g)
{
	int n;
	
	m_bidirectional = b;
	m_halfspeed     = false;
	m_x = (g[1] & 0x1F);
	m_y = ((g[1] & 0xE0) >> 1) | ((g[2] & 1) << 7);
	m_frame = g[4] & 7;
}

ManicVGuardian::ManicVGuardian(jswByte *r, jswByte *g, jswByte *s) : ManicGuardian(r,g,s)
{
	m_x = (g[3] & 0x1F);
	m_y = g[2] & 0x7F;
	m_frame = (g[1] & 3);
	m_speed = (signed char)(g[4]);
}

ManicGuardian::~ManicGuardian()
{
	delete m_bmp;
	delete m_bmpFlash;
	delete m_bmpSmall;
	delete m_bmpSmallFlash;
}

ManicHGuardian::~ManicHGuardian() 
{
}

ManicVGuardian::~ManicVGuardian() 
{
}


jswByte ManicGuardian::getInk   (void) { return  m_guard[0] & 0x07; }
jswByte ManicGuardian::getPaper (void) { return (m_guard[0] & 0x38) >> 3; }
jswByte ManicGuardian::getBright(void) { return (m_guard[0] & 0x40) >> 6; }
jswByte ManicGuardian::getFlash (void) { return (m_guard[0] & 0x80) >> 7; }
jswByte ManicHGuardian::getFlash(void) { return 0; }



void ManicGuardian::setInk(jswByte i)
{
        i &= 0x07;
        m_guard[0] &= 0xF8; m_guard[0] |= i;
        constructSprites();
}


void ManicGuardian::setPaper(jswByte i)
{
        i &= 0x07;
        m_guard[0] &= 0xC7; m_guard[0] |= (i << 3);
        constructSprites();
}


void ManicGuardian::setBright(jswByte i)
{
        i = i ? 0x40 : 0;
        m_guard[0] &= 0xBF; m_guard[0] |= i;
        constructSprites();
}


void ManicGuardian::setFlash(jswByte i)
{
        i = i ? 0x80 : 0;
        m_guard[0] &= 0x7F; m_guard[0] |= i;
        constructSprites();
}

jswByte ManicHGuardian::getY(void)
{
	return ((m_guard[1] & 0xE0) >> 1) | ((m_guard[2] & 1) << 7);
}


void ManicHGuardian::setY(jswByte b)
{
	b /= 8;
	m_guard[1] &= 0x1F; m_guard[1] |= ((b & 7) << 5);
	m_guard[2] &= 0xFE; m_guard[2] |= ((b & 8) >> 3);
	m_guard[3] &= 0xF7; m_guard[3] |= (b & 8);
	m_y = ((m_guard[1] & 0xE0) >> 1) | ((m_guard[2] & 1) << 7);
}

jswByte ManicHGuardian::getX(void)
{
	return (m_guard[1] & 0x1F);
}


void ManicHGuardian::setX(jswByte b)
{
	m_guard[1] &= 0xE0; m_guard[1] |= (b & 0x1F);
	m_x = b;
}

signed char ManicHGuardian::getSpeed(void)
{
	if (m_guard[0] & 0x80) return 1;
	return 2;
}

void ManicHGuardian::setSpeed(signed char c)
{
	if (c < 2) m_guard[0] |= 0x80;
	else	   m_guard[0] &= 0x7F;
}



jswByte ManicVGuardian::getY(void)
{
	return m_guard[2] & 0x7F;
}


void ManicVGuardian::setY(jswByte b)
{
	m_y = m_guard[2] = b & 0x7F;
}

jswByte ManicVGuardian::getX(void)
{
	return (m_guard[3] & 0x1F);
}


void ManicVGuardian::setX(jswByte b)
{
	m_guard[3] &= 0xE0; m_guard[3] |= (b & 0x1F);
	m_x = b;
}


signed char ManicVGuardian::getSpeed(void)
{
	return (signed char)(m_guard[4]);
}

void ManicVGuardian::setSpeed(signed char c)
{
	m_guard[4] = c;
	m_y = m_guard[2] & 0x7F;	
	m_speed = c;
}



void ManicHGuardian::move(void)
{
	m_halfspeed = !m_halfspeed;
	if (m_guard[0] & 0x80)
	{
		if (m_halfspeed) return;
	}

	switch(m_frame)
	{
		case 0: case 1: case 2:
			++m_frame;
			break;

		case 3:	
			if (m_x == (m_guard[6] & 0x1F))	/* Right limit */
			{
				m_frame = 7;
				break;
			}
			m_x++;
			m_frame = 0;
			break;

		case 4: 
			if (m_x == (m_guard[5] & 0x1F))	/* Left limit */
			{
				m_frame = 0;
				break;
			}
			m_x--;
			m_frame = 7;
			break;
		default:
			--m_frame;
			break;
	}
}


void ManicVGuardian::move(void)
{
	signed short y = m_y & 0x7F;
	signed short v = m_speed;
	++m_frame;
	m_frame &= 0xFB;

	/* If outside bounds, reverse. */
	y += v;
	if ((y <  (m_guard[5] & 0x7F)) ||
	    (y >= (m_guard[6] & 0x7F)))
	{
		m_speed = -m_speed;	
	}
	else
	{
		m_y = y & 0x7F;
	}
}


void ManicVGuardian::draw(VideoSurface *s)
{
	int frame = (m_frame & 3) * 32;

	if (m_guard[0] == 0 || m_guard[0] == 0xFF) return;
	getBmp()->toSurface(s, 16 * m_x, 2 * m_y, frame, 0, 32, 32); 
}

void ManicVGuardian::drawThumb(VideoSurface *s, int cx, int cy)
{
	int frame = (m_frame & 3) * 16;

	if (m_guard[0] == 0 || m_guard[0] == 0xFF)
	{
		s->drawSmallText(cx, cy, "N/", ZX_WHITE, ZX_BLACK);
		s->drawSmallText(cx, cy+CHAR_SH, "/A", ZX_WHITE,
                                        ZX_BLACK);
		return;
	}
	getBmpSmall()->toSurface(s, cx, cy, frame, 0, 16, 16); 
}


void ManicHGuardian::draw(VideoSurface *s)
{
	int frame;

	if (m_bidirectional) frame =  (m_frame & 7) * 32;
	else		     frame = ((m_frame & 3) | 4) * 32;

	if (m_guard[0] == 0 || m_guard[0] == 0xFF) return;
	getBmp()->toSurface(s, 16 * m_x, m_y, frame, 0, 32, 32); 
}

void ManicHGuardian::drawThumb(VideoSurface *s, int cx, int cy)
{
	int frame;

	if (m_bidirectional) frame =  (m_frame & 7) * 16;
	else		     frame = ((m_frame & 3) | 4) * 16;

	if (m_guard[0] == 0 || m_guard[0] == 0xFF) 
	{
		s->drawSmallText(cx, cy, "N/", ZX_WHITE, ZX_BLACK);
		s->drawSmallText(cx, cy+CHAR_SH, "/A", ZX_WHITE,
                                        ZX_BLACK);
		return;
	}
	getBmpSmall()->toSurface(s, cx, cy, frame, 0, 16, 16); 
}



void ManicHGuardian::setBounds(jswByte b1, jswByte b2)
{
	m_guard[5] = (b1 & 0x1F) | (m_guard[1] & 0xE0);
	m_guard[6] = (b2 & 0x1F) | (m_guard[1] & 0xE0);	
}




void ManicHGuardian::drawBounds(VideoSurface *s, int ink, int cx, int cy, 
		int cyminor)
{
	int gxl = m_guard[5] & 0x1F;
	int gxr = m_guard[6] & 0x1F;
	int gx  = m_guard[1] & 0x1F;
	int gy  = (m_guard[1] >> 5) | (m_guard[3] & 8);
	int gi  = (m_guard[0] & 7) | ((m_guard[0] & 0x40) >> 3);

	s->box( gxl * 16, gy * 16, (gxr-gxl) * 16 + 32, 32, gi);
	s->box( gx  * 16, gy * 16, 32, 32, gi);
	if (ink >= 0) 
	{
		s->box( cx * 16,  cy * 16, 32, 32, ink);
		s->box( gxl * 16, cy * 16, (gxr-gxl) * 16 + 32, 32, ink);
	}
}



void ManicVGuardian::drawBounds(VideoSurface *s, int ink, int cx, int cy, 
		int cyminor)
{
	int gyt = m_guard[5];
	int gyb = m_guard[6];
	int gy  = m_guard[2];
	int gx  = m_guard[3] & 0x1F;
	int gi  = (m_guard[0] & 7) | ((m_guard[0] & 0x40) >> 3);

	s->box( gx * 16, gyt * 2, 32, (gyb-gyt) * 2 + 32, gi);
	s->box( gx * 16, gy * 2, 32, 32, gi);
	if (ink >= 0) 
	{
		s->box( cx * 16, cy * 16 + cyminor * 2, 32, 32, ink);
		s->box( cx * 16, gyt * 2, 32, (gyb-gyt) * 2 + 32, ink);
	}
}


void ManicVGuardian::setBounds(jswByte b1, jswByte b2)
{
	m_guard[5] = b1 & 0x7F;
	m_guard[6] = b2 & 0x7F;	
}



void ManicGuardian::constructSprites()
{
        int fr, fg, bg;
        jswByte *frame;

	fg = getInk()   + (getBright() ? 8 : 0);
	bg = getPaper() + (getBright() ? 8 : 0);

	if (m_sprites == NULL)	frame = m_room + 768;
	else			frame = m_sprites;

        for (fr = 0; fr < 8; fr++)
        {
                VideoBitmap *spb = bitmapFromSprite(16, 16, frame + 32 * fr,
                                                        1, fg, bg);
                VideoBitmap *spbf= bitmapFromSprite(16, 16, frame + 32 * fr,
                                                        1, bg, fg);
                VideoBitmap *spbs = bitmapFromSprite(16,16, frame + 32 * fr,
                                                        0, fg, bg);
                VideoBitmap *spbsf= bitmapFromSprite(16,16, frame + 32 * fr,
                                                        0, bg, fg);
                spb ->toSurface(m_bmp,           (32 * fr), 0, 0, 0, 32, 32);
                spbf->toSurface(m_bmpFlash,      (32 * fr), 0, 0, 0, 32, 32);
                spbs->toSurface(m_bmpSmall,      (16 * fr), 0, 0, 0, 16, 16);
                spbsf->toSurface(m_bmpSmallFlash,(16 * fr), 0, 0, 0, 16, 16);
                delete spbsf;
                delete spbs;
                delete spbf;
                delete spb;
        }
}



ManicSkylab::ManicSkylab(jswByte *room, jswByte *guard) 
: ManicVGuardian(room, guard)
{
	m_frame = 0;

}


void ManicSkylab::move(void)
{
	signed short y = m_y & 0x7F;
	signed short v = m_speed;

	/* If outside bounds, reverse. */
	y += v;
	if ((m_y <  (m_guard[5] & 0x7F)) ||
	    (m_y >= (m_guard[6] & 0x7F)))
	{
		++m_frame;
		if (m_frame == 8)
		{
			m_frame = 0;
			m_y = m_guard[5];
			m_x = (m_x + 8) & 0x1F;
		}
	}
	else
	{
		m_y = y & 0x7F;
	}
}


void ManicSkylab::draw(VideoSurface *s)
{
	if (m_guard[0] == 0 || m_guard[0] == 0xFF) return;
	getBmp()->toSurface(s, 16 * m_x, 2 * m_y, 32 * m_frame, 0, 32, 32); 
}


void ManicSkylab::drawThumb(VideoSurface *s, int cx, int cy)
{
	if (m_guard[0] == 0 || m_guard[0] == 0xFF)
	{
		s->drawSmallText(cx, cy, "N/", ZX_WHITE, ZX_BLACK);
		s->drawSmallText(cx, cy+CHAR_SH, "/A", ZX_WHITE,
                                        ZX_BLACK);
		return;
	}
	getBmpSmall()->toSurface(s, cx, cy, 16 * m_frame, 0, 16, 16); 
}


void ManicSkylab::drawBounds(VideoSurface *s, int ink, int cx, int cy,
                int cyminor)
{
	int x;
	for (x = 0; x < 32; x += 8)
	{
		int gyt = m_guard[5];
		int gyb = m_guard[6];
		int gy  = m_guard[2];
		int gx  = (m_guard[3] + x) & 0x1F;
		int gi  = (m_guard[0] & 7) | ((m_guard[0] & 0x40) >> 3);
		
		s->box( gx * 16, gyt * 2, 32, (gyb-gyt) * 2 + 32, gi);
		s->box( gx * 16, gy * 2, 32, 32, gi);
		if (x == 0 && ink >= 0)
		{
			s->box( cx * 16, cy * 16, 32, 32, ink);
			s->box( cx * 16, gyt * 2, 32, (gyb-gyt) * 2 + 32, ink);
		}
	}
}




ManicEugene::ManicEugene(jswByte *room, int bg) 
	: ManicVGuardian(room, init(room, bg), room + 736)
{

}


jswByte *ManicEugene::init(jswByte *room, int bg)
{
	m_guard[0] = (bg << 3) | 7;
	m_guard[1] = 0;
	m_guard[2] = 0;
	m_guard[3] = 15;
	m_guard[4] = 1;
	m_guard[5] = 0;
	m_guard[6] = 88; 
	return m_guard;
}

void ManicEugene::draw(VideoSurface *s)
{
	getBmp()->toSurface(s, 16 * m_x, 2 * m_y, 0, 0, 32, 32); 
}


ManicKong::ManicKong(jswByte *room, int bg) 
	: ManicVGuardian(room, init(room, bg))
{

}


jswByte *ManicKong::init(jswByte *room, int bg)
{
	m_guard[0] = (bg << 3) | 4;
	m_guard[1] = 0;
	m_guard[2] = 0;
	m_guard[3] = 15;
	m_guard[4] = 1;
	m_guard[5] = 0;
	m_guard[6] = 0; 
	return m_guard;
}

void ManicKong::draw(VideoSurface *s)
{
	int frame = (m_frame & 2) * 16;
	getBmp()->toSurface(s, 16 * m_x, 2 * m_y, frame, 0, 32, 32); 
}

