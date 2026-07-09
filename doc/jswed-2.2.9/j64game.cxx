/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2004-5  John Elliott <jce@seasip.demon.co.uk>

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
#include "room.hxx"
#include "j64game.hxx"
#include "j64room.hxx"
#include "fontedit.hxx"
#include "j64attrpage.hxx"
#include "j64patchpage.hxx"
#include "spritelist.hxx"
#include "vspritelist.hxx"
#include "j48sprite.hxx"
#include "j128mem.hxx"
#include "j64roompage.hxx"
#include "j64guardpage.hxx"
#include "j128msg.hxx"
#include "j64guard.hxx"
#include "j64telpage.hxx"
#include "j48splpage.hxx"
#include "j64splpage.hxx"
#include "j128glist.hxx"
#include "tunepage128mini.hxx"

#define MEMMAP_LEN 95
#define MEMMAP_BASE 0x8640

Jsw64Game::Jsw64Game(SpectrumMemory *mem) : JswGame(mem)
{
	m_patchPage = NULL;
	m_sprPage   = NULL;
	m_memPage   = NULL;
	m_roomPage  = NULL;
	m_guardPage = NULL;
	m_titlePage = NULL;
	m_telPage   = NULL;
	m_splPage   = NULL; 
	m_fePage    = NULL;
	m_attrPage  = NULL;
	m_tunePage  = NULL;
	switch(mem->peek(0x85C9))
	{
		case 'V': case 'W':	m_objectMask = 0x7F; 
					m_objectBase = 0xC000;
					break;
		default:		m_objectMask = 0x3F; 
					m_objectBase = 0xA400;	
					break;
	}
	m_objectCount    = 0x85CA;	
	m_roomNameOffset = 0xB6;
}

Jsw64Game::~Jsw64Game()
{
}

char Jsw64Game::getVariant()
{
	return m_mem->peek(0x85C9);
}

char *Jsw64Game::getID()
{
	char *s = JswGame::getID();

	sprintf(s + 30, " {JSW64:%c}", getVariant());
	return s;
}


void Jsw64Game::addPages(VideoNotebook *n)
{
	m_patchPage = new Jsw64PatchPage   (this);
	m_sprPage   = new Jsw48SpritePage  (this);
	m_memPage   = new Jsw128MemPage    (this, false);
	m_roomPage  = new Jsw64RoomPage    (this);
	m_guardPage = new Jsw64GuardPage   (this);
	m_titlePage = new Jsw128MessagePage(m_mem);
	m_telPage   = new Jsw64TelPage     (this);
	m_splPage   = new Jsw64SplPage     (this); 
	m_fePage    = new FontEditPage     (this);
	m_attrPage  = new Jsw64AttrPage    (this);
	m_tunePage  = new TuneEdit128Page  (this);
	n->addPage(m_patchPage);
	n->addPage(m_titlePage);
	n->addPage(m_memPage);
	n->addPage(m_sprPage);
	n->addPage(m_roomPage);
	n->addPage(m_guardPage);
	n->addPage(m_telPage);
	n->addPage(m_splPage); 
	n->addPage(m_fePage);
	n->addPage(m_attrPage);
	n->addPage(m_tunePage);
	Game::addPages(n);
}


void Jsw64Game::deletePages()
{
	if (m_patchPage) delete m_patchPage;
	if (m_roomPage)  delete m_roomPage;
	if (m_titlePage) delete m_titlePage;
	if (m_telPage)   delete m_telPage;
	if (m_splPage)   delete m_splPage; 
	if (m_sprPage)   delete m_sprPage;
	if (m_fePage)    delete m_fePage;
	if (m_attrPage)  delete m_attrPage;
	if (m_guardPage) delete m_guardPage;
	if (m_tunePage)  delete m_tunePage;
	if (m_memPage)   delete m_memPage;
	m_patchPage = NULL;
	m_roomPage = NULL;
	m_titlePage = NULL;
	m_memPage = NULL;
	m_telPage  = NULL;
	m_splPage  = NULL; 
	m_sprPage = NULL;
	m_memPage = NULL;
	m_fePage   = NULL;
	m_attrPage = NULL;
	m_guardPage = NULL;
	m_tunePage = NULL;
}

int Jsw64Game::guardTableCount(void)
{
	int n, m, c = 0;
        for (n = 0; n < MEMMAP_LEN; n++)
        {
                m = m_mem->peek(n + MEMMAP_BASE);

                if (m >= 0x20 && m < 0x40) ++c;
        }
	return c;
}

int  Jsw64Game::chooseGuardTable(int *ng)
{
	static char tName[32][40];
	int gaddr[32];
	pChar tPtr[32];
	int rv;

        int n, m, c = 0;
        for (n = 0; n < MEMMAP_LEN; n++)
        {
                m = m_mem->peek(n + MEMMAP_BASE);

                if (m >= 0x20 && m < 0x40) 
		{
			tPtr[c] = &(tName[c][0]);
			gaddr[c] = (n + 0x98) << 8;
			sprintf(tName[c++], "Table %2d at %02x00", (m & 0x1F), (n + 0x98));
		}
        }
	VideoMenu vm(c, "Guardian tables", tPtr);
	for (n = 0; n < c; n++)
	{
		if (*ng == gaddr[n]) vm.setSelected(n);
	}
	rv = vm.doModal();

	c = (vm.getSelected() % 32);
	*ng = gaddr[c];
        return rv;
}


