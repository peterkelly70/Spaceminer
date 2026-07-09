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


VideoNotePage::VideoNotePage() : VideoForm(0, 0, 0, 0)
{
	m_x = m_y = m_w = m_h = 0;
	m_lx = m_ly = m_lw = m_lh =0;
	m_title = NULL;
	m_lbitmap = NULL;
	m_bitmap = NULL;
	m_concealed = 1;
}

VideoNotePage::~VideoNotePage()
{
	if (m_title) delete [] m_title;
	if (m_bitmap) delete m_bitmap;
	if (m_lbitmap) delete m_lbitmap;
}


void VideoNotePage::setRectangle(int x, int y, int w, int h)
{
	int concealed = m_concealed;

	if (x == m_x && y == m_y && w == m_w && h == m_h) return;

	if (m_w && m_h && !concealed) 
	{
		if (onConceal()) return;
	}
	if (m_bitmap) 
	{
		delete m_bitmap; m_bitmap = NULL; 
	}
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	if (!concealed) onReveal(VPR_RIGHT);
}



void VideoNotePage::setLeftRectangle(int lx, int ly, int lw, int lh)
{
        int concealed = m_concealed;

        if (lx == m_lx && ly == m_ly && lw == m_lw && lh == m_lh) return;

        if (m_lw && m_lh && !concealed)
        {
                if (onConceal()) return;
        }
        if (m_lbitmap)
        {
                delete m_lbitmap; m_lbitmap = NULL;
        }
        m_lx = lx;
        m_ly = ly;
        m_lw = lw;
        m_lh = lh;
        if (!concealed) onReveal(VPR_LEFT);
}



char *VideoNotePage::getTitle()
{
	return m_title;
}

void VideoNotePage::setTitle(char *s)
{
	if (m_title) delete [] m_title;
	if (!s) 
	{
		m_title = NULL;
		return;
	}
	m_title = new char[1+strlen(s)];
	strcpy(m_title, s);
}

int VideoNotePage::onReveal(int rectangle)
{
	showChildren(0);
	if (rectangle & VPR_LEFT)
	{
		if (m_lbitmap) m_lbitmap->toScreen(m_lx, m_ly);
		else	       redraw(VPR_LEFT);
	}
	if (rectangle & VPR_RIGHT)
	{
		if (m_bitmap)  m_bitmap->toScreen(m_x, m_y);
		else           redraw(VPR_RIGHT);
	}
	if (getFocus()) getFocus()->drawFocus(1);
	m_concealed = 0;
	return 0; 	
}


int VideoNotePage::onConceal()
{
	if (!m_bitmap) m_bitmap = newVideoBitmap(m_x, m_y, m_w, m_h);
	else	       m_bitmap->fromScreen(m_x, m_y);

        if (!m_lbitmap) m_lbitmap = newVideoBitmap(m_lx, m_ly, m_lw, m_lh);
        else            m_lbitmap->fromScreen(m_lx, m_ly);

	if (getFocus()) getFocus()->drawFocus(0);
	hideChildren();
	m_concealed = 1;
	return 0;
}

int VideoNotePage::redraw(int which)
{
	if (which & VPR_RIGHT)
	{
		videoScreen->fillBox(m_x, m_y, m_w, m_h, ZX_WHITE);
        	for (int n = 0; n < m_ccount; n++)
        	{
               		m_controls[n]->redraw();
                	m_controls[n]->drawFocus(n == m_focus);
       		 }
	}
        if (which & VPR_LEFT)
        {	
                videoScreen->fillBox(m_lx, m_ly, m_lw, m_lh, ZX_WHITE);

	        for (int n = 0; n < m_ccount; n++) 
		    if (m_controls[n]->m_x < m_x)
       		    {
               		m_controls[n]->redraw();
               		m_controls[n]->drawFocus(n == m_focus);
        	    }
	}
	return 0;
}


int VideoNotePage::preContextMenu(void)
{
	return -1;
}

void VideoNotePage::postContextMenu(void)
{

}

