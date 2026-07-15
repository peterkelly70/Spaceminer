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

class Room;

class Jsw64GuardianTableForm: public VideoForm, public EditChangeListener
{
protected:
	VideoButton	*m_ok, *m_cancel;
	VideoLabel	*m_label;
	VideoSpinControl *m_spinSize;
	Room		*m_room;
	char		m_buf[1025];
	VideoBitmap	*m_bmp;
public:
	Jsw64GuardianTableForm(Room *rm);
	virtual ~Jsw64GuardianTableForm();
	virtual int doModal();
        virtual void redraw(void);
	void redrawMap(void);
//        virtual int onButtonSelect(VideoButton *b);
	virtual void onEditChange(VideoControl *vc);
protected:
	virtual void hideChildren(void);
	virtual void showChildren(int redraw = 1);
};


