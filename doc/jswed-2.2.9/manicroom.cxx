/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

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
#include "room.hxx"
#include "manicroom.hxx"
#include "manicgame.hxx"

static jswByte query[] = { 0x00, 0x00, 0x3C, 0x42, 0x04, 0x08, 0x00, 0x08, 0x00 };


ManicRoom::ManicRoom(ManicGame *g, jswByte *data, int roomNo) : 
	Room(8, roomNo), m_data(data), m_game(g)
{

}

int ManicRoom::getTitle(char *buf)
{
	sprintf(buf, "%-32.32s", m_data + 0x200);
        for (int n = 0; n < 32; n++)
        {
                if (buf[n] == ZXCH_COPY ) buf[n] = ZXK_COPY;
                if (buf[n] == ZXCH_POUND) buf[n] = ZXK_POUND;
        }
	return 32;
}

void ManicRoom::setTitle(const char *buf)
{
	char fmt[33];

	sprintf(fmt, "%-32.32s", buf);
	memcpy(m_data + 0x200, fmt, 32);
}


jswByte *ManicRoom::getCellPattern(int type)
{
	if (type == 8) /* item */
	{
		m_itemPattern[0] = ZX_WHITE;
		memcpy(m_itemPattern + 1, m_data + 691, 8);
		return m_itemPattern;
	}
	if (type < 0 | type > 7) 
	{
		query[0] = type & 0xFF;
		return query;
	}
	return m_data + 544 + 9 * type;
}


int ManicRoom::getCellAt(int x, int y)
{
	jswByte attrib = m_data[y * 32 + x];
	int n;

	for (n = 0; n < 8; n++)
	{
		if (m_data[544 + 9 * n] == attrib) return n;
	}
	query[0] = attrib;
	return 0x100 | attrib;
}

int ManicRoom::getConveyorDir()
{
	return m_data[623] & 1;
}


int ManicRoom::getConveyorY()
{
	return m_data[625] & 7;
}


void ManicRoom::drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw)
{
        int len;
        int w=m_data[624]+(256*m_data[625]);
        int x =  (w & 0x1F);
        int y = ((w & 0xE0) >> 1) | ((w & 0x800) >> 4); //   ((w >> 1) & 0xF0);

        if (m_data[626] > 32) // Damaged room 
		m_data[626]=0;

	len = m_data[626];

	y = y >> 4;
        for (int n = 0; n < len; n++)
        {
                if (y >= 0 && y < 16 && x >= 0 && x < 32)
                {
			drawCell(bm, x, y, flashonly, draw);
		}
		x++;
	}
	Room::drawOneOffs(bm, flashonly, draw);
}


void ManicRoom::setCell(int x, int y, int type)
{
        if (x < 0 || x > 31 || y < 0 || y > 15) return;
	if (type < 0x100)
	{
		type &= 7;
		type = m_data[544 + 9 * type];
	}
	m_data[32 * y + x] = type & 0xFF;
}


int ManicRoom::getBackground()
{
	return (m_data[544] >> 3) & 0x0F;
}


int ManicRoom::getItemCoords(int *xs, int *ys, int *counts)
{
	int count = 0;
	int n, m, found;
	jswByte *ob;

	for (n = 0; n < 5; n++)
	{
		ob = m_data + 629 + 5 * n;
		if (ob[0] == 0xFF || ob[0] == 0) continue;

		xs[count] = ob[1] & 0x1F;
		ys[count] = (ob[0] >> 3) & 7;
		found = 0;
		for (m = 0; m < n; m++)
		{
			if (ys[m] == ys[count] && xs[m] == xs[count])
			{
				++counts[m];
				found = 1;
				break;
			}
		}
		if (!found) ++count;
	}
	return count;
}



bool ManicRoom::getPortal(Portal &p)
{
	p.attribute = m_data[655];
	p.sprite    = m_data + 656;
        p.x         =   m_data[690] & 0x1F;
        p.y         = ((m_data[690] & 0xE0) >> 5) | (m_data[691] & 8) ;
	return true;
}


int ManicRoom::getCellBehaviour(int idx)
{
	switch(idx)
	{
		case 0: return CB_AIR; 
		case 1: 
		case 7: return CB_WATER;
		case 2: return CB_CRUMBLY;
		case 3: return CB_EARTH;
		case 4: if (getConveyorDir()) return CB_RCONV;
			return CB_LCONV;
		case 5: 
		case 6: return CB_FIRE;
	}
	return CB_WATER;
}


int ManicRoom::getCellTypeCount()
{
	return 8;
}

int ManicRoom::getRoomLength() const
{
	return 1024;
}


jswByte ManicRoom::getSolarAttr()
{
	if (m_game->roomHasFeature(m_roomNumber, FEAT_SOLAR))
	// XXX Peek the game engine
		return ZX_BRWHITE * 8 + ZX_YELLOW;	
	else	return 0;
}

void ManicRoom::getSolarPos(int *x, int *y)
{
	// XXX Peek the game engine
	*y = 0;
	*x = 23;
}

void ManicRoom::setSolarPos(int x, int y)
{
	// XXX poke the game engine
}


void ManicRoom::getCells(Cell *c)
{
	int n;
	jswByte *b = m_data + 544;
	for (n = 0; n < 8; n++)
	{
		memcpy(c[n].pattern, b, 9);
		b += 9;
		c[n].behaviour = getCellBehaviour(n);
	}
}


