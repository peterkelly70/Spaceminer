/************************************************************************

    JSWED 2.1.5 - Editor for Jet Set Willy and derivatives

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
#include "j64roomform.hxx"
#include "spritelist.hxx"
#include "jswdirbox.hxx"


Jsw64RoomForm::Jsw64RoomForm(int roomno, JswGame *g, jswByte *room) : 
		VideoForm( 3 * CHAR_W, 4 * CHAR_H, 34 * CHAR_W, 22 * CHAR_H)
{
	m_roomno = roomno;
	m_room = room;
	m_game = g;
	m_edit = new VideoTextEdit(3 * CHAR_W + CHAR_SW, m_y + 2 * CHAR_H, 
				  33 * CHAR_W + 4, CHAR_H + 4);
	m_edit->setLimit(32);
	m_edit->bindFixed((char *)room + 0xB6);

	m_cc[0] = new VideoColourChooser(10 * CHAR_W + CHAR_SW, m_y + 4 * CHAR_H);
	m_cc[1] = new VideoColourChooser(10 * CHAR_W + CHAR_SW, m_y + 6 * CHAR_H);
	m_cc[2] = new VideoColourChooser(10 * CHAR_W + CHAR_SW, m_y + 8 * CHAR_H);
	m_cc[3] = new VideoColourChooser(10 * CHAR_W + CHAR_SW, m_y + 10 * CHAR_H);
	m_cc[0]->setColour(room[0xDE] & 7);
	m_cc[1]->setColour((room[0xDE] >> 3) & 7);
	m_cc[2]->setColour(room[0xDD] & 7);
	m_cc[3]->setColour((room[0xDD] >> 3) & 7);

	m_check[0] = new VideoCheckbox(33 * CHAR_W, m_y + 4 * CHAR_H);
	m_check[1] = new VideoCheckbox(33 * CHAR_W, m_y + 6 * CHAR_H);
	m_check[2] = new VideoCheckbox(33 * CHAR_W, m_y + 8 * CHAR_H);
	m_check[3] = new VideoCheckbox(10 * CHAR_W + CHAR_SW, m_y + 12 * CHAR_H);
	m_check[4] = new VideoCheckbox(17 * CHAR_W + CHAR_SW, m_y + 12 * CHAR_H);
	m_check[5] = new VideoCheckbox(33 * CHAR_W, m_y + 10 * CHAR_H);
	m_check[6] = new VideoCheckbox(33 * CHAR_W, m_y + 12 * CHAR_H);
	m_check[7] = new JswDirCheckbox(35 * CHAR_W, m_y + 12 * CHAR_H);
	m_check[8] = new VideoCheckbox(33 * CHAR_W, m_y + 14 * CHAR_H);
	m_check[0]->setCheck(room[0xDE] & 0x80);
	m_check[1]->setCheck(room[0xDE] & 0x40);
	m_check[2]->setCheck(room[0xFF] & 0x1);
	m_check[3]->setCheck(room[0xDD] & 0x40);
	m_check[4]->setCheck(room[0xDD] & 0x80);
	m_check[6]->setCheck(room[0xFF] & 0x4);
	m_check[7]->setCheck(room[0xFF] & 0x2);
	m_check[8]->setCheck(room[0xFF] & 0x8);
	((JswDirCheckbox *)m_check[7])->setVertical();

	m_override = g->allowSpriteOverride();
	m_wsprite  = g->decodeWillySprite(m_roomno, room[m_override]);

	m_lbl[0] = new VideoLabel(3 * CHAR_W + CHAR_SW, m_y + 4 * CHAR_H, "Border");
	m_lbl[1] = new VideoLabel(3 * CHAR_W + CHAR_SW, m_y + 6 * CHAR_H, "Willy ");
	m_lbl[2] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 4 * CHAR_H, "Superjump   ");
	m_lbl[3] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 6 * CHAR_H, "Rigor Mortis");
	m_lbl[4] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 8 * CHAR_H, "No Kamikaze ");
	m_lbl[5] = new VideoLabel(3 * CHAR_W + CHAR_SW, m_y + 8 * CHAR_H, "Solar ");
	m_lbl[6] = new VideoLabel(3 * CHAR_W + CHAR_SW, m_y + 12 * CHAR_H, "Bright");
	m_lbl[7] = new VideoLabel(11 * CHAR_W + CHAR_SW, m_y + 12 * CHAR_H, "Flash");
	m_lbl[8] = new VideoLabel(3 * CHAR_W + CHAR_SW, m_y + 14 * CHAR_H, "Air");
	m_lbl[9] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 10 * CHAR_H, "Bonus room ");
	m_lbl[10] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 12 * CHAR_H, "Escalators");
	m_lbl[11] = new VideoLabel(20 * CHAR_W + CHAR_SW, m_y + 14 * CHAR_H, "Land safely");

	m_spinAir = new VideoSpinControl(10 * CHAR_W + CHAR_SW,
					m_y + 14 * CHAR_H, 7 * CHAR_W, 4 + CHAR_H);
	m_spinAir->setLimits(0, 162);
	
	if (room[0xE0] == 0xFF) m_air = 162;
	else 
	{
		m_air = (room[0xDF] - 37) * 6;

		if      (room[0xE0] &  4) m_air += 5;
		else if (room[0xE0] &  8) m_air += 4;
		else if (room[0xE0] & 16) m_air += 3;
		else if (room[0xE0] & 32) m_air += 2;
		else if (room[0xE0] & 64) m_air += 1;
	}
	m_spinAir->bind(&m_air);

	m_ok = new VideoButton(ZXE_OK, 30 * CHAR_W, m_y + 20 * CHAR_H, 
		     "  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 22 * CHAR_W, m_y + 20 * CHAR_H, 
                     "Cancel");

	m_btnSprite = new VideoButton(-1, 28 * CHAR_W, m_y + 18 * CHAR_H, " Change ");
	m_btnSprite->setListener(this);

        m_bmpSprite = bitmapFromSprite(16, 16, g->getMem()->memoryAt(256 * m_wsprite),
					1, ZX_BRWHITE, ZX_BLACK);

}

Jsw64RoomForm::~Jsw64RoomForm()
{
	int n;
	
	for (n = 0; n < sizeof(m_lbl  )/sizeof(m_lbl  [0]); n++) 
		delete m_lbl[n];
	for (n = 0; n < sizeof(m_cc   )/sizeof(m_cc   [0]); n++) 
		delete m_cc[n];
	for (n = 0; n < sizeof(m_check)/sizeof(m_check[0]); n++) 
		delete m_check[n];
	delete m_spinAir;
	delete m_edit;
	delete m_ok;
	delete m_cancel;
	delete m_bmpSprite;
	delete m_btnSprite;
}

void Jsw64RoomForm::redraw(void)
{
	VideoForm::redraw();
	
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room properties          "
                              "         ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
	videoScreen->drawText(m_x + 17 * CHAR_W + CHAR_SW, m_y + 16 * CHAR_H, "Willy's sprite: ", ZX_BLACK, ZX_WHITE);
	m_bmpSprite->toScreen(m_x + 17 * CHAR_W + CHAR_SW, m_y + 17 * CHAR_H);
}



int Jsw64RoomForm::doModal()
{
	int r = VideoForm::doModal();

	if (r == ZXE_OK)
	{
		m_room[0xDE] = m_cc[0]->getColour() 
			+  8 * m_cc[1]->getColour()
			+ (m_check[1]->getCheck() ? 64 : 0)
			+ (m_check[0]->getCheck() ? 128 : 0);

		m_room[0xDD] = m_cc[2]->getColour() 
			+  8 * m_cc[3]->getColour()
			+ (m_check[3]->getCheck() ? 64 : 0)
			+ (m_check[4]->getCheck() ? 128 : 0);

		if (m_air >= 162)
		{
			m_room[0xE0] = 0xFF;
			m_room[0xDF] = 0xFF;
		}
		else
		{
			m_room[0xDF] = 37 + (m_air / 6);
			switch(m_air % 6)
			{
				case 0: m_room[0xE0] = 0x80; break;
				case 1: m_room[0xE0] = 0xC0; break;
				case 2: m_room[0xE0] = 0xE0; break;
				case 3: m_room[0xE0] = 0xF0; break;
				case 4: m_room[0xE0] = 0xF8; break;
				case 5: m_room[0xE0] = 0xFC; break;
			}
	
		}

		m_room[0xFF] &= 0x70;
		m_room[0xFF] |= (m_check[2]->getCheck() ? 0x01 : 0);
		m_room[0xFF] |= (m_check[5]->getCheck() ? 0x80 : 0);
		m_room[0xFF] |= (m_check[6]->getCheck() ? 0x04 : 0);
		m_room[0xFF] |= (m_check[7]->getCheck() ? 0x02 : 0);
		m_room[0xFF] |= (m_check[8]->getCheck() ? 0x08 : 0);

		if (m_override) m_room[m_override] = m_game->encodeWillySprite(m_roomno, m_wsprite);
	}
	return r;
}


void Jsw64RoomForm::showChildren(int redraw)
{
	// Order does matter - it dictates tabbing order 
        addChild(m_edit);
	// Left column
	addChild(m_cc[0]);
	addChild(m_lbl[0]);
	addChild(m_cc[1]);
	addChild(m_lbl[1]);
	addChild(m_cc[2]);
	addChild(m_lbl[5]);
	addChild(m_cc[3]);
	addChild(m_lbl[6]);
	addChild(m_check[3]);
	addChild(m_lbl[7]);
	addChild(m_check[4]);
	addChild(m_lbl[8]);
	addChild(m_spinAir);

	// Right column
	addChild(m_check[0]);
	addChild(m_lbl[2]);
	addChild(m_check[1]);
	addChild(m_lbl[3]);
	addChild(m_check[2]);
	addChild(m_lbl[4]);
	addChild(m_check[5]);
	addChild(m_lbl[9]);
	addChild(m_check[6]);
	addChild(m_lbl[10]);
	addChild(m_check[7]);
	addChild(m_check[8]);
	addChild(m_lbl[11]);

	if (m_override) addChild(m_btnSprite);
	addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}

void Jsw64RoomForm::hideChildren(void)
{
	VideoForm::hideChildren();
        removeChild(m_edit);
        removeChild(m_ok);
        removeChild(m_cancel);
	removeChild(m_cc[0]);
	removeChild(m_cc[1]);
	removeChild(m_cc[2]);
	removeChild(m_cc[3]);
	if (m_override) removeChild(m_btnSprite);
	removeChild(m_check[4]);
	removeChild(m_check[3]);
	removeChild(m_spinAir);
	removeChild(m_lbl[8]);

	removeChild(m_check[8]);
	removeChild(m_lbl[11]);
	removeChild(m_check[6]);
	removeChild(m_lbl[10]);
	removeChild(m_check[7]);
	removeChild(m_lbl[5]);
	removeChild(m_lbl[4]);
	removeChild(m_check[0]);
	removeChild(m_lbl[3]);
	removeChild(m_check[1]);
	removeChild(m_lbl[2]);
	removeChild(m_check[2]);
	removeChild(m_check[5]);
	removeChild(m_lbl[9]);

	removeChild(m_lbl[1]);
	removeChild(m_lbl[0]);
}


int Jsw64RoomForm::onButtonSelect(VideoButton *b)
{
	int rv;
	int id = b->getId();

	if (id != -1)    return ZXE_CONTINUE;
	if (!m_override) return ZXE_CONTINUE; 

        SpriteList list(m_game->getMem(), 2, 20 * CHAR_H, VIDEO_W - 4,
                                               CHAR_H + CHAR_SH + 40);
        m_game->fillSprites(m_roomno, &list, 0);
        list.setTitle("Choose sprite for Willy");

	list.setSelected(m_wsprite * 256);

	rv = list.doModal();   
     
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK)   return ZXE_CONTINUE;

	m_wsprite = (list.getSelected() / 256);

        delete m_bmpSprite;
	m_bmpSprite = bitmapFromSprite(16, 16, m_game->getMem()->memoryAt(256 * m_wsprite),
                                        1, ZX_BRWHITE, ZX_BLACK);
	m_bmpSprite->toScreen(m_x + 17 * CHAR_W + CHAR_SW, m_y + 17 * CHAR_H);
	return ZXE_CONTINUE;
}

