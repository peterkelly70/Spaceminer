
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
#include "j48udgsform.hxx"

Jsw48UdgsForm::Jsw48UdgsForm(jswByte *room) : VideoForm(CHAR_W + CHAR_SW, CHAR_H,
						    CHAR_W * 37, CHAR_H * 25)
{
	int n;

	m_room = room;

	m_ok     = new VideoButton(ZXE_OK,     30 * CHAR_W, 21 * CHAR_H, 
						"  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 30 * CHAR_W, 23 * CHAR_H, 
                     				"Cancel");
	for (n = 0; n < 4; n++)
		m_edit[n] = new BitmapEditor(CHAR_W * ((n * 9)+2), 3 * CHAR_H,
				8, 8, 1);  
	for (n = 0; n < 3; n++)
		m_edit[n+4] = new BitmapEditor(CHAR_W * ((n * 9)+2), 15*CHAR_H,
				8, 8, 1);
}

Jsw48UdgsForm::~Jsw48UdgsForm()
{
	if (m_ok) delete m_ok;
	if (m_cancel) delete m_cancel;
}

void Jsw48UdgsForm::redraw(void)
{
	VideoForm::redraw();
	videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_H, "Air", ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW +  9*CHAR_W, m_y +      CHAR_H, "Water",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 18*CHAR_W, m_y +      CHAR_H, "Earth",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 27*CHAR_W, m_y +      CHAR_H, "Fire",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW,             m_y + 13 * CHAR_H, "Ramp",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW +  9*CHAR_W, m_y + 13 * CHAR_H, "Conveyor", ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 18*CHAR_W, m_y + 13 * CHAR_H, "Item",     ZX_BLACK, ZX_WHITE);	
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room graphics            "
                              "            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int Jsw48UdgsForm::doModal()
{
	return VideoForm::doModal();
}

void Jsw48UdgsForm::showChildren(int redraw)
{
        int n;
        for (n = 0; n < 7; n++) addChild(m_edit[n]);

        m_edit[0]->setBitmap(m_room + 0xA1);
        m_edit[1]->setBitmap(m_room + 0xAA);
        m_edit[2]->setBitmap(m_room + 0xB3);
        m_edit[3]->setBitmap(m_room + 0xBC);
        m_edit[4]->setBitmap(m_room + 0xC5);
        m_edit[5]->setBitmap(m_room + 0xCE);
        m_edit[6]->setBitmap(m_room + 0xE1);

        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void Jsw48UdgsForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
        for (n = 0; n < 7; n++) removeChild(m_edit[n]);

        removeChild(m_ok);
        removeChild(m_cancel);

}

