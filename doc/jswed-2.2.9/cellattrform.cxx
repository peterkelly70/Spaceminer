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
#include "cellattrform.hxx"

CellAttrForm::CellAttrForm(jswByte *attr) : VideoForm(10 * CHAR_W + CHAR_SW, CHAR_H,
						    CHAR_W * 20, CHAR_H * 14)
{
	int n, x, y;
	m_attr = attr;

	m_cancel = new VideoButton(ZXE_CANCEL, m_x + CHAR_W, 13 * CHAR_H, "Cancel");
	m_ok     = new VideoButton(ZXE_OK,     m_x + 12 * CHAR_W, 13 * CHAR_H, "  OK  ");

	x = m_x + 6 * CHAR_W;
	y = m_y + 2 * CHAR_H + CHAR_SH;
	m_ink    = new VideoColourChooser(x, y + 1 * CHAR_H);
	m_paper  = new VideoColourChooser(x, y + 3 * CHAR_H);
	m_bright = new VideoCheckbox(x + 7 * CHAR_W, y + 5 * CHAR_H);
	m_flash  = new VideoCheckbox(x + 7 * CHAR_W, y + 7 * CHAR_H);
}

CellAttrForm::~CellAttrForm()
{
	delete m_ink;
	delete m_paper;
	delete m_bright;
	delete m_flash;
	delete m_ok;
	delete m_cancel;
}

void CellAttrForm::redraw(void)
{
	VideoForm::redraw();
	
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 3 * CHAR_H, "Ink", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 5 * CHAR_H, "Paper", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 7 * CHAR_H, "Bright", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 9 * CHAR_H, "Flash", ZX_BLACK, ZX_WHITE);

	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Custom cell         ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
}



int CellAttrForm::doModal()
{
	return VideoForm::doModal();
}


void CellAttrForm::showChildren(int redraw)
{
        int n;
	addChild(m_ink);
	addChild(m_paper);
	addChild(m_bright);
	addChild(m_flash);
	m_ink   ->setColour (m_attr[0] & 7);       
	m_paper ->setColour((m_attr[0] >> 3) & 7); 
	m_bright->setCheck  (m_attr[0] & 64); 
	m_flash ->setCheck  (m_attr[0] & 128);
        addChild(m_ok);
        addChild(m_cancel);
	VideoForm::showChildren(redraw);
}


void CellAttrForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();

	*m_attr = m_ink   ->getColour()          | 
		(m_paper ->getColour() << 3)     | 
		(m_bright->getCheck() ?  64 : 0) | 
		(m_flash ->getCheck() ? 128 : 0);

	removeChild(m_ink);
	removeChild(m_paper);
	removeChild(m_bright);
	removeChild(m_flash);

        removeChild(m_ok);
        removeChild(m_cancel);

}


