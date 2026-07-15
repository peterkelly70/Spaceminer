/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "j128guard.hxx"
#include "fixedguard.hxx"
#include "room.hxx"

JswFixedGuard::JswFixedGuard(int type, JswGame *g, Room *r, int bg) :
	Jsw128Guard::Jsw128Guard(m_dummyGuard, doConstruct(type, g, r, bg), g->getMem(), bg)
{
}

int JswFixedGuard::doConstruct(int type, JswGame *g, 
			Room *r, int bg)
{
	static jswByte dummyGuard[8] = {0x02, 0x27, 0xCF, 0xD0,
					0x00, 0xC9, 0xD0, 0xD0 };
	int y, x;
	SpectrumMemory *mem = g->getMem();

	dummyGuard[1] = 0x27;
	switch(type)
	{
		case FG_WILLY:
			dummyGuard[0] = 2;
			dummyGuard[1] |= 0xF0;
			g->getStartPos(&x, &y);
			dummyGuard[3] = dummyGuard[6] = dummyGuard[7] = y;
			dummyGuard[5] = g->getWillySpritePage();
			break;
		case FG_MARIA:
                        dummyGuard[0] = 0x02;   
			dummyGuard[1] |= 0xF0;   
			g->getMariaPos(&x, &y);
			x |= (4 << 5);

                        dummyGuard[3] = dummyGuard[6] = dummyGuard[7] = y;
                        dummyGuard[5] = mem->peek(0x955D);
                        break;
		case FG_TOILET:
                        dummyGuard[0] = 2;      
			g->getToiletPos(&x, &y);

                        dummyGuard[3] = dummyGuard[6] = dummyGuard[7] = y;
                        dummyGuard[5] = mem->peek(0x95b3);
                        break;
		case FG_RESET:
			dummyGuard[0] = 2;
			dummyGuard[1] = 0xF4;
			x = r->getRestartX();
			y = r->getRestartY();
			dummyGuard[3] = dummyGuard[6] = dummyGuard[7] = y;
			dummyGuard[5] = g->getWillySpritePage();
			break;

	}
	memcpy(m_dummyGuard, dummyGuard, 8);
	return x;
}

