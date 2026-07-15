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

class Jsw128PatchPage;
class Jsw48SpritePage;
class Jsw128MessagePage;
class Jsw128MemPage;
class Jsw48RoomPage;
class Jsw48TelPage;
class Jsw48SplPage;
class FontEditPage;
class Jsw128AttrPage;
class TuneEdit128Page;

class Jsw128Game : public JswGame
{
public:
	Jsw128Game(SpectrumMemory *mem);
	virtual char *getID();
	virtual ~Jsw128Game();	
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
	virtual Jsw64Game  *upgrade64(char c);
	virtual jswByte *getFont(int *len);
	virtual jswByte *getTitleAttrs(void);
	virtual bool imcMode(void);
	virtual unsigned getMemmap();
	virtual unsigned getMapsize();
        virtual int getRoomCount();
protected:
        virtual Room *newRoom(int room);
	virtual void deleteSprites(void);
	Jsw128PatchPage   *m_patchPage;
	Jsw48SpritePage   *m_sprPage;
	Jsw128MessagePage *m_titlePage;
	Jsw128MemPage     *m_memPage;
	Jsw48RoomPage	  *m_roomPage;
	Jsw48TelPage      *m_telPage;
        Jsw48SplPage      *m_splPage;
	FontEditPage      *m_fePage;
	Jsw128AttrPage	  *m_attrPage;
	TuneEdit128Page   *m_tunePage;
};



