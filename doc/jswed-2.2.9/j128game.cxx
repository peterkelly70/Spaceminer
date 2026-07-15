/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

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
#include "j128game.hxx"
#include "j128patchpage.hxx"
#include "j48sprite.hxx"
#include "j48roompage.hxx"
#include "j128msg.hxx"
#include "j128mem.hxx"
#include "j48telpage.hxx"
#include "j48splpage.hxx"
#include "fontedit.hxx"
#include "j128attrpage.hxx"
#include "vspritelist.hxx"
#include "j48room.hxx"
#include "tunepage128mini.hxx"

#define MEMMAP_LEN 95
#define MEMMAP_BASE 0x864C

Jsw128Game::Jsw128Game(SpectrumMemory *mem) : JswGame(mem)
{
	m_patchPage = NULL;
	m_titlePage = NULL;
	m_sprPage   = NULL;
	m_memPage   = NULL;
	m_roomPage  = NULL;
	m_telPage   = NULL;
	m_splPage   = NULL;
	m_fePage    = NULL;
	m_attrPage  = NULL;
	m_tunePage  = NULL;
	m_objectBase = 0xC000;
	m_objectMask = 0xFF;
}

Jsw128Game::~Jsw128Game()
{
}

char *Jsw128Game::getID()
{
	char *s = JswGame::getID();

	strcpy(s + 31, " [JSW128]");
	return s;
}


void Jsw128Game::addPages(VideoNotebook *n)
{
	m_patchPage = new Jsw128PatchPage  (this);
	m_titlePage = new Jsw128MessagePage(m_mem);
	m_memPage   = new Jsw128MemPage    (this, true);
	m_sprPage   = new Jsw48SpritePage  (this);
	m_roomPage  = new Jsw48RoomPage    (this);
	m_telPage   = new Jsw48TelPage     (this);
	m_splPage   = new Jsw48SplPage     (this);
	m_fePage    = new FontEditPage     (this);
	m_attrPage  = new Jsw128AttrPage   (this);
	m_tunePage  = new TuneEdit128Page  (this);
	n->addPage(m_patchPage);
	n->addPage(m_titlePage);
	n->addPage(m_memPage);
	n->addPage(m_sprPage);
	n->addPage(m_roomPage);
	n->addPage(m_telPage);
	n->addPage(m_splPage);
	n->addPage(m_fePage);
	n->addPage(m_attrPage);
	n->addPage(m_tunePage);
	Game::addPages(n);
}


void Jsw128Game::deletePages()
{
	if (m_patchPage) delete m_patchPage;
	if (m_titlePage) delete m_titlePage;
	if (m_memPage)   delete m_memPage;
	if (m_sprPage)   delete m_sprPage;
	if (m_roomPage)  delete m_roomPage;
	if (m_telPage)   delete m_telPage;
	if (m_splPage)   delete m_splPage;
	if (m_fePage)    delete m_fePage;
	if (m_attrPage)  delete m_attrPage;
	if (m_tunePage)  delete m_tunePage;
	m_patchPage = NULL;
	m_titlePage = NULL;
	m_memPage = NULL;
	m_sprPage = NULL;
	m_roomPage = NULL;
	m_telPage  = NULL;
	m_splPage  = NULL;
	m_fePage   = NULL;
	m_attrPage = NULL;
	m_tunePage = NULL;
}

int Jsw128Game::guardTableCount(void)
{
	int n, m, c = 0;
        for (n = 0; n < MEMMAP_LEN; n++)
        {
                m = m_mem->peek(n + MEMMAP_BASE);

                if (m >= 0x20 && m < 0x40) ++c;
        }
	return c;
}

int  Jsw128Game::chooseGuardTable(int *ng)
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


jswByte *Jsw128Game::getGuards(int room)
{
	jswByte *b = getRoom(room);

	return m_mem->memoryAt(b[0xDF] + 256 * b[0xE0]);
}


void Jsw128Game::fillSprites(int room, SpriteList *s, int every)
{
        int n,m,i, step, spriteBase, spriteAddr;

	step = every ? 32 : 256;

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


void Jsw128Game::fillSprites(int room, VSpriteList *s)
{
        int n;

        for (n = 0x0; n < MEMMAP_LEN; n++)
        {
		if (m_mem->peek(n + MEMMAP_BASE) == 0x60) 
			s->addSprite(0x9800 + 256 * n);
        }
}


void Jsw128Game::deleteSprites(void)
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
void Jsw128Game::onChangeMemmap(void)
{
	if (m_sprPage) m_sprPage->reload(this);	
}

jswByte *Jsw128Game::getRoom(int room)
{
	if (room < 0)   return NULL;
	if (room < 64)  return m_mem->memoryAt((room & 0x3F) << 8, 1);
	if (room < 128) return m_mem->memoryAt((room & 0x3F) << 8, 3);
	if (room < 192) return m_mem->memoryAt((room & 0x3F) << 8, 4);
	if (room < 256) return m_mem->memoryAt((room & 0x3F) << 8, 6);
	return 0;
}

int Jsw128Game::allowSuperJump(void)
{
	// Only allow this if the game's at HL5 or above
	if (m_mem->peek(0x85C2) != 0xC3) return 1;
	return 0; 
}


int Jsw128Game::allowSpriteOverride(void)
{
        if (m_mem->peek(0x9654) == 0x20 &&
            m_mem->peek(0x9655) == 0x84) return 0; 
        return 0xED;
}


int Jsw128Game::decodeWillySprite(int room, int sprite)
{
	int ws = JswGame::decodeWillySprite(room, sprite);
	if (ws < 0x80) ws = 0x9D;
	return ws;
}


int Jsw128Game::encodeWillySprite(int room, int sprite)
{
	if (sprite == 0x9D) return 0;
        return sprite;
}


int Jsw128Game::roomReserved(int room)
{
        return 0;
}


int Jsw128Game::teleUnload(void)
{
	if (m_mem->peek(35696) != 205) return ZXE_CONTINUE;
	jswByte tpage = m_mem->peek(35698);

	if (tpage >= 0x98) m_mem->poke(MEMMAP_BASE + tpage - 0x98, 0);

	return JswGame::teleUnload();
}

int Jsw128Game::teleLoad(void)
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


int Jsw128Game::supportsExtGuards(void)
{
        return 1;
}



jswByte *Jsw128Game::getFont(int *len)
{
        if (len) *len = 96;
        return m_mem->memoryAt(0xC100);
}


jswByte *Jsw128Game::getTitleAttrs(void)
{
        return m_mem->memoryAt(0xD000, 7);
}


bool Jsw128Game::imcMode(void)
{
	return false;
}


unsigned Jsw128Game::getMapsize()
{
        return MEMMAP_LEN;
}

unsigned Jsw128Game::getMemmap()
{
        return MEMMAP_BASE;
}


Room *Jsw128Game::newRoom(int room)
{
        jswByte *rm = getRoom(room);
        if (rm == NULL) return NULL;
        return new Jsw48Room(this, rm, room);
}

int Jsw128Game::getRoomCount()
{
	return 256;
}
