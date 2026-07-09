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

class Jsw64RoomForm: public VideoForm, ButtonSelectListener
{
protected:
	VideoTextEdit *m_edit;
	VideoButton   *m_ok, *m_cancel, *m_btnSprite;
	VideoBitmap   *m_bmpSprite;
	VideoColourChooser *m_cc[4];
	VideoCheckbox      *m_check[9];
	VideoLabel    *m_lbl[12];
	VideoSpinControl *m_spinAir;
	JswGame       *m_game;
	jswByte 	*m_room;
	jswByte		m_air;
	int		m_override, m_roomno;
	jswByte m_wsprite;
public:
	Jsw64RoomForm(int roomno, JswGame *g, jswByte *room);
	virtual ~Jsw64RoomForm();
	virtual int doModal();
        virtual void redraw(void);
        virtual int onButtonSelect(VideoButton *b);
protected:
	virtual void hideChildren(void);
	virtual void showChildren(int redraw = 1);
};


