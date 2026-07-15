/************************************************************************

    JSWED 2.1.6 - Editor for Jet Set Willy and derivatives

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

class Jsw64BarrierForm: public VideoForm, ListSelectListener
{
protected:
// Drawing
	VideoBitmap	*m_bmBg;
	VideoBitmap	*m_bmTile[7];
// Controls
	ScrollingList	*m_list[2];
	VideoButton 	*m_bnOk, *m_bnCancel;
	
// Data
	JswGame		*m_game;
	jswByte		*m_room;
protected:
	virtual void showChildren(int redraw = 1);
	virtual void hideChildren();
public: 
        Jsw64BarrierForm(jswByte *roomData, JswGame *g, VideoBitmap *bg);
	virtual ~Jsw64BarrierForm();
// Overrides
	void drawData(void);
	virtual void redraw(void);

	virtual void onSelect(ScrollingList *list, int sel);
};


