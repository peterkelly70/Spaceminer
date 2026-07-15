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
#include "j64room.hxx"
#include "jswgame.hxx"
#include "j64game.hxx"



Jsw64Room::Jsw64Room(Jsw64Game *g, jswByte *data, int roomNumber) : 
	Room(16, roomNumber), m_game(g), m_data(data) 
{

}

int Jsw64Room::getTitle(char *buf)
{
	sprintf(buf, "%-32.32s", m_data + 0xB6);
	return 32;
}

void Jsw64Room::setTitle(const char *buf)
{
	char fmt[33];

	sprintf(fmt, "%-32.32s", buf);
	memcpy(m_data + 0xB6, fmt, 32);
}

jswByte *Jsw64Room::getCellPattern(int cell)
{
	char var = m_game->getVariant();
	int max;
	jswByte *base;

	switch(var)
	{
		case 'V': case 'X':
			base = m_data + 0x6E;
			max = 8;
			break;

		case 'W': case 'Y': case 'Z':
			base = m_data + 0x41;
			max = 13;
			if (cell >= 13 && cell < 16)	// Global cell patterns
			{
				base = m_game->getMem()->memoryAt(0xf50f, 7);
				cell -= 13;
				max = 3;
			}
			break;

		case '[':
			base = m_data + 0x26;
			max = 16;
			break;
	}
	if (cell >= 0x100)
	{
		memcpy(m_queryPattern + 1, m_data + 0x4B, 8);
		m_queryPattern[0] = cell & 0xFF;
		return m_queryPattern;
	}
	else if (cell >= max)
	{
                m_itemPattern[0] = ZX_WHITE;
                memcpy(m_itemPattern + 1, m_data + 0xE1, 8);
		return m_itemPattern;
	}
	return base + 9 * cell;
}

jswByte *Jsw64Room::getBits()
{
	switch(m_game->getVariant())
	{
		case 'V': return m_data + 0x140;
		case 'W': return m_data + 0x100;
		case 'X': return m_data + 0x340;
		case 'Y': return m_data + 0x300;
		case 'Z': return m_data + 0x200;
		case '[': return m_data + 0x300;
	}
	return NULL;
}

int Jsw64Room::getCellSize()
{
	switch(m_game->getVariant())
	{
		case 'V': case 'X': return 3;
		case 'Z': return 8;
	}
	return 4;
}


int Jsw64Room::getCellAt(int x, int y)
{
        int n;
	jswByte attrib;
	jswByte *bits = getBits();
	int nBits     = getCellSize();
	unsigned bitsOffset;
	jswByte smask, dmask, value;

	if (nBits == 8)
	{
		attrib = bits[y * 32 + x];
		for (n = 0; n < 13; n++)
		{
			if (m_data[0x41 + 9 * n] == attrib) return n;
       		}
		jswByte *base = m_game->getMem()->memoryAt(0xf50f, 7);
		for (n = 0; n < 3; n++)
		{
			if (base[n * 9] == attrib)
			{
				return n + 13;
			}
		}
		m_queryPattern[0] = attrib;
        	return 0x100 | attrib;
	}
	bitsOffset = ((y * 32) + x) * nBits;
	bits += (bitsOffset / 8);
	smask = 0x80 >> (bitsOffset & 7);
	dmask = 1 << (nBits - 1);
	for (n = 0, value = 0; n < nBits; n++)
	{
		if (bits[0] & smask) value |= dmask;
		smask = smask >> 1;
		dmask = dmask >> 1;
		if (!smask)
		{
			smask = 0x80;
			++bits;
		}
	}
	return value;	
}


int Jsw64Room::getConveyorDir()
{
        return (m_data[0xD6] & 1);
}

int Jsw64Room::getConveyorY()
{
        return 0;
}

