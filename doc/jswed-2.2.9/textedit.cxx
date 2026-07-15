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

VideoTextEdit::VideoTextEdit(int x, int y, int w, int h): VideoControl(x,y,w,h)
{
	m_buf = new char[256];
	m_buflen = 256;
	m_buf[0] = 0;
	m_tcount = 0;
	m_ot = NULL;
	m_curpos = 0;
	m_haveTimer = 0;
	m_limit = -1;
	m_bind  = NULL;
	m_bindFixed = 0;
	m_listener = NULL;
	m_hexOnly = false;
	m_numeric = false;
	m_ctrlQ = false;
}

VideoTextEdit::~VideoTextEdit()
{
	updateBind();
	delete [] m_buf;
	if (m_haveTimer) setTickListener(m_ot);
}


void VideoTextEdit::bind(char *s)
{
	m_bind = s;
	m_bindFixed = 0;

	setText(s);
}

void VideoTextEdit::bindFixed(char *s)
{
	if (m_limit == -1) return;
	m_bind = s;
	m_bindFixed = 1;

	char *t = new char[1 + m_limit]; 
	if (!t) return;
	memcpy(t, s, m_limit);	
	t[m_limit] = 0;
	setText(t);
	delete [] t;
}


void VideoTextEdit::updateBind(void)
{
	int n;

	if (m_listener) m_listener->onEditChange(this);

	if (!m_bind) return;
	if (m_limit == -1) 
	{
		strcpy(m_bind, m_buf);
		return;
	}
	if (m_bindFixed)
	{
		strncpy(m_bind, m_buf, m_limit);
		for (n = strlen(m_buf); n < m_limit; n++)
		{
			m_bind[n] = ' ';
		}
		return;
	}
	strncpy(m_bind, m_buf, m_limit);
	m_bind[m_limit - 1] = 0;
}


int VideoTextEdit::getLimit()
{
	return m_limit;
}

void VideoTextEdit::setLimit(int n)
{
	if (n < (int)strlen(m_buf))
	{
		m_buf[n] = 0;
	        if (m_curpos > (int)strlen(m_buf)) m_curpos = strlen(m_buf);
	        redrawText();
		updateBind();
	}
	m_limit = n;
}


void VideoTextEdit::redrawText(void)
{
        unsigned int cw = (m_w - 4) / CHAR_W;

	if (!m_visible) return;

        videoScreen->fillBox (m_x + 1, m_y + 1, m_w - 2,  m_h - 2, ZX_WHITE);
	if (cw < strlen(m_buf) + 1)
	{
		char *nbuf = new char[cw + 1];
		int from;
	
		if (!nbuf) return;

		from = m_curpos - (cw / 2); if (from < 0) from = 0;
		strncpy(nbuf, m_buf + from, cw);
		nbuf[cw] = 0;	
	        for (unsigned int n = 0; n < cw; n++)
       		{
       		        if (nbuf[n] == ZXCH_COPY)  nbuf[n] = ZXK_COPY;
	                if (nbuf[n] == ZXCH_POUND) nbuf[n] = ZXK_POUND;
		}
		videoScreen->drawText(m_x + 2, m_y + 2, nbuf, ZX_BLACK, ZX_WHITE);	
		delete [] nbuf;
	}
	else 
	{
		char *nbuf = new char[1 + strlen(m_buf)];

		if (!nbuf) return;
		strcpy(nbuf, m_buf);	
                for (unsigned int n = 0; n < strlen(nbuf); n++)
                {
       		        if (nbuf[n] == ZXCH_COPY)  nbuf[n] = ZXK_COPY;
	                if (nbuf[n] == ZXCH_POUND) nbuf[n] = ZXK_POUND;
                }

		videoScreen->drawText(m_x + 2, m_y + 2, nbuf, ZX_BLACK, ZX_WHITE);
		delete [] nbuf;
	}
}

void VideoTextEdit::redraw(void)
{
	if (!m_visible) return;
	videoScreen->box     (m_x,     m_y,     m_w,      m_h, ZX_BLACK);
	redrawText();
}

void VideoTextEdit::setText(char *s)
{
	if (strlen(s) >= m_buflen)
	{
		char *t = new char[1 + strlen(s)];
		if (!t) return;

		delete [] m_buf;
		m_buflen = 1 + strlen(s);
		m_buf = t;
	}
	strcpy(m_buf, s);
	/* if (m_curpos > (int)strlen(s)) */m_curpos = strlen(s);
	redrawText();
	updateBind();
}

char *VideoTextEdit::getText(void)
{
	return m_buf;
}


void VideoTextEdit::insertChar(char c)
{
	// Swallow characters that exceed buffer size
	if (m_limit >= 0 && (int)strlen(m_buf) >= m_limit) return;

	if (strlen(m_buf) + 1 >= m_buflen)
	{
		char *t = new char[2 * m_buflen];
		if (!t) return;
		strcpy(t, m_buf);
		delete [] m_buf;
		m_buf = t;
		m_buflen *= 2;
	}

	memmove(m_buf + m_curpos + 1, m_buf + m_curpos, 
			strlen(m_buf) - m_curpos + 1);

	m_buf[m_curpos++] = c;
	redrawText();
	updateBind();
}





