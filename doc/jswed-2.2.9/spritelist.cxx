/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004,2006  John Elliott <jce@seasip.demon.co.uk>

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
#include "spritelist.hxx"

/* Scrolling list box for sprites */

typedef jswByte *PBYTE;

SpriteList::SpriteList(SpriteList *other, int x, int y, int w, int h) :
	VideoControl(x,y,w,h)
{
	int n;

	construct(other->m_mem);
	for (n = 0; n < other->m_itemCount; n++)
	{
		addSprite(other->m_itemList[n].zxAddress,
			  other->m_itemList[n].bitmap);
	}	
}



SpriteList::SpriteList(SpectrumMemory *mem, int x, int y, int w, int h) : 
	VideoControl(x,y,w,h)
{
	construct(mem);
}

void SpriteList::construct(SpectrumMemory *mem)
{
	m_title = "";

	m_mem = mem;
        m_fcW = (m_w - 2 * CHAR_W) / 32;

        m_bitmap = newVideoBitmap(m_w, m_h);

        m_topRow    = 0;
        m_selected  = 0;
        m_itemCount = 0;
        m_itemMax   = 10;
        m_itemList  = new SpriteEntry[m_itemMax];
        for (int n = 0; n < m_itemMax; n++) 
	{	
		m_itemList[n].zxAddress = 0;
		m_itemList[n].bitmap    = NULL;
	}	
	m_listener = NULL;
	m_oldSelected = m_oldNotified = -1;
	m_selectAction = ZXE_OK;
}

SpriteList::~SpriteList()
{
	emptyBuffer();
	if (m_bitmap) delete m_bitmap;
	if (m_itemList) delete [] m_itemList;
}


void SpriteList::addSprite(int address)
{
	addSprite(address, m_mem->memoryAt(address));
}

void SpriteList::addSprite(int address, jswByte *image)
{
	int n;

	if (m_itemCount >= m_itemMax)
	{
		SpriteEntry *pc = new SpriteEntry[2 * m_itemMax];
		if (!pc) return;
		
		for (n = 0; n < m_itemMax; n++) 
		{
			pc[n] = m_itemList[n];
		}
		for (; n <  2 * m_itemMax; n++) 
		{
			pc[n].zxAddress = 0;
			pc[n].bitmap    = NULL;
		}
		delete [] m_itemList; 
		m_itemList = pc;
		m_itemMax *= 2;
	}
	m_itemList[m_itemCount  ].bitmap    = image;
	m_itemList[m_itemCount++].zxAddress = address;
}


void SpriteList::drawFixed()
{
        // White menu box
        videoScreen->fillBox(m_x + 1,       m_y + CHAR_H,
                     m_w - 2,       m_h - CHAR_H - 1, ZX_BRWHITE);
        // Black outline
        videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
        // Title bar
        videoScreen->fillBox(m_x, m_y, m_w, CHAR_H, ZX_BLACK);
        // ZX logo
        videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}


void SpriteList::drawTitle(int x, int y)
{
	char s[40];
	int tw = (m_w / CHAR_W) - 7;

	sprintf(s, "%-*.*s", tw, tw, m_title);
	videoScreen->drawText(x, y, s, ZX_BRWHITE, ZX_BLACK);		
}


