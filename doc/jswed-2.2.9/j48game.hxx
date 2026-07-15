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

class Jsw48MessagePage;
class Jsw48SpritePage;
class Jsw48RoomPage;
class Jsw48TelPage;
class Jsw48PatchPage;
class Jsw48SplPage;
class Jsw48AttrPage;

class Jsw48Game : public JswGame
{
public:
	Jsw48Game(SpectrumMemory *mem);
	virtual char *getID();
	virtual ~Jsw48Game();	
        virtual void addPages(VideoNotebook *n);
	virtual void deletePages(void);
        virtual void fillSprites(int room, SpriteList *s, int every = 1);
	virtual void fillSprites(int room, VSpriteList *s);
	virtual jswByte *getRoom(int room);
	virtual int roomReserved(int room);
	virtual int getRoomCount();
protected:
	virtual void deleteSprites(void);
	virtual Room *newRoom(int room);

	Jsw48MessagePage *m_titlePage;
	Jsw48SpritePage  *m_spritePage;
	Jsw48RoomPage	 *m_roomPage;
	Jsw48TelPage     *m_telPage;
	Jsw48PatchPage   *m_patchPage;
	Jsw48SplPage     *m_splPage;
	Jsw48AttrPage    *m_attrPage;
};



