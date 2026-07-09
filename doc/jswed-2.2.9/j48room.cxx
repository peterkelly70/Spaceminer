/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004,2005  John Elliott <jce@seasip.demon.co.uk>

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
#include "j48room.hxx"
#include "jswgame.hxx"

#define CELL_AIR 	0
#define CELL_EARTH	1
#define CELL_WATER	2
#define CELL_FIRE	3
#define CELL_RAMP	4
#define CELL_CONVEYOR	5
#define CELL_ITEM	6

Jsw48Room::Jsw48Room(JswGame *g, jswByte *data, int roomNumber) : 
			Room(6, roomNumber), m_data(data), m_game(g) 
{

}

int Jsw48Room::getTitle(char *buf)
{
	sprintf(buf, "%-32.32s", m_data + 0x80);
        for (int n = 0; n < 32; n++)
        {
                if (buf[n] == ZXCH_COPY ) buf[n] = ZXK_COPY;
                if (buf[n] == ZXCH_POUND) buf[n] = ZXK_POUND;
        }
	return 32;
}

void Jsw48Room::setTitle(const char *buf)
{
	char fmt[33];

	sprintf(fmt, "%-32.32s", buf);
	memcpy(m_data + 0x80, fmt, 32);
}


jswByte *Jsw48Room::getCellPattern(int type)
{
	switch(type)
	{
		case CELL_AIR:		return m_data + 0xA0; 
		case CELL_EARTH:	return m_data + 0xA9; 
		case CELL_WATER:	return m_data + 0xB2; 
		case CELL_FIRE:		return m_data + 0xBB; 
		case CELL_RAMP:		return m_data + 0xC4; 
		case CELL_CONVEYOR:	return m_data + 0xCD; 
		case CELL_ITEM:
			m_itemPattern[0] = ZX_WHITE;
			memcpy(m_itemPattern + 1, m_data + 0xE1, 8);
			return m_itemPattern;
	}
	return NULL;
}


int Jsw48Room::getCellAt(int x, int y)
{
        jswByte cell = m_data[y * 8 + (x / 4)];

        cell = (cell << ((x % 4) * 2)) & 0xC0;
	return (cell >> 6);
}

int Jsw48Room::getConveyorDir()
{
	return (m_data[0xD6] & 1);
}

int Jsw48Room::getConveyorY()
{
	return 0;
}

void Jsw48Room::drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw)
{
	drawConveyor(bm, flashonly, draw);
	drawRamp(bm, flashonly, draw);
	Room::drawOneOffs(bm, flashonly, draw);
}

void Jsw48Room::drawConveyor(VideoBitmap *bm, bool flashonly, bool draw)
{
	int len;
	int w = m_data[0xD7] + 256 * m_data[0xD8];
	int x = (w & 0x1F);
	int y = (w >> 5) & 0x0F;
	int n;

/* Draw the conveyor */
	jswByte *pattern = getCellPattern(CELL_CONVEYOR);

	if (m_data[0xD9] > 32) m_data[0xD9] = 0;	// Damaged room
	len = m_data[0xD9];

	for (n = 0; n < len; n++)
	{
		if (y >= 0 && y < 16 && x >= 0 && x < 32)
		{
	        	if (!flashonly || (pattern[0] & 0x80))
		        {
				if (draw) m_cellBitmaps[CELL_CONVEYOR * 2 + 1]->toSurface(bm, x * 16, y * 16);
				else drawCell(bm, x, y, flashonly, false);
			}
			x++;
		}

	}	
}


void Jsw48Room::drawRamp(VideoBitmap *bm, bool flashonly, bool draw)
{
        int len;
	int n, step;
        int w = m_data[0xDB] + (256*m_data[0xDC]);
        int x = (w & 0x1F);
        int y = ((w >> 1) & 0xF0);

        if (m_data[0xDD] > 32) /* Damaged room */
		m_data[0xDD]=0;

        len = m_data[0xDD];
        if (m_data[0xDA] != 1) len = (-len);
        if (len < 0) { len = (-len); step = (-1); } else step = 1;

	y = (y >> 4);

	for (n = 0; n < len; n++)
	{
		if (y >= 0 && y < 16 && x >= 0 && x < 32)
	        {
			if (draw) m_cellBitmaps[CELL_RAMP * 2]->toSurface(bm, x * 16, y * 16);
			else drawCell(bm, x, y, flashonly, false);
		}
		x += step; --y;
	}	
}



void Jsw48Room::setCell(int x, int y, int type)
{
	if (x < 0 || x > 31 || y < 0 || y > 15) return;
	type &= 3;
	int dx = (x % 4) * 2;
	jswByte andmask = 0x03;
	jswByte ormask  = type;
	jswByte *cell = m_data + (y * 8 + (x / 4));

	andmask = andmask << (6-dx);
	ormask  = ormask <<  (6-dx);

	*cell &= ~andmask;
	*cell |= ormask;
}