void Jsw64Room::setCell(int x, int y, int type)
{
        int n;
	jswByte attrib;
	jswByte *bits = getBits();
	int nBits     = getCellSize();
	unsigned bitsOffset;
	jswByte smask, dmask;

	if (nBits == 8)
	{
		if (type < 0x100) 
		{
			bits[y * 32 + x] = m_data[0x41 + 9 * type];
		}
		else
		{
			bits[y * 32 + x] = type & 0xFF;
		}
		return;
	}
	bitsOffset = ((y * 32) + x) * nBits;
	bits += (bitsOffset / 8);
	smask = 1 << (nBits - 1);
	dmask = 0x80 >> (bitsOffset & 7);
	for (n = 0; n < nBits; n++)
	{
		if (type & smask) bits[0] |= dmask;
		else		  bits[0] &= ~dmask;
		smask = smask >> 1;
		dmask = dmask >> 1;
		if (!dmask)
		{
			dmask = 0x80;
			++bits;
		}
	}
}



int Jsw64Room::getBackground()
{
	int n, max;
	switch(m_game->getVariant())
	{
		case 'V': case 'X': max = 8; break;
		default: max = 16; break;
	}	
	for (n = 0; n < max; n++)
	{
		// If we find an 'air' cell, return its colour
		if (getCellBehaviour(n) == CB_AIR)
		{
			jswByte *pattern = getCellPattern(n);
			return ((*pattern) >> 3)  & 0x0F;
		}
	}
	return 0;	// Default to black
}

int Jsw64Room::getItemCoords(int *px, int *py, int *counts)
{
	int count = 0;
	int n, m, x, y;
	int rm, found = 0;

	for (n = m_game->getItemCount() ; n <= 0xFF; n++)
	{
		m_game->itemPos(n, &rm, &px[count], &py[count]);
		if (rm == m_roomNumber)
		{
			y = (py[count] / 8) & 0x0F;
			x =  px[count] & 0x1F;
			found = 0;
			for (m = 0; m < count; m++)
			{
				if (py[m] == y && px[m] == x)
				{
					found = 1;
					++counts[m];
					break;
				}
			}
			if (!found)
			{
				py[count] = y;
				px[count] = x;
				counts[count] = 1;
				++count;
			}
		}
	}
	return count;
}



jswByte *Jsw64Room::getGuardianBuffer()
{
	unsigned offset = m_data[0xDB] *256 + m_data[0xDA];

	return addressOf(offset);
}


jswByte *Jsw64Room::addressOf(unsigned offset)
{
	unsigned upperLimit;

	switch(m_game->getVariant())
	{
		case 'V': case 'W':	upperLimit = 0x8200; break;
		default:		upperLimit = 0x8400; break;
	}	
	if (offset >= 0x8000 && offset < upperLimit)
	{
		return m_data + (offset - 0x8000);
	}
 	return m_game->getMem()->memoryAt(offset);
}



/* Populate a portal structure */
bool Jsw64Room::getPortal(Portal &p)
{
	unsigned offset = m_data[0xEE] + 256 * m_data[0xEF];

	if (offset == 0) return false;

	p.sprite    = addressOf(offset);
	p.x         =   m_data[0xF0] & 0x1F;
	p.y         = ((m_data[0xF0] & 0xE0) >> 5) | ((m_data[0xF1] & 1) << 3);
	p.attribute = m_data[0xF4];

	return true;
}

