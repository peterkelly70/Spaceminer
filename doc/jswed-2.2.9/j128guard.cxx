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

/* JSW128 guardian. This actually contains nearly all of the JSW128 
 * guardians as well; all the bits which are a strict superset of the 
 * JSW128 guardian definition */

#include "jswed.hxx"
#include "j128guard.hxx"

#include "stop.xbm"
#include "trigger.xbm"
#include "opener.xbm"

Jsw128Guard::Jsw128Guard(jswByte *g, jswByte x, SpectrumMemory *mem, int bg) :
		JswGuardian(g, x, mem, bg)
{
	constructSprites();
	if (getType() == 0xA8)
	{
		m_frame = 0;
	}
}

Jsw128Guard::~Jsw128Guard()
{
}


void Jsw128Guard::constructSprites()
{
	int fr, fg, type;
	jswByte *frame;
	VideoBitmap *spb, *spbs;

	if (isArrow())
	{
		jswByte shape[3];

		shape[0] = shape[2] = m_guard[6];
		shape[1] = 0xFF;			// Arrow shape
		spb = bitmapFromSprite(8, 3, shape, 1, ZX_WHITE, ZX_TRANS);
		spb->toSurface(m_bmp, 0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		m_bmp->makeTransparent();
		delete spb;
		return;		
	}
	type = getType();
        fg = m_guard[1] & 0x0F;
        frame = m_mem->memoryAt(m_guard[5] * 256);

	switch(type)
	{
		case 0x88:	// Trigger
		spb = bitmapFromXbm(trigger_width, trigger_height, 
				trigger_bits, 0, ZX_BRORANGE, ZX_BLACK);
		spb->toSurface(m_bmp,      0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;

		case 0xD8:	// Weak Trigger (patched MM only)
		spb = bitmapFromXbm(trigger_width, trigger_height, 
				trigger_bits, 0, ZX_ORANGE, ZX_BLACK);
		spb->toSurface(m_bmp,      0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;

		case 0x98:	// Switch
		frame += (m_guard[2] & 0xE0);
		spb = bitmapFromSprite(16, 8, frame, 1, fg, ZX_TRANS);
		spb->toSurface(m_bmp,      0, 0);
		delete spb;
		spb = bitmapFromSprite(16, 8, frame, 1, fg, m_bgcol);
		spb->toSurface(m_bmpSmall, 0, 0);
		m_bmp->makeTransparent();
		delete spb;
		return;

		case 0xB8:	// Stopper
		spb = bitmapFromXbm(stop_width, stop_height, stop_bits, 
				0, ZX_BRRED, ZX_BLACK);
		spb->toSurface(m_bmp,      0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;

		case 0xC8:	// Stopper
		spb = bitmapFromXbm(stop_width, stop_height, stop_bits, 
				0, ZX_RED, ZX_BLACK);
		spb->toSurface(m_bmp,      0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;

		case 0xA8:	// Opener
		spb = bitmapFromXbm(opener_width, opener_height, opener_bits, 
				0, ZX_BRORANGE, ZX_BLACK);
		spb->toSurface(m_bmp,      0, 0);
		spb->toSurface(m_bmpSmall, 0, 0);
		delete spb;
		return;

	}

	for (fr = 0; fr < 8; fr++)
	{
		spb = bitmapFromSprite(16, 16,frame+32*fr, 1, fg, ZX_TRANS);
		spbs = bitmapFromSprite(16,16,frame+32*fr, 0, fg, m_bgcol);
		spb ->toSurface(m_bmp,      (32 * fr), 0, 0, 0, 32, 32);
		spbs->toSurface(m_bmpSmall, (16 * fr), 0, 0, 0, 16, 16);
		delete spbs;
		delete spb;
	}
	m_bmp->makeTransparent();
}




void Jsw128Guard::drawFrames(VideoSurface *s, int x, int y)
{
	int n, frame, type;

	type = getType();

        switch(type)
        {
		default:
			s->fillBox(x, y, 480, 32, ZX_WHITE);
			break;
                case 1:         // H
                case 2:         // V
                case 5:         // NW/SE
                case 6:         // NE/SW
                case 7:         // Vertical, cycling
                case 9:         // H, cycling
                case 10:        // V
                case 13:        // NW/SE, cycling
                case 14:        // NE/SW, cycling
                case 15:        // V, cycling
		case 0x18:	// Eugene
		case 0x28:	// Eugene
		case 0x38:	// Eugene
		case 0x58:	// Lift
		case 0x68:	// Scenery
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
		case 0x08:	// Skylab
		for (n = 0; n < 8; n++)
		{
			frame = 32 * n;

			VideoBitmap *b2 = newVideoBitmap(32,32);
			b2->getSurface()->fillBox(0, 0, 32, 32, m_bgcol);
			m_bmp->toSurface(b2->getSurface(),0,0,frame, 0, 32, 32);
			b2->toSurface(s, x + 64 * n, y, 0, 0, 32, 32);
			delete b2;
		}
		break;
	}
}


void Jsw128Guard::draw(VideoSurface *s)
{
        int frame = ((m_guard[1] & m_guard[0]) | m_guard[2]) & 0xE0;
        int x     = (m_guard[2] & 0x1F) * 16;
        int y     = (m_guard[3] & 0xFE);
	int type;

        if (m_guard[0] == 0xFF) return;

	type = getType();
        switch(type)
        {
                case 0: break;  // Nothing
                case 1:         // H
                case 2:         // V
                case 5:         // NW/SE
                case 6:         // NE/SW
                case 7:         // Vertical, cycling
                case 9:         // H, cycling
                case 10:        // V
                case 13:        // NW/SE, cycling
                case 14:        // NE/SW, cycling
                case 15:        // V, cycling
		case 0x08:
		case 0x18:
		case 0x28:
		case 0x38:
		case 0x58:	// Lift
		case 0x68:	// Scenery
                        m_bmp->toSurface(s, x, y, frame, 0, 32, 32);
                        break;
		case 0x98:	// Switch
                        m_bmp->toSurface(s, x, y, 0, 0, 32, 16);
                        break;
		case 0xA8:	// Opening wall
			drawOpeningWall(s);
			break;
		case 3:		// Rope
			drawRope(s);
			break;

		case 4:		// Arrow
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

void Jsw128Guard::eucycle(void)
{
        int b, c;
        b = m_guard[1] & 0xF8;
        c = m_guard[1] & 7;

        c++; c &= 7;
        m_guard[1] = b | c;

	constructSprites();	
}


void Jsw128Guard::cycle(void)
{
        int b, c;
        b = m_guard[1] & 0xF8;
        c = m_guard[1] & 7;

        c++; if (c == 7) c = 2;
        m_guard[1] = b | c;

	constructSprites();	
}


void Jsw128Guard::hshift(void)
{
	if (m_guard[7] & 0x20)
	{
		m_guard[7] ^= 0x40;
		if (m_guard[7] & 0x40) return;
	}

	if (m_guard[0] & 0x80)    // going right
	{
		m_guard[0] = (m_guard[0] + 0x20) | 0x80;
		if (m_guard[0] < 0xA0)
		{
			if ((m_guard[2] & 0x1F) != (m_guard[7] & 0x1F)) ++m_guard[2];
			else m_guard[0] = (m_guard[0] & 0x0F) | 0x60;
		}
	}
	else	// Going left
	{
		m_guard[0] = (m_guard[0] - 0x20) & 0x7F;
		if (m_guard[0] >= 0x60)
		{
			if ((m_guard[2] & 0x1F) != (m_guard[6] & 0x1F)) --m_guard[2];
			else m_guard[0] = (m_guard[0] & 0x0F) | 0x80;
		}
	}	
}

void Jsw128Guard::vshift()
{
	m_guard[0] ^= 8;
	if (m_guard[0] & 0x18) m_guard[0] += 0x20;

	m_guard[3] += (signed char)(m_guard[4]);
	if (m_guard[3] >= m_guard[7])	// Reverse at bottom
	{
		m_guard[4] = -((signed char)m_guard[4]);
	}
	else if (m_guard[3] <= m_guard[6]) 
	{
		m_guard[3] = m_guard[6];
                m_guard[4] = -((signed char)m_guard[4]);

	}
}


int Jsw128Guard::setPos(int cx, int cy)
{
	int dy, ystep, rv = 0; 
	int type;

        switch(type = getType())
	{
		case 1:	/* Horizontal & vertical */
		case 9:
		case 2:
		case 7:
		case 10:
		case 15:
		case 0x08:
		case 0x18:
		case 0x28:
		case 0x38:
		case 0x98:
			m_x = cx;
			m_guard[2] = (m_guard[2] & 0xE0) | cx;
			rv = (m_frame & 0xE0) | cx;
			break;
		case 0xA8:
			m_x = cx;
			m_guard[6] = cx & 0x1F;
			m_guard[7] = cy;
			rv = cx;
			break;
                case 5:         // NE/SW
		case 6:		// NW/SE
                case 13:        // NE/SW, cycling
		case 14:	// NW/SE, cycling
			ystep = (signed char)m_guard[4];
			if (type == 6 || type == 14) ystep = -ystep;
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

void Jsw128Guard::drawBounds(VideoSurface *s, int ink, int cx, int cy, int cyminor)
{
	int n, x, y, p, dy, dx, tx, ty, by, bx, gi, ystep, ty4, by4,type;

	gi = m_guard[1] & 0x0F;
	type = getType();
	switch(type)
	{
		case 1:
		case 9:
		s->box( (m_guard[6] & 0x1F) * 16,
			m_guard[3] & 0xFE,
			((m_guard[7] & 0x1F)- (m_guard[6] & 0x1F)) * 16 + 32, 32, gi);
		s->box( m_x * 16, m_guard[3] & 0xFE, 32, 32, gi);
		if (ink >= 0) s->box( cx * 16,  m_guard[3] & 0xFE, 32, 32, ink);
		break;

		case 2:
		case 10:
		case 7:
		case 15:
		case 0x18:
		case 0x28:
		case 0x38:
		x  = (m_guard[2] & 0x1F) * 16;
		ty = m_guard[6] & 0xFE;
		by = (m_guard[7] & 0xFE) + 32;
		s->box( x, ty, 32, by - ty, gi);
                if (ink >= 0) s->box( cx * 16, ty, 32, by - ty, ink);
		break;

		case 0x98:
		x  = (m_guard[2] & 0x1F) * 16;
		y  = m_guard[3] & 0xFE;
		s->box( x, y, 32, 32, gi);
                if (ink >= 0) s->box( cx * 16, ty, 32, 32, ink);
		break;
		
		case 0xA8:
		x   = m_guard[6] & 0x1F;
		ty  = m_guard[7] & 0xFE;
		by  = ty + 2 * m_guard[3];
		s->box( x, ty, 16, by - ty, ZX_BLACK);
                if (ink >= 0) s->box( cx * 16, cy, 32, by - ty, ink);

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
		if (type == 6 || type == 14) ystep = -ystep; 
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
	}

}


void Jsw128Guard::move()
{

        switch(getType())
        {
                case 0: break;  // Nothing
		case 9:	cycle();
                case 1: if (m_guard[7] & 0x80) hshift();
			hshift();
			break;

		case 14: cycle();
		case 6:	 if (m_guard[7] & 0x80) hshift();
			 hshift();
			 if (m_guard[0] & 0x80) m_guard[3] += m_guard[4];
			 else		        m_guard[3] -= m_guard[4];
			 break;

                case 13: cycle();
                case 5:  if (m_guard[7] & 0x80) hshift();
			 hshift();
                         if (m_guard[0] & 0x80) m_guard[3] -= m_guard[4]; 
                         else                   m_guard[3] += m_guard[4];
			 break;
		case 7:
		case 15:
			cycle();
                case 2:	
		case 10: 
			vshift();
                        break;
	
		case 3: // Rope
			if (!(m_guard[0] & 0x40)) moveRope();
			break;

		case 0x08:	// Skylab
			m_guard[3] += (signed char)(m_guard[4]);
			if (((signed char)m_guard[4]) < 0)
			{
				if (m_guard[3] < m_guard[7]) 
					m_guard[3] = m_guard[7];
			}
			else
			{
				if (m_guard[3] > m_guard[7]) 
					m_guard[3] = m_guard[7];
			}
			if (m_guard[3] == m_guard[7])
			{
				if ((m_guard[2] & 0xE0) == 0xE0)
				{
					m_guard[2] = (m_guard[2]+8) & 0x1F;
					m_guard[3] = m_guard[6];
				}
				else	
				{
					m_guard[2] += 0x20;
				//	constructSprites();
				}
			}
			break;
		case 0x28:
			cycle();
		case 0x18:
			if (m_guard[3] == m_guard[7] || 
			    m_guard[3] == m_guard[8]) break;
			m_guard[3] += (signed char)(m_guard[4]);
        		break;
		case 0x38:
			eucycle();
			if (m_guard[3] == m_guard[7] || 
			    m_guard[3] == m_guard[8]) break;
			m_guard[3] += (signed char)(m_guard[4]);
        		break;
		case 0x58:	// Lift
			m_guard[3] += (signed char)(m_guard[4]);
			if (m_guard[3] >= m_guard[7])	// Reverse at bottom
			{
				m_guard[4] = -((signed char)m_guard[4]);
			}
			else if (m_guard[3] <= m_guard[6]) 
			{
				m_guard[3] = m_guard[6];
                		m_guard[4] = -((signed char)m_guard[4]);
			}
			break;
		case 0xA8:	// Opening wall
			if (m_guard[3])
				m_frame = (m_frame + 1) % (m_guard[3]);
			else	m_frame = 0;
			break;
	}
}


void Jsw128Guard::drawThumb(VideoSurface *s, int cx, int cy)
{
        int frame = ((m_guard[1] & m_guard[0]) | m_guard[2]) & 0xE0;
	frame /= 2;
        if (m_guard[0] == 0xFF)
	{
                        s->drawSmallText(cx, cy, "En", ZX_WHITE, ZX_BLACK);
                        s->drawSmallText(cx, cy+CHAR_SH, "d ", ZX_WHITE, 
                                        ZX_BLACK);
	}

	int type = getType();

        switch(type)
        {
                case 0: break;  // Nothing
                case 1:         // H
                case 2:         // V
                case 5:         // NW/SE
                case 6:         // NE/SW
                case 7:         // Vertical, cycling
                case 9:         // H, cycling
                case 10:        // V
                case 13:        // NW/SE, cycling
                case 14:        // NE/SW, cycling
                case 15:        // V, cycling
		case 0x08:	// Skylab
		case 0x18:	// Eugene
		case 0x28:	// Eugene
		case 0x38:	// Eugene
		case 0x58:	// (Lift)
		case 0x68:	// (Scenery)
                        m_bmpSmall->toSurface(s, cx, cy, frame, 0, 16, 16);
                        break;
		case 0x88:	// Trigger
		case 0x98:	// Switch
		case 0xA8:	// Opening wall
		case 0xB8:	// Stopper
		case 0xC8:	// Stopper
		case 0xD8:	// Weak trigger
		case 4:		// Arrow
			m_bmpSmall->toSurface(s, cx, cy, 0, 0, 16, 16);
			break;
                case 3:         // Rope
                       	s->drawText(cx, cy, "R", ZX_WHITE, ZX_BLACK);	 
                        break;
		default:
			s->drawSmallText(cx, cy, "N ", ZX_WHITE, ZX_BLACK);
			s->drawSmallText(cx, cy+CHAR_SH, " A", ZX_WHITE, 
					ZX_BLACK);
	}
}

void Jsw128Guard::setFrame(jswByte b)
{
        switch(getType())
        {
                case 0: break;  // Nothing
                case 1:         // H
                case 2:         // V
                case 5:         // NW/SE
                case 6:         // NE/SW
                case 7:         // Vertical, cycling
                case 9:         // H, cycling
                case 10:        // V
                case 13:        // NW/SE, cycling
                case 14:        // NE/SW, cycling
                case 15:        // V, cycling
		case 0x08:	// Skylab
		case 0x18:	// Angry Eugene
		case 0x28:	// Angry Eugene, cycling
		case 0x38:	// Angry Eugene, cycling
		case 0x58:	// (Lift)
		case 0x68:	// (Scenery)
		case 0x98:	// Switch
			m_frame = (b << 5) & 0xE0;

			m_guard[2] = (m_guard[2] & 0x1F) | m_frame;
			constructSprites();
			break;
	}
}


//
// When setting the vertical step, reset X and Y
//
void    Jsw128Guard::setStep(jswByte step)  
{
	switch(getType())
	{
		case 0xA8:
			m_guard[3] = step;
			m_guard[4] = step;
			m_guard[5] = 0;
			break;

		default:
			m_guard[4] = step & 0xFE;
			m_guard[2] &= 0xE0;
			m_guard[2] |= (m_orig[2] & 0x1F);
			m_guard[3] = m_orig[3]; 
			break;
	}
}




jswByte Jsw128Guard::bumpFrame(void)
{
        switch(getType())
        {
                case 0: break;  // Nothing
                case 1:         // H
                case 2:         // V
                case 5:         // NW/SE
                case 6:         // NE/SW
                case 7:         // Vertical, cycling
                case 9:         // H, cycling
                case 10:        // V
                case 13:        // NW/SE, cycling
                case 14:        // NE/SW, cycling
                case 15:        // V, cycling

			m_frame = (m_frame + 0x20) & 0xE0;

			m_guard[2] = (m_guard[2] & 0x1F) | m_frame;
			constructSprites();
			break;
	}
	return m_frame;
}




jswByte Jsw128Guard::getFastAnim(void)      
{ 
	switch(getType())
	{
		case 2: case 10: case 7: case 15:
			return m_guard[0] & 0x10; 
		default:
			return m_guard[7] & 0x80;
	}
}


void    Jsw128Guard::setFastAnim(jswByte b) 
{
	switch(getType())
	{
		case 2: case 10: case 7: case 15:
			m_guard[0] &= ~0x10;
			if (b) m_guard[0] |= 0x10;
			break;
		default:
			m_guard[7] &= ~0x80;
			if (b) m_guard[7] |= 0x80;
			break;
	}
}

void Jsw128Guard::getBounds(jswByte *b1, jswByte *b2)
{
	switch(getType())
	{
	case 1:  case 5: case 6:
	case 9: case 13: case 14:
		*b1 = m_guard[6] & 0x1F;
		*b2 = m_guard[7] & 0x1F;
		break;
	default:
		*b1 = m_guard[6];
		*b2 = m_guard[7];
		break;
	}
}


void Jsw128Guard::setBounds(jswByte b1, jswByte b2)
{
	jswByte x;
	int type = getType();

	switch(type)
	{
		case 0x02: case 0x0A: case 0x07: case 0x0F:
		case 0x08: case 0x18: case 0x28: case 0x38:
		case 0x58: case 0x68:
		b1 &= 0xFE;
		b2 &= 0xFE;
		m_guard[6] = b1;
		m_guard[7] = b2;
		if (m_guard[3] <= m_guard[6]) m_guard[3] = m_guard[6];
		if (m_guard[3] >= m_guard[7]) m_guard[3] = m_guard[7];
		break;

		case 1: case 5: case 6: case 9: case 13: case 14:
		x = m_guard[2] & 0x1F;
		b1 &= 0x1F;
		b2 &= 0x1F;
		m_guard[6] = (m_guard[6] & 0xE0) | b1;
		m_guard[7] = (m_guard[7] & 0xE0) | b2;

		if (x <= (m_guard[6] & 0x1F)) x = m_guard[6] & 0x1F;
		if (x >= (m_guard[7] & 0x1F)) x = m_guard[7] & 0x1F;
		m_guard[2] = (m_guard[2] & 0xE0) | x;
		break;
	}
}


bool Jsw128Guard::isArrow(void)
{
	if (getType() == 4) return true;
	return false; 
}


void Jsw128Guard::setY(jswByte y)
{
        jswByte oy = m_orig[3] & 0xFE;
        m_orig[3]  = y & 0xFE;
        m_guard[3] += (m_orig[3] - oy);
}


jswByte Jsw128Guard::getDirection(void)      { return m_guard[0] & 0x10; }
void    Jsw128Guard::setDirection(jswByte b)
{
        m_guard[0] &= ~0x10;
        if (b) m_guard[0] |= 0x10;
}


void Jsw128Guard::setStopped(jswByte s)
{
	if (s)	m_guard[0] |= 0x40;
	else	m_guard[0] &= ~0x40;
}


void Jsw128Guard::setSlow(jswByte s)
{
	if (s)	m_guard[7] |= 0x20;
	else	m_guard[7] &= ~0x20;
}

int Jsw128Guard::getType()
{
	int type = m_guard[0] & 0x0F;
	if (type == 8) type = m_guard[0];
	return type;
}


void Jsw128Guard::drawOpeningWall(VideoSurface *s)
{
	int x = (m_guard[6] & 0x1F) * 16;
	int y = m_guard[7] & 0xFE;
	int h  = 2 * m_frame - 1;
	int fh = (2 * m_guard[3]) - h;
	int w = 14;

	x++;
	y++;
	if (h  > 0) s->fillBox(x, y, w, h, m_bgcol);
	if (fh > 0) s->fillBox(x, y+h, w, fh, ZX_ORANGE);
}
