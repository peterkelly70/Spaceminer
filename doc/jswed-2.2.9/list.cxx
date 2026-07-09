/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2005-6  John Elliott <jce@seasip.demon.co.uk>

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

/* Implementation of the scrolling listbox */

ScrollingList::ScrollingList(int x, int y, int w, int h)
		: VideoControl(x,y,w,h)
{
	m_fcW = (m_w / CHAR_W);
	m_fcH = (m_h / CHAR_H);

	m_bitmap = newVideoBitmap(m_w, m_h);

	m_ctrlQ     = false;
	m_click     = 0;
	m_topRow    = 0;
	m_selected  = 0;
	m_itemCount = 0;
	m_itemMax   = 10;
	m_itemList  = new pChar[m_itemMax];
	for (int n = 0; n < m_itemMax; n++) m_itemList[n] = NULL;
	m_selectAction = ZXE_OK;
	m_listener = NULL;
}

ScrollingList::~ScrollingList()
{
	emptyBuffer();
	delete [] m_itemList;
	if (m_bitmap) delete m_bitmap;
}

void ScrollingList::copyFrom(ScrollingList *sl)
{
	int n;
	for (n = 0; n < sl->getItemCount(); n++)
	{
		addString(sl->getItem(n));
	}
}

void ScrollingList::addString(char *s)
{
	int n;

	if (m_itemCount >= m_itemMax)
	{
		pChar *pc = new pChar[2 * m_itemMax];
		if (!pc) return;
		for (n = 0; n < m_itemMax; n++) pc[n] = m_itemList[n];
		for (; n <  2 * m_itemMax; n++) pc[n] = NULL;

		delete [] m_itemList;
		m_itemList = pc;
		m_itemMax *= 2;
	}
	m_itemList[m_itemCount] = new char[1 + strlen(s)];
	if (!m_itemList[m_itemCount]) return;
	strcpy(m_itemList[m_itemCount], s);
	++m_itemCount;
}


