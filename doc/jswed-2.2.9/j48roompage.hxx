#ifndef JSW48ROOMPAGE_HXX_INCLUDED
#define JSW48ROOMPAGE_HXX_INCLUDED

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

#include "j48roomnav.hxx"

class Jsw48RoomPage : public Jsw48RoomNav
{
protected:
	int m_acyminor;		// For the arrow: "minor" cy, 0-7
	int m_cx, m_cy;		// Cursor coordinates
	int m_ocx, m_ocy;	// Cursor coordinates at start of drag
	int m_ct;		// Current cell type (air, earth etc.)
	int m_oct;		// Previous cell type
	int m_oft;		// Free object count, as displayed
	int m_cdir;		// Conveyor direction
	int m_mb;		// 1 if in a mouse drag, else 0.

	// Room as it was before changes
	jswByte undoBuffer[256];

/* Overrides */
	virtual void drawFixed(void);	          // Draw fixed parts of screen
        virtual void drawData(int flashonly = 0); // Draw the room
	virtual void flashColours(void);	  // Redraw flashing bits
        virtual void drawCellType(void);	  // Show current cell type
        virtual void drawCursor(void);	  	  // Draw the cursor
        virtual void drawGuardList(void);	  // Animate guardian list
	virtual int onHotspot(int id);		// Action when hotspot clicked

	void setCellType(int ct);		// Set current cell type
	void putCell(int shifted);
	void drawConveyorCursor(VideoSurface *s, int ink);
	void drawRampCursor(VideoSurface *s, int ink);
	void drawXCursor(VideoSurface *s, int x, int y, int ink);
	int roomProperties(void);
	int roomExits(void);
	int roomClipboard(void);
	int roomUdgs(void);
	int roomClrs(void);

	void undo(void);
	int conveyorMenu(void);
	int guardianMenu(void);
public:
	Jsw48RoomPage(JswGame *game);
	virtual ~Jsw48RoomPage();

	virtual void loadRoom(int room, int reload = 0);

	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
	virtual int onButtonUp  (int x, int y, int button);
	virtual int onMouseMove (int x, int y, int xrel, int yrel);
        virtual int preContextMenu(void);
};

#endif //ndef JSW48ROOMPAGE_HXX_INCLUDED