int Jsw64Room::getMaxGuards()
{
	unsigned offset = m_data[0xDA] + 256 * m_data[0xDB];
	switch(m_game->getVariant())
	{
		case 'V':	return 13;
		case 'W':	return 8;
		case 'X':	if (offset >= 0x8000 && offset < 0x8069)
				{
					return (0x8068-offset) / 8;
				}
				if (offset >= 0x8100 && offset < 0x8340)
				{
					return (0x8340-offset) / 8;
				}
				return 16;	/* Random! */
		case 'Y':	if (offset >= 0x8000 && offset < 0x8041)
				{
					return (0x8040-offset) / 8;
				}
				if (offset >= 0x8100 && offset < 0x8300)
				{
					return (0x82FF-offset) / 8;
				}
				return 16;	/* Random! */
		case 'Z':	if (offset >= 0x8000 && offset < 0x8041)
				{
					return (0x8040-offset) / 8;
				}
				if (offset >= 0x8100 && offset < 0x8200)
				{
					return (0x81FF-offset) / 8;
				}
				return 16;	/* Random! */
		case '[':	if (offset >= 0x8000 && offset < 0x8021)
				{
					return (0x8020-offset) / 8;
				}
				if (offset >= 0x8100 && offset < 0x8300)
				{
					return (0x82FF-offset) / 8;
				}
				return 16;	/* Random! */
	}

	// This is an inelegant fix. It ensures the room custom sprites
	// are in the right place for any guardians that use them. And
	// it's inelegant because it shouldn't go here, having nothing 
	// to do with getting the max guardian count.
	char var = m_game->getVariant();
	int len = (var == 'V' || var == 'W') ? 0x200 : 0x400;
	memcpy(m_game->getMem()->memoryAt(0x8000), m_data, len);
	return 0;
}



int Jsw64Room::getCellBehaviour(int index) 
{
	jswByte mask, val;
	jswByte *map;

	switch(m_game->getVariant())
	{
		case 'V': case 'X':
			if (index < 0 || index > 7) return CB_WATER;
			if (index & 1) mask = 0xF0;
			else	       mask = 0x0F; 
			val = m_data[0x69 + (index/2)] & mask;
			if (index & 1) val = val >> 4;
			return val;
		case 'W': case 'Y': case 'Z':
			if (index < 0 || index > 15) return CB_WATER;
			map = m_game->getMem()->memoryAt(0xF4FF, 7);
			return map[index];	
		case '[':
			if (index < 0 || index > 15) return CB_WATER;
			map = m_game->getMem()->memoryAt(0xF519, 7);
			return map[index];
	}
	return CB_WATER;
}


void Jsw64Room::setCellBehaviour(int index, int cb)
{
	jswByte mask, val;
	jswByte *map;

	switch(m_game->getVariant())
	{
		case 'V': case 'X':
			if (index < 0 || index > 7) return;
			if (index & 1) { mask = 0xF0; val = cb << 4; }
			else	       { mask = 0x0F; val = cb; }
			m_data[0x69 + (index/2)] &= ~mask;
			m_data[0x69 + (index/2)] |= (val & mask);
			break;
		case 'W': case 'Y': case 'Z':
			if (index < 0 || index > 15) return;
			map = m_game->getMem()->memoryAt(0xF4FF, 7);
			map[index] = cb;
			break;
		case '[':
			if (index < 0 || index > 15) return;
			map = m_game->getMem()->memoryAt(0xF519, 7);
			map[index] = cb;
			break;
	}
}


