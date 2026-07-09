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

class Jsw64Game;
class Room;

class Jsw64GraphicsForm: public VideoForm, public SpriteListListener
{
protected:
	BitmapEditor  *m_edit;
	SpriteList    *m_list;
	VideoButton   *m_ok, *m_cancel;
	jswByte *m_room;
	jswByte m_undoBuf[32];
	Jsw64Game *m_game;
	int	m_max;
public:
	Jsw64GraphicsForm(Jsw64Game *g, Room *rm, jswByte *room);
	virtual ~Jsw64GraphicsForm();
	virtual int doModal();
        virtual void redraw(void);
	virtual void onChangeSprite(jswByte *newBitmap);
protected:
	virtual void showChildren(int redraw = 1);
	virtual void hideChildren(void);

	void undo();
};


