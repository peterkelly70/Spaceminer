
/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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

/* Geoff-mode guardian. These are also a superset of the JSW48 ones. */

#include "jswed.hxx"
#include "geoffguard.hxx"

GeoffGuard::GeoffGuard(jswByte *g, jswByte x, SpectrumMemory *mem, int bg) :
		JswGuardian(g, x, mem, bg)
{
	// The start byte is normally copied to byte 2 of the guardian data at
	// #8100-#8147, except for arrows; the top 4 bits of the start byte for
	// an arrow specify the screen row (0-15) the arrow travels along, and
	// the bottom 4 bits are multiplied by 8 and added to the arrow's timer
	// value (byte 4). This is done at #8BC2.
	constructSprites();
	if (isArrow())
	{
		m_guard[2] = (x & 0xF0) + 4; 
		m_guard[4] += 8 * (x & 0x0F);
	}
}

GeoffGuard::~GeoffGuard()
{
}


void GeoffGuard::constructSprites()
{
	int fr, fg;
	jswByte *frame;

	if (isArrow())
	{
		jswByte shape[3];

		shape[0] = shape[2] = m_guard[6];
		shape[1] = 0x7E;			// Arrow shape
		VideoBitmap *spb = bitmapFromSprite(8, 3, shape, 1, ZX_WHITE, 
						ZX_TRANS);
		spb->toSurface(m_bmp, 0, 0);
		m_bmp->makeTransparent();
		delete spb;
		spb = bitmapFromSprite(8, 3, shape, 1, ZX_WHITE, m_bgcol);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;		
	}

        fg = m_guard[1] & 0x0F;

        frame = m_mem->memoryAt(m_guard[5] * 256);

	for (fr = 0; fr < 8; fr++)
	{
		VideoBitmap *spb = bitmapFromSprite(16, 16, frame + 32 * fr,
							1, fg, ZX_TRANS);
		VideoBitmap *spbs = bitmapFromSprite(16,16, frame + 32 * fr,
							0, fg, m_bgcol);
		spb ->toSurface(m_bmp,      (32 * fr), 0, 0, 0, 32, 32);
		spbs->toSurface(m_bmpSmall, (16 * fr), 0, 0, 0, 16, 16);
		delete spbs;
		delete spb;
	}
	m_bmp->makeTransparent();
}




void GeoffGuard::drawFrames(VideoSurface *s, int x, int y)
{
	int n, frame;

	switch(m_guard[0] & 0x03)
        {
		default:
			s->fillBox(x, y, 480, 32, ZX_WHITE);
			break;
		case 1:
		case 2:	
		for (n = 0; n < 8; n++)
		{
			frame = ((m_guard[1] & (32 * n)) | m_guard[2]) & 0xE0;

			VideoBitmap *b2 = newVideoBitmap(32,32);
			b2->getSurface()->fillBox(0, 0, 32, 32, m_bgcol);
			m_bmp->toSurface(b2->getSurface(),0,0,frame, 0, 32, 32);
			b2->toSurface(s, x + 64 * n, y, 0, 0, 32, 32);
			delete b2;
		}
		break;
	}
}


void GeoffGuard::draw(VideoSurface *s)
{
        int frame = ((m_guard[1] & m_guard[0]) | m_guard[2]) & 0xE0;
        int x     = (m_guard[2] & 0x1F) * 16;
        int y     = (m_guard[3] & 0xFE);

        if (m_guard[0] == 0xFF) return;

        switch(m_guard[0] & 0x03)
        {
		case 1:
		case 2:
                        m_bmp->toSurface(s, x, y, frame, 0, 32, 32);
                        break;
		case 3:	drawRope(s); break;

		case 0:		// Arrow
			if (m_guard[0] & 0x80)
			{
				++m_guard[4];	// Moving right
				// Make firing sound at 0F4h
			}
			else
			{
				--m_guard[4];	// Moving left
				// Make firing sound at 02Ch	
			}
			if (!(m_guard[4] & 0xE0))
			{
				x = m_guard[4] * 16;
				y = m_guard[2] - 2;
				m_bmp->toSurface(s, x, y, 0, 0, 16, 6);
			}
			break;
		}
	
}


void GeoffGuard::cycle(void)
{
        int b, c;
        b = m_guard[1] & 0xF8;
        c = m_guard[1] & 7;

        c++; if (c == 7) c = 2;
        m_guard[1] = b | c;

	constructSprites();	
}


