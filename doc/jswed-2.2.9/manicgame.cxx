/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-5  John Elliott <jce@seasip.demon.co.uk>

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
#include "room.hxx"
#include "spritelist.hxx"
#include "manicgame.hxx"
#include "manicpatchpage.hxx"
#include "manicmsg.hxx"
#include "manicsprite.hxx"
#include "manicattrpage.hxx"
#include "manicroompage.hxx"
#include "manicguardpage.hxx"
#include "manicportalpage.hxx"
#include "manicroom.hxx"

ManicGame::ManicGame(SpectrumMemory *mem) : Game(mem)
{
	m_patchPage  = NULL;
	m_msgPage    = NULL;
	m_attrPage   = NULL;
	m_roomPage   = NULL;
	m_sprPage    = NULL;
	m_portalPage = NULL;
	m_guardPage  = NULL;

// Set initial SNA stack 
        mem->pokeSnaHeader(23, 0xFE);
        mem->pokeSnaHeader(24, 0x5B); 
	mem->poke(0x5BFE, 0x00);
	mem->poke(0x5BFF, 0x84);
	m_roomCache = NULL;
	m_roomCacheSize = 0;
}	

ManicGame::~ManicGame()
{
	deletePages();
        if (m_roomCache)
        {
                for (int n = 0; n < m_roomCacheSize; n++)
                        if (m_roomCache[n]) delete m_roomCache[n];
                delete m_roomCache;
        }
}


void ManicGame::addPages(VideoNotebook *n)
{
	if (!m_patchPage)  m_patchPage  = new ManicPatchPage(this);
	if (!m_msgPage)    m_msgPage    = new ManicMessagePage(getMem());
	if (!m_sprPage)    m_sprPage    = new ManicSpritePage(this);
	if (!m_attrPage)   m_attrPage   = new ManicAttrPage(this);
	if (!m_roomPage)   m_roomPage   = new ManicRoomPage(this); 
	if (!m_guardPage)  m_guardPage  = new ManicGuardPage(this);
	if (!m_portalPage) m_portalPage = new ManicPortalPage(this); 
	n->addPage(m_patchPage);
	n->addPage(m_msgPage);
	n->addPage(m_sprPage);
	n->addPage(m_roomPage);
	n->addPage(m_guardPage);
	n->addPage(m_portalPage);
	n->addPage(m_attrPage);
	Game::addPages(n);
}


void ManicGame::deletePages(void)
{
	if (m_attrPage)   delete m_attrPage;
	if (m_sprPage)    delete m_sprPage;
	if (m_patchPage)  delete m_patchPage;
	if (m_msgPage)    delete m_msgPage;
	if (m_roomPage)   delete m_roomPage;
	if (m_guardPage)  delete m_guardPage;
	if (m_portalPage) delete m_portalPage;
	m_patchPage  = NULL;
	m_msgPage    = NULL;
	m_attrPage   = NULL;
	m_sprPage    = NULL;
	m_roomPage   = NULL;
	m_guardPage  = NULL;
	m_portalPage = NULL;
}


void ManicGame::fillSprites(int room, SpriteList *s, int every)
{
        int n, m, step;

        step = every ? 0x20 : 0x100;

        for (n = 33280; n < 33536; n += step)
        {
                s->addSprite(n);
        }
	for (n = 0xB000; n < 0xFFFF; n += 1024)
	{
		for (m = 768; m < 1024; m += step)
		{
			s->addSprite(n + m);
		}
	}
}


jswByte *ManicGame::getRoom(int room)
{
	if (room < 0 || room > 19) return NULL;
	return m_mem->memoryAt(0xB000 + 1024 * room);	
}

int ManicGame::prevRoom(int room)
{
	if (room <= 0) return room;
	return --room;
}

int ManicGame::nextRoom(int room)
{
	if (room >= 19) return room;
	return ++room;
}

JswRoomList *ManicGame::getRoomList(void)
{
        int n;
        jswByte *b;
        char rname[40];

        JswRoomList *s = new JswRoomList();

        for (n = 0; n < 20; n++) 
        {
                b = getRoom(n);
                sprintf(rname, "%03d %-32.32s", n, b + 512);
                s->addString(rname);
        }
        return s;
}


bool ManicGame::featuresSettableByRoom()
{
	if (m_mem->peek(xltAddress(0x8770)) == 0x73)
	{
		return true;
	}
	return false;
}

bool ManicGame::roomHasFeature(jswByte room, jswByte feature)
{
/* See if my MM patch is installed */
	if (featuresSettableByRoom())
	{
		return (getRoom(room)[627] & feature);
	}
/* Use hardcoded room IDs */
	switch (feature)
	{
		case FEAT_EUGENE:
			return (m_mem->peek(xltAddress(0x8773)) == room);
		case FEAT_SKYLABS:
			return (m_mem->peek(xltAddress(0x877b)) == room);
		case FEAT_VGUARDS:
			// If have skylabs, can't have vertical guardians
			if (m_mem->peek(xltAddress(0x877b)) == room)
				return false;
			return (m_mem->peek(xltAddress(0x8783)) <= room);
		case FEAT_KONG:
			// If have skylabs, can't have Kong
			if (m_mem->peek(xltAddress(0x877b)) == room)
				return false;
			if (m_mem->peek(xltAddress(0x878B)) == room) 
				return true;
			if (m_mem->peek(xltAddress(0x8793)) == room) 
				return true;
			return false;
		case FEAT_SOLAR:
			// If have skylabs, can't have solar power
			if (m_mem->peek(xltAddress(0x877b)) == room)
				return false;
			return (m_mem->peek(xltAddress(0x879B)) == room);
	} 
	return false;
}



typedef Room *RoomPtr;

Room *ManicGame::newRoom(int r)
{
	if (r < 0 || r > 19) return NULL;
	return new ManicRoom(this, getRoom(r), r); 
}

Room *ManicGame::getRoomClass(int room)
{
	Room *r, **rc;
	int newsize, n;

	if (room < 0 || room >= 20) return NULL;

	if (room < m_roomCacheSize)
	{
		if (m_roomCache[room]) return m_roomCache[room];
		r = newRoom(room);
		m_roomCache[room] = r;
		return r;
	}

	r = newRoom(room);
	if (m_roomCacheSize == 0) newsize = 64;
	else			  newsize = 2 * m_roomCacheSize;

	rc = new RoomPtr[newsize];
	for (n = 0; n < newsize; n++) rc[n] = NULL;
	if (m_roomCache)
	{
		for (n = 0; n < m_roomCacheSize; n++) rc[n] = m_roomCache[n];
		delete m_roomCache;
	}
	m_roomCache = rc;
	m_roomCacheSize = newsize;
	rc[room] = r;
	return r;
}


