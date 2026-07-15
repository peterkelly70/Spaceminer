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
#include "hexedit.hxx"

HexEditor::HexEditor(jswByte *data, int count) : VideoForm( 3 * CHAR_W, 13 * CHAR_H,
						34 * CHAR_W,  6 * CHAR_H)
{
	if (count > 8) count = 8;
	m_count = count;
	m_gdata = data;
	for (int n = 0; n < m_count; n++)
	{
		char buf[3];
		sprintf(buf, "%02x", data[n]);

		m_edit[n] = new VideoTextEdit(m_x + (4*n+1)*CHAR_W,
					      m_y + 2 * CHAR_H,
					      3*CHAR_W+CHAR_SW, 4 + CHAR_H);
		m_edit[n]->setLimit(2);
		m_edit[n]->setText(buf);
	}
	m_ok     = new VideoButton(ZXE_OK, m_x + CHAR_W, m_y + 4 * CHAR_H, "  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 * CHAR_W,
					       m_y + 4 * CHAR_H, "Cancel");

}


HexEditor::~HexEditor()
{
	delete m_ok;
	delete m_cancel;
	for (int n = 0; n < m_count; n++) delete m_edit[n];	
}

int HexEditor::doModal(void)
{
	int rv = VideoForm::doModal();
	if (rv != ZXE_OK) return rv;

	for (int n = 0; n < m_count; n++)
	{
		int hex;

		if (sscanf(m_edit[n]->getText(), "%x", &hex)) m_gdata[n] = hex;
	}
	return rv;
}

void HexEditor::redraw(void)
{
        VideoForm::redraw();
 
        videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
        videoScreen->drawText(m_x, m_y, "Edit guardian bytes"
                              "               ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);                       
}

void HexEditor::hideChildren(void)
{
        for (int n = 0; n < m_count; n++) removeChild(m_edit[n]);
        removeChild(m_ok);
        removeChild(m_cancel);     
	VideoForm::hideChildren();
}

void HexEditor::showChildren(int redraw)
{
	for (int n = 0; n < m_count; n++) addChild(m_edit[n]);
	addChild(m_ok);
	addChild(m_cancel);	
	VideoForm::showChildren(redraw);
}