void ManicRoom::setCells(Cell *c)
{
	int n;
	jswByte *b = m_data + 544;
	for (n = 0; n < 8; n++)
	{
		memcpy(b, c[n].pattern, 9);
		b += 9;
	}
}




void ManicRoom::clear()
{
	int n;
	memset(m_data, 0, 1024);
	memset(m_data + 512, ' ', 32);
	for (n = 629; n <= 654; n += 5) m_data[n] = 0xFF;
	for (n = 702; n <= 730; n += 7) m_data[n] = 0xFF;
	for (n = 733; n <= 761; n += 7) m_data[n] = 0xFF;
}


extern void flip8(jswByte *data);

void ManicRoom::mirrorH()
{
	int x, y, n;
	jswByte bt;
	bool skylabs = m_game->roomHasFeature(m_roomNumber, FEAT_SKYLABS);
	bool vguards = m_game->roomHasFeature(m_roomNumber, FEAT_VGUARDS);

	for (y = 0; y < 16; y++)
		for (x = 0; x < 16; x++)
	{
		bt = m_data[y * 32 + (31-x)];
		m_data[y*32 + (31-x)] = m_data[y*32 + x];
		m_data[y*32 + x] = bt;
	}
	/* Willy's start position */
	x = 30 - (m_data[620] & 0x1F);
	m_data[620] = (m_data[620] & 0xE0) | x;
	m_data[618] ^= 1;

	/* Conveyor animation */
	m_data[623] ^= 1;
	x = (32 - (m_data[624] & 0x1F) - m_data[626]) & 0x1F;
	m_data[624] = (m_data[624] & 0xE0) | x;
	
	/* Mirror items */
	for (n = 630; n <= 650; n += 5)
	{
		x = 31 - (m_data[n] & 0x1F);
		m_data[n] = (m_data[n] & 0xE0) | x;
	}

	/* Mirror portal position */
	x = 30 - (m_data[688] & 0x1F);
	m_data[688] = (m_data[688] & 0xE0) | x;
	x = 30 - (m_data[690] & 0x1F);
	m_data[690] = (m_data[690] & 0xE0) | x;
	/* Mirror horizontal guardians */
	for (n = 702; n <= 723; n += 7)
	{
		x = 30 - (m_data[n+1] & 0x1F);
		m_data[n+1] = (m_data[n+1] & 0xE0) | x;
		m_data[n+4] = 7 - m_data[n+4];
		y = 30 - (m_data[n+5] & 0x1F);
		x = 30 - (m_data[n+6] & 0x1F);
		m_data[n+5] = (m_data[n+5] & 0xE0) | x;
		m_data[n+6] = (m_data[n+6] & 0xE0) | y;
	}
	/* Mirror vertical guardians */
	if (skylabs || vguards) 
	{
		for (n = 733; n <= 754; n += 7)
		{
			x = 31 - (m_data[n+3] & 0x1F);
			m_data[n+3] = (m_data[n+3] & 0xE0) | x;
		}
	}
/* Flip horizontal bitmaps */
	for (n = 545; n <= 608; n += 9)
	{
		flip8(m_data + n);
	}
	flip8(m_data + 692);
	createCellPatterns();
}



void ManicRoom::mirrorV()
{
	int x, y, n;

	for (y = 0; y < 8; y++) 
		for (x = 0; x < 32; x++)
	{
		jswByte *b = m_data +     y  * 32 + x;
		jswByte *c = m_data + (15-y) * 32 + x;
		jswByte a  = *b;
		*b = *c;
		*c = a;	
	}
	/* Mirror items */
	for (n = 630; n <= 650; n += 5)
	{
		y = ((m_data[n] & 0xE0) >> 5) | (m_data[n+2] & 8);
		y = 15 - y;
		m_data[n] = (m_data[n] & 0x1F) | ((y & 7) << 5) ;
		m_data[n+1] &= 0xFE;
		m_data[n+1] |= ((y & 8) >> 3);
		m_data[n+2] &= ~8;
		m_data[n+2] |= (y & 8);	
	}
	for (n = 545; n <= 608; n += 9)
	{
		for (x = 0; x < 4; x++)
		{
			y = m_data[7 + n - x];
			m_data[7 + n - x] = m_data[n + x];
			m_data[n + x] = y;	
		}
	}
	for (x = 0; x < 4; x++)
	{
		y = m_data[699 + n - x];
		m_data[699 + n - x] = m_data[692 + n];
		m_data[692 + n] = y;	
	}
	createCellPatterns();
}


void ManicRoom::getRoomMemMap(char *buf)
{
	bool vguards = m_game->roomHasFeature(m_roomNumber, FEAT_VGUARDS);

	buf[1024] = 0;
	memset(buf    , 'b', 512);
	memset(buf+512, 'x', 256);
	memset(buf+512, 't', 32);
	memset(buf+544, 'c', 72);
	memset(buf+656, 's', 32);
	memset(buf+702, 'g', 30);
	if (vguards)
	{
		memset(buf + 733, 'g', 28);
	}
	else
	{
		memset(buf + 736, 's', 32);
	}
	memset(buf + 768, 's', 256);
}