int Jsw48Room::getBackground()
{
	return (m_data[0xA0] >> 3) & 0x0F;
}


int Jsw48Room::getRestartX()
{
	if (!m_game->imcMode()) return -1;
	return m_data[0xEE] & 0x1F;
}       


int Jsw48Room::getRestartY()
{
	if (!m_game->imcMode()) return -1;
	return m_data[0xED];
}


void Jsw48Room::setRestartX(int x)
{       
	if (m_game->imcMode() && x >= 0 && x <= 31)
	{
		m_data[0xEE] = (m_data[0xEE] & 0xE0) | x;
	}
}       


void Jsw48Room::setRestartY(int y)
{
	if (m_game->imcMode() && y >= 0)
	{
		m_data[0xED] = y & 0xFE;
	}
}

int Jsw48Room::getItemCoords(int *px, int *py, int *counts)
{
	int count = 0;
	int n, m, x, y, found;
	int rm;

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
				if (px[m] == x && py[m] == y)
				{
					++counts[m];
					found = 1;
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


jswByte *Jsw48Room::getGuardianBuffer()
{
	int bg = getBackground();
	int msk = m_game->getGuardianMask();
	jswByte *b = m_data + 0xf0;
	jswByte *g = m_game->getGuards(m_roomNumber);

	memset(m_guardianBuffer, 0xFF, sizeof(m_guardianBuffer));
	for (int n = 0; n < 8; n++)
	{
		int ng = b[0] & msk;
		if (ng == msk) break;
		memcpy(m_guardianBuffer + 8 * n, g + 8 * ng, 8);
		m_guardianBuffer[8 * n + 2] = b[1];
		b += 2;
	}
	return m_guardianBuffer;
}


int Jsw48Room::getCellBehaviour(int idx)
{
	switch(idx)
	{
		case 0: return CB_AIR;
		case 1: return CB_WATER;
		case 2: return CB_EARTH;
		case 3: return CB_FIRE;
		case 4: if (getConveyorDir()) return CB_RCONV;
			return CB_LCONV;
		case 5: if (m_data[0xDA] & 1) return CB_RRAMP;
			return CB_LRAMP;
	}
	return CB_WATER;
}

int Jsw48Room::getCellTypeCount() 
{
	return 4;
}

int Jsw48Room::getRoomLength() const
{
	return 256;
}


void Jsw48Room::getCells(Cell *c)
{
	int n;
	jswByte *b = m_data + 0xA0;
	for (n = 0; n < 6; n++)
	{
		memcpy(c[n].pattern, b, 9);
		b += 9;
		c[n].behaviour = getCellBehaviour(n);
	}
}


void Jsw48Room::setCells(Cell *c)
{
	int n;
	jswByte *b = m_data + 0xA0;
	for (n = 0; n < 6; n++)
	{
		memcpy(b, c[n].pattern, 9);
		b += 9;
	}
}



void Jsw48Room::clear()
{
	memset(m_data,        0, 160);
	memset(m_data + 0xF0, 0,  16);
	m_data[0xF0] = 0xFF;
	setTitle("");
}


static jswByte flip4(jswByte b)
{
	return ((b &  3) << 6) | ((b &  12) << 2) |
		((b & 48) >> 2) | ((b & 192) >> 6);
}


void Jsw48Room::mirrorH()
{
	int x, y;
	for (y = 0; y < 16; y++) for (x = 0; x < 4; x++)
	{
		jswByte *b = m_data + (y*8) + x;
		jswByte *c = m_data + (y*8) + (7-x);
		jswByte  a = *b;
		*b = flip4(*c);
		*c = flip4(a);
	}
}



void Jsw48Room::mirrorV()
{
	int x,y;
	for (y = 0; y < 8; y++) for (x = 0; x < 8; x++)
	{
		jswByte *b = m_data +     y  * 8 + x;
		jswByte *c = m_data + (15-y) * 8 + x;
		jswByte a  = *b;
		*b = *c;
		*c = a;
	}
}

void Jsw48Room::createCellPatterns()
{
	Room::createCellPatterns();
	m_cellBitmaps[cellCount()*2]->getSurface()->fillReplace(0, 0, 16, 16,
			ZX_BLACK, ZX_TRANS);
	m_cellBitmaps[cellCount()*2]->makeTransparent();
}

void Jsw48Room::getRoomMemMap(char *buf)
{
	strcpy(buf,
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"	// 32
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"	// 64
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"	// 96
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"	// 128
			"tttttttttttttttttttttttttttttttt"	// 160
			"cccccccccccccccccccccccccccccccc"	// 192
			"ccccccccccccccccccccccxxxxxxxxxx"	// 224
			"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"	// 256
		);
}
