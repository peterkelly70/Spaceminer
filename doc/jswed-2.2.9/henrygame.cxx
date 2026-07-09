/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005  John Elliott <jce@seasip.demon.co.uk>

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
#include "henrygame.hxx"
#include "j48sprite.hxx"
#include "j48roompage.hxx"
#include "henryroompage.hxx"
#include "henrypatchpage.hxx"
#include "j48splpage.hxx"
#include "henrymsg.hxx"
#include "vspritelist.hxx"
#include "fontedit.hxx"
#include "henryattrpage.hxx"
#include "room.hxx"
#include "j48room.hxx"

HenryGame::HenryGame(SpectrumMemory *mem) : JswGame(mem)
{
	m_patchPage  = NULL;
	m_spritePage = NULL;
	m_titlePage  = NULL;
	m_roomPage   = NULL;
	m_splPage    = NULL;
	m_fePage     = NULL;
	m_attrPage   = NULL;
        // Initial PC for Henry's Hoard
        m_mem->poke(0x5BFE, 0x86);
        m_mem->poke(0x5BFF, 0x88);
	m_spriteBase = 0x9B00;
}

HenryGame::~HenryGame()
{
}

char *HenryGame::getID()
{
	char *s = JswGame::getID();

	strcpy(s + 32, " [HENRY]");
	return s;
}


void HenryGame::addPages(VideoNotebook *n)
{
	m_patchPage  = new HenryPatchPage  (this);
	m_titlePage  = new HenryMessagePage(this);
	m_spritePage = new Jsw48SpritePage (this);
	m_roomPage   = new HenryRoomPage   (this);
	m_splPage    = new Jsw48SplPage    (this);
	m_fePage     = new FontEditPage    (this);
	m_attrPage   = new HenryAttrPage   (this);
	n->addPage(m_patchPage);
	n->addPage(m_titlePage);
	n->addPage(m_spritePage);
	n->addPage(m_roomPage);
	n->addPage(m_splPage);
	n->addPage(m_fePage);
	n->addPage(m_attrPage);
	Game::addPages(n);
}


void HenryGame::deletePages()
{
	if (m_patchPage)  delete m_patchPage;
	if (m_titlePage)  delete m_titlePage;
	if (m_spritePage) delete m_spritePage;
	if (m_roomPage)   delete m_roomPage;
	if (m_splPage)    delete m_splPage;
	if (m_fePage)     delete m_fePage;
	if (m_attrPage)   delete m_attrPage;
	m_patchPage  = NULL;
	m_titlePage  = NULL;
	m_spritePage = NULL;
	m_roomPage   = NULL;
	m_splPage    = NULL;
	m_fePage     = NULL;
	m_attrPage   = NULL;
}


void HenryGame::fillSprites(int room, SpriteList *s, int every)
{
	// Fill 
        int n, step;

	step = every ? 0x20 : 0x100;

        for (n = m_spriteBase; n < 0xC000; n += step)
        {
                if (n == 0x9E00) n = 0xA600;
		if (n == 0xAC80) n = 0xAD00;
		if (n == 0xAE40) n = 0xAF00;
                s->addSprite(n);
        }
}

void HenryGame::fillSprites(int room, VSpriteList *s)
{
        int n;

        for (n = m_spriteBase; n < 0xC000; n += 0x100)
        {
                if (n == 0x9E00) n = 0xA600;
                s->addSprite(n);
        }
}


void HenryGame::deleteSprites(void)
{
        int n;
        
	for (n = m_spriteBase; n < 0xC000; n += 0x20)
        {
		if (n == 0x9E00) n = 0xA600;
		if (n == 0xAC80) n = 0xAD00;
		if (n == 0xAE40) n = 0xAF00;
		memset(m_mem->memoryAt(n), 0, 0x20);
	}
}



jswByte *HenryGame::getRoom(int room)
{
	if (roomReserved(room)) return NULL;
        if (room < 0)   return NULL;
        if (room < 50 || room == 53)  
		return m_mem->memoryAt(((room & 0x3F) << 8) + 0xC000);
        return NULL;
}



int HenryGame::allowSpriteOverride(void)
{
	return 0;
}

int  HenryGame::decodeWillySprite(int room, int sprite)
{
	if (room == m_mem->peek(0x8504)) return m_mem->peek(0x8508); // Manic
	if (room == m_mem->peek(0x850B)) return m_mem->peek(0x850F); // Chuckie
	if (room == m_mem->peek(0x8512)) return m_mem->peek(0x8516); // Chuckie
	if (room == m_mem->peek(0x8519)) return m_mem->peek(0x851D); // Reversed
	if (room == m_mem->peek(0x8520)) return m_mem->peek(0x8523); // Drop
	return m_mem->peek(0x9652);
}


