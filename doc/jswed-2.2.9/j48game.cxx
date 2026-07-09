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
#include "jswgame.hxx"
#include "j48game.hxx"
#include "j48msg.hxx"
#include "j48sprite.hxx"
#include "j48roompage.hxx"
#include "j48telpage.hxx"
#include "j48patchpage.hxx"
#include "j48splpage.hxx"
#include "vspritelist.hxx"
#include "j48attrpage.hxx"
#include "room.hxx"
#include "j48room.hxx"

Jsw48Game::Jsw48Game(SpectrumMemory *mem) : JswGame(mem)
{
	m_titlePage  = NULL;
	m_spritePage = NULL;
	m_roomPage   = NULL;
	m_splPage    = NULL;
	m_attrPage   = NULL;
}

Jsw48Game::~Jsw48Game()
{
}

char *Jsw48Game::getID()
{
	char *s = JswGame::getID();

	strcpy(s + 32, " [JSW48]");
	return s;
}


void Jsw48Game::addPages(VideoNotebook *n)
{
	m_titlePage  = new Jsw48MessagePage(m_mem);
	m_spritePage = new Jsw48SpritePage (this);
	m_roomPage   = new Jsw48RoomPage   (this);
	m_telPage    = new Jsw48TelPage    (this);
	m_patchPage  = new Jsw48PatchPage  (this);	
	m_splPage    = new Jsw48SplPage    (this);
	m_attrPage   = new Jsw48AttrPage   (this);
	n->addPage(m_patchPage);
	n->addPage(m_titlePage);
	n->addPage(m_spritePage);
	n->addPage(m_roomPage);
	n->addPage(m_telPage);
	n->addPage(m_splPage);
	n->addPage(m_attrPage);
	Game::addPages(n);
}


void Jsw48Game::deletePages()
{
	if (m_patchPage)  delete m_patchPage;
	if (m_titlePage)  delete m_titlePage;
	if (m_spritePage) delete m_spritePage;
	if (m_roomPage)   delete m_roomPage;
	if (m_telPage)    delete m_telPage;
	if (m_splPage)    delete m_splPage;
	if (m_attrPage)   delete m_attrPage;
	m_titlePage  = NULL;
	m_spritePage = NULL;
	m_roomPage   = NULL;
	m_telPage    = NULL;
	m_patchPage  = NULL;
	m_splPage    = NULL;
	m_attrPage   = NULL;
}


void Jsw48Game::fillSprites(int room, SpriteList *s, int every)
{
        int n, step;

	step = every ? 0x20 : 0x100;

        for (n = m_spriteBase; n < 0xC000; n += step)
        {
// Sprite page in use by rope coordinate table.
		if (m_mem->peek(0x931D) == (n >> 8)) continue;
                if (n == m_guardsBase) n = m_guardsEnd;
                if (n < 0xC000) s->addSprite(n);
        }
}


void Jsw48Game::fillSprites(int room, VSpriteList *s)
{
        int n;

        for (n = m_spriteBase; n < 0xC000; n += 0x100)
        {
                if (n == m_guardsBase) n = m_guardsEnd;
		if (n < 0xC000) s->addSprite(n);
        }
}

void Jsw48Game::deleteSprites(void)
{
        int n;

        for (n = m_spriteBase; n < 0xC000; n += 0x100)
        {
                if (n == m_guardsBase) n = m_guardsEnd;
		if (n < 0xC000) memset(m_mem->memoryAt(n), 0, 256);
        }
}



jswByte *Jsw48Game::getRoom(int room)
{
	if (roomReserved(room)) return NULL;
        if (room < 0)   return NULL;
        if (room < getRoomCount())  return m_mem->memoryAt(((room & 0x3F) << 8) + 0xC000);
        return NULL;
}

int Jsw48Game::roomReserved(int room)
{
	if (room >= getRoomCount()) return 1;

        if (m_mem->peek(35696) != 205) return 0;

	return (room == m_mem->peek(35698));
}


unsigned JswGame::getMapsize()
{
        return 0;
}

unsigned JswGame::getMemmap()
{
        return 0;
}

int Jsw48Game::getRoomCount()
{
	return 64;
}

Room *Jsw48Game::newRoom(int room)
{
	jswByte *rm = getRoom(room);
	if (rm == NULL) return NULL;
	return new Jsw48Room(this, rm, room);
}


