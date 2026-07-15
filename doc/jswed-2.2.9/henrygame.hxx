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


class Jsw48SpritePage;
class HenryRoomPage;
class HenryPatchPage;
class Jsw48SplPage;
class HenryMessagePage;
class FontEditPage;
class HenryAttrPage;

class HenryGame : public JswGame
{
public:
	HenryGame(SpectrumMemory *mem);
	virtual char *getID();
	virtual ~HenryGame();	
        virtual void addPages(VideoNotebook *n);
	virtual void deletePages(void);
        virtual void fillSprites(int room, SpriteList *s, int every = 1);
        virtual void fillSprites(int room, VSpriteList *s);
        virtual jswByte *getRoom(int room);
        virtual int allowSpriteOverride(void);
        virtual int  decodeWillySprite(int room, int sprite);
	virtual int  getStartRoom(void);
        virtual int getMariaRoom();
        virtual int getToiletRoom();
        virtual void setStartRoom(int);
        virtual void setMariaRoom(int);
        virtual void setToiletRoom(int);

        virtual void getStartPos(int *x, int *y);
        virtual void setStartPos(int x, int y);
	virtual void setMariaPos(int x, int y);
	virtual void setToiletPos(int x, int y);
	virtual int  roomReserved(int room);

	virtual int canTeleport(void);

	bool isLater(void);	
	void decodeString(int addr, char *buf);
	void encodeString(int addr, char *buf);

	void getScrolly(char *buf);
	void setScrolly(char *buf);
	virtual jswByte *getFont(int *len);
	virtual void deleteSprites(void);
	virtual jswByte *getTitleAttrs(void);
	virtual int getRoomCount();
protected:
	virtual Room *newRoom(int room);

	HenryPatchPage   *m_patchPage;
	HenryMessagePage *m_titlePage;
	Jsw48SpritePage  *m_spritePage;
	HenryRoomPage    *m_roomPage;
        Jsw48SplPage     *m_splPage;
        FontEditPage     *m_fePage;
	HenryAttrPage    *m_attrPage;
};



