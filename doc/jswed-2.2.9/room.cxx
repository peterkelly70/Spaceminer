/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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
#include "room.hxx"

#include "sun.xbm"


Portal::Portal()
{
	x = y = 0;
	attribute = 0;
	sprite = NULL;
}


bool Portal::operator == (Portal &v)
{
	if (attribute != v.attribute) return false;

	if (sprite && v.sprite) return !memcmp(sprite, v.sprite, 32);
	return false;
}


void Portal::operator = (Portal &v)
{
	x = v.x;
	y = v.y;
	attribute = v.attribute;
	sprite    = v.sprite;
}


bool Portal::operator != (Portal &v)
{
	if (attribute != v.attribute) return true;

	if (sprite && v.sprite) return memcmp(sprite, v.sprite, 32);
	return true;
}


typedef VideoBitmap *BitmapPointer;

Room::Room(int ncells, int roomNumber) : m_ncells(ncells), 
			m_roomNumber(roomNumber)
{
	int j;

	m_cellBitmaps = new BitmapPointer[ncells * 2 + 1];
	for (j = 0; j < 2 * ncells + 1; j++)
	{
		m_cellBitmaps[j] = NULL;
	}
	m_zxCellBitmaps = new jswByte[18 * ncells + 9];
	m_portalBitmap = NULL;
	m_solarBitmap = NULL;
	m_solarFlag = 0xFF;
}


Room::~Room()
{
	for (int j = 0; j < 2 * m_ncells + 1; j++)
	{
		if (m_cellBitmaps[j]) delete m_cellBitmaps[j];
	}
	delete m_cellBitmaps;	
	delete m_zxCellBitmaps;
	if (m_portalBitmap) delete m_portalBitmap;
	if (m_solarBitmap) delete m_solarBitmap;
}


void Room::createCellPatterns()
{
	jswByte *bm;

	for (int j = 0; j < 2 * m_ncells + 1; j++)
	{
		bm = m_zxCellBitmaps + 9 * j;		
		jswByte *buf = getCellPattern(j/2);

// If the cell hasn't changed, don't recreate the bitmaps.
		if (m_cellBitmaps[j])
		{
			if (!memcmp(bm, buf, 9)) continue;
//printf("Regen %d: bm[0]=%02x buf[0]=%02x\n", j, bm[0], buf[0]);
			if (m_cellBitmaps[j]) delete m_cellBitmaps[j];
			m_cellBitmaps[j] = NULL;
		}
		if (buf) 
		{
			int ink   = buf[0] & 7;
			int paper = (buf[0] >> 3) & 7;

			if (buf[0] & 64)  
			{
				ink |= ZX_BRIGHT; 
				paper |= ZX_BRIGHT; 
			}
			m_cellBitmaps[j] = ::bitmapFromSprite(8, 8, buf+1, 1, ink, paper);
			memcpy(bm, buf, 9);
		}
	}
	Portal p;
      
	/* If there is no portal, delete any portal bitmap we have */
	if (!getPortal(p))
	{
		if (m_portalBitmap) delete m_portalBitmap;
		m_portalBitmap = NULL;
	}
	else
	{
		/* If the portal has changed since this was set up */
		if (p != m_portal 
		|| (p.sprite && memcmp(m_portalSprite, p.sprite, 32)))
		{
			if (m_portalBitmap) delete m_portalBitmap;
			m_portal = p;
			/* Take a copy of the sprite */
			memcpy(m_portalSprite, p.sprite, 32);
			p.sprite = m_portalSprite;

			int ink   = m_portal.attribute & 7;
			int paper = (m_portal.attribute >> 3) & 7;

			if (m_portal.attribute & 64)  
			{
				ink |= ZX_BRIGHT; 
				paper |= ZX_BRIGHT; 
			}
			m_portalBitmap = ::bitmapFromSprite(16, 16, m_portal.sprite, 1, ink, paper);
		}
	}
	jswByte sa = getSolarAttr();
	/* If there is no solar power, delete the solar-power bitmap */
	{
		/* If the portal has changed since this was set up */
		if (sa != m_solarFlag)
		{
			if (m_solarBitmap) delete m_solarBitmap;
			int ink = sa ? ZX_BRORANGE: ZX_ORANGE;
		    
		        m_solarBitmap = ::bitmapFromXbm(sun_width, sun_height,
					sun_bits, 0, ZX_BLACK, ink);	
		}
	}
}






void Room::drawCell(int x, int y, int type)
{
	if (type == CT_SOLAR)
	{
		if (!m_solarBitmap) createCellPatterns();
		if (m_solarBitmap) m_solarBitmap->toScreen(x, y);
		else videoScreen->fillBox(x, y, 16, 16, ZX_BLACK);	
		return;	
	}
	if (type == CT_ITEM)
	{
		int n = m_ncells * 2;

		m_cellBitmaps[n]->toScreen(x, y);
		return;
	}
	if (type < 0 || type >= m_ncells) return;

	if (!m_cellBitmaps[type * 2])
	{
		createCellPatterns();	
	}
	if (m_cellBitmaps[type * 2])
	{
		m_cellBitmaps[type*2]->toScreen(x, y);
	}
}	