int HenryGame::getStartRoom(void)
{
        return m_mem->peek(0x88B6);
}

void HenryGame::setStartRoom(int room)
{
	m_mem->poke(0x88B6, room);
}


int HenryGame::getMariaRoom(void)
{
	return -1;
}

void HenryGame::setMariaRoom(int room)
{
}

void HenryGame::setToiletPos(int x, int y)
{
}

void HenryGame::setMariaPos(int x, int y)
{
}

int HenryGame::getToiletRoom(void)
{
	return -1;
}

void HenryGame::setToiletRoom(int room)
{
}




void HenryGame::getStartPos(int *x, int *y)
{
        *x = m_mem->peek(0x88BB) & 0x1F;
        *y = m_mem->peek(0x88B1);
}


void HenryGame::setStartPos(int x, int y)
{
        m_mem->poke(0x88BB, ((y << 1) & 0xE0) | (x & 0x1F));
        m_mem->poke(0x88BC, (m_mem->peek(0x88BC) & 0xFE) | (y >> 7));
        m_mem->poke(0x88B1, y);
}


int HenryGame::roomReserved(int room)
{
	if (room > 63) return 1;
	if (room >= 50 && room != 53) return 1;

	return 0;
}


int HenryGame::canTeleport(void)
{
	return 0;
}


void HenryGame::decodeString(int addr, char *buf)
{
	int length = m_mem->peek(addr);
	int n;

	for (n = 0; n < length; n++) buf[n] = m_mem->peek(addr+n+1) + 32;
	buf[n] = 0;	
}


void HenryGame::encodeString(int addr, char *buf)
{
	int length = strlen(buf);
	int n;
	m_mem->poke(addr, length);

	if (!length)
	{
		m_mem->poke(addr, 1);
		m_mem->poke(addr+1, ' ');
		return;
	}
	for (n = 0; n < length; n++) m_mem->poke(addr+n+1, buf[n] - 32);
}



void HenryGame::getScrolly(char *buf)
{
	char b2[256];

        decodeString(0x99F2, buf);
        if (isLater()) 
	{
		decodeString(0xFF34, b2); strcat(buf, b2);
                decodeString(0x9731, b2); strcat(buf, b2);
                decodeString(0xAC80, b2); strcat(buf, b2);
                decodeString(0x9F00, b2); strcat(buf, b2);

	}
	else
	{
        	decodeString(0xAC80, b2); strcat(buf, b2);
	        decodeString(0x9731, b2); strcat(buf, b2);
       		decodeString(0x9F00, b2); strcat(buf, b2);
	}
}



void HenryGame::setScrolly(char *buf)
{
	char b2[256];

	strncpy(b2, buf, 195); b2[195] = 0; buf += strlen(b2);
	encodeString(0x99F2, b2);
	if (isLater())
	{
		strncpy(b2, buf, 185); b2[185] = 0; buf += strlen(b2);
		encodeString(0xFF34, b2);
                strncpy(b2, buf, 107); b2[107] = 0; buf += strlen(b2);
                encodeString(0x9731, b2);
                strncpy(b2, buf, 128/*124*/); b2[128] = 0; buf += strlen(b2);
                encodeString(0xAC80, b2);
                strncpy(b2, buf, 254); b2[254] = 0; buf += strlen(b2);
                encodeString(0x9F00, b2);
	}
	else
	{
                strncpy(b2, buf, 128/*124*/); b2[128] = 0; buf += strlen(b2);
                encodeString(0xAC80, b2);
                strncpy(b2, buf, 107); b2[107] = 0; buf += strlen(b2);
                encodeString(0x9731, b2);
                strncpy(b2, buf, 254); b2[254] = 0; buf += strlen(b2);
                encodeString(0x9F00, b2);
	}
}



bool HenryGame::isLater(void)
{
	if (m_mem->peek(0x9566) == 201) return false;
	return true;
}


jswByte *HenryGame::getFont(int *len)
{
	if (len) *len = 0x3B;
	return m_mem->memoryAt(0x85E0);
}


jswByte *HenryGame::getTitleAttrs(void)
{
        return m_mem->memoryAt(0x7800);	// Dummy
}

int HenryGame::getRoomCount()
{
	return 54;
}
        

Room *HenryGame::newRoom(int room)
{
        jswByte *rm = getRoom(room);
        if (rm == NULL) return NULL;
        return new Jsw48Room(this, rm, room);
}

