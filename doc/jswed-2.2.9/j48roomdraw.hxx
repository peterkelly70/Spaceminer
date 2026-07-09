#ifndef JSW48ROOMDRAW_HXX_INCLUDED
#define JSW48ROOMDRAW_HXX_INCLUDED

/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2004-5  John Elliott <jce@seasip.demon.co.uk>

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

#include "j128guard.hxx"
#include "fixedguard.hxx"

#define GUARD_MAX 75	/* Absolute maximum number of guards */

class JswGame;

class Jsw48RoomDraw : public VideoNotePage, TickListener
{
protected:
/* Overridables */
        virtual void drawFixed(void);             // Draw fixed parts of screen
        virtual void drawData(int flashonly = 0); // Draw the room
        virtual void flashColours(void);          // Redraw flashing bits
        virtual void drawCellType(void);          // Show current cell type
        virtual void drawCursor(void);            // Draw the cursor
        virtual void drawGuardList(void);         // Animate guardian list

	JswGame *m_game;
	int m_room;
	int m_prevRoom;
	int m_ticker;
	TickListener *m_ot;
	int m_haveTimer;
	int m_visible;
	int m_drawnFixed;
	int m_maxGuards;
	JswGuardian *m_guards[GUARD_MAX + FG_MAX];

	inline Room *getRoom() { return m_game->getRoomClass(m_room); }

        VideoBitmap *m_bmRoom, *m_bmAnim;

	void flashSprite(VideoBitmap *bmp, jswByte attr);	
	void drawBasic(int flashonly);
	void drawItems(void);	
	void gotoRoom(int room);

        jswByte *getRoom(int room);
public:
	Jsw48RoomDraw(JswGame *game);
	virtual ~Jsw48RoomDraw();

	VideoBitmap *bitmapFromSprite(jswByte *sprite);

	virtual void loadRoom(int room, int reload = 0);
	virtual void checkSpecials(void);

// Overrides 
        virtual int onReveal(int whichRectangle);
        virtual int onConceal();
        virtual int redraw(int whichRectangle);
	virtual void intPreContextMenu(void);
        virtual void postContextMenu(void);

	virtual void onTick(void);	
};

#endif //ndef JSW48ROOMDRAW_HXX_INCLUDED