int VideoTextEdit::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_ENTER:	m_ctrlQ = false; return ZXE_OK;
		case ZXK_BREAK:	m_ctrlQ = false; return ZXE_CANCEL;
		case ZXK_LDEL:
			if (m_curpos) 
			{
				int l = strlen(m_buf) - m_curpos;
				drawCursor(0);
				memmove(m_buf + m_curpos - 1, m_buf + m_curpos,
					l + 1);
				--m_curpos;
				redrawText();
				updateBind();
			}
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		case ZXK_RDEL:
		case 127:	// Forward delete
			if (m_curpos < (int)strlen(m_buf))
			{
				int l = strlen(m_buf) - m_curpos;
				memmove(m_buf + m_curpos, m_buf + m_curpos + 1,
					l);
				redrawText();
				updateBind();
			}
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		case ZXK_ZAP:
			drawCursor(0);
			m_buf[0] = 0;
			m_curpos = 0;
			redrawText();
			updateBind();
			m_ctrlQ = false;
			return ZXE_CONTINUE;	
		case ZXK_RIGHT:
			if (m_ctrlQ)
			{
				drawCursor(0);
				m_curpos = strlen(m_buf);
				redrawText();
			}
			else if (m_buf[m_curpos]) 
			{
				drawCursor(0);
				++m_curpos;
				redrawText();
				updateBind();
			}
			m_ctrlQ = false;
			return ZXE_CONTINUE;

		case ZXK_LEFT:
			if (m_ctrlQ)
			{
				drawCursor(0);
				m_curpos = 0;
				redrawText();

			}
			else if (m_curpos > 0) 
			{
				drawCursor(0);
				--m_curpos;
				redrawText();
			}
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		case ZXK_POUND:			// Pound sign
			insertChar(ZXCH_POUND);
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		case ZXK_COPY:			// Copyright
			insertChar(ZXCH_COPY);	// ZX Copyright
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		case ZXK_SHIFTSPACE:
			insertChar(' ');	// Treat shifted space
			m_ctrlQ = false;	// just like space
			return ZXE_CONTINUE;
	}
	if (m_numeric)
	{
		if (isdigit(keysym))
		{
			insertChar(keysym);
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		}
	}
	else if (m_hexOnly)
	{
		if ((keysym >= '0' && keysym <= '9') ||
		    (keysym >= 'A' && keysym <= 'F') ||
		    (keysym >= 'a' && keysym <= 'f')) 
		{
			insertChar(keysym);
			m_ctrlQ = false;
			return ZXE_CONTINUE;
		}
	}
	else if (keysym >= ' ' && keysym <= 255) 
	{	
		insertChar(keysym);
		m_ctrlQ = false;
		return ZXE_CONTINUE;
	}
	m_ctrlQ = (keysym == ZXK_CTRLQ);
	return VideoControl::onKeyDown(keysym);
}


void VideoTextEdit::onTick(void)
{
	++m_tcount;
	drawCursor();
	if (m_ot) m_ot->onTick();
}

void VideoTextEdit::drawCursor(int draw)
{
	if (!m_visible) return;

	int cfg = ZX_BRBLUE;
	int cbg = ZX_BRWHITE;
	int vcurpos;
	char c;

        unsigned int cw = (m_w - 4) / CHAR_W;
        unsigned int cl = 0;

        if (cw < strlen(m_buf) + 1)
        {
                cl = 1;
        }

	if (m_tcount & 16) 	 { int a = cfg; cfg = cbg; cbg = a; }
	if (!m_focused || !draw) { cfg = ZX_BLACK; cbg = ZX_WHITE;  }

	c = m_buf[m_curpos]; 
	if (!c) c = ' ';
	if (c == ZXCH_POUND) c = ZXK_POUND;
	if (c == ZXCH_COPY)  c = ZXK_COPY; 

	if (cl)
	{
                int from;

                from = m_curpos - (cw / 2); if (from < 0) from = 0;
		vcurpos = m_curpos - from; 
	}
	else vcurpos = m_curpos;
	videoScreen->drawText(m_x + 2 + CHAR_W * vcurpos, m_y + 2, c, cfg, cbg);
}

void VideoTextEdit::drawFocus(int focused)
{
	if (!m_visible) return;
	if (focused)
	{
		if (!m_haveTimer)
		{
			m_ot = setTickListener(this);
			m_tcount = 0;
			m_haveTimer = 1;
		}
	}
	else if (m_haveTimer) 
	{
		setTickListener(m_ot);
		m_ot = NULL;
		m_haveTimer = 0;
	}
	

	VideoControl::drawFocus(focused);
	drawCursor();
}
