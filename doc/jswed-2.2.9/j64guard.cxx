/************************************************************************

    JSWED 2.1.6 - Editor for Jet Set Willy and derivatives

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

/* JSW64 guardian. This is almost the same as the JSW128 guardian, but since
 * the editor handles them a bit differently, drawBounds() and setPos() work
 * differently. */

#include "jswed.hxx"
#include "j64guard.hxx"

Jsw64Guard::Jsw64Guard(jswByte *g, jswByte x, SpectrumMemory *mem, int bg) :
		Jsw128Guard(g, x, mem, bg)
{
}

Jsw64Guard::~Jsw64Guard()
{
}

void Jsw64Guard::drawBounds(VideoSurface *s, int ink, int cx, int cy, int cyminor)
{
	int n, x, y, p, dy, dx, tx, ty, by, bx, gi, ystep, ty4, by4, type;

	gi = m_guard[1] & 0x0F;
	switch(getType())
	{
		case 1:
		case 9:
		s->box( (m_guard[6] & 0x1F) * 16,
			m_guard[3] & 0xFE,
			((m_guard[7] & 0x1F)- (m_guard[6] & 0x1F)) * 16 + 32, 32, gi);
		s->box( m_x * 16, m_guard[3] & 0xFE, 32, 32, gi);
		if (ink >= 0) s->box( cx * 16,  cy * 16, 32, 32, ink);
		break;

		case 2:
		case 7:
		case 10:
		case 15:
		case 0x08:
		case 0x18:
		case 0x28:
		case 0x38:
		case 0x58:
		case 0x68:
		x  = (m_guard[2] & 0x1F) * 16;
		ty = m_guard[6] & 0xFE;
		by = (m_guard[7] & 0xFE) + 32;
		s->box( x, ty, 32, by - ty, gi);
                if (ink >= 0) 
		{
			s->box( cx * 16, ty, 32, by - ty, ink);
			s->box( cx * 16, cy * 16, 32, 32, ink);
		}
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

		case 4: 	// Arrow
		s->box(0, m_guard[2] - 2, 512, 6, gi);
		if (ink >= 0) s->box(0, cy * 16 + (cyminor*2),    512, 6, ink);
		break;

		case 5:
		case 6:
                case 13:
                case 14:	// Diagonals

		ystep = ((signed char)m_guard[4]);
		if ((m_guard[0] & 0x0F) == 6 || (m_guard[0] & 0x0F) == 14)
			ystep = -ystep; 
		// (tx, ty) = current guardian location 
		tx = m_orig[2] & 0x1F;	// Start X position
		ty = m_orig[3];		// Start Y position
		bx = tx;		// Cursor position
		by = cy*16;
		// Move to left X
		dx = tx - (m_guard[6] & 0x1F);	
		ty += (dx * ystep) * 4;
		by += (dx * ystep) * 4;
		tx -= dx;
		bx -= dx;

		tx *= 16;
		bx *= 16;
		ty4 = (m_guard[6] & 0x1F) * 16;
		by4 = (m_guard[7] & 0x1F) * 16 + 15;	
		for (n = ty4; n <= by4; n += 8)
		{
			s->box(tx, ty, 32, 32, gi);
			if (ink >= 0) s->box(bx, by, 32, 32, ink);
			bx+=8;
			tx+=8;
			ty -= 2*ystep;
			by -= 2*ystep;
		}

		case 0x98:	// Switch
		x  = (m_guard[2] & 0x1F) * 16;
		ty =  m_guard[3] & 0xFE;
		s->box( x, ty, 32, 16, gi);
                if (ink >= 0) 
		{
			s->box( cx * 16, cy * 16, 32, 16, ink);
		}
		break;

		case 0xA8:	// Moving wall
		x  = (m_guard[6] & 0x1F) * 16;
		ty =  m_guard[7] & 0xFE;
		by =  ty + 2 * m_guard[3];
		s->box( x, ty, 16, by - ty, ZX_BRORANGE);
                if (ink >= 0) 
		{
			s->box( cx * 16, cy * 16, 16, by - ty, ink);
		}
		break;
	}

}

int Jsw64Guard::setPos(int cx, int cy)
{
	int dy, ystep, rv = 0; 

        switch(getType())
	{
		case 0xA8:	/* Moving wall */
			m_guard[6] = m_x = cx;
			m_guard[7] = 16 * cy;
			rv = m_guard[2];
			break;

		case 0x88:	/* Trigger. Triggers don't have a position */
		case 0xD8:	/* Weak trigger. */
			rv = m_guard[2];
			break;

		case 1:	/* Horizontal & vertical */
		case 9:
		case 2:
		case 7:
		case 10:
		case 15:
		case 0x08:	// Skylab
		case 0x18:	// Eugene(s)
		case 0x28:
		case 0x38:
		case 0x58:
		case 0x68:
		case 0x98:	/* Switch. */
			m_x = cx;
			m_guard[2] = (m_guard[2] & 0xE0) | cx;
			rv = (m_frame & 0xE0) | cx;
			m_guard[3] = cy * 16;
			break;
                case 5:         // NE/SW
		case 6:		// NW/SE
                case 13:        // NE/SW, cycling
		case 14:	// NW/SE, cycling
			ystep = (signed char)m_guard[4];
			if ((m_guard[0] & 0x0F) == 6 || 
			    (m_guard[0] & 0x0F) == 14) ystep = -ystep;
                	dy = cy - (m_orig[3] >> 4);	// Required difference in Y
			if (ystep) 
			{
				dy = (dy * 4) / ystep; 	// Required diff X
	                	rv = ((m_orig[2] & 0x1F) + dy) % 32;
			}
			else 	
			{
				rv = cx;
			}
			m_x = rv;
			m_guard[3] = m_orig[3];
			rv |= (m_frame & 0xE0);
			m_guard[2] = (m_guard[2] & 0xE0) | m_x;
			m_orig[2] = m_guard[2];
			break;
		case 3: /* Rope */
			rv = m_guard[2] = m_x = cx;
			break;
		case 4:	/* Arrow */
			rv = m_guard[2] = (cy * 16) + (cx*2) + 2;
			break;	

	}
	return rv;
}

void Jsw64Guard::setInk(int ink)
{
	if (isArrow())
	{
		ink &= 7;
		m_guard[1] &= 0xF8;
		m_guard[1] |= ink;
		constructSprites();
		return;
	}
	Jsw128Guard::setInk(ink);
}

void Jsw64Guard::constructSprites()
{
	if (isArrow())
	{
		jswByte shape[3];
		
		shape[0] = shape[2] = m_guard[6];
		shape[1] = m_guard[3];
		VideoBitmap *spb = bitmapFromSprite(8, 3, shape, 1, 
				m_guard[1] & 7, ZX_TRANS);
		spb->toSurface(m_bmp, 0, 0);
		delete spb;
		spb = bitmapFromSprite(8, 3, shape, 1, m_guard[1] & 7, m_bgcol);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		m_bmp->makeTransparent();
		return;
	}
	Jsw128Guard::constructSprites();
}