void SpriteList::draw(int reDraw)
{
	int n, x, bg;

	drawTitle(m_x, m_y);

	for (n = 0; n < m_fcW; n++)
	{
		x = m_x + n * 32 + CHAR_W;

		// Try to minimise unnecessary redrawing of bits that 
		// haven't changed.
		if (!reDraw                        && 	// redraw all
                     m_oldTopRow == m_topRow       && 	// has scrolled
	             (n + m_topRow) != m_oldSelected &&	// selection changed
                     (n + m_topRow) != m_selected ) continue;

                if (m_selected != n + m_topRow)
                {
                        videoScreen->fillReplace(x, m_y + CHAR_H, 32, 32,
                           ZX_BRCYAN, ZX_BRWHITE);
                        bg = ZX_BRWHITE;
                }
                else    bg = ZX_BRCYAN;

                drawItem(x, m_y + CHAR_H, n + m_topRow, bg);

                if (m_selected == n + m_topRow)
                {
                        videoScreen->fillReplace(x, m_y + CHAR_H, 32, 32,
                                ZX_BRWHITE, ZX_BRCYAN);
                }
	}
	// Draw scroll controls
	if (m_topRow != m_oldTopRow || reDraw)
	{
	    if (m_topRow > 0)
		 videoScreen->drawText(m_x + 1, m_y + m_h - CHAR_H - 1, 11, 
                               ZX_BLACK, ZX_BRWHITE);
	    else videoScreen->drawText(m_x + 1, m_y + m_h - CHAR_H - 1, ' ',
                               ZX_BLACK, ZX_BRWHITE);
	    if (m_topRow   < (m_itemCount - 1) &&
		m_selected < (m_itemCount - 1))
                videoScreen->drawText(m_x + m_w - (CHAR_W + 1),
                              m_y + m_h - CHAR_H, 12, 
                              ZX_BLACK, ZX_BRWHITE);
            else  videoScreen->drawText(m_x + m_w - (CHAR_W + 1),
                              m_y + m_h - CHAR_H, ' ', 
                              ZX_BLACK, ZX_BRWHITE);

            videoScreen->fillBox(m_x + m_w - (CHAR_W + 1), m_y + m_h - 1, 
			 CHAR_W, 1, ZX_BLACK);	
	}
	m_oldTopRow   = m_topRow;
	m_oldSelected = m_selected;
}



void SpriteList::emptyBuffer()
{
	for (int n = 0; n < m_itemCount; n++)
	{
		m_itemList[n].zxAddress = 0;
		m_itemList[n].bitmap    = NULL;
	}
	m_itemCount = 0;
}



int SpriteList::onSelect(void)
{
	notify();
	return m_selectAction;
}

void SpriteList::notify(void)
{
        if (m_selected != m_oldNotified && m_listener)
        {
		if (m_selected >= m_itemCount) 
		{
			m_listener->onChangeSprite(NULL);
		} 
                else m_listener->onChangeSprite(m_itemList[m_selected].bitmap);
        }
	m_oldNotified = m_selected;
}

int SpriteList::onKeyDown(int key)
{
        switch (key)
        {
                case ZXK_BREAK: return ZXE_CANCEL;
                case ZXK_ENTER:
                case ZXK_EDIT:  return onSelect();

                case ZXK_LEFT:  --m_selected;
                                if (m_selected < 0) m_selected = 0;
                               	if (m_selected < m_topRow) --m_topRow; 
				draw(0);
                                return ZXE_CONTINUE;
                case ZXK_RIGHT:  ++m_selected;
                                if (m_selected >= m_itemCount) 
					m_selected = m_itemCount - 1;
				if (m_selected >= (m_topRow + m_fcW - 2))
					++m_topRow;
                                draw(0);
                                return ZXE_CONTINUE;

                case ZXK_PGUP:  m_selected -= (m_fcW - 3);
                                if (m_selected < 0) m_selected = 0;
                                if (m_selected < m_topRow) m_topRow = m_selected;
                                draw(0);
                                return ZXE_CONTINUE;
                case ZXK_PGDN:  m_selected += (m_fcW - 3);
                                if (m_selected >= m_itemCount)
                                        m_selected = m_itemCount - 1;
                                if (m_selected >= (m_topRow + m_fcW - 2))
                                        m_topRow = m_selected - m_fcW + 3;
                                draw(0);
                                return ZXE_CONTINUE;

        }
        return KeyListener::onKeyDown(key);
}


