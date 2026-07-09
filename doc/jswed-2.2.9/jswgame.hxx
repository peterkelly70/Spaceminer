
#ifndef JSWGAME_HXX_INCLUDED
#define JSWGAME_HXX_INCLUDED 

/************************************************************************

    JSWED 2.2.1 - Editor for Jet Set Willy and derivatives

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

#include "snapio.hxx"
#include "jnotebook.hxx"
#include "jswroomlist.hxx"

class Room;
class Jsw128Game;
class Jsw64Game;
class SpriteList;
class VSpriteList;
class JswGuardian;
class GuardianEditor;

class JswGame : public Game
{
public:
	virtual ~JswGame();
	virtual int getStartRoom();
	virtual int getMariaRoom();
	virtual int getToiletRoom();
	virtual void setStartRoom(int);
	virtual void setMariaRoom(int);
	virtual void setToiletRoom(int);

	virtual void addPages(VideoNotebook *n) = 0;
	virtual void deletePages(void) = 0;
	virtual void fillSprites(int room, SpriteList *s, int every = 1) = 0;
	virtual void fillSprites(int room, VSpriteList *s) = 0;
        virtual void onChangeMemmap(void);
	virtual jswByte *getRoom(int room) = 0;
	virtual Room *getRoomClass(int room);

	virtual void objectDelete(int index);
	virtual void objectInsert(int room, int x, int y);
        virtual int  itemPos(int index, int *room, int *x, int *y);
        virtual int  objectAt(int room, int x, int y);
	        int  getItemCount(void);
	virtual int  getRoomCount(void) = 0;
	virtual int  guardTableCount(void);
	virtual int  chooseGuardTable(int *ng);
	virtual int  chooseGuard(VideoBitmap *bm, jswByte *table, 
				int *ng, int bg);
	virtual jswByte *getGuards(int room);
	virtual int allowSuperJump(void);
	virtual int allowSpriteOverride(void);
	virtual int decodeWillySprite(int room, int sprite);
	virtual int encodeWillySprite(int room, int sprite);
	virtual void getStartPos(int *x, int *y);
	virtual void setStartPos(int x, int y);
	virtual void getMariaPos(int *x, int *y);
	virtual void setMariaPos(int x, int y);
	virtual void getToiletPos(int *x, int *y);
	virtual void setToiletPos(int x, int y);
	virtual JswRoomList *getRoomList(void);
	virtual int prevRoom(int room);
	virtual int nextRoom(int room);
	virtual int roomReserved(int room) = 0;
	virtual int canTeleport(void);
	virtual jswByte *getTeleportList(void);
	virtual int toggleTeleporters(void);
	virtual int teleUnload(void);
	virtual int teleLoad(void);
	virtual int teleLoad(int page);
	virtual int chooseGuardianType(int *type);

	virtual int guardianEditor(int room, int guardian, jswByte *table, 
			int x = 15, VideoBitmap *backg = NULL);

	virtual int supportsExtGuards(void);
	virtual jswByte *getFont(int *len);

	virtual int clearRooms(void);
	virtual int clearSprites(void);
	virtual int clearGuardians(void);
	virtual jswByte *getTitleAttrs(void);
	virtual jswByte getWillySpritePage(void);

	inline int getGuardianMask(void) { return m_guardianMask; }
	virtual JswGuardian *newGuardian(jswByte *guard, int x, int bg);
	virtual GuardianEditor *newGuardianEditor(int room, jswByte *guard, VideoBitmap *bg, int x);
	virtual int afterLoadFix(void);

	virtual bool imcMode(void);

	virtual unsigned getMemmap(); 
	virtual unsigned getMapsize();
	Jsw128Game *upgrade(void);
	virtual Jsw64Game  *upgrade64(char c);
protected:
	virtual void deleteSprites(void) = 0;
	virtual void checkMariaToilet(void);
	int m_objectBase, m_objectMask, m_objectXY, m_objectCount,
	    m_objectStat, m_guardianMask, m_guardsBase, m_guardsEnd,
	    m_spriteBase;

	virtual Room *newRoom(int room) = 0;

	Room **m_roomCache;
	int  m_roomCacheSize;
	int  m_roomNameOffset;
	JswGame(SpectrumMemory *mem);
};

int saveFile(JswGame **gp);
int saveFile(char *filename, JswGame **gp);
int loadFile(JswGame **gp);
int loadFile(char *filename, JswGame **gp);

#endif // ndef JSWGAME_HXX_INCLUDED