jswByte *Jsw64Game::getGuards(int room)
{
	jswByte *b = getRoom(room);

	return m_mem->memoryAt(b[0xDF] + 256 * b[0xE0]);
}


void Jsw64Game::fillSprites(int room, SpriteList *s, int every)
{
        int n,m,i, step;
	int roomSprites[256];
	int spriteAddr, spriteBase;
	
	step = every ? 32 : 256;
	Room *rm;
	if (room >= 0)	rm = getRoomClass(room);
	else		rm = NULL;
	if (rm)
	{
		rm->getSprites(roomSprites, step);
		for (i = 0; roomSprites[i]; i++)
		{
               		s->addSprite(roomSprites[i]);
		}
	}

	// Rewritten to try and stop it stuttering (including the first 
	// sprite in a set twice). This is probably caused by the Windows
	// C++ compiler, because it only occurs in Windows builds. 
        for (n = 0; n < MEMMAP_LEN; n++)
        {
		m = m_mem->peek(n + MEMMAP_BASE);

		if (m == 0x60) 
		{
			spriteBase = (n + 0x98) * 256;
			for (i = 0; i < 256; i += step)
			{
				spriteAddr = spriteBase + i;
                		s->addSprite(spriteAddr);
			}
		}
        }
}


void Jsw64Game::fillSprites(int room, VSpriteList *s)
{
        int n;
	int roomSprites[256];

	Room *rm;
	if (room >= 0)	rm = getRoomClass(room);
	else		rm = NULL;
	if (rm)
	{
		rm->getSprites(roomSprites, 256);
		for (int i = 0; roomSprites[i]; i++)
		{
               		s->addSprite(roomSprites[i]);
		}
	}

        for (n = 0x0; n < MEMMAP_LEN; n++)
        {
		if (m_mem->peek(n + MEMMAP_BASE) == 0x60) 
			s->addSprite(0x9800 + 256 * n);
        }
}


void Jsw64Game::deleteSprites(void)
{
        int n;

        for (n = 0x0; n < MEMMAP_LEN; n++)
        {
                if (m_mem->peek(n + MEMMAP_BASE) == 0x60)
                        memset(m_mem->memoryAt(0x9800 + 256 * n), 0, 256);
        }
}



//
// The memory map has changed. Any screen that depends on it must update
// itself when it is next displayed.
//
void Jsw64Game::onChangeMemmap(void)
{
	if (m_sprPage) m_sprPage->reload(this);	
}

jswByte *Jsw64Game::getRoom(int room)
{
	if (room < 0)   return NULL;
	switch(getVariant())
	{
		case 'V': case 'W':
		if (room < 32)  return m_mem->memoryAt((room & 0x1F) << 9, 1);
		if (room < 64)  return m_mem->memoryAt((room & 0x1F) << 9, 3);
		if (room < 96)  return m_mem->memoryAt((room & 0x1F) << 9, 4);
		if (room < 128) return m_mem->memoryAt((room & 0x1F) << 9, 6);
		break;

		default:
		if (room < 16) return m_mem->memoryAt((room & 0x0F) << 10, 1);
		if (room < 32) return m_mem->memoryAt((room & 0x0F) << 10, 3);
		if (room < 48) return m_mem->memoryAt((room & 0x0F) << 10, 4);
		if (room < 64) return m_mem->memoryAt((room & 0x0F) << 10, 6);
	}
	return 0;
}

int Jsw64Game::allowSuperJump(void)
{
	// Only allow this if the game's at HL5 or above
	if (m_mem->peek(0x85C2) != 0xC3) return 1;
	return 0; 
}


int Jsw64Game::allowSpriteOverride(void)
{
        if (m_mem->peek(0x9654) == 0x20 &&
            m_mem->peek(0x9655) == 0x84) return 0; 
        return 0xED;
}


int Jsw64Game::decodeWillySprite(int room, int sprite)
{
	int ws = JswGame::decodeWillySprite(room, sprite);
	if (ws < 0x80) ws = 0x9D;
	return ws;
}


int Jsw64Game::encodeWillySprite(int room, int sprite)
{
	if (sprite == 0x9D) return 0;
        return sprite;
}


int Jsw64Game::roomReserved(int room)
{
	if (room >= getRoomCount()) return 1;
        return 0;
}


int Jsw64Game::teleUnload(void)
{
	if (m_mem->peek(35696) != 205) return ZXE_CONTINUE;
	jswByte tpage = m_mem->peek(35698);

	if (tpage >= 0x98) m_mem->poke(MEMMAP_BASE + tpage - 0x98, 0);

	return JswGame::teleUnload();
}