void ScrollingList::drawFixed()
{
	if (!m_visible) return;

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


void ScrollingList::drawTitle(int x, int y)
{
	if (!m_visible) return;

	char s[40];
	sprintf(s, "%-*.*s", m_fcW - 7, m_fcW - 7, m_title);
	videoScreen->drawText(x, y, s, ZX_BRWHITE, ZX_BLACK);		
}


void ScrollingList::drawItem(int x, int y, int index, int bgcol)
{
	char s[50];

	if (index >= m_itemCount)
		sprintf(s, "%-*.*s", m_fcW - 2, m_fcW - 2, "");
	else 	sprintf(s, "%-*.*s", m_fcW - 2, m_fcW - 2, m_itemList[index]);

	videoScreen->drawText(x + CHAR_W, y, s, ZX_BLACK, bgcol);
}


void ScrollingList::draw(int reDraw)
{
	int n, y, bg;

	if (!m_visible) return;
	drawTitle(m_x, m_y);

	for (n = 0; n < m_fcH - 2; n++)
	{
		y = m_y + (1 + n) * CHAR_H;

		// Try to minimise unnecessary redrawing of bits that 
		// haven't changed.
		if (!reDraw                        && 	// redraw all
                     m_oldTopRow == m_topRow       && 	// has scrolled
	             (n + m_topRow) != m_oldSelected &&	// selection changed
                     (n + m_topRow) != m_selected ) continue;

                if (m_selected != n + m_topRow)
                {
                        videoScreen->fillReplace(m_x, y, m_w, CHAR_H,
                           ZX_BRCYAN, ZX_BRWHITE);
                        bg = ZX_BRWHITE;
                }
                else    bg = ZX_BRCYAN;

                drawItem(m_x, y, n + m_topRow, bg);

                if (m_selected == n + m_topRow)
                {
                        videoScreen->fillReplace(m_x, y, m_w, CHAR_H,
                                ZX_BRWHITE, ZX_BRCYAN);
                }
	}
	// Draw scroll controls
	if (m_topRow != m_oldTopRow || reDraw)
	{
	    if (m_topRow > 0)
		 videoScreen->drawText(m_x + m_w - (CHAR_W + 1), m_y, '^', 
                               ZX_BRWHITE, ZX_BLACK);
	    else videoScreen->drawText(m_x + m_w - (CHAR_W + 1), m_y, ' ',
                               ZX_BRWHITE, ZX_BLACK);
	    if (m_topRow   < (m_itemCount - 1) &&
		m_selected < (m_itemCount - 1))
                videoScreen->drawText(m_x + m_w - (CHAR_W + 1),
                              m_y + m_h - CHAR_H, 9, 
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



void ScrollingList::emptyBuffer()
{
	for (int n = 0; n < m_itemCount; n++)
	{
		delete [] m_itemList[n];
		m_itemList[n] = NULL;
	}
	m_itemCount = 0;
}



int ScrollingList::onSelect(void)
{
	if (m_listener)
	{
		m_listener->onSelect(this, m_selected);
	}
	return m_selectAction;
}

int ScrollingList::onKeyDown(int key)
{
        switch (key)
        {
                case ZXK_BREAK: m_ctrlQ = false; return ZXE_CANCEL;
                case ZXK_ENTER:
                case ZXK_EDIT:  m_ctrlQ = false; return onSelect();

		case ZXK_LEFT:	if (!m_ctrlQ) break;
				// Fall through...
                case ZXK_UP:    if (m_ctrlQ) m_selected = m_topRow;
				else --m_selected;
                                if (m_selected < 0) m_selected = 0;
                               	if (m_selected < m_topRow) --m_topRow; 
				draw(0);
				m_ctrlQ = false;
                                return ZXE_CONTINUE;
		case ZXK_RIGHT:	if (!m_ctrlQ) break;
				// Fall through...
                case ZXK_DOWN:  if (m_ctrlQ) 
				     m_selected = (m_topRow + m_fcH - 3);
				else ++m_selected;
                                if (m_selected >= m_itemCount) 
					m_selected = m_itemCount - 1;
				if (m_selected >= (m_topRow + m_fcH - 2))
					++m_topRow;
                                draw(0);
				m_ctrlQ = false;
                                return ZXE_CONTINUE;

                case ZXK_PGUP:  if (m_ctrlQ) m_selected = 0;
				else m_selected -= (m_fcH - 3);
                                if (m_selected < 0) m_selected = 0;
                                if (m_selected < m_topRow) m_topRow = m_selected;
                                draw(0);
				m_ctrlQ = false;
                                return ZXE_CONTINUE;
                case ZXK_PGDN:  if (m_ctrlQ) m_selected = m_itemCount - 1;
				else m_selected += (m_fcH - 3);
                                if (m_selected >= m_itemCount)
                                        m_selected = m_itemCount - 1;
                                if (m_selected >= (m_topRow + m_fcH - 2))
                                        m_topRow = m_selected - m_fcH + 3;
                                draw(0);
				m_ctrlQ = false;
                                return ZXE_CONTINUE;

        }
	m_ctrlQ = (key == ZXK_CTRLQ);
        return KeyListener::onKeyDown(key);
}


int ScrollingList::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (m_click) return MouseListener::onMouseMove(x,y,xrel,yrel);

        if (x >= m_x && x < (m_x + m_w) &&
            y >= (m_y + CHAR_H) && y < (m_y + m_h - CHAR_H))
        {
                int nsel = ((y - m_y) / CHAR_H) - 1;
		int osel = m_selected;

                m_selected = m_topRow + nsel;
                if (m_selected >= m_itemCount) m_selected = osel;
                if (m_selected != osel) draw(0);
                return ZXE_CONTINUE;
        }
        return MouseListener::onMouseMove(x, y, xrel, yrel);
}

int ScrollingList::onButtonDown(int x, int y, int b)
{
	if (x < m_x || x >= (m_x + m_w))
	{
	        return ButtonListener::onButtonDown(x, y, b);
	}
	if (y >= m_y && y < (m_y + CHAR_H) && m_topRow > 0)
	{
		--m_topRow;
		--m_selected;
		draw(1);
		return ZXE_CONTINUE;	
	}
	if (y >= (m_y + m_h - CHAR_H) && (y < m_y + m_h) && 
             m_selected < (m_itemCount - 1) &&
             m_topRow   < (m_itemCount - 1))
	{
		++m_selected;
		++m_topRow;
		draw(1);
		return ZXE_CONTINUE;
	}
        if (y >= (m_y + CHAR_H) && y < (m_y + m_h - CHAR_H))
        {
                int nsel = ((y - m_y) / CHAR_H) - 1;

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


static int cmpstr(const void *a, const void *b)
{
	return strcmp( *((pChar *)a), * ((pChar *)b));
}

void ScrollingList::sortItems(int (*compar)(const void *, const void *))
{
	if (!compar) compar = cmpstr;
	if (!m_itemCount) return;
	qsort(m_itemList, m_itemCount, sizeof(pChar), compar);
}


void ScrollingList::checkLimits()
{
        if (m_selected >= m_itemCount) m_selected = m_itemCount - 1;

        if (m_selected < m_topRow) m_topRow = m_selected;
        if (m_selected >= (m_topRow + m_fcH - 2))
                m_topRow = m_selected - m_fcH + 3;
}

void ScrollingList::redraw(void)
{
	drawFixed();
	draw(1);
}

int ScrollingList::doModal()
{
	if (m_bitmap) m_bitmap->fromScreen(m_x, m_y);
	checkLimits();
	setVisible(1, 0);
        drawFixed();
        draw(1);

        KeyListener    *kl = setKeyListener   (this);
        ButtonListener *bl = setButtonListener(this);
        MouseListener  *ml = setMouseListener (this);

        int rv = videoEvent();

	setVisible(0, 0);
        setMouseListener(ml);
        setKeyListener(kl);
        setButtonListener(bl);
        if (m_bitmap) m_bitmap->toScreen(m_x, m_y);
	
	return rv;
}