jswByte GeoffGuard::dstep(jswByte x)
{
	switch(m_guard[0] & 0x18)
	{
		case 8:    return x * 2;
		case 0x18: return x * 4;
		case 0x10: return x & 0xFE;
		default:   return 0;
	}
}

void GeoffGuard::hshift(void)
{
	// Bits 0-1 of c = sprite phase
	// Bits 7-2 of c = X
	int flg   = m_guard[0];
	int speed = m_guard[4];
	int x = ((m_guard[2] & 0x1F) << 2) + 
		((flg        & 0x60) >> 5);
	int dy;
	
	int newx = x + speed;	// New, after-move, value of x
	if ((flg & 0x18) == 0x10)
	{
		dy = (m_guard[6] & 1) * 2;
	}
	else dy = dstep(1);
	// Diagonal up/down moves
	if (m_guard[1] & 0x10) m_guard[3] -= dy * speed;
	else		       m_guard[3] += dy * speed;

	--m_guard[6];
	if (!m_guard[6]) // New direction
	{
		m_guard[6] = m_guard[7];
		if (flg & 4)	// Wraparound	
		{
			newx -= m_guard[7] * speed;
			dy = dstep(m_guard[7]) * speed;

			if (m_guard[1] & 0x10) m_guard[3] += dy;
			else		       m_guard[3] -= dy;
		}
		else	// Standard left/right type
		{
			m_guard[4] = 0x100 - speed;
			m_guard[0] ^= 0x80;
		}
	}
//
// Store the new values 
//
	m_guard[0] &= ~0x60;
	m_guard[0] |= (newx << 5) & 0x60;

	m_guard[2] &= ~0x1F;
	m_guard[2] |= (newx >> 2) & 0x1F;
}

void GeoffGuard::vshift()
{
	int speed = m_guard[4];

	m_guard[3] += speed;

	if (speed & 0x80) m_guard[0] -= 0x20;
	else		  m_guard[0] += 0x20;

	--m_guard[6];
	if (m_guard[6]) return;

	m_guard[6] = m_guard[7];
	if (m_guard[0] & 4)
	{
		// Wraparound
		m_guard[3] -= m_guard[7] * speed;
	}
	else
	{
		m_guard[4] = 0x100 - speed;	// Reverse
	}
}


int GeoffGuard::setPos(int cx, int cy)
{
	int rv = 0; 

        switch(m_guard[0] & 3)
	{
		case 1:	/* Horizontal & diagonal */
                        m_x = cx;
                        m_orig[2] = m_guard[2] = (m_guard[2] & 0xE0) | cx;
                        rv = (m_frame & 0xE0) | cx;
			// Direction
                       	m_guard[0] &= 0x7F; m_guard[0] |= (m_orig[0] & 0x7f);
			m_guard[6] = m_orig[6]; // Step
			m_guard[3] = m_orig[3]; // Y
			m_guard[4] = m_orig[4]; // Speed
			break;

		case 2: /* Vertical */
			m_x = cx;
			m_orig[2] = (m_orig[2] & 0xE0) | cx;
			m_guard[2] = (m_guard[2] & 0xE0) | cx;
			rv = (m_frame & 0xE0) | cx;
			break;
		case 3: /* Rope */
			rv = m_guard[2] = m_x = cx;
			break;
		case 0:	/* Arrow */
			m_guard[4] = m_orig[4] + ((cx / 2) & 0x0F);
			m_guard[2] = (cy * 16) + 4;
			rv = (cy * 16) + ((cx / 2) & 0x0F);	
			break;	

	}
	return rv;
}

