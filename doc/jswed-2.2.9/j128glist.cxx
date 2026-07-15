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

/* Scrolling guardian list 
 * Entry form: 
 * 
 * 1...5...10...15...20...25...30
 * xx Horizontal Colour ********
 **/

#include "jswed.hxx"
#include "j128guard.hxx"
#include "j128glist.hxx"

static char *types[16] = {
	"None",
	"Horizontal",
	"Vertical",
	"Rope",
	"Arrow",
	"Diagonal /",
	"Diagonal \\",
	"Vertical Colour",
	"None",
	"Horizontal Colour",
	"Vertical",
	"None",
	"None",
	"Diagonal / Colour",
	"Diagonal \\ Colour",
	"Vertical Colour"
};

Jsw128GuardList::Jsw128GuardList(VideoBitmap *bmp, jswByte *list, 
				 SpectrumMemory *mem, int bg, int max) : 
		ScrollingList (CHAR_W * 3, 256, CHAR_W * 34, VIDEO_H - 256)
{
	int n, x, t;
	jswByte *b = list;
	char buf[30];

	videoScreen->drawText(CHAR_W * 8, 0, "Please wait...", 
				ZX_BRWHITE, ZX_BLACK);

	setTitle("Choose new guardian");

        m_bmAnim = newVideoBitmap(512, 256);

	m_bmAnim->fromScreen(m_x, VIDEO_H - 256);
	for (n = 0; n < max; n++)
	{
		t = b[0] & 0x0F;
		videoScreen->box(CHAR_W * 8, 0, 14 * CHAR_W, CHAR_H, 
					(n % 16));
		sprintf(buf, "%02x %-17.17s", n, types[t]);
		addString(buf);

		switch(t)
		{
			case 1: case 5:  case 6:
			case 9: case 13: case 14: x = b[6]; break;

			case 4: x = 64; break;

			default: x = 15; break;
		}

		m_guard[n] = new Jsw128Guard(b, x, mem, bg);
		b += 8;
	}
	m_bmAnim->toScreen(m_x, VIDEO_H - 256);
	m_bmBg = bmp;
	m_tl = setTickListener(this);
	m_tc = 0;
	m_lock = 0;
	m_max = max;
}





Jsw128GuardList::~Jsw128GuardList()
{
	for (int n = 0; n < m_max; n++) delete m_guard[n];
	delete m_bmAnim;
	setTickListener(m_tl);
}


void Jsw128GuardList::onTick(void)
{
	++m_tc;
	if (m_selected < 0 || m_selected >= m_max) return;

	if ((m_tc % 5) == 0)	
	{
		if (m_lock) return;
		++m_lock;
                m_bmBg->toSurface(m_bmAnim, 0, 0);
                m_guard[m_selected]->move();
                m_guard[m_selected]->draw(m_bmAnim->getSurface());
		m_guard[m_selected]->drawBounds(m_bmAnim->getSurface());
		m_bmAnim->toScreen(CHAR_W * 8, 0);
		--m_lock;	
	}
}


void Jsw128GuardList::drawItem(int x, int y, int index, int background)
{
	ScrollingList::drawItem(x, y, index, background);	
}



int Jsw128GuardList::onMouseMove(int x, int y, int xrel, int yrel)
{
        int rv;
        ++m_lock;
        rv = ScrollingList::onMouseMove(x, y, xrel, yrel);
        --m_lock;
        return rv;
}

int Jsw128GuardList::onButtonDown(int x, int y, int button)
{
        int rv;
        ++m_lock;
        rv = ScrollingList::onButtonDown(x, y, button);
        --m_lock;
        return rv;
}


int Jsw128GuardList::onKeyDown(int key)
{
	int rv;
	++m_lock;
	rv = ScrollingList::onKeyDown(key);
	--m_lock;
	return rv;
}

