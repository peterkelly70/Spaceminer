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

VideoSpinControl::VideoSpinControl(int x, int y, int w, int h): 
		VideoControl(x,y,w,h),
                m_te(x + CHAR_W, y, w - 2 * CHAR_W, h)
{
	m_listener = NULL;
	m_te.setLimit(3);
	m_te.setListener(this);
	m_bind  = NULL;
	m_delta = 1;
	m_signed = 0;
	m_ll = -0x200;
	m_ul = 0x200;
}

void VideoSpinControl::setVisible(int v, int r)
{
	m_te.setVisible(v, r);
	VideoControl::setVisible(v, r);
}

VideoSpinControl::~VideoSpinControl()
{
//	updateBind();
// On the contrary....
}


void VideoSpinControl::bind(jswByte *s)
{
	m_bind = s;
	if (s) setValue(*s);
}


void VideoSpinControl::updateBind(void)
{
	int n;

	if (m_bind) 
	{
		n = atoi(m_te.getText());
		if (n <= m_ll) n = m_ll;
		if (n >= m_ul) n = m_ul;
		*m_bind = atoi(m_te.getText());
	}
	if (m_listener) m_listener->onEditChange(this);
}



void VideoSpinControl::onEditChange(VideoControl *v)
{
	if (v == &m_te) updateBind();
}

void VideoSpinControl::redraw(void)
{
	if (!m_visible) return;
	m_te.setVisible(1, 1);
	videoScreen->drawText(m_x, m_y + 2, "-", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(m_x + m_w - CHAR_W,
				   m_y + 2, "+", ZX_BLACK, ZX_BRWHITE);
	
	videoScreen->box(m_x, m_y, CHAR_W, m_h, ZX_BLACK);
	videoScreen->box(m_x + m_w - CHAR_W, m_y, CHAR_W, m_h, ZX_BLACK);
}


int VideoSpinControl::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_ENTER:
		case ZXK_BREAK:
		case ZXK_LDEL:
		case ZXK_RDEL:
		case 127:	
		case ZXK_ZAP:
		case ZXK_RIGHT:
		case ZXK_LEFT:
			return m_te.onKeyDown(keysym);

	}
	if (keysym >= '0' && keysym <= '9') 
	{	
		return m_te.onKeyDown(keysym);
		return ZXE_CONTINUE;
	}
	if (keysym == '+')
	{
		onSpin(1);
		return ZXE_CONTINUE;
	}
        if (keysym == '-')
        {
                onSpin(-1);
                return ZXE_CONTINUE;
        }
	return VideoControl::onKeyDown(keysym);
}


void VideoSpinControl::drawFocus(int focused)
{
	if (!m_visible) return;
	m_te.drawFocus(focused);
	VideoControl::drawFocus(focused);
}


int VideoSpinControl::onButtonUp(int x, int y, int b)
{
	return VideoControl::onButtonUp(x,y,b);
}

int VideoSpinControl::onButtonDown(int x, int y, int b)
{
	if (x >= m_x && x < m_te.getX() ) 
	{
		onSpin(-1);
		return ZXE_CONTINUE;
	}
	if (x >= m_te.getX() + m_te.getW() && x < m_x + m_w)
	{
		onSpin(1);
		return ZXE_CONTINUE;
	}

        return VideoControl::onButtonDown(x,y,b);
}


void VideoSpinControl::onSpin(int up)
{
	int n = getValue();

        n = (n + (up * m_delta)) & 0xFF;

	if (n >= m_ul) n = m_ul;
	if (n <= m_ll) n = m_ll;
        setValue(n);
}


void VideoSpinControl::setDelta(int delta) 
{
	if (delta == m_delta) return;

        int n = getValue();

	n -= (n % delta);
	setValue(n);
	m_delta = delta;
}


int VideoSpinControl::getValue(void)
{
	return atoi(m_te.getText());
}

void VideoSpinControl::setValue(int v)
{
	char buf[5]; 

	v &= 0xFF;
	if (m_signed) v = ((signed char)v);
	sprintf(buf, "%03d", v);
	m_te.setText(buf);
	updateBind();
}


void VideoSpinControl::setLimits(int ll, int ul)
{
	m_ll = ll;
	m_ul = ul;
}

