#ifndef HEXEDIT_HXX_INCLUDED
#define HEXEDIT_HXX_INCLUDED

/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

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

#include "hotspotmgr.hxx"

class HexEditPage : public VideoNotePage, TickListener, HotspotManager
{
// Data
protected:
	SpectrumMemory *m_mem, *m_workMem;
	TickListener *m_ot;

	bool m_haveTimer;
	bool m_ctlQ;
	int m_ticker;
	int m_base, m_bank;
	int m_cx, m_cy, m_ocx, m_ocy;
	jswByte *m_pattern;
	int m_patLen;

	void undo(void);
	void drawCursor();
	void drawCell(int x, int y);
	void bottomBar();
	int cursorAddress();
	int getAddress();
	int find();
	int findAgain();
public:
	HexEditPage(SpectrumMemory *m);
	virtual ~HexEditPage();

	virtual int preContextMenu(void);
	virtual void postContextMenu(void);

	virtual int onReveal(int rect);
	virtual int onConceal();
        virtual int redraw(int whichRectangle);
	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
	virtual void onTick(void);
	virtual int onHotspot(int id);
};


#endif	// def HEXEDIT_HXX_INCLUDED

