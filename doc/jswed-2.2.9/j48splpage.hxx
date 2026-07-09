#ifndef JSW48SPLPAGE_HXX_INCLUDED
#define JSW48SPLPAGE_HXX_INCLUDED

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

#include "j48roomnav.hxx"

class Jsw48SplPage : public Jsw48RoomNav
{
protected:
	bool m_imcmode;
	int m_cx, m_cy;		// Cursor coordinates
	int m_ocx, m_ocy;	// Cursor coordinates at start of drag

/* Overrides */
	virtual void drawFixed(void);	          // Draw fixed parts of screen
        virtual void drawData(int flashonly = 0); // Draw the room
    //    virtual void drawCellType(void);	  // Show current cell type
        virtual void drawCursor(void);	  	  // Draw the cursor
        virtual void drawGuardList(void);	  // Animate guardian list
	virtual int onHotspot(int id);		// Action when hotspot clicked

	virtual int preContextMenu(void);

	virtual int onSelect(void);
	int m_mode, m_prevMode;

	VideoBitmap *m_bmWilly, *m_bmMaria, *m_bmToilet, *m_bmRestart;
public:
	Jsw48SplPage(JswGame *game);
	virtual ~Jsw48SplPage();

	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
        virtual int onMouseMove (int x, int y, int xrel, int yrel);
};

#endif //ndef JSW48SPLPAGE_HXX_INCLUDED
