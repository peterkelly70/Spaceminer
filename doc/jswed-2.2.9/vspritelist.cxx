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
#include "vspritelist.hxx"

/* Scrolling list box for sprites */

VSpriteList::VSpriteList(SpectrumMemory *mem)
	: ScrollingList(8 * CHAR_W, 12 * CHAR_H, 23 * CHAR_W, 8 * CHAR_H)
{
	m_mem = mem;
}

VSpriteList::~VSpriteList()
{
}



void VSpriteList::drawTitle(int x, int y)
{
	char s[40];
	int tw = (m_w / CHAR_W) - 7;

	sprintf(s, "%-*.*s", tw, tw, m_title);
	videoScreen->drawText(x, y, s, ZX_BRWHITE, ZX_BLACK);		
}



void VSpriteList::drawItem(int x, int y, int index, int background)
{
	char s[20];
	int sprpage;

	if (index >= m_itemCount) 
	{
		videoScreen->fillBox(x, y, m_w, CHAR_H, background);
		return;
	}

	strncpy(s, m_itemList[index], 4); s[4] = 0;
	sscanf(s, "%x", &sprpage);
	sprintf(s, "%x: ", sprpage);

	videoScreen->drawText(x + CHAR_W, y, s, ZX_BLACK, background);
	for (int n = 0; n < 8; n++)
	{
		VideoBitmap *bmp = bitmapFromSprite(16, 16,
                        m_mem->memoryAt(sprpage + 32 * n), 0);
	        bmp->getSurface()->fillReplace(0, 0, 32, 32, ZX_BRWHITE, background);
		bmp->toScreen(x + (7 + 2*n) * CHAR_W, y);
		delete bmp;
	}
}



void VSpriteList::addSprite(int id)
{
	char s[6];

	sprintf(s, "%04x:", id);
	addString(s);
}

void VSpriteList::setSelected(int id)
{
	int n;
	char s[6];
	sprintf(s, "%04x:", id);

	for (n = 0; n < m_itemCount; n++)
	{
		if (!strncmp(s, m_itemList[n], 5)) ScrollingList::setSelected(n);
	}
}


int  VSpriteList::getSelected(void)
{
	char s[6];
	int id;

	strncpy(s, m_itemList[ScrollingList::getSelected()], 4);
	s[4] = 0;
	sscanf(s, "%x", &id);
	return id;		
}

