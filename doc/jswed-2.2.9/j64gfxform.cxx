
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
#include "spritelist.hxx"
#include "j64gfxform.hxx"
#include "game.hxx"
#include "room.hxx"
#include "jswgame.hxx"
#include "j64game.hxx"

Jsw64GraphicsForm::Jsw64GraphicsForm(Jsw64Game *g, Room *rm, jswByte *room) :
	VideoForm(CHAR_W + CHAR_SW, CHAR_H, CHAR_W * 37, CHAR_H * 29)
{
	int n;
	int sprites[256];

	m_game = g;
	m_room = room;
	if (g->getVariant() <= 'W') m_max = 0x8200;
	else			    m_max = 0x8400;

	rm->getSprites(sprites, 32);
	memcpy(g->getMem()->memoryAt(0x8000), room, m_max - 0x8000); 

	m_ok     = new VideoButton(ZXE_OK,     30 * CHAR_W, 28 * CHAR_H, 
						"  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 23 * CHAR_W, 28 * CHAR_H, 
                     				"Cancel");
	m_edit = new BitmapEditor(CHAR_W * 3,  3 * CHAR_H, 16, 16, 1);  
	m_list = new SpriteList(g->getMem(), m_x + 6, 23 * CHAR_H + CHAR_SH, 
				CHAR_W * 37 - 10, CHAR_H + CHAR_SH + 40);

	
	for (n = 0; sprites[n]; n++)
	{
		m_list->addSprite(sprites[n]);
	}
	m_list->setListener(this);
	m_list->setTitle("Room 16x16 graphics");
	m_list->setSelectAction(ZXE_CONTINUE);
	if (sprites[0])
		m_edit->setBitmap(m_game->getMem()->memoryAt(sprites[0]));
}

Jsw64GraphicsForm::~Jsw64GraphicsForm()
{
	if (m_ok) delete m_ok;
	if (m_cancel) delete m_cancel;
	if (m_edit) delete m_edit;
	if (m_list) delete m_list;
}

void Jsw64GraphicsForm::redraw(void)
{
	VideoForm::redraw();

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room 16x16 graphics      "
                              "            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int Jsw64GraphicsForm::doModal()
{
	int rv = VideoForm::doModal();

	memcpy(m_room, m_game->getMem()->memoryAt(0x8000), m_max - 0x8000); 
	return rv;
}

void Jsw64GraphicsForm::showChildren(int redraw)
{
        int n;
	addChild(m_edit);
	addChild(m_list);
        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void Jsw64GraphicsForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
        removeChild(m_edit);
        removeChild(m_list);
        removeChild(m_ok);
        removeChild(m_cancel);
}


void Jsw64GraphicsForm::onChangeSprite(jswByte *newBitmap)
{
        m_edit->setBitmap(newBitmap);
        memcpy(m_undoBuf, newBitmap, 32);
}
