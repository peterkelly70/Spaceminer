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

class Jsw64PatchPage;
class Jsw48SpritePage;
class Jsw128MemPage;
class Jsw64RoomPage;
class Jsw64GuardPage;
class Jsw128MessagePage;
class Jsw64TelPage;
class Jsw64SplPage;
class Jsw64AttrPage;
class FontEditPage;
class TuneEdit128Page;

class Jsw64Game : public JswGame
{
public:
	Jsw64Game(SpectrumMemory *mem);
	char getVariant();
	virtual char *getID();
	virtual ~Jsw64Game();	
        virtual void addPages(VideoNotebook *n);
	virtual void deletePages(void);
        virtual void fillSprites(int room, SpriteList *s, int every = 1);
        virtual void fillSprites(int room, VSpriteList *s);
	virtual void onChangeMemmap(void);
        virtual jswByte *getRoom(int room);
	virtual int guardTableCount(void);
	virtual jswByte *getGuards(int room);
	virtual int chooseGuardTable(int *ng);
        virtual int allowSuperJump(void);
	virtual int allowSpriteOverride(void);
        virtual int decodeWillySprite(int room, int sprite);
        virtual int encodeWillySprite(int room, int sprite);
	virtual int roomReserved(int room);
	virtual int teleUnload(void);
	virtual int teleLoad(void);
	virtual int supportsExtGuards(void);
	int upgrade128(void);
	virtual jswByte *getFont(int *len);
	virtual jswByte *getTitleAttrs(void);
	virtual bool imcMode(void);
        virtual unsigned getMemmap();
        virtual unsigned getMapsize();
        virtual int getRoomCount();
	virtual JswGuardian *newGuardian(jswByte *guard, int x, int bg);
	virtual int chooseGuardianType(int *t);
protected:
        virtual Room *newRoom(int room);
	virtual void deleteSprites(void);
	Jsw64PatchPage	  *m_patchPage;
	Jsw48SpritePage	  *m_sprPage;
	Jsw128MemPage     *m_memPage;
	Jsw64RoomPage	  *m_roomPage;
	Jsw64GuardPage	  *m_guardPage;
	Jsw128MessagePage *m_titlePage;
	Jsw64TelPage      *m_telPage;
        Jsw64SplPage      *m_splPage;
	FontEditPage      *m_fePage;
	Jsw64AttrPage	  *m_attrPage;
	TuneEdit128Page	  *m_tunePage;
};



