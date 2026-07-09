/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2006  John Elliott <jce@seasip.demon.co.uk>

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

#include "jswed.hxx"
#include "hotspotmgr.hxx"


HotspotManager::HotspotManager()
{
	m_hsCount = 0;
	m_hsMax   = 30;
	m_hs      = new HSPTR[m_hsMax];	
}


HotspotManager::~HotspotManager()
{
	for (int n = 0; n < m_hsCount; n++) delete m_hs[n];
	delete [] m_hs;
}


void HotspotManager::addHotspot(Hotspot *hs)
{
	if (m_hsCount == m_hsMax)	
	{
		HSPTR *h = new HSPTR[m_hsMax * 2];
		if (!h) return;

		for (int n = 0; n < m_hsMax; n++) h[n] = m_hs[n];
		delete [] m_hs;
		m_hs = h;
		m_hsMax *= 2;
	}
	m_hs[m_hsCount++] = hs;
}


int HotspotManager::onKey(int keysym)
{
	int id;

	for (int n = 0; n < m_hsCount; n++)
	{
		id = m_hs[n]->keyTest(keysym);

		if (id != -1) return onHotspot(id);
	}
	return -1;
}

int HotspotManager::onButton(int x, int y, int button)
{
        int id;

        for (int n = 0; n < m_hsCount; n++)
        {
                id = m_hs[n]->hitTest(x, y);

                if (id != -1) return onHotspot(id);
        }
	return -1;
}

