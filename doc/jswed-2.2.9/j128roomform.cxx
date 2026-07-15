
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
#include "jswgame.hxx"
#include "j128roomform.hxx"
#include "spritelist.hxx"

Jsw128RoomForm::Jsw128RoomForm(int roomno, JswGame *g, jswByte *room) : 
		VideoForm( 3 * CHAR_W, 10 * CHAR_H, 34 * CHAR_W, 12 * CHAR_H)
{
	m_roomno = roomno;
	m_room = room;
	m_game = g;
	m_edit = new VideoTextEdit(3 * CHAR_W + CHAR_SW, 13 * CHAR_H, 
				  33 * CHAR_W + 4, CHAR_H + 4);
	m_edit->setLimit(32);
	m_edit->bindFixed((char *)room + 128);

	m_cc = new VideoColourChooser(10 * CHAR_W + CHAR_SW, 16 * CHAR_H);
	m_cc->setColour(room[0xDE] & 7);

	m_check = new VideoCheckbox(30 * CHAR_W, 16 * CHAR_H);
	if (g->allowSuperJump())
	{
		m_check->setCheck(room[0xDE] & 0x80);
	}
	else room[0xDE] &= 0x7F;

	m_override = g->allowSpriteOverride();
	m_wsprite  = g->decodeWillySprite(m_roomno, room[m_override]);

	m_lbl[0] = new VideoLabel(3 * CHAR_W + CHAR_SW, 16 * CHAR_H, "Border");
	m_lbl[1] = new VideoLabel(20 * CHAR_W + CHAR_SW, 16 * CHAR_H, "Superjump");

	m_ok = new VideoButton(ZXE_OK, 30 * CHAR_W, 20 * CHAR_H, 
		     "  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 22 * CHAR_W, 20 * CHAR_H, 
                     "Cancel");

	m_btnSprite = new VideoButton(-1, 28 * CHAR_W, 18 * CHAR_H, " Change ");
	m_btnSprite->setListener(this);

        m_bmpSprite = bitmapFromSprite(16, 16, g->getMem()->memoryAt(256 * m_wsprite),
					1, ZX_BRWHITE, ZX_BLACK);

}

Jsw128RoomForm::~Jsw128RoomForm()
{
	delete m_lbl[1];
	delete m_lbl[0];
	delete m_edit;
	delete m_check;
	delete m_cc;
	delete m_ok;
	delete m_cancel;
	delete m_bmpSprite;
	delete m_btnSprite;
}

void Jsw128RoomForm::redraw(void)
{
	VideoForm::redraw();
	
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room properties          "
                              "         ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
	videoScreen->drawText(m_x + CHAR_SW, m_y + 8 * CHAR_H + CHAR_SH, "Willy's sprite: ", ZX_BLACK, ZX_WHITE);
	m_bmpSprite->toScreen(m_x + CHAR_SW + 15 * CHAR_W, m_y + 8 * CHAR_H);
}



int Jsw128RoomForm::doModal()
{
	int r = VideoForm::doModal();

	if (r == ZXE_OK)
	{
		m_room[0xDE] = (m_room[0xDE] & (~7)) | (m_cc->getColour());
		if (m_check->getCheck()) m_room[0xDE] |= 0x80;
		else m_room[0xDE] &= 0x7F;

		if (m_override) m_room[m_override] = m_game->encodeWillySprite(m_roomno, m_wsprite);
	}
	return r;
}


void Jsw128RoomForm::showChildren(int redraw)
{
	// Order does matter - it dictates tabbing order 
        addChild(m_edit);
	addChild(m_cc);
       	if (m_game->allowSuperJump())  
	{
		addChild(m_check);
		addChild(m_lbl[1]);
	}
	if (m_override) addChild(m_btnSprite);
	addChild(m_ok);
        addChild(m_cancel);
	addChild(m_lbl[0]);
	VideoForm::showChildren(redraw);
}

void Jsw128RoomForm::hideChildren(void)
{
	VideoForm::hideChildren();
        removeChild(m_edit);
        removeChild(m_ok);
        removeChild(m_cancel);
	removeChild(m_cc);
	if (m_override) removeChild(m_btnSprite);
        if (m_game->allowSuperJump())  
	{
		removeChild(m_lbl[1]);
		removeChild(m_check);
	}
	removeChild(m_lbl[0]);
}


int Jsw128RoomForm::onButtonSelect(VideoButton *b)
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
        m_bmpSprite->toScreen(m_x + CHAR_SW + 15 * CHAR_W, m_y + 8 * CHAR_H);
	return ZXE_CONTINUE;
}

