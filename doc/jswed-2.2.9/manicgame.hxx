#ifndef MANICGAME_HXX_INCLUDED
#define MANICGAME_HXX_INCLUDED 

/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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

#include "jswroomlist.hxx"

class ManicPatchPage;
class ManicMessagePage;
class ManicAttrPage;
class ManicRoomPage;
class ManicGuardPage;
class ManicSpritePage;
class ManicPortalPage;
class SpriteList;
class Room;

#define FEAT_EUGENE	0x80
#define	FEAT_SKYLABS	0x40
#define FEAT_VGUARDS	0x20
#define FEAT_KONG	0x10
#define FEAT_SOLAR	0x08

class ManicGame : public Game
{
public:
	virtual ~ManicGame();

	virtual void addPages(VideoNotebook *n);
	virtual void deletePages(void);
	virtual void fillSprites(int room, SpriteList *s, int every = 1);
	inline int getStartRoom() { return 0; }
	int prevRoom(int room);
	int nextRoom(int room);

	bool featuresSettableByRoom();
	bool roomHasFeature(jswByte room, jswByte feature);

	// Given a Bug-Byte address, convert it into a Software Projects
	// address iff this game uses the SP engine.
	virtual unsigned short xltAddress(unsigned short address) = 0;
	
	virtual jswByte *getRoom(int room);
	virtual Room *getRoomClass(int room);
	virtual JswRoomList *getRoomList(void);

        inline jswByte getWillySpritePage(void)
	{
		return 0x82;
	}

protected:
	ManicGame(SpectrumMemory *mem);
// Pages
	ManicPatchPage   *m_patchPage;
	ManicMessagePage *m_msgPage;
	ManicAttrPage    *m_attrPage;
	ManicRoomPage    *m_roomPage;
	ManicGuardPage   *m_guardPage;
	ManicPortalPage  *m_portalPage;
	ManicSpritePage  *m_sprPage;

        virtual Room *newRoom(int room);

        Room **m_roomCache;
        int  m_roomCacheSize;
};

#endif // ndef MANICGAME_HXX_INCLUDED
