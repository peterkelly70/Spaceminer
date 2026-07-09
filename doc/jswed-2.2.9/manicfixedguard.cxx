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
#include "manicgame.hxx"
#include "manicguardian.hxx"
#include "manicfixedguard.hxx"

ManicFixedGuard::ManicFixedGuard(int type, ManicGame *g, jswByte *room, int bg) :
	ManicVGuardian(room, m_dummyGuard, doConstruct(type, g, room, bg))
{
}

jswByte *ManicFixedGuard::doConstruct(int type, ManicGame *g, jswByte *room, int bg)
{
	static jswByte dummyGuard[8] = {0x07, 0x27, 0xCF, 0xD0,
					0x00, 0xC9, 0xD0, 0xD0 };
	int y, x;
	jswByte *sprite;
	SpectrumMemory *mem = g->getMem();

	switch(type)
	{
		case MFG_WILLY:
			dummyGuard[0] = (bg << 3) | 7;
			dummyGuard[1] = room[617] + 4 * room[618];
			dummyGuard[2] = room[616] / 2;
			dummyGuard[3] = x = room[620] & 0x1F;
			dummyGuard[4] = 0;
			dummyGuard[5] = room[616] / 2;	
			dummyGuard[6] = room[616] / 2;
			sprite = mem->memoryAt(256 * g->getWillySpritePage());
			break;
/*		case MFG_PORTAL:
			dummyGuard[0] = room[655];
			dummyGuard[1] = 0;
			dummyGuard[2] = 8 * ((room[691] & 8) | (room[690] >> 5));
			dummyGuard[3] = x = room[688] & 0x1F;
			dummyGuard[4] = 0;
			dummyGuard[5] = dummyGuard[2];
			dummyGuard[6] = dummyGuard[2];
			sprite = room + 656;
			break; */
	}
	memcpy(m_dummyGuard, dummyGuard, 8);
	return sprite;
}

void ManicFixedGuard::move()
{

}


void ManicFixedGuard::draw(VideoSurface *s)
{
        int frame = (m_dummyGuard[1] & 7) * 32;

        getBmp()->toSurface(s, 16 * m_x, 2 * m_y, frame, 0, 32, 32);
}
