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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "j48game.hxx"
#include "guardedit.hxx"
#include "geoffgame.hxx"
#include "geoffguard.hxx"
#include "jswdirbox.hxx"
#include "geoffgedit.hxx"


GeoffGame::GeoffGame(SpectrumMemory *mem) : Jsw48Game(mem)
{
}

GeoffGame::~GeoffGame()
{
}

char *GeoffGame::getID()
{
        char *s = JswGame::getID();

        strcpy(s + 31, " [GEOFF1]");
        return s;                                                               
}


JswGuardian *Geoff2Game::newGuardian(jswByte *guard, int x, int bg)
{
        return new GeoffGuard(guard, x, m_mem, bg);
}


GuardianEditor *Geoff2Game::newGuardianEditor(int room, jswByte *guard, VideoBitmap *b, int x)
{
        return new GeoffGuardEditor(room, this, guard, b, x);
}


int GeoffGame::getMariaRoom(void)
{
        for (int n = 0; n < 64; n++)
        {
                jswByte *room = getRoom(n);

                if (room[0xEE] == 0x3B && room[0xEF] == 0x95) return n;
        }
        return 0;
}


void GeoffGame::setMariaRoom(int r)
{
        for (int n = 0; n < 64; n++)
        {
                jswByte *room = getRoom(n);

                if (n == r)
                {
                        room[0xEE] = 0x3B;
                        room[0xEF] = 0x95;
                }
                else if (room[0xEE] == 0x3B && room[0xEF] == 0x95)
                {
                        room[0xEE] = 0x36;
                        room[0xEF] = 0x96;
                }
        }

}


void GeoffGame::setToiletRoom(int r)
{
        for (int n = 0; n < 64; n++)
        {
                jswByte *room = getRoom(n);

		if (n == r)
		{
			room[0xEE] = 0x84;
			room[0xEF] = 0x95;
		}
		else if (room[0xEE] == 0x84 && room[0xEF] == 0x95) 
		{
			room[0xEE] = 0x36;
			room[0xEF] = 0x96;
		}
        }
	
}



int GeoffGame::getToiletRoom(void)
{
	for (int n = 0; n < 64; n++)
	{
		jswByte *room = getRoom(n);

		if (room[0xEE] == 0x84 && room[0xEF] == 0x95) return n;
	}
	return 0;
}

void GeoffGame::getToiletPos(int *x, int *y)
{
	int hl = m_mem->peek(0x95b8) + 256 * m_mem->peek(0x95B9);
	hl -= 0x5C00;

	*x = (hl & 0x1F);
	*y = (hl & 0x1E0) >> 1;
}


void GeoffGame::setToiletPos(int x, int y)
{
        int hl;

	hl = 0x6800 + x + ((y & 64) *  32) 
			+ ((y &  7) * 256)
			+ ((y & 56) /   8);
        m_mem->poke(0x95be, (hl & 0xFF));
        m_mem->poke(0x95bf, (hl >> 8));


        y *= 2;
        y &= 0x1E0;
        m_mem->poke(0x958E, x | (y & 0xE0));

        hl = 0x5C00 + y + x;
        m_mem->poke(0x95b8, (hl & 0xFF));
        m_mem->poke(0x95b9, (hl >> 8));
        hl += 32;
        m_mem->poke(0x95bb, (hl & 0xFF));
        m_mem->poke(0x95bc, (hl >> 8));

}




Geoff2Game::Geoff2Game(SpectrumMemory *mem) : GeoffGame(mem)
{
	m_objectCount = 0x8451;
	m_objectXY    = 0xBB00;
	m_objectBase  = 0xBA00;
	m_objectStat  = 0xBA00;
	m_guardsEnd   = 0xC000;
	m_guardsBase  = 0xBC00;
	m_spriteBase  = 0x9800;
}

Geoff2Game::~Geoff2Game()
{
}

char *Geoff2Game::getID()
{
        char *s = JswGame::getID();

        strcpy(s + 31, " [GEOFF2]");
        return s;                                                               
}

jswByte *Geoff2Game::getTitleAttrs(void)
{
	return m_mem->memoryAt(0xB700);
}

jswByte Geoff2Game::getWillySpritePage(void)
{
        return 0xB5;
}


