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
#include "j128game.hxx"
#include "sftxgame128.hxx"

Softricks128Game::Softricks128Game(SpectrumMemory *mem) : Jsw128Game(mem)
{
	m_objectCount = 0x83FF;
	m_objectXY    = 0xC600;
	m_objectBase  = 0xC000;
	m_objectStat  = 0xC500;
	m_guardianMask= 0xFF;
}

Softricks128Game::~Softricks128Game()
{
}

char *Softricks128Game::getID()
{
        char *s = JswGame::getID();

        strcpy(s + 28, " [SOFTRI128]");
        return s;                                                               
}

