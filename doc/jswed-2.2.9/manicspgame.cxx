/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "manicgame.hxx"
#include "manicspgame.hxx"

ManicSPGame::ManicSPGame(SpectrumMemory *mem) : ManicGame(mem)
{
}

ManicSPGame::~ManicSPGame()
{
}

char *ManicSPGame::getID()
{
        char *s = ManicGame::getID();

        strcpy(s + 29, " [MANIC:SP]");
        return s;
}



unsigned short ManicSPGame::xltAddress(unsigned short address)
{
	if (address < 0x846E) return address;
	if (address < 0x8C2A) return address + 6;
	if (address < 0x8C73) return address + 13;
	if (address < 0x9357) return address + 11;
	return address;
}


