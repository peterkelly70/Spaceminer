#ifndef HOTSPOTMGR_HXX_INCLUDED
#define HOTSPOTMGR_HXX_INCLUDED

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

#include "hotspot.hxx"

class HotspotManager
{
private:
/* Hotspots (areas that can be clicked) */

	HSPTR *m_hs;		// List of pointers to the hotspots
	int m_hsCount;		// No. of entries in the list
	int m_hsMax;		// Max. entries in the list

protected:
	HotspotManager();
	virtual ~HotspotManager();

	void addHotspot(Hotspot *hs);		// Add a new hotspot
	virtual int onHotspot(int id) = 0;	// Action when hotspot clicked

	int onKey(int keysym);
	int onButton(int x, int y, int button);
};

#endif //ndef HOTSPOTMGR_HXX_INCLUDED
