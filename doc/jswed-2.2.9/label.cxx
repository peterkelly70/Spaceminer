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

VideoLabel::VideoLabel(int x, int y, int w, int h): VideoControl(x,y,w,h)
{
	m_buf = new char[256];
	m_buflen = 256;
	m_buf[0] = 0;
}


VideoLabel::VideoLabel(int x, int y, char *s) : 
	VideoControl(x, y, CHAR_W  * strlen(s), CHAR_H)
{
	m_buflen  = 1 + strlen(s);
	m_buf = new char[m_buflen];
	strcpy(m_buf, s);
}


VideoLabel::~VideoLabel()
{
	delete [] m_buf;
}


void VideoLabel::redrawText(void)
{
        unsigned int cw = (m_w - 4) / CHAR_W;

	if (!m_visible) return;

        videoScreen->fillBox (m_x + 1, m_y + 1, m_w - 2,  m_h - 2, ZX_WHITE);
	
	char *nbuf = new char[1 + strlen(m_buf)];

	if (!nbuf) return;
	strcpy(nbuf, m_buf);	
        for (unsigned int n = 0; n < cw; n++)
        {
		if (nbuf[n] == ZXCH_COPY)  nbuf[n] = ZXK_COPY;
                if (nbuf[n] == ZXCH_POUND) nbuf[n] = ZXK_POUND;
	}

	videoScreen->drawText(m_x + 2, m_y + 2, nbuf, ZX_BLACK, ZX_WHITE);
	delete [] nbuf;
}

void VideoLabel::redraw(void)
{
	redrawText();
}

void VideoLabel::setText(char *s)
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
	redrawText();
}

char *VideoLabel::getText(void)
{
	return m_buf;
}


int VideoLabel::canFocus()
{
	return 0;
}