void Jsw64Room::drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw)
{
        int len;
        int w = m_data[0xD7]+(256*m_data[0xD8]);
        int x =  (w & 0x1F);
        int y =  (w >> 5) & 0x0F;

        if (m_data[0xD9] > 32) // Damaged room 
		m_data[0xD9]=0;

	len = m_data[0xD9];

//	y = y >> 4;
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



int Jsw64Room::getCellTypeCount()
{
	switch(m_game->getVariant())
	{
		case 'V': case 'X': return 8;
	}
	return 16;
}


int Jsw64Room::getRoomLength() const
{
	switch(m_game->getVariant())
	{
		case 'V': case 'W': return 512;
	}
	return 1024;
}


jswByte Jsw64Room::getSolarAttr()
{
	return m_data[0xDD];
}

void Jsw64Room::getSolarPos(int *x, int *y)
{
	if (x) *x = m_data[0xDC] & 0x1F;
	if (y) *y = m_data[0xDC] >> 5;
}

void Jsw64Room::setSolarPos(int x, int y)
{
	if (x >  31) x = 31;
	if (x <= 0)  x = 0;
	if (y >  7)  y = 7;
	if (y <= 0)  y = 0;;

	m_data[0xDC] = (y << 5) | x;
}


void Jsw64Room::getCells(Cell *c)
{
	int n, max;
	jswByte *base;
	bool globals = false;

	switch(m_game->getVariant())
	{
		case 'V': case 'X':
			base = m_data + 0x6E;
			max = 8;
			break;

		case 'W': case 'Y': case 'Z':
			base = m_data + 0x41;
			max = 13;
			globals = true;
			break;

		case '[':
			base = m_data + 0x26;
			max = 16;
			break;
	}
	for (n = 0; n < max; n++)
	{
		memcpy(c[n].pattern, base, 9);
		base += 9;
		c[n].behaviour = getCellBehaviour(n);
	}

	if (globals)	// Global cell patterns
	{
		base = m_game->getMem()->memoryAt(0xf50f, 7);
		max = 3;
		for (n = 0; n < max; n++)
		{
			memcpy(c[n+13].pattern, base, 9);
			base += 9;
			c[n+13].behaviour = getCellBehaviour(n+13);
		}
	}
}


void Jsw64Room::setCells(Cell *c)
{
	int n, max;
	jswByte *base;
	bool globals = false;

	switch(m_game->getVariant())
	{
		case 'V': case 'X':
			base = m_data + 0x6E;
			max = 8;
			break;

		case 'W': case 'Y': case 'Z':
			base = m_data + 0x41;
			max = 13;
			globals = true;
			break;

		case '[':
			base = m_data + 0x26;
			max = 16;
			break;
	}
	for (n = 0; n < max; n++)
	{
		memcpy(base, c[n].pattern, 9);
		base += 9;
		setCellBehaviour(n, c[n].behaviour);
	}

	if (globals)	// Global cell patterns
	{
		base = m_game->getMem()->memoryAt(0xf50f, 7);
		max = 3;
		for (n = 0; n < max; n++)
		{
			memcpy(base, c[n+13].pattern, 9);
			base += 9;
			setCellBehaviour(n + 13, c[n+13].behaviour);
		}
	}
}


static char no_patch[] =
{
	0xA0, 0x86, 0x9F, 0x86, 0x9F, 0x86
};

void Jsw64Room::clear()
{
	int len;

	switch(m_game->getVariant())
	{
		case 'V':
		case 'W':	len = 512; break;
		case 'X':
		case 'Y':	
		case 'Z':
		case '[':	len = 1024; break;
	}
	memset(m_data,          0, len);
	memset(m_data + 0xB6, ' ', 32);
	m_data[0xDB] = 0x80;
	m_data[0xDE] = 0x38;	/* White Willy */
	m_data[0xDF] = 0xFF;
	m_data[0xE0] = 0xFF;
	memcpy(m_data + 0xF9, no_patch, 6);
	m_data[0] = 0xFF;	// 2.1.6 End of grauniads
}


void Jsw64Room::mirrorH()
{
	int y, x;
	jswByte *bits = getBits();

	for (y = 0; y < 16; y++)
	{
		for (x = 0; x < 16; x++)
		{
			int c1 = getCellAt(x, y);
			int c2 = getCellAt(31 - x, y);
			setCell(x, y, c2);
			setCell(31 - x, y, c1);
		}
	}
}


void Jsw64Room::mirrorV()
{
	int y, x;
	jswByte *bits = getBits();

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int c1 = getCellAt(x, y);
			int c2 = getCellAt(x, 15 - y);
			setCell(x, y, c2);
			setCell(x, 15 - y, c1);
		}

	}
}

void Jsw64Room::createCellPatterns()
{
	int max;

	Room::createCellPatterns();

	max = cellCount() * 2;
	m_cellBitmaps[max]->getSurface()->fillReplace(
			0, 0, 16, 16, ZX_BLACK, ZX_TRANS);
	m_cellBitmaps[max]->makeTransparent();
}


