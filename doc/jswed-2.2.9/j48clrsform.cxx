
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
#include "j48clrsform.hxx"

Jsw48ClrsForm::Jsw48ClrsForm(jswByte *room) : VideoForm(CHAR_W + CHAR_SW, CHAR_H,
						    CHAR_W * 37, CHAR_H * 25)
{
	int n, x, y;

	m_room = room;

	m_ok     = new VideoButton(ZXE_OK,     29 * CHAR_W, 24 * CHAR_H, 
						"  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 21 * CHAR_W, 24 * CHAR_H, 
                     				"Cancel");
	for (n = 0; n < 6; n++)
	{
		x = CHAR_W * (((n % 3) * 9) + 9);
		y = CHAR_H * 4;

		if (n >= 3) y += 12 * CHAR_H;

		m_ink[n]    = new VideoColourChooser(x, y);
		m_paper[n]  = new VideoColourChooser(x, y + 2 * CHAR_H);
		m_bright[n] = new VideoCheckbox(x + 7 * CHAR_W, y + 4 * CHAR_H);
		m_flash[n]  = new VideoCheckbox(x + 7 * CHAR_W, y + 6 * CHAR_H);
	}	
}

Jsw48ClrsForm::~Jsw48ClrsForm()
{
	for (int n = 0; n < 6; n++) 
	{
		delete m_ink[n];
		delete m_paper[n];
		delete m_bright[n];
		delete m_flash[n];
	}
	if (m_ok) delete m_ok;
	if (m_cancel) delete m_cancel;
}

void Jsw48ClrsForm::redraw(void)
{
	VideoForm::redraw();

	int x = m_x + CHAR_SW;

	videoScreen->drawText(x +  7*CHAR_W, m_y +      CHAR_H, "Air",      ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(x + 16*CHAR_W, m_y +      CHAR_H, "Water",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(x + 25*CHAR_W, m_y +      CHAR_H, "Earth",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(x +  7*CHAR_W, m_y + 13 * CHAR_H, "Fire",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(x + 16*CHAR_W, m_y + 13 * CHAR_H, "Ramp",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(x + 25*CHAR_W, m_y + 13 * CHAR_H, "Conveyor", ZX_BLACK, ZX_WHITE);	

	videoScreen->drawText(x, m_y + 3 * CHAR_H, "   Ink", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 5 * CHAR_H, " Paper", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 7 * CHAR_H, "Bright", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 9 * CHAR_H, " Flash", ZX_BLACK, ZX_WHITE);

        videoScreen->drawText(x, m_y + 15 * CHAR_H, "   Ink", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 17 * CHAR_H, " Paper", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 19 * CHAR_H, "Bright", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(x, m_y + 21 * CHAR_H, " Flash", ZX_BLACK, ZX_WHITE);

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room graphics : Colours  "
                              "            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int Jsw48ClrsForm::doModal()
{
	return VideoForm::doModal();
}

#define SCOL(x,y) \
	m_ink   [x]->setColour (m_room[y] & 7);       \
	m_paper [x]->setColour((m_room[y] >> 3) & 7); \
	m_bright[x]->setCheck  (m_room[y] & 64); \
	m_flash [x]->setCheck  (m_room[y] & 128);

#define GCOL(x,y) m_room[y] = \
	 m_ink   [x]->getColour()           | \
	(m_paper [x]->getColour() << 3)     | \
	(m_bright[x]->getCheck() ?  64 : 0) | \
	(m_flash [x]->getCheck() ? 128 : 0);

void Jsw48ClrsForm::showChildren(int redraw)
{
        int n;
        for (n = 0; n < 6; n++) 
	{
		addChild(m_ink[n]);
		addChild(m_paper[n]);
		addChild(m_bright[n]);
		addChild(m_flash[n]);
	}
	SCOL(0, 0xA0);
	SCOL(1, 0xA9);
	SCOL(2, 0xB2);
	SCOL(3, 0xBB);
	SCOL(4, 0xC4);
	SCOL(5, 0xCD);

        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void Jsw48ClrsForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
        GCOL(0, 0xA0);
        GCOL(1, 0xA9);
        GCOL(2, 0xB2);
        GCOL(3, 0xBB);
        GCOL(4, 0xC4);
        GCOL(5, 0xCD);

	for (n = 0; n < 6; n++) 
	{
		removeChild(m_ink[n]);
		removeChild(m_paper[n]);
		removeChild(m_bright[n]);
		removeChild(m_flash[n]);
	}
        removeChild(m_ok);
        removeChild(m_cancel);

}

