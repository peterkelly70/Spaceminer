/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "jswgame.hxx"
#include "j64fbarrierform.hxx"

static const unsigned st_pointers[] =
{
	0xC000, 0xD000,	// 1 => Title top
	0xC800, 0xD100,	// 2 => Title bottom
	0xE2FF, 0xEAFF, // 3 => Buffer 1
	0xEBFF, 0xF3FF, // 4 => Buffer 2
	0xC000, 0xD200, // 5 => Title top, alt attrs
	0xC800, 0xD300, // 6 => Title bottom, alt attrs
	0xC000, 0xD400, // 7 => Title top, alt attrs 2
};

Jsw64BarrierForm::Jsw64BarrierForm(jswByte *room, JswGame *g, VideoBitmap *bg)
	 : VideoForm(CHAR_W * 3, CHAR_H, CHAR_W * 34, CHAR_H * 28)
{
	jswByte *bits, *attrs;

	m_room   = room;
	m_bmBg   = bg;

	m_bnOk = new VideoButton(ZXE_OK,  m_x +      CHAR_W, 
                                     m_y + m_h - 2 * CHAR_H, "  OK  ");
	m_bnCancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 * CHAR_W,
                                     m_y + m_h - 2 * CHAR_H, "Cancel");

	m_list[0] = new ScrollingList(m_x + CHAR_W, m_y + 18 * CHAR_H, 
					15 * CHAR_W, 7 * CHAR_H);
	m_list[1] = new ScrollingList(m_x + 18*CHAR_W, m_y + 18 * CHAR_H, 
					15 * CHAR_W, 7 * CHAR_H);

	m_list[0]->setTitle("Top");
	m_list[1]->setTitle("Bottom");
	m_list[0]->addString("Normal");
	m_list[0]->addString("Title, top");
	m_list[0]->addString("Title, bottom");
	m_list[0]->addString("Buffer 1");
	m_list[0]->addString("Buffer 2");
	m_list[1]->addString("Normal");
	m_list[1]->addString("Title, top");
	m_list[1]->addString("Title, bottom");
	m_list[1]->addString("Buffer 1");
	m_list[1]->addString("Buffer 2");
	int top = (m_room[0xD6] >> 2) & 7;
	int btm = (m_room[0xD6] >> 5) & 7;
	m_list[0]->setSelected(top);
	m_list[1]->setSelected(btm);

	m_list[0]->setSelectAction(ZXE_CONTINUE);
	m_list[1]->setSelectAction(ZXE_CONTINUE);
	m_list[0]->requireClick(true);
	m_list[1]->requireClick(true);
	m_list[0]->setListener(this);
	m_list[1]->setListener(this);
	int n, x, y;
	for (n = 0; n < 7; n++)
	{
		bits  = g->getMem()->memoryAt(st_pointers[2*n  ], 7);
		attrs = g->getMem()->memoryAt(st_pointers[2*n+1], 7);
		m_bmTile[n] = bitmapFromScreen(64, bits, 1, 
				ZX_BRORANGE, ZX_ORANGE);
		for(y = 0; y < 8; y++) for (x = 0; x < 32; x++)
		{
			int fg = (attrs[y*32+x] & 7);
			int bg = (attrs[y*32+x] >> 3) & 7;
			if (attrs[y*32+x] & 64)
			{
				fg |= ZX_BRIGHT;
				bg |= ZX_BRIGHT;
			}
			m_bmTile[n]->getSurface()->fillReplace(x * 16, y * 16, 16, 16,
						ZX_ORANGE, bg);
			m_bmTile[n]->getSurface()->fillReplace(x * 16, y * 16, 16, 16,
						ZX_BRORANGE, fg);
		}
	}
}


Jsw64BarrierForm::~Jsw64BarrierForm()
{
	int n;
	for (n = 0; n < 7; n++)
	{
		delete m_bmTile[n];
	}
	delete m_bnOk;
	delete m_bnCancel;
	delete m_list[0];
	delete m_list[1];
}

void Jsw64BarrierForm::redraw()
{
        videoScreen->fillBox(m_x, m_y, m_w, CHAR_H, ZX_WHITE);
        videoScreen->fillBox(m_x, m_y, CHAR_W, m_h, ZX_WHITE);
        videoScreen->fillBox(m_x + m_w - CHAR_W, m_y, CHAR_W, m_h, ZX_WHITE);
        videoScreen->fillBox(m_x, m_y + 17 * CHAR_H, m_w, m_h - 17 * CHAR_H, ZX_WHITE); 
        videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
        for (int n = 0; n < m_ccount; n++)
        {
                m_controls[n]->redraw();
                m_controls[n]->drawFocus(n == m_focus);
        }
	drawData();
}


void Jsw64BarrierForm::drawData()
{
	int top = (m_room[0xD6] >> 2) & 7;
	int btm = (m_room[0xD6] >> 5) & 7;

	if (m_bmBg) m_bmBg->toScreen(m_x + CHAR_W, m_y + CHAR_H);

	if (top) m_bmTile[top - 1]->toScreen(m_x + CHAR_W, m_y + CHAR_H);
	if (btm) m_bmTile[btm - 1]->toScreen(m_x + CHAR_W, m_y + 9 * CHAR_H);

}

void Jsw64BarrierForm::showChildren(int redraw)
{
	addChild(m_list[0]);
	addChild(m_list[1]);
	addChild(m_bnOk);
	addChild(m_bnCancel);
	VideoForm::showChildren(redraw);
}

void Jsw64BarrierForm::hideChildren(void)
{
	addChild(m_list[0]);
	addChild(m_list[1]);
	removeChild(m_bnOk);
	removeChild(m_bnCancel);
	VideoForm::hideChildren();
}


void Jsw64BarrierForm::onSelect(ScrollingList *list, int nSel)
{
	if (list == m_list[0])
	{
		m_room[0xD6] &= 0xE3;
		m_room[0xD6] |= (nSel << 2);
		drawData();
	}
	if (list == m_list[1])
	{
		m_room[0xD6] &= 0x1F;
		m_room[0xD6] |= (nSel << 5);
		drawData();
	}
}

