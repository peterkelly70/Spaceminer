/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2005  John Elliott <jce@seasip.demon.co.uk>

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

class JumpRoomForm : public VideoForm
{
protected:
	VideoTextEdit *m_edit;
	VideoLabel *m_label;
	VideoButton *m_ok, *m_cancel;
	int m_number;
public:
	inline int getNumber() { return m_number; }

	JumpRoomForm() : VideoForm(10 * CHAR_W, 10 * CHAR_H,
			20 * CHAR_W, 6 * CHAR_H)
	{
		m_number = 0;
		m_label = new VideoLabel(m_x + CHAR_W, m_y + 3 * CHAR_SH,
				"Room no.");
		m_edit = new VideoTextEdit(m_x + 10 * CHAR_W, 
				m_y + 3 * CHAR_SH, 4 * CHAR_W + CHAR_W, 
				4 + CHAR_H);
		m_edit->setLimit(3);
		m_edit->setText("");
		m_edit->setNumeric();
		m_ok = new VideoButton(ZXE_OK, m_x + CHAR_W, m_y + 4 * CHAR_H,
			"  OK  ");
		m_cancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 *CHAR_W,
			m_y + 4 * CHAR_H, "Cancel");
	}

	~JumpRoomForm()
	{
		delete m_label;
		delete m_edit;
		delete m_ok;
		delete m_cancel;
	}

	virtual int doModal()
	{
		int rv = VideoForm::doModal();
		if (rv != ZXE_OK) return rv;
		if (!sscanf(m_edit->getText(), "%d", &m_number))
			return ZXE_CANCEL;
		return ZXE_OK;
	}

	virtual void redraw()
	{
		VideoForm::redraw();
		redrawBorder("Go to");
	}
protected:
	virtual void hideChildren()
	{
		removeChild(m_label);
		removeChild(m_edit);
		removeChild(m_ok);
		removeChild(m_cancel);
		VideoForm::hideChildren();
	}

	virtual void showChildren(int redraw)
	{
		addChild(m_edit);
		addChild(m_label);
		addChild(m_ok);
		addChild(m_cancel);
		VideoForm::showChildren(redraw);
	}
};


int jumpRoomNumber(int *number)
{
	JumpRoomForm jrf;

	int rv = jrf.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_OK) *number = jrf.getNumber();
	else		  *number = -1;
	return ZXE_CONTINUE;
}