void GeoffGuard::drawBounds(VideoSurface *s, int ink, int cx, int cy, int cyminor)
{
	int flg, speed;
	jswByte xb;
        int x, y, p, dy, dx, tx, ty, by, bx, gi;
	int rl, rr;

        gi = m_guard[1] & 0x0F;		// Colour
	rl = m_guard[7] - m_guard[6];	// Number of frames traversed
	rr = m_guard[6];		// Number of frames to traverse

	switch(m_guard[0] & 3)
	{
		case 1:	// Horizontal or diagonal
	        flg   = m_guard[0];
		speed = m_guard[4];
		x = ((m_guard[2] & 0x1F) << 2) + ((flg & 0x60) >> 5);
		y = m_guard[3] & 0xFE;
		p = rr;

		if ((m_orig[0] & 0x80) != (m_guard[0] & 0x80))
		{
			rr = rl;
			rl = p;
			p  = rr;
			speed = 0x100 - speed;
		}

		bx = x + (rr * speed);
		tx = x - (rl * speed);
	
		ty = y;
		by = y + 32;	
		for (xb = p; xb < m_guard[7]; xb++)
		{
		        if ((flg & 0x18) == 0x10) dy = (xb & 1) * 2;
       		 	else dy = dstep(1);
        		if (m_guard[1] & 0x10) ty += dy * speed;
			else                   ty -= dy * speed;
		}
		for (xb = p; xb > 0; xb--)
		{
                        if ((flg & 0x18) == 0x10) dy = (xb & 1) * 2;
                        else dy = dstep(1);
                        if (m_guard[1] & 0x10) by -= dy * speed;
                        else                   by += dy * speed;
		}
                dx = (m_orig[2] & 0x1F) * 16;

		for (xb = tx, y = ty, x = m_guard[7]; x > 0; x--)
		{
			if ((m_guard[0] & 0x18) == 0x10) dy = (x & 1) * 2; 
			else dy = dstep(1);

			s->box(4 * xb, y & 0xFE, 32, 32, gi);
	
			if (ink >= 0) s->box(4 * xb - dx + 16 * cx,
					     (y & 0xFE), 32, 32, ink); 

			if (m_guard[1] & 0x10) y -= (dy * speed);
			else		       y += (dy * speed);
			xb += speed;

	
		}
		break;

		case 2: // Vertical
		speed = m_orig[4];

		rl = m_orig[7] - m_orig[6];
		rr = m_orig[6];

		ty = (m_orig[3] - rl * speed) & 0xFE;
		by = ((m_orig[3] + rr * speed) + 32) & 0xFE;
		 x = (m_orig[2] & 0x1F) * 16;

		s->box(x, ty, 32, by - ty, gi);
		if (ink >= 0) s->box(cx * 16, ty, 32, by - ty, ink);
		break;
	

		case 3:		// Rope
		y = 0;
		ty = 0;
		tx = m_guard[7];
		bx = 0;
		x  = 8 * m_guard[2]; // Initial X

	        do
       		{
			s->fillBox( (x+bx)    * 2, ty, 2, 2, ZX_WHITE);
			s->fillBox( (x-bx)    * 2, ty, 2, 2, ZX_WHITE);
			s->fillBox(  x        * 2, y*6, 2, 2, ZX_WHITE);
			if (ink >= 0)
			{
				s->fillBox( (cx*8+bx) * 2, ty, 2, 2, ink);
				s->fillBox( (cx*8-bx) * 2, ty, 2, 2, ink);
				s->fillBox( (cx * 8)  * 2, y*6, 2, 2, ink);
			}
	                p = (y + tx) & 0x7F;
			dy = m_mem->peek(0x8380 + p);
			dx = m_mem->peek(0x8300 + p);

			ty += dy;
			bx += dx;
		}
        	while ((y++) < m_guard[4]);
		break;

		case 0: 	// Arrow
		s->box(0, m_guard[2] - 2, 512, 6, gi);
		if (ink >= 0)
		{
			s->box(0,     cy * 16 + 4,    512, 6, ink);
			s->box(16*cx, cy * 16 + 4,     16, 6, ink);
		}
		break;

	}

}


void GeoffGuard::move()
{
        switch(m_guard[0] & 3)
        {
		case 1:	hshift(); break;
		case 2: vshift(); break;
		case 3: moveRope(); break;
		break;
        }
}


