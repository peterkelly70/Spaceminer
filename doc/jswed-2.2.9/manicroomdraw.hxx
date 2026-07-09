#ifndef MANICROOMDRAW_HXX_INCLUDED
#define MANICROOMDRAW_HXX_INCLUDED

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

#include "manicguardian.hxx"
//#include "fixedguard.hxx"
class ManicGame;

#define VG_MAX 4	/* 2 for the portal and Willy */
#define HG_MAX 4

class ManicRoomDraw : public VideoNotePage, TickListener
{
protected:
/* Overridables */
        virtual void drawFixed(void);             // Draw fixed parts of screen
        virtual void drawData(int flashonly = 0); // Draw the room
        virtual void flashColours(void);          // Redraw flashing bits
        virtual void drawCellType(void);          // Show current cell type
        virtual void drawCursor(void);            // Draw the cursor
        virtual void drawGuardList(void);         // Animate guardian list
        virtual void drawObject(int ink, int paper, int x, int y);

	ManicGame *m_game;
	int m_room;
	int m_prevRoom;
	int m_ticker;
	TickListener *m_ot;
	int m_haveTimer;
	int m_visible;
	int m_drawnFixed;
	ManicHGuardian *m_hguards[HG_MAX];
	ManicVGuardian *m_vguards[VG_MAX];
	ManicVGuardian *m_vgwilly, *m_vgeugene, *m_vgkong;

	inline Room *getRoom() { return m_game->getRoomClass(m_room); }

	VideoBitmap *m_bmRoom, *m_bmAnim, *m_bmItem;

	void flashSprite(VideoBitmap *bmp, jswByte attr);	
//	void drawConveyor(int draw = 1);
//	void drawConveyor(int x, int y, int len, jswByte *room, int draw);
	void drawBasic(int flashonly);
//	void drawCell(jswByte *room, int x, int y, int flashonly);
	void drawObjects(void);	
	void gotoRoom(int room);

	bool m_hasvguards, m_skylabs, m_solar, m_kong, m_eugene;

        jswByte *getRoom(int room);
public:
	ManicRoomDraw(ManicGame *game);
	virtual ~ManicRoomDraw();

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

#endif //ndef MANICROOMDRAW_HXX_INCLUDED
