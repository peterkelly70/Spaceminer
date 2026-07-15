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
#include "manicgame.hxx"
#include "manicroomform.hxx"
#include "spritelist.hxx"

ManicRoomForm::ManicRoomForm(int roomno, ManicGame *g, jswByte *room) : 
		VideoForm( 3 * CHAR_W, 10 * CHAR_H, 34 * CHAR_W, 12 * CHAR_H)
{
	m_roomno = roomno;
	m_room = room;
	m_game = g;
	m_edit = new VideoTextEdit(3 * CHAR_W + CHAR_SW, 13 * CHAR_H, 
				  33 * CHAR_W + 4, CHAR_H + 4);
	m_edit->setLimit(32);
	m_edit->bindFixed((char *)room + 512);

	m_cc = new VideoColourChooser(10 * CHAR_W + CHAR_SW, 16 * CHAR_H);
	m_cc->setColour(room[627] & 7);

	m_lbl[0] = new VideoLabel(3 * CHAR_W + CHAR_SW, 16 * CHAR_H, "Border");
	m_lbl[1] = new VideoLabel(3 * CHAR_W + CHAR_SW, 18 * CHAR_H, "Air");

        m_spinAir = new VideoSpinControl(10 * CHAR_W + CHAR_SW,
                                       18 * CHAR_H,
                                       7 * CHAR_W, CHAR_H + 4);

	m_spinAir->setLimits(0, 161);
//        m_spinAir->setListener(this);
	m_air = (room[700] - 37) * 6;
	if      (room[701] & 4)  m_air += 5;
	else if (room[701] & 8)  m_air += 4;
	else if (room[701] & 16) m_air += 3;
	else if (room[701] & 32) m_air += 2;
	else if (room[701] & 64) m_air += 1;
	m_spinAir->bind(&m_air);

	m_ok = new VideoButton(ZXE_OK, 30 * CHAR_W, 20 * CHAR_H, 
		     "  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 22 * CHAR_W, 20 * CHAR_H, 
                     "Cancel");
}



ManicRoomForm::~ManicRoomForm()
{
	delete m_lbl[0];
	delete m_lbl[1];
	delete m_edit;
	delete m_spinAir;
	delete m_cc;
	delete m_ok;
	delete m_cancel;
}

void ManicRoomForm::redraw(void)
{
	VideoForm::redraw();
	
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room properties          "
                              "         ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int ManicRoomForm::doModal()
{
	int r = VideoForm::doModal();

	if (r == ZXE_OK)
	{
		m_room[627] = (m_room[627] & (~7)) | (m_cc->getColour());

		if (m_air > 161) m_air = 161;
		m_room[700] = 37 + (m_air / 6);
		switch(m_air % 6)
		{
			case 0: m_room[701] = 0x80; break;
			case 1: m_room[701] = 0xC0; break;
			case 2: m_room[701] = 0xE0; break;
			case 3: m_room[701] = 0xF0; break;
			case 4: m_room[701] = 0xF8; break;
			case 5: m_room[701] = 0xFC; break;
		}
	}
	return r;
}


void ManicRoomForm::showChildren(int redraw)
{
	// Order does matter - it dictates tabbing order 
        addChild(m_edit);
	addChild(m_cc);
	addChild(m_spinAir);
	addChild(m_ok);
        addChild(m_cancel);
	addChild(m_lbl[0]);
	addChild(m_lbl[1]);
	VideoForm::showChildren(redraw);
}

void ManicRoomForm::hideChildren(void)
{
	VideoForm::hideChildren();
        removeChild(m_edit);
        removeChild(m_ok);
        removeChild(m_cancel);
	removeChild(m_cc);
	removeChild(m_lbl[0]);
	removeChild(m_lbl[1]);
	removeChild(m_spinAir);
}

