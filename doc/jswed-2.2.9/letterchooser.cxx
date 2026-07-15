/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2006  John Elliott <jce@seasip.demon.co.uk>

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
#include "letterchooser.hxx"



LetterChooser::LetterChooser(int x, int y, int w, int h, jswByte *font, int count) :
	VideoControl(x, y, w, h)
{
	m_font = font;
	m_count = count;
	m_selected = 0;
	m_listener = NULL;
}

LetterChooser::~LetterChooser()
{
}

void LetterChooser::redraw(void)
{
	VideoBitmap *bmp = newVideoBitmap(m_w, m_h);
	VideoSurface *s  = bmp->getSurface();
	int n, x, y;

	s->fillBox(0, 0, m_w, m_h, ZX_BRWHITE);
	s->box(0, 0, m_w, m_h, ZX_BLACK);
	x = CHAR_W;
	y = CHAR_H;
	for (n = 0; n < m_count; n++)
	{
                VideoBitmap *b2 = bitmapFromSprite(8, 8, m_font + 8*n, 1,
			ZX_BLACK, (n == m_selected) ? ZX_BRCYAN : ZX_BRWHITE);
		b2->toSurface(s, x, y);
		x += 16; 
		if ((x + 2*CHAR_W) > m_w) { x = CHAR_W; y += 16; }
		delete b2;
	}
	bmp->toScreen(m_x, m_y);
	delete bmp;
}

void LetterChooser::redrawSelection()
{
	int chw = (m_w / CHAR_W) - 2;
	int x = m_x + (CHAR_W * (1 + (m_selected % chw)));
	int y = m_y + (CHAR_H * (1 + (m_selected / chw)));

	VideoBitmap *b2 = bitmapFromSprite(8, 8, m_font + 8*m_selected, 1,
			ZX_BLACK, ZX_BRCYAN);
	b2->toScreen(x, y);
	delete b2;
}

void LetterChooser::getCharPos(int ch, int *x, int *y)
{
	int w;
	
	w = (m_w - 2 * CHAR_W) / 16;	

	*y = m_y + CHAR_H + ((ch / w) * 16);
	*x = m_x + CHAR_W + ((ch % w) * 16);
}



void LetterChooser::setSelected(int n)
{
	int x,y;
	getCharPos(m_selected, &x, &y);
	videoScreen->fillReplace(x, y, 16, 16, ZX_BRCYAN, ZX_BRWHITE);

	if (m_listener)
	{
		m_listener->onChangeSprite(m_font + (n * 8));
	} 
	m_selected = n;
        getCharPos(m_selected, &x, &y);
        videoScreen->fillReplace(x, y, 16, 16, ZX_BRWHITE, ZX_BRCYAN);
}


int LetterChooser::onKeyDown(int keysym)
{
        int w = (m_w - 2 * CHAR_W) / 16;

	switch(keysym)
	{
		case ZXK_UP: if (m_selected >= w) setSelected(m_selected - w);
			     break;
		case ZXK_DOWN: if (m_selected + w < m_count) setSelected(m_selected + w);
				break;
		case ZXK_LEFT: 
		case '-':       if (m_selected > 0) setSelected(m_selected - 1);
				break;
		case ZXK_RIGHT:
		case '+':	if (m_selected < (m_count-1)) setSelected(m_selected + 1);
				break;
		default:
				return VideoControl::onKeyDown(keysym);
	}
	return ZXE_CONTINUE;

}


int LetterChooser::onButtonDown(int x, int y, int button)
{
	int x1, y1, w, ns;

        w = (m_w - 2 * CHAR_W);

        x1 = x - (m_x + CHAR_W);
	y1 = y - (m_y + CHAR_H);

	if (x1 < 0 || x1 > w || y1 < 0) return VideoControl::onButtonDown(x, y, button);

	x1 /= 16; y1 /= 16; w /= 16;

	ns = (y1 * w) + x1;
	if (ns < m_count) setSelected(ns);

	return VideoControl::onButtonDown(x, y, button);
}