int Jsw64Game::teleLoad(void)
{
	int n;
	for (n = 82; n >= 0; n--)
	{
		if (m_mem->peek(MEMMAP_BASE + n) == 0)
		{
			m_mem->poke(MEMMAP_BASE+n, 0x61);
			JswGame::teleLoad(n + 0x98);
			return ZXE_CONTINUE;
		}
	}
	VideoMenu vm("Memory Full", "Cancel", NULL);
	n = vm.doModal();
	if (n >= ZXE_QUIT) return n;
	return ZXE_CONTINUE;	
}

/*
int Jsw64Game::teleLoad(int page)
{
	static jswByte tport[] = {
        	0xD5,			// PUSH DE
		0xDD,0xE5,		// PUSH IX
		0xDD,0x21,0x4A,0x01,	// LD IX,buffer
		0x11,0x04,0x00,		// LD DE,4
		0xDD,0x46,0x00,		// LD B,(IX+0)
		0xDD,0x23,		// INC IX
		0x78,			// LD A,B
       		0xB7,			// OR A
		0x28,0x30,		// JR Z,NOTEL
// LOOP:
		0x3A,0x20,0x84,		// LD A,(SHEET)
		0xDD,0xBE,0x00,		// CP (IX+0)
		0x20,0x24,		// JR NZ,NOTME
		0x3A,0xCF,0x85,		// LD A,(WILLYY)
		0xDD,0xBE,0x01,		// CP (IX+1)
		0x20,0x1C,		// JR NZ,NOTME
		0x3A,0xD3,0x85,		// LD A,(WILLYX)
		0xE6,0x1F,		// AND 1Fh
		0xDD,0xBE,0x02,		// CP (IX+2)
		0x20,0x12,		// JR NZ,NOTME
		0xDD,0x7E,0x03,		// LD A,(IX+3)
		0x32,0x20,0x84,		// LD (SHEET),A
		0xE5,			// PUSH HL
		0x00,0x00,0x00,		// Teleporter FX
		0xE1,			// POP HL
		0xDD,0xE1,		// POP IX
		0xD1,			// POP DE
		0xC1,			// POP BC
		0xC3,0x12,0x89,		// JP L8912
// NOTME:
 		0xDD,0x19,		// ADD IX,DE
		0x10,0xD0,		// DJNZ LOOP
// NOTEL:
		0xDD,0xE1,		// POP IX
		0xD1,			// POP DE
		0x01,0xFE,0xEF,		// LD BC,EFFEh
		0xC9,			// RET
		0x00 };

	memcpy(m_mem->memoryAt(page * 256), tport, 
		sizeof(tport));
	m_mem->poke(page * 256 + 6, page);
        m_mem->poke(35696, 0xCD);
        m_mem->poke(35697, 0x00);
        m_mem->poke(35698, page);
	return ZXE_CONTINUE;
}
*/



int Jsw64Game::supportsExtGuards(void)
{
        return 1;
}



jswByte *Jsw64Game::getFont(int *len)
{
        if (len) *len = 96;
        return m_mem->memoryAt(0xC100);
}


jswByte *Jsw64Game::getTitleAttrs(void)
{
        return m_mem->memoryAt(0xD000, 7);
}


bool Jsw64Game::imcMode(void)
{
	return false;
}

unsigned Jsw64Game::getMapsize()
{
	return MEMMAP_LEN;
}

unsigned Jsw64Game::getMemmap()
{
	return MEMMAP_BASE;
}

int Jsw64Game::getRoomCount()
{
	switch(getVariant())
	{
		case 'V': case 'W': return 128;
	}
	return 64;	
}

Room *Jsw64Game::newRoom(int room)
{
        jswByte *rm = getRoom(room);
        if (rm == NULL) return NULL;
        return new Jsw64Room(this, rm, room);
}


JswGuardian *Jsw64Game::newGuardian(jswByte *guard, int x, int bg)
{
	return new Jsw64Guard(guard, x, m_mem, bg);
}


int Jsw64Game::chooseGuardianType(int *t)
{
	int rv;

	static int types[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 13, 14,
       				0x08, 0x18, 0x28, 0x38, 0x88, 0x98, 
				0xA8, 0xB8, 0xC8 };
 	VideoMenu vm128("Guardian type",
			"Blank",                        // 0
			"Horizontal",                   // 1
			"Vertical",                     // 2
			"Rope",                         // 3
			"Arrow",                        // 4
			"Diagonal NW/SE",               // 5
			"Diagonal NE/SW",               // 6 
			"Vertical multicolour",         // 7
			"Horizontal multicolour",       // 8 -> 9
			"NW/SE multicolour",            // 9 -> 13
			"NE/SW multicolour",            // 10 -> 14
			"Skylab",			// 11 -> 08
			"Angry Eugene",			// 12 -> 18
			"Angry Eugene multiclr",	// 13 -> 28
			"Angry Eugene multi2",		// 14 -> 38
			"Trigger",			// 15 -> 88
			"Switch",			// 16 -> 98
			"Opening wall",			// 17 -> A8
			"Stopper (move+draw)",		// 18 -> B8
			"Stopper (move only)",		// 19 -> C8
			NULL);
	rv = vm128.doModal();
	if (t) 
	{
		*t = types[vm128.getSelected()];	
	}
	return rv;
}





