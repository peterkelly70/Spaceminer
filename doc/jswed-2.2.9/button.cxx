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

VideoButton::VideoButton(int id, int x, int y, int w, int h) : 
	VideoControl(x,y,w,h)
{
	m_id = id;
        m_buf = new char[256];
        m_buflen = 256;
        m_buf[0] = 0;
	m_listener = NULL;
}

VideoButton::VideoButton(int id, int x, int y, char *s) :
        VideoControl(x, y, 4 + CHAR_W  * strlen(s), 4 + CHAR_H)
{
	m_id = id;
        m_buflen  = 1 + strlen(s);
        m_buf = new char[m_buflen];
        strcpy(m_buf, s);
	m_listener = NULL;
}

VideoButton::~VideoButton()
{
	delete [] m_buf;
}

// Overrides
void VideoButton::redraw(void)
{
        unsigned int cw = (m_w - 4) / CHAR_W;

        videoScreen->fillBox (m_x + 1, m_y + 1, m_w - 2,  m_h - 2, ZX_BRWHITE);

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
        char *nbuf = new char[1 + strlen(m_buf)];

        if (!nbuf) return;
        strcpy(nbuf, m_buf);
        for (unsigned int n = 0; n < cw; n++)
        {
                if (nbuf[n] == ZXCH_COPY)  nbuf[n] = ZXK_COPY;
                if (nbuf[n] == ZXCH_POUND) nbuf[n] = ZXK_POUND;
        }

        videoScreen->drawText(m_x + 2, m_y + 2, nbuf, ZX_BLACK, ZX_BRWHITE);
        delete [] nbuf;
}

int VideoButton::onKeyDown(int keysym)
{
	if (keysym == ZXK_ENTER || keysym == ' ' ||
		(keysym == ZXK_BREAK && m_id == ZXE_CANCEL) ) 
	{
		if (m_listener) return m_listener->onButtonSelect(this);
		else            return m_id;
	}
	return ZXE_CONTINUE;
}


int VideoButton::onButtonDown(int x, int y, int button)
{
	if (x >= m_x && x < (m_x + m_w) &&
            y >= m_y && y < (m_y + m_h)) 
	{
		if (m_listener) return m_listener->onButtonSelect(this);
		else return m_id;
	}
	return ZXE_CONTINUE;
}



void VideoButton::setText(char *s)
{
        if (strlen(s) >= m_buflen)
        {
                char *t = new char[1 + strlen(s)];
                if (!t) return;

                delete m_buf;
                m_buflen = 1 + strlen(s);
                m_buf = t;
        }
        strcpy(m_buf, s);
        redraw();
}

char *VideoButton::getText(void)
{
        return m_buf;
}

