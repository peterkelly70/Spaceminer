/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004  John Elliott <jce@seasip.demon.co.uk>

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
#include "j64targetform.hxx"
#include "room.hxx"
#include "jswgame.hxx"

typedef char *pchar;

Jsw64TargetForm::Jsw64TargetForm(JswGame *g, Room *dstRoom, jswByte *srcRoom) : 
VideoForm(3 * CHAR_W + CHAR_SW, CHAR_H, CHAR_W * 34, CHAR_H * 21)
{
	int n, x, y;

	int adrw = 256 * g->getWillySpritePage();
	m_bmWilly = ::bitmapFromSprite(16,16, g->getMem()->memoryAt(adrw),
			ZX_WHITE, ZX_BRBLUE);
	m_ticker = 0;
	m_px = m_cx = srcRoom[0xF6] & 0x1F;
	m_py = m_cy = (srcRoom[0xF6] >> 5) | ((srcRoom[0xF7] & 1) << 3);
	m_dstRoom = dstRoom;
	m_srcRoom = srcRoom;

	m_cancel = new VideoButton(ZXE_CANCEL, m_x + CHAR_W, 19 * CHAR_H + CHAR_SH, "Cancel");
	m_ok     = new VideoButton(ZXE_OK,     m_x + 27 * CHAR_W, 19 * CHAR_H + CHAR_SH, "  OK  ");
	m_bmFixed = newVideoBitmap(512, 256);
	m_bmAnim  = newVideoBitmap(512, 256);

	dstRoom->createCellPatterns();
	for (y = 0; y < 16; y++) for (x = 0; x < 32; x++)
        {
		dstRoom->drawCell(m_bmFixed, x, y, false, false);
	}

	dstRoom->drawOneOffs(m_bmFixed, false);
}


Jsw64TargetForm::~Jsw64TargetForm()
{
	delete m_bmFixed;
	delete m_bmAnim;
	delete m_bmWilly;
	delete m_ok;
	delete m_cancel;
	m_bmFixed = NULL;
	m_bmAnim = NULL;
	m_bmWilly = NULL;
}

void Jsw64TargetForm::redraw(void)
{
	char caption[40];

	VideoForm::redraw();
	drawCursor();

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Set Portal destination            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int Jsw64TargetForm::doModal()
{
	int rv;

	TickListener *tl = setTickListener(this);
	rv = VideoForm::doModal();
	setTickListener(tl);
	if (rv == ZXE_OK)
	{
		m_srcRoom[0xF6] = (m_px & 0x1F) | ((m_py << 5) & 0xE0);
		m_srcRoom[0xF7] = 0x5C | ((m_py >> 3) & 1);
		m_srcRoom[0xF8] = (m_py & 0x0F) * 16;
		printf("%d, %d -> %02x %02x %02x\n", m_px, m_py,
				m_srcRoom[0xF6], m_srcRoom[0xF7],
				m_srcRoom[0xF8]);
	}
	return rv;
}


void Jsw64TargetForm::showChildren(int redraw)
{
        int n;
        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void Jsw64TargetForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();

        removeChild(m_ok);
        removeChild(m_cancel);
}



void Jsw64TargetForm::onTick()
{
	++m_ticker;
	drawCursor();	
}

void Jsw64TargetForm::drawCursor()
{
	if (!m_bmFixed) return;

	m_bmFixed->toSurface(m_bmAnim, 0, 0);
	m_bmWilly->toSurface(m_bmAnim, 16 * m_px, 16 * m_py);
        int i = ZX_BRWHITE;
        if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;
	        
        int w = 32, h = 32;
        if (m_cx == 31) w = 16;
        if (m_cy == 15) h = 16;
	
        m_bmAnim->getSurface()->box(m_cx * 16, m_cy * 16, w, h, i);
	m_bmAnim->toScreen(m_x + CHAR_W, m_y + 2 * CHAR_H);
}



int Jsw64TargetForm::onButtonDown(int x, int y, int button)
{
	int x0 = m_x + CHAR_W;
	int y0 = m_y + 2 * CHAR_H;

	if (x >= x0 && y < (x0 + 512) &&
	    y >= y0 && y < (y0 + 256))
	{
		m_cx = (x - x0) / 16;
		m_cy = (y - y0) / 16;
	
		m_px = m_cx;
		m_py = m_cy;
		return ZXE_CONTINUE;
	}
	return VideoForm::onButtonDown(x, y, button);
}



int Jsw64TargetForm::onMouseMove(int x, int y, int xrel, int yrel)
{
	int x0 = m_x + CHAR_W;
	int y0 = m_y + 2 * CHAR_H;

	if (x >= x0 && y < (x0 + 512) &&
	    y >= y0 && y < (y0 + 256))
	{
		m_cx = (x - x0) / 16;
		m_cy = (y - y0) / 16;
	}
	return VideoForm::onMouseMove(x, y, xrel, yrel);
}


int Jsw64TargetForm::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_LEFT:
			if (m_cx > 0) --m_cx;
			return ZXE_CONTINUE;
		case ZXK_RIGHT:
			if (m_cx < 31) ++m_cx;
			return ZXE_CONTINUE;
		case ZXK_UP:
			if (m_cy > 0) --m_cy;
			return ZXE_CONTINUE;
		case ZXK_DOWN:
			if (m_cy < 15) ++m_cy;
			return ZXE_CONTINUE;
		case ZXK_ENTER:
		case ' ':
			m_px = m_cx;
			m_py = m_cy;
			return ZXE_CONTINUE;
	}
	return VideoForm::onKeyDown(keysym);
}
