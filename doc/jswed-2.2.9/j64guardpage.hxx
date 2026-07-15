#ifndef JSW64GUARDPAGE_HXX_INCLUDED
#define JSW64GUARDPAGE_HXX_INCLUDED

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

#include "j48roomnav.hxx"

class Jsw64GuardPage : public Jsw48RoomNav
{

protected:
	int m_acyminor;		// For the arrow: "minor" cy, 0-7
	int m_cx, m_cy;		// Cursor coordinates
	int m_ocx, m_ocy;	// Cursor coordinates at start of drag
	int m_ctype;		// Current cell type (=guardian no.)
	int m_octype;		// Previous cell type
	int m_oft;		// Free object count, as displayed
	int m_cdir;		// Conveyor direction
	int m_mb;		// 1 if in a mouse drag, else 0.
	int m_maxGuards;	// Max guardians in this room

	// Room as it was before changes
	jswByte undoBuffer[1024];

/* Overrides */
	virtual void drawFixed(void);	          // Draw fixed parts of screen
        virtual void drawData(int flashonly = 0); // Draw the room
	virtual void flashColours(void);	  // Redraw flashing bits
        virtual void drawCellType(void);	  // Show current cell type
        virtual void drawCursor(void);	  	  // Draw the cursor
        virtual void drawGuardList(void);	  // Animate guardian list
	virtual int onHotspot(int id);		// Action when hotspot clicked

	void setCellType(int ct);		// Set current cell type
	void putCell(void);	

	void undo(void);
	int guardianMenu(void); 
	const char *guardType(int ng);

public:
	Jsw64GuardPage(JswGame *game);
	virtual ~Jsw64GuardPage();

	virtual void loadRoom(int room, int reload = 0);

	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
	virtual int onButtonUp  (int x, int y, int button);
	virtual int onMouseMove (int x, int y, int xrel, int yrel);
        virtual int preContextMenu(void); 
        virtual int clipMenu(void); 
};

#endif //ndef JSW64GUARDPAGE_HXX_INCLUDED
