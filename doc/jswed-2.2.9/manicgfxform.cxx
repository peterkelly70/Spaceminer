
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
#include "manicgfxform.hxx"

ManicGraphicsForm::ManicGraphicsForm(jswByte *room, bool vertis) : VideoForm(CHAR_W + CHAR_SW, CHAR_H,
						    CHAR_W * 37, CHAR_H * 25)
{
	int n;

	m_room = room;

	m_ok     = new VideoButton(ZXE_OK,     30 * CHAR_W, 24 * CHAR_H, 
						"  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 23 * CHAR_W, 24 * CHAR_H, 
                     				"Cancel");
	m_edit[0] = new BitmapEditor(CHAR_W * 3,  3 * CHAR_H, 16, 16, 1);  
	if (!vertis)
	{
		m_edit[1] = new BitmapEditor(CHAR_W * 20, 3 * CHAR_H, 16, 16, 1);  
	}
	else m_edit[1] = NULL;
}

ManicGraphicsForm::~ManicGraphicsForm()
{
	if (m_ok) delete m_ok;
	if (m_cancel) delete m_cancel;
	if (m_edit[0]) delete m_edit[0];
	if (m_edit[1]) delete m_edit[1];
}

void ManicGraphicsForm::redraw(void)
{
	VideoForm::redraw();
/*
	videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_H, "Air", ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW +  9*CHAR_W, m_y +      CHAR_H, "Water",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 18*CHAR_W, m_y +      CHAR_H, "Earth",    ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 27*CHAR_W, m_y +      CHAR_H, "Fire",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW,             m_y + 13 * CHAR_H, "Ramp",     ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW +  9*CHAR_W, m_y + 13 * CHAR_H, "Conveyor", ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(m_x + CHAR_SW + 18*CHAR_W, m_y + 13 * CHAR_H, "Item",     ZX_BLACK, ZX_WHITE);	
*/	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Room 16x16 graphics      "
                              "            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int ManicGraphicsForm::doModal()
{
	return VideoForm::doModal();
}

void ManicGraphicsForm::showChildren(int redraw)
{
        int n;
        for (n = 0; n < 2; n++) 
	{
		if (m_edit[n]) addChild(m_edit[n]);
	}
        if (m_edit[0]) m_edit[0]->setBitmap(m_room + 656);
        if (m_edit[1]) m_edit[1]->setBitmap(m_room + 736);

        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void ManicGraphicsForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
        for (n = 0; n < 2; n++) 
	{
		if (m_edit[n]) removeChild(m_edit[n]);
	}
        removeChild(m_ok);
        removeChild(m_cancel);

}

