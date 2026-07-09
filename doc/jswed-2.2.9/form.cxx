/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005-6  John Elliott <jce@seasip.demon.co.uk>

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

VideoForm::VideoForm(int x, int y, int w, int h)
{
	m_controls = new PVCONTROL[8];
	m_cmax = 8;
	m_ccount = 0;
	m_focus = 0;
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	for (int n = 0; n < m_cmax; n++) m_controls[n] = NULL;
}

VideoForm::~VideoForm()
{
	delete [] m_controls;
}


int VideoForm::doModal()
{
	VideoBitmap *vbm = newVideoBitmap(m_x, m_y, m_w, m_h);
	
	showChildren();
        redraw();
        KeyListener    *kl = setKeyListener   (this);
        ButtonListener *bl = setButtonListener(this);
        MouseListener  *ml = setMouseListener (this);

        int rv = videoEvent();
	hideChildren();
        setMouseListener(ml);
        setKeyListener(kl);
        setButtonListener(bl);
	if (vbm) 
	{
		vbm->toScreen(m_x, m_y);
		delete vbm;
	}
        return rv;

}

int VideoForm::addChild(VideoControl *child)
{
	int n;

	if (m_ccount == m_cmax)
	{
		PVCONTROL *pv = new PVCONTROL[2 * m_cmax];
		if (!pv) return -1;	// Out of memory
		for (n = 0; n < m_cmax; n++) pv[n] = m_controls[n];
		m_cmax *= 2;
		for (;      n < m_cmax; n++) pv[n] = NULL;
		delete [] m_controls;
		m_controls = pv;
	} 
	m_controls[m_ccount++] = child;
	return 0;
}


int VideoForm::removeChild(VideoControl *child)
{
	int n, m, c;
	for (c = n = 0; n < m_ccount; n++) if (m_controls[n] == child)
	{
		for (m = n; m < (m_ccount-1); m++) 
			m_controls[m] = m_controls[m+1];
		m_controls[m] = NULL;
		--m_ccount;
		++c;
	}
	return (c > 0) ? 0 : -1;
}

int VideoForm::setFocus(VideoControl *child)
{
	int n, c;
        for (c = n = 0; n < m_ccount; n++) if (m_controls[n] == child)
        {
		m_focus = n;
		c = 1;
		drawFocus();
		break;
        }
        return (c > 0) ? 0 : -1;

}

VideoControl *VideoForm::getFocus()
{
	if (m_focus >= m_ccount) return NULL;
	return m_controls[m_focus]; 
}


void VideoForm::drawFocus(void)
{
	int n;
//
// Draw controls that _lose_ focus before controls that _gain_ it,
// so there's never a time when two controls have focus at once.
//
	for (n = 0; n < m_ccount; n++) if (n != m_focus) 
		m_controls[n]->drawFocus(0);
        for (n = 0; n < m_ccount; n++) if (n == m_focus) 
		m_controls[n]->drawFocus(1);

}

void VideoForm::redraw(void)
{
	videoScreen->fillBox(m_x, m_y, m_w, m_h, ZX_WHITE);
        for (int n = 0; n < m_ccount; n++)
        {
                m_controls[n]->redraw();
		m_controls[n]->drawFocus(n == m_focus);
        }
}

void VideoForm::redrawBorder(char *s)
{
	char buf[81];

	sprintf(buf, "%-*.*s", m_w / CHAR_W, m_w / CHAR_W, s);
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
	videoScreen->drawText(m_x, m_y, buf, ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}


int VideoForm::onMouseMove(int x, int y, int xrel, int yrel)
{
	int rv = ZXE_CONTINUE;

	for (int n = 0; n < m_ccount; n++)
	{
		if (m_controls[n]->hitTest(x, y))
		{
			rv = m_controls[n]->onMouseMove(x, y, xrel, yrel);
		}
	}
	return rv;
}


int VideoForm::onButtonUp  (int x, int y, int button)
{
	int rv = ZXE_CONTINUE; 

        for (int n = 0; n < m_ccount; n++)
        {
                if (m_controls[n]->hitTest(x, y))
                {
			rv = m_controls[n]->onButtonUp(x, y, button);
                }
        }
	return rv;
}


int VideoForm::onButtonDown(int x, int y, int button)
{
	int rv = ZXE_CONTINUE;

        for (int n = 0; n < m_ccount; n++)
        {
                if (m_controls[n]->hitTest(x, y) && m_controls[n]->canFocus())
                {
			if (n != m_focus)
			{
				m_focus = n;
				drawFocus();
			}
                        rv = m_controls[n]->onButtonDown(x, y, button);
                }
        }
	return rv;
}

int VideoForm::onKeyUp  (int keysym)
{
	if (m_controls[m_focus]) return m_controls[m_focus]->onKeyUp(keysym);
	return KeyListener::onKeyUp(keysym);
}


int VideoForm::onKeyDown(int keysym)
{
	if (keysym == ZXK_TAB)
	{
		int of = m_focus;

		do
		{
			m_focus++;
			if (m_focus >= m_ccount) m_focus = 0;
			if (m_focus == of) break;
		} while (!m_controls[m_focus]->canFocus());
		drawFocus();
	}
	if (keysym == ZXK_BACKTAB)
	{
		int of = m_focus;

		do
		{
			m_focus--;
			if (m_focus < 0) m_focus = m_ccount - 1;
			if (m_focus == of) break;
		} while (!m_controls[m_focus]->canFocus());
		drawFocus();
	}
	
	if (m_controls[m_focus]) return m_controls[m_focus]->onKeyDown(keysym);
	return KeyListener::onKeyDown(keysym);
}


void VideoForm::hideChildren()
{
        for (int n = 0; n < m_ccount; n++)
        {
                m_controls[n]->setVisible(0);
        }

}

void VideoForm::showChildren(int redraw)
{
        for (int n = 0; n < m_ccount; n++)
        {
                m_controls[n]->setVisible(1, redraw);
        }
}