int SpriteList::onMouseMove(int x, int y, int xrel, int yrel)
{
        if (x >= m_x && x < (m_x + m_w) &&
            y >= (m_y + CHAR_H) && y < (m_y + m_h - CHAR_H))
        {
                int nsel = ((x - m_x - CHAR_W) / 32);
		int osel = m_selected;

                m_selected = m_topRow + nsel;
                if (m_selected >= m_itemCount) m_selected = osel;
                if (m_selected != osel) draw(0);
                return ZXE_CONTINUE;
        }
        return MouseListener::onMouseMove(x, y, xrel, yrel);
}

int SpriteList::onButtonDown(int x, int y, int b)
{
	if (x < m_x || x >= (m_x + m_w))
	{
	        return ButtonListener::onButtonDown(x, y, b);
	}
	if (x >= m_x && x < (m_x + CHAR_H) && m_topRow > 0)
	{
		--m_topRow;
		--m_selected;
		draw(1);
		return ZXE_CONTINUE;	
	}
	if (x >= (m_x + m_w - CHAR_W) && (x < m_x + m_w)  && 
             m_selected < (m_itemCount - 1) &&
             m_topRow   < (m_itemCount - 1))
	{
		++m_selected;
		++m_topRow;
		draw(1);
		return ZXE_CONTINUE;
	}
        if (x >= (m_x + CHAR_W) && x < (m_x + m_w - CHAR_W))
        {
                int nsel = ((x - m_x - CHAR_W) / 32);

		int osel = m_selected;
                m_selected = m_topRow + nsel;
                if (m_selected >= m_itemCount) 
		{
			m_selected = osel;
			return ZXE_CONTINUE;
		}
		if (m_selected != osel) draw(0);
                return onSelect();
        }
	return ButtonListener::onButtonDown(x, y, b); 
}


// This relies on a pointer to a struct being the pointer to its first 
// member.
static int cmpint(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

void SpriteList::sortItems(int (*compar)(const void *, const void *))
{
	if (!compar) compar = cmpint;
	if (!m_itemCount) return;
	qsort(m_itemList, m_itemCount, sizeof(int), compar);
}


void SpriteList::checkLimits()
{
        if (m_selected >= m_itemCount) m_selected = m_itemCount - 1;

        if (m_selected < m_topRow) m_topRow = m_selected;
        if (m_selected >= (m_topRow + m_fcW - 2))
                m_topRow = m_selected - m_fcW + 3;
}

void SpriteList::redraw(void)
{
	drawFixed();
	draw(1);
}

int SpriteList::doModal()
{
	if (m_bitmap) m_bitmap->fromScreen(m_x, m_y);
        drawFixed();
        draw(1);

        KeyListener    *kl = setKeyListener   (this);
        ButtonListener *bl = setButtonListener(this);
        MouseListener  *ml = setMouseListener (this);

        int rv = videoEvent();

        setMouseListener(ml);
        setKeyListener(kl);
        setButtonListener(bl);
        if (m_bitmap) m_bitmap->toScreen(m_x, m_y);
	
	return rv;
}

void SpriteList::drawItem(int x, int y, int index, int background)
{
	char s[5];

	if (index >= m_itemCount) 
	{
		videoScreen->fillBox(x, y, 32, 40, background);
		return;
	}
	
	int      sprpage = m_itemList[index].zxAddress;
	jswByte *bitmap  = m_itemList[index].bitmap;

	VideoBitmap *bmp = bitmapFromSprite(16, 16, bitmap, 1);

	bmp->getSurface()->fillReplace(0, 0, 32, 32, ZX_BRWHITE, background);
	bmp->toScreen(x, y);
	sprintf(s, "%04x", sprpage);
	videoScreen->drawSmallText(x, y + 32, s, ZX_BLACK, background); 
	delete bmp;
}

void SpriteList::setSelected(int address)
{
	int n;
	for (n = 0; n < m_itemCount; n++) 
		if (address == m_itemList[n].zxAddress)
		{
			m_selected = n;
			if (m_visible) redraw();
			return;
		}
}


int SpriteList::getSelected(jswByte **bitmap)
{
	if (bitmap)
	{
		*bitmap = m_itemList[m_selected].bitmap;
	}
	return m_itemList[m_selected].zxAddress;
}


