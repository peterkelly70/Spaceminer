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


/* The first constructor takes options as a variable-length list and converts them into the
 * (count, title, array) format taken by the second. */

VideoMenu::VideoMenu(char *title, ...)
{
	char *opt;
	int count = 10;
	int cur = 0;
//	arglist ap;
	va_list ap;
//
// Parameter parsing. The array "opts" will grow if more than 10 options
// are present.
//

        pChar *opts = new pChar[count];

        if (!opts)
        {
                jswLog("JSWED Menu: Out of memory\n");
		return;
        }

        va_start(ap, title);

        while (( opt = va_arg(ap, pChar) ))
        {
                opts[cur] = opt;
                ++cur;
                if (cur == count)
                {
                        pChar *olds = opts;
                        opts = new pChar[2 * count];
                        if (!opts)
                        {
                                jswLog("JSWED Menu: Out of memory\n");
                                delete [] olds;
                                va_end(ap); 
				return;
                        }
                        for (int n = 0; n < count; n++) opts[n] = olds[n];
                        delete [] olds;
                        count *= 2;
                }
        }
        /* The array "opts" now holds "cur"  options. */
	construct(cur, title, opts);
	delete [] opts;
	va_end(ap);
}

VideoMenu::VideoMenu(int count, char *title, char **opts)
{
	construct(count, title, opts);
}


void VideoMenu::construct(int cur, char *title, char **opts)
{
	int n, m;

//
// Data initialisation
//
	m_options = NULL;
	m_title   = NULL;
	m_bitmap  = NULL;
	m_count = 0;
	m_selected = 0;

	m_x = 0;
	m_y = 0;
	m_w = 0;
	m_h = 0;

	/* The array "opts" now holds "cur"  options. Allocate our own copies
           of the option strings; the ones we were given might change.  */	

	m_options = new pChar[cur];
	if (!m_options)
	{
		jswLog("JSWED Menu: Out of memory\n");
		return;
	}

	for (n = 0; n < cur; n++)
	{
		m_options[n] = new char[1 + strlen(opts[n])];
		if (!m_options[n])
		{
	                jswLog("JSWED Menu: Out of memory\n");
                	for (m = 0; m < n; m++) delete [] m_options[m];
			delete [] m_options;
			m_options = NULL;
                	return;
		}
		strcpy(m_options[n], opts[n]);

		int ow = (2 + strlen(opts[n])) * CHAR_W;
		if (ow > m_w) m_w = ow;
	}

	m_title = new char[1 + strlen(title)];
        if (!m_title)
        {
        	jswLog("JSWED Menu: Out of memory\n");
                for (m = 0; m < cur; m++) delete [] m_options[m];
                delete [] m_options;
		m_options = NULL;
                return;
	}
	strcpy(m_title, title);
        int ow = (6 + strlen(m_title)) * CHAR_W;
        if (ow > m_w) m_w = ow;

	m_count = cur;
	m_h = (2 + cur) * CHAR_H;
	m_x = (VIDEO_W - m_w) / 2;
	m_y = (VIDEO_H - m_h) / 2;

	// Don't actually save the screen here; wait until a doMenu() call.
	m_bitmap = newVideoBitmap(m_w, m_h);
}



VideoMenu::~VideoMenu()
{
	if (m_options)
	{
		for (int n = 0; n < m_count; n++) delete [] m_options[n];
		delete [] m_options;
	}
	if (m_title) delete [] m_title;
	if (m_bitmap) delete m_bitmap;
}


int VideoMenu::doModal()
{
	if (!m_count) return -1;	// No options

	if (m_bitmap) m_bitmap->fromScreen(m_x, m_y);
	drawFixed();
	draw();
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

void VideoMenu::setSelected(int n)
{
	if (n < 0 || n >= m_count) return;
	m_selected = n;
}


void VideoMenu::drawFixed()
{
	// White menu box
	videoScreen->fillBox(m_x + 1,       m_y + CHAR_H,
		     m_w - 2,       m_h - CHAR_H - 1, ZX_BRWHITE);
	// Black outline
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
	// Title bar
	videoScreen->fillBox(m_x, m_y, m_w, CHAR_H, ZX_BLACK);
	// Title text
	videoScreen->drawText(m_x, m_y, m_title, ZX_BRWHITE, ZX_BLACK);
	// ZX logo
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);	
	// options
	for (int n = 0; n < m_count; n++)
	{
		videoScreen->drawText(m_x + CHAR_W, m_y + CHAR_H * (1+n), 
			      m_options[n], ZX_BLACK, ZX_BRWHITE); 
	}
}


void VideoMenu::draw()
{
	int y;
	for (int n = 0; n < m_count; n++)
	{
		y = m_y + (n+1) * CHAR_H;	
		if (n == m_selected)
		{
			videoScreen->fillReplace(m_x, y, m_w, CHAR_H, 
				ZX_BRWHITE, ZX_BRCYAN);
		}
		else
		{
			videoScreen->fillReplace(m_x, y, m_w, CHAR_H,
				 ZX_BRCYAN, ZX_BRWHITE);
		}
	}
}



int VideoMenu::onKeyDown(int k)
{
	switch (k)
	{
		case ZXK_BREAK:	return ZXE_CANCEL;
		case ZXK_ENTER:
		case ZXK_EDIT:	return ZXE_OK;

		case ZXK_UP:	--m_selected;
				if (m_selected < 0) m_selected = m_count - 1;
				draw(); 
				return ZXE_CONTINUE;
                case ZXK_DOWN:  ++m_selected;
                                if (m_selected >= m_count) m_selected = 0;
                                draw();
                                return ZXE_CONTINUE;
                case ZXK_PGUP:  m_selected = 0;
                                draw();
                                return ZXE_CONTINUE;
                case ZXK_PGDN:  m_selected = m_count - 1;
                                draw();
                                return ZXE_CONTINUE;

	}
	return KeyListener::onKeyDown(k);
}


int VideoMenu::onButtonDown(int x, int y, int b)
{
        if (x >= m_x && x < (m_x + m_w) &&
            y >= (m_y + CHAR_H) && y < (m_y + m_h - CHAR_H))
        {
                int nsel = ((y - m_y) / CHAR_H) - 1;

                m_selected = nsel;
                draw();
                return ZXE_OK;
        }

	return ButtonListener::onButtonDown(x, y, b);	
}


int VideoMenu::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + m_w) &&
	    y >= (m_y + CHAR_H) && y < (m_y + m_h - CHAR_H))
	{
		int nsel = ((y - m_y) / CHAR_H) - 1;

		m_selected = nsel;
		draw();		
		return ZXE_CONTINUE;
	}
	return MouseListener::onMouseMove(x, y, xrel, yrel);
}