void Room::drawCell(VideoBitmap *bm, int x, int y, bool flashonly, bool animated)
{
	int cell = getCellAt(x, y);
	jswByte *pattern = getCellPattern(cell);

	if (!pattern) return;
	/* "?" type cell. These are not cached, but generated on the fly. */
	if (cell >= m_ncells)
	{
		int ink   = pattern[0] & 7;
		int paper = (pattern[0] >> 3) & 7;

		if (pattern[0] & 64)  
		{
			ink |= ZX_BRIGHT; 
			paper |= ZX_BRIGHT; 
		}
		if (!flashonly || (pattern[0] & 0x80))
		{
			VideoBitmap *bm2 = ::bitmapFromSprite(8, 8, 
					pattern+1, 1, ink, paper);
			bm2->toSurface(bm, x * 16, y * 16);
		}
		return;
	}

	if (!flashonly || (pattern[0] & 0x80))
	{
		m_cellBitmaps[cell * 2 + (animated ? 1 : 0)]->toSurface(bm, x * 16, y * 16);
	}	
}


void Room::flashCells()
{
	int n, im, ink, paper;
	jswByte *bm;

	for (n = 0; n < 2*m_ncells; n++)
	{
		bm = m_zxCellBitmaps + 9 * n;
		m_cellBitmaps[n]->flash(bm[0]);
	}
	if (m_portalBitmap) m_portalBitmap->flash(m_portal.attribute);
	if (m_solarBitmap)  m_solarBitmap->flash(getSolarAttr());
}




#define CONVEYORANIM 4  /* 2 or even 1 look nicer, but 4 matches JSW best */

void Room::animateConveyor()
{
	int direction = getConveyorDir() & 1;
	int n; 
	int y = getConveyorY();

	for (n = 0; n < m_ncells; n++)
	{
		if (direction == 1)
		{
			m_cellBitmaps[n*2+1]->getSurface()->rotateR(0, 2*y,   16, 2, CONVEYORANIM);
			m_cellBitmaps[n*2+1]->getSurface()->rotateL(0, 2*y+4, 16, 2, CONVEYORANIM);
		}
		else
		{
			m_cellBitmaps[n*2+1]->getSurface()->rotateL(0, 2*y,   16, 2, CONVEYORANIM);
			m_cellBitmaps[n*2+1]->getSurface()->rotateR(0, 2*y+4, 16, 2, CONVEYORANIM);
		}
	}
}


void Room::drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw)
{
	if (m_portalBitmap)
	{
		m_portalBitmap->toSurface(bm, 16 * m_portal.x, 16 * m_portal.y);
	}
	if (m_solarBitmap && getSolarAttr())
	{
		int x, y;
		x = y = -1;
		getSolarPos(&x, &y);
		if (x >=0 && y >= 0)
		{
			m_solarBitmap->toSurface(bm, 16 * x, 16 * y);
		}
	}
}


int Room::getRestartX()
{
	return -1;
}

int Room::getRestartY()
{
	return -1;
}

void Room::setRestartX(int x)
{

}

void Room::setRestartY(int y)
{

}

void Room::drawItems(VideoBitmap *bm, int ink)
{
	int xs[256];
	int ys[256];
	int counts[256];
	int n, count;
	char caption[3];

	count = getItemCoords(xs, ys, counts);
	for (n = 0; n < count; n++)
	{
		drawItem(bm, xs[n], ys[n], ink);
		if (counts[n] > 1)
		{
			if (counts[n] < 256)
			{
				sprintf(caption, "%x", counts[n]);
			}
			else
			{
				sprintf(caption, "??");
			}
			if (strlen(caption) == 1) bm->getSurface()->drawSmallText
				(16*xs[n] + 4, 16*ys[n] + 4, caption, 11 - ink, ZX_BLACK);
			else bm->getSurface()->drawSmallText
				(16*xs[n], 16*ys[n] + 4, caption, 11 - ink, ZX_BLACK);
		}
		++ink;
		if (ink == ZX_WHITE) ink = ZX_MAGENTA;
	}
}


void Room::drawItem(VideoBitmap *bm, int x, int y, int ink)
{
	int n = m_ncells * 2;

	m_cellBitmaps[n]->getSurface()->fillReplace(0,0,16, 16, ZX_WHITE, ink);
	m_cellBitmaps[n]->toSurface(bm, x * 16, y * 16);
	m_cellBitmaps[n]->getSurface()->fillReplace(0,0,16, 16, ink, ZX_WHITE);
}



jswByte *Room::getGuardianBuffer()
{
	return NULL;
}

bool Room::getPortal(Portal &p)
{
	return false;
}

int Room::getMaxGuards()
{
	return 8;
}

void Room::setCellBehaviour(int idx, int cb)
{
}


void Room::getSolarPos(int *x, int *y)
{
}

void Room::setSolarPos(int x, int y)
{
}

jswByte Room::getSolarAttr()
{
	return 0;
}

void Room::setGuardianTable(int offset)
{
}


void Room::getSprites(int *buf, int granularity)
{
	*buf = 0;
}


void Room::setPortalPos(int x, int y)
{
	m_portal.x = x;
	m_portal.y = y;
}
