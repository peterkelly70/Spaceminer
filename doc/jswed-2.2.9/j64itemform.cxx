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
#include "j64itemform.hxx"

typedef char *pchar;

Jsw64ItemForm::Jsw64ItemForm(jswByte *data) : VideoForm(10 * CHAR_W + CHAR_SW, 
		CHAR_H, CHAR_W * 20, CHAR_H * 17)
{
	int n, x, y;

	m_data = data;

	m_cancel = new VideoButton(ZXE_CANCEL, m_x + CHAR_W, 15 * CHAR_H, "Cancel");
	m_ok     = new VideoButton(ZXE_OK,     m_x + 12 * CHAR_W, 15 * CHAR_H, "  OK  ");

	x = m_x + 6 * CHAR_W;
	y = m_y + 2 * CHAR_H + CHAR_SH;
	m_edit   = new BitmapEditor(x, y, 8, 8, 1);
}

Jsw64ItemForm::~Jsw64ItemForm()
{
	delete m_edit;
	delete m_ok;
	delete m_cancel;
}

void Jsw64ItemForm::redraw(void)
{
	VideoForm::redraw();
	
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Item graphic        ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int Jsw64ItemForm::doModal()
{
	return VideoForm::doModal();
}


void Jsw64ItemForm::showChildren(int redraw)
{
        int n;
	addChild(m_edit);
	m_edit->setBitmap(m_data);
        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void Jsw64ItemForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();

	removeChild(m_edit);

        removeChild(m_ok);
        removeChild(m_cancel);

}


