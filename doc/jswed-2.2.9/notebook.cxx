/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

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

VideoNotebook::VideoNotebook(int x, int y, int w, int h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_pw = 0;
	m_pages = new PVPAGE[5];
	m_pcount = 0;
	m_pmax = 5;
	m_active = 0;
	m_title = NULL;
}


VideoNotebook::~VideoNotebook()
{
	if (m_title) delete [] m_title;
	delete [] m_pages;
}

void VideoNotebook::setTitle(char *s)
{
	if (m_title) delete m_title;
	m_title = new char[1 + strlen(s)];
	if (m_title) strcpy(m_title, s);
}

int VideoNotebook::addPage(VideoNotePage *p)
{
	int tw = (1+strlen(p->getTitle())) * CHAR_W;

	if (tw > m_pw) m_pw = tw;
	
	if (m_pcount == m_pmax)
	{
		PVPAGE *pvp = new PVPAGE[2 * m_pmax];

		if (!pvp)
		{
	                jswLog("JSWED Notebook: Out of memory\n");
			return -1;
		} 
		for (int n = 0; n < m_pmax; n++) pvp[n] = m_pages[n];
		delete [] m_pages;
		m_pages = pvp;
		m_pmax *= 2;
	}
	m_pages[m_pcount++] = p;
	return 0;
}

void VideoNotebook::drawFixed(void)
{
	int n;
        int ph = CHAR_H * (2 + m_pcount);

	for (n = 0; n < m_pcount; n++)
	{
		m_pages[n]->setRectangle(m_x + m_pw, m_y, m_w - m_pw, m_h); 
		m_pages[n]->setLeftRectangle(m_x, m_y + ph, m_pw, m_h - ph);
	}
	m_pages[m_active]->onReveal();

	videoScreen->fillBox(m_x, m_y,    m_pw, ph,       ZX_BRWHITE);
	videoScreen->box(m_x, m_y, m_pw, ph, ZX_BLACK);
	videoScreen->fillBox(m_x, m_y,      m_pw, CHAR_SH, ZX_BLACK);
	videoScreen->smallZxLogo(m_pw - 6 * CHAR_SW, m_y);

	if (m_title) videoScreen->drawSmallText(m_x, m_y, m_title, 
					ZX_BRWHITE, ZX_BLACK);

	for (n = 0; n < m_pcount; n++)
	{
		videoScreen->drawText(m_x + (CHAR_W / 2),
			      m_y + (CHAR_H * (1+n)),
			      m_pages[n]->getTitle(),
			      ZX_BLACK, ZX_BRWHITE);
	}
}

void VideoNotebook::draw()
{
        int y;
        for (int n = 0; n < m_pcount; n++)
        {
                y = m_y + (n+1) * CHAR_H;
                if (n == m_active)
                {
                        videoScreen->fillReplace(m_x, y, m_pw, CHAR_H,
				ZX_BRWHITE, ZX_BRCYAN);
                }
                else
                {
                        videoScreen->fillReplace(m_x, y, m_pw, CHAR_H, 
				ZX_BRCYAN, ZX_BRWHITE);
                }
        }

}


void VideoNotebook::selectPage(int n)
{
	if (n < 0 || n >= m_pcount) return;

	if (m_pages[m_active]->onConceal()) return;
	if (m_pages[n]       ->onReveal())  return;

	m_active = n;
	draw();	
}



int VideoNotebook::onButtonDown(int x, int y, int button)
{
        int ph = CHAR_H * (2 + m_pcount);
        if (x >= m_x && x < (m_x + m_pw) && y >= m_y && y < (m_y + ph))
        {
		int so = y - (m_y + CHAR_H);

		if (so >= 0 && so < (CHAR_H * m_pcount))
		{
			selectPage(so / CHAR_H);	
			return ZXE_CONTINUE;	
		} 
        }
        return m_pages[m_active]->onButtonDown(x,y,button);
}


int VideoNotebook::onButtonUp(int x, int y, int button)
{
        return m_pages[m_active]->onButtonUp(x,y,button);
}



int VideoNotebook::onKeyDown(int keysym)
{
	if (keysym == ZXK_ALTUP)
	{
		if (m_active) selectPage(m_active - 1);
		else	      selectPage(m_pcount - 1);
		return ZXE_CONTINUE;	
	}
	if (keysym == ZXK_ALTDOWN)
	{
                if (m_active < (m_pcount-1))	selectPage(m_active + 1);
                else          			selectPage(0);
                return ZXE_CONTINUE;
	}
	return m_pages[m_active]->onKeyDown(keysym);
}




int VideoNotebook::onMouseMove(int x, int y, int xrel, int yrel)
{
        int ph = CHAR_H * (2 + m_pcount);
	if (x >= m_x && x < (m_x + m_pw) && y >= m_y && y < (m_y + ph)) 
	{
		// ignored
	}
	return m_pages[m_active]->onMouseMove(x,y,xrel,yrel);
}


int VideoNotebook::doModal()
{
        if (!m_pcount) return -1;        // No options

        drawFixed();
	draw();
        KeyListener    *kl = setKeyListener   (this);
        ButtonListener *bl = setButtonListener(this);
        MouseListener  *ml = setMouseListener (this);

        int rv = videoEvent();

        setMouseListener(ml);
        setKeyListener(kl);
        setButtonListener(bl);
        return rv;
}