void Jsw64Room::getRoomMemMap(char *buf)
{
	int len, boffs, blen, maxspr;
	int portoff, guardoff, gtoff;
	jswByte *gtable;
	int n;

	switch(m_game->getVariant())
	{
		case 'V': 
			len   = 512;
			boffs = 0x140;
			blen  = 0xc0;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x69, RMM_INUSE, 5);
			memset(buf + 0x6E, RMM_CELL, 72);
			break;	
		case 'W': 
			len = 512;
			boffs = 0x100;
			blen  = 0x100;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x41, RMM_CELL, 117);
			break;
		case 'X': 
			len   = 1024;
			boffs = 0x340;
			blen  = 0xc0;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x69, RMM_INUSE, 5);
			memset(buf + 0x6E, RMM_CELL, 72);
			break;	
		case 'Y': 
			len = 1024;
			boffs = 0x300;
			blen  = 0x100;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x41, RMM_CELL, 117);
			break;
		case 'Z': 
			len = 1024;
			boffs = 0x200;
			blen  = 0x200;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x41, RMM_CELL, 117);
			break;
		case '[': 
			len = 1024;
			boffs = 0x300;
			blen  = 0x100;
			memset(buf, RMM_FREE, len);
			memset(buf + 0x21, RMM_INUSE, 5);
			memset(buf + 0x26, RMM_CELL, 144);
			break;
		default:
			buf[0] = 0;
			return;
	}
	maxspr = 0x80 + (len / 256);
	portoff  = m_data[0xEE] + 256 * m_data[0xEF];
	guardoff = m_data[0xDA] + 256 * m_data[0xDB];
	buf[len] = 0;
	memset(buf + boffs, RMM_BITMAP, blen);
	memset(buf + 0xB6, RMM_TITLE, 32);
	memset(buf + 0xD6, RMM_INUSE, 42);
	if (portoff >= 0x8000 && portoff <= (0x8000 + len - 32))
	{
		memset(buf + (portoff - 0x8000), 's', 32);
	}
	if (guardoff < 0x8000 || guardoff >= (0x8000 + len)) 
	{
		gtable = m_game->getMem()->memoryAt(guardoff);
	}
	else	
	{
		gtoff = guardoff - 0x8000;
		gtable = m_data + gtoff;
		n = 8 * getMaxGuards();
		while (n >= 0)
		{
			buf[n + gtoff] = RMM_GUARDIAN;
			--n;
		}
	}
	n = 0;
	while (gtable[n] != 0xFF)
	{
		switch (gtable[n] & 0x0F) 
		{
			case 1: case 2: case 5: case 6: case 7:
			case 9: case 10: case 13: case 14: case 15:

			if (gtable[n + 5] >= 0x80 && gtable[n + 5] < maxspr)
			{
				memset(buf + 256 * (gtable[n+5] - 0x80),
					RMM_SPRITE, 256);
			}
		}
		n += 8;
	}
}


void Jsw64Room::setGuardianTable(int offset)
{
	m_data[0xDA] = offset & 0xFF;
       	m_data[0xDB] = (offset >> 8) & 0xFF;
}


void Jsw64Room::getSprites(int *list, int granularity)
{
	int n, offset = 0;
	bool okay;

	char buf[1025];
	getRoomMemMap(buf);

	for (offset = 0; buf[offset]; offset++)
	{
		okay = true;
		for (n = 0; n < granularity; n++) 
		{
			if (buf[offset + n] != RMM_SPRITE && 
			    buf[offset + n] != RMM_FREE)
			{
				okay = false;
				break;
			}
		}
		if (!okay) continue;
		// We have a buffer of at least 'granularity' bytes
		list[0] = offset + 0x8000;
		++list;
		offset += (granularity - 1);
	}
	list[0] = 0;
}


void Jsw64Room::setPortalPos(int x, int y)
{
	m_data[0xF0] = x + ((y << 5) & 0xE0);
	m_data[0xF1] = 0x5C | ((y >> 3) & 1);
	m_data[0xF2] = x + ((y << 5) & 0xE0);
	m_data[0xF3] = 0x60 | (y & 8);
	Room::setPortalPos(x, y);
}