void GeoffGuard::drawThumb(VideoSurface *s, int cx, int cy)
{
        int frame = ((m_guard[1] & m_guard[0]) | m_guard[2]) & 0xE0;
	frame /= 2;
        if (m_guard[0] == 0xFF)
	{
                        s->drawSmallText(cx, cy, "En", ZX_WHITE, ZX_BLACK);
                        s->drawSmallText(cx, cy+CHAR_SH, "d ", ZX_WHITE, 
                                        ZX_BLACK);
	}

	if ((m_guard[0] & 0x1F) == 4) m_bmpSmall->toSurface(s, cx, cy, 0, 0, 16, 16);
	else if ((m_guard[0] & 0x1F) == 3) s->drawText(cx, cy, "R", ZX_WHITE, ZX_BLACK);
        else switch(m_guard[0] & 3)
        {
                case 1:         // H
                case 2:         // V
                        m_bmpSmall->toSurface(s, cx, cy, frame, 0, 16, 16);
                        break;
		default:
			s->drawSmallText(cx, cy, "N ", ZX_WHITE, ZX_BLACK);
			s->drawSmallText(cx, cy+CHAR_SH, " A", ZX_WHITE, 
					ZX_BLACK);
	}
}

jswByte GeoffGuard::bumpFrame(void)
{
        switch(m_guard[0] & 3)
        {
                case 0:
		case 3: break;  // Nothing
                case 1:         // H
                case 2:         // V

			m_frame = (m_frame + 0x20) & 0xE0;

			m_guard[2] = (m_guard[2] & 0x1F) | m_frame;
			constructSprites();
			break;
	}
	return m_frame;
}


void GeoffGuard::setFrame(jswByte b)
{
        switch(m_guard[0] & 3)
        {
                case 0:
		case 3: break;  // Nothing
                case 1:         // H
                case 2:         // V

			m_frame = (b << 5) & 0xE0;

			m_guard[2] = (m_guard[2] & 0x1F) | m_frame;
			constructSprites();
			break;
	}
}




void GeoffGuard::setY(jswByte y)
{
	m_guard[3] = m_orig[3] = y & 0xFE;
        m_guard[2] = m_orig[2]; // X
        m_guard[6] = m_orig[6]; // Step
        m_guard[4] = m_orig[4]; // Speed
        m_guard[0] &= 0x7F; m_guard[0] |= (m_orig[0] & 0x7f);
}


void    GeoffGuard::setStep(jswByte step)  
{
	m_guard[4] = m_orig[4] = step; 
        m_guard[2] = m_orig[2]; // X
        m_guard[6] = m_orig[6]; // Tick
        m_guard[3] = m_orig[3]; // Y
        m_guard[0] &= 0x7F; m_guard[0] |= (m_orig[0] & 0x7f);
}

jswByte GeoffGuard::getDirection(void)      { return m_guard[1] & 0x10; }
void    GeoffGuard::setDirection(jswByte b) 
{
	m_guard[1] &= ~0x10;
	if (b) m_guard[1] |= 0x10;
}

jswByte GeoffGuard::getWrap(void)      { return m_guard[0] & 4; }
void    GeoffGuard::setWrap(jswByte w) 
{
	m_guard[0] &= ~4; 
	m_orig [0] &= ~4; 
	if (w)
	{
		m_guard[0] |= 4;
		m_orig[0]  |= 4;
	}
} 

jswByte GeoffGuard::getDiagonal(void) { return (m_guard[0] & 0x18) >> 3; }
void GeoffGuard::setDiagonal(jswByte d)
{
	d &= 3;
	d <<= 3;
	m_guard[0] &= ~0x18;
	m_guard[0] |= d;
	m_orig[0] &= ~0x18;
	m_orig[0] |= d;
}


jswByte GeoffGuard::getTick(void)      { return m_orig[6]; }
void    GeoffGuard::setTick(jswByte t) 
{ 
	m_guard[6] = m_orig[6] = t; 
	m_guard[2] = m_orig[2];	// X
        m_guard[3] = m_orig[3]; // Y
        m_guard[4] = m_orig[4]; // Speed
        m_guard[0] &= 0x7F; m_guard[0] |= (m_orig[0] & 0x7f);
}

jswByte GeoffGuard::getTickLimit(void) { return m_orig[7]; }
void    GeoffGuard::setTickLimit(jswByte t)
{
	m_guard[7] = m_orig[7] = t;
	m_guard[6] = m_orig[6]; // Step
        m_guard[2] = m_orig[2]; // X
	m_guard[3] = m_orig[3]; // Y
	m_guard[4] = m_orig[4]; // Speed
        m_guard[0] &= 0x7F; m_guard[0] |= (m_orig[0] & 0x7f);
}



bool GeoffGuard::isArrow(void)
{
	if (m_guard[0] && (m_guard[0] & 3) == 0) return true;
	return false; 
}


