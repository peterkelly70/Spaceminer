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
#include "jswgame.hxx"
#include "j128guard.hxx"
#include "j128glist.hxx"
#include "guardedit.hxx"
#include "j128gedit.hxx"
#include "room.hxx"

JswGame::JswGame(SpectrumMemory *mem) : Game(mem)
{
	m_upgrade = NULL;
	m_objectXY    = 0xA500;
	m_objectCount = 0xA3FF;
	m_objectBase  = 0xA400;
	m_objectStat  = 0xA400;
	m_objectMask  = 0x3F;
	m_guardianMask= 0x7F;
	m_guardsBase  = 0xA000;
	m_guardsEnd   = 0xA600;
	m_spriteBase  = 0x9C00;

	/* Initial PC for JSW48 */
	m_mem->poke(0x5BFE, 0xCA);
	m_mem->poke(0x5BFF, 0x87);
	m_roomCache = NULL;
	m_roomCacheSize = 0;
	m_roomNameOffset = 128;
}

JswGame::~JswGame()
{
	if (m_roomCache)
	{
		for (int n = 0; n < m_roomCacheSize; n++) 
			if (m_roomCache[n]) delete m_roomCache[n];
 		delete m_roomCache;
	}
}


/* Check for Ian Collier's "avoid infinite death" patch. If it's present,
 * start position is set by room. As you can see from the function name,
 * I wasn't sure whether to treat it as a full "mode" or not.
 *
 * Need to back this patch out if going to JSW128.
 * XXX Disallow applying it in Geoff or Andy Mode. 
 */
bool JswGame::imcMode(void)
{
	return (m_mem->peek(0x8C01) == 0x3A); 
}

int JswGame::getStartRoom(void)
{
	if (imcMode()) return m_mem->peek(0x8420);
	return m_mem->peek(0x87EB);
}

void JswGame::setStartRoom(int room)
{
	if (imcMode()) m_mem->poke(0x8420, room);
	else m_mem->poke(0x87EB, room);
}

int JswGame::getMariaRoom(void)
{
        return m_mem->peek(0x9538);
}

void JswGame::setMariaRoom(int room)
{
        m_mem->poke(0x9538, room);
	checkMariaToilet();
}

int JswGame::getToiletRoom(void)
{
        return m_mem->peek(0x9588);
}

void JswGame::setToiletRoom(int room)
{
        m_mem->poke(0x959E, room);
	m_mem->poke(0x9588, room);
	checkMariaToilet();
}

int JswGame::chooseGuardianType(int *t)
{
	int rv;

	if (supportsExtGuards())
	{
		static int types[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 13, 14 };
 		VideoMenu vm128("Guardian type",
				"Blank",                        // 0
				"Horizontal",                   // 1
				"Vertical",                     // 2
				"Rope",                         // 3
				"Arrow",                        // 4
				"Diagonal NW/SE",               // 5
				"Diagonal NE/SW",               // 6 
				"Vertical multicolour",         // 7
				"Horizontal multicolour",       // 8 -> 9
				"NW/SE multicolour",            // 9 -> 13
				"NE/SW multicolour",            // 10 -> 14
				NULL);
		rv = vm128.doModal();
		if (t) 
		{
			*t = types[vm128.getSelected()];	
		}
	}
	else
	{
        	VideoMenu vm48("Guardian type", "Blank", "Horizontal", 
				"Vertical", "Rope", "Arrow", NULL);
		rv = vm48.doModal();
		if (t) *t = vm48.getSelected();
	}
	return rv;
}





void JswGame::objectDelete(int index)
{
	int objmin = m_mem->peek(m_objectCount);

	if (objmin == 0xFF) return;	// Can't delete last object
	// Move all objects up one
	for (int n = index; n > objmin; n--)
	{
		m_mem->poke(m_objectBase + n, m_mem->peek(m_objectBase + n - 1));
		m_mem->poke(m_objectXY   + n, m_mem->peek(m_objectXY   + n - 1));
		m_mem->poke(m_objectStat + n, m_mem->peek(m_objectStat + n - 1));
	}
	m_mem->poke(m_objectCount, objmin + 1);
}


void JswGame::objectInsert(int room, int x, int y)
{
	int objmin = m_mem->peek(m_objectCount);
	
	if (!objmin) return;	// All objects taken

	--objmin;
	y *= 2;
	m_mem->poke(m_objectStat + objmin, ((y & 0x80) | 0x40)); 
	m_mem->poke(m_objectXY   + objmin, (x & 31) | ((y & 0x70) << 1));
	m_mem->poke(m_objectBase + objmin,
		  (m_mem->peek(m_objectBase + objmin) & ~m_objectMask) | room); 
	m_mem->poke(m_objectCount, objmin);
}


int  JswGame::itemPos(int index, int *room, int *x, int *y)
{
	int oy = m_mem->peek(m_objectStat + index) & 0x80;
	int ox = m_mem->peek(m_objectXY   + index);

	if (index < m_mem->peek(m_objectCount)) return -1;

	*room = m_mem->peek(m_objectBase + index) & m_objectMask;


	*x = (ox & 0x1F);
	*y = ((ox & 0xE0) >> 2)	| (oy >> 1); 
//	printf("%d: %02x %02x -> %d %d %d\n",
//		index, oy, ox, *room, *x, *y);
	return 0;
}


int  JswGame::objectAt(int room, int x, int y)
{
        int objmin = m_mem->peek(m_objectCount);
        int n, room1, x1, y1;

	for (n = objmin; n <= 0xFF; n++)
	{
		if (!itemPos(n, &room1, &x1, &y1))
		{
			if (x1 == x && y1 == y && room1 == room) 
				return n;
		}

	}
	return -1;
}

int JswGame::guardTableCount(void)
{
	return 1;
}


int  JswGame::chooseGuardTable(int *ng)
{
	*ng = m_guardsBase;
	return ZXE_OK;
}

jswByte *JswGame::getGuards(int room)
{
        return m_mem->memoryAt(m_guardsBase);
}

int JswGame::chooseGuard(VideoBitmap *bm, jswByte *table, int *ng, int bg)
{
	Jsw128GuardList gl(bm, table, m_mem, bg, getGuardianMask());

	int rv = gl.doModal();

	if (rv >= ZXE_CANCEL) return rv;
	*ng = gl.getSelected();	
	return rv;
}

int JswGame::allowSuperJump(void)
{
	return 0;
}


int JswGame::allowSpriteOverride(void)
{
	if (m_mem->peek(0x9654) == 0x20 &&
            m_mem->peek(0x9655) == 0x84) return 0;	
	return m_mem->peek(0x9654);
}


int JswGame::decodeWillySprite(int room, int sprite)
{
	int j = allowSpriteOverride();
	
	if (j) return sprite;
	
	if (room == m_mem->peek(0x9657)) return m_mem->peek(0x965B);
	else				 return m_mem->peek(0x9652);
}

int JswGame::encodeWillySprite(int room, int sprite)
{
	int j = allowSpriteOverride();

	if (j) return sprite;

	return 0;
}



void JswGame::getStartPos(int *x, int *y)
{
	if (imcMode()) 
	{
		*x = m_mem->peek(0x80EE) & 0x1F;
		*y = m_mem->peek(0x80ED);
	}
	else
	{
		*x = m_mem->peek(0x87F0) & 0x1F;
		*y = m_mem->peek(0x87e6);
	}
}


void JswGame::setStartPos(int x, int y)
{
	if (imcMode()) 
	{
		m_mem->poke(0x80EE, ((y << 1) & 0xE0) | (x & 0x1F));
		m_mem->poke(0x80EF, (0x5C | (y >> 7)));
		m_mem->poke(0x80ED, y);
	}
	else
	{
		m_mem->poke(0x87F0, ((y << 1) & 0xE0) | (x & 0x1F));
		m_mem->poke(0x87F1, (m_mem->peek(0x87F1) & 0xFE) | (y >> 7));
		m_mem->poke(0x87E6, y);
	}
}

void JswGame::getMariaPos(int *x, int *y)
{
	int hl = m_mem->peek(0x955F) + 256 * m_mem->peek(0x9560);
	int y1, y2, y3;

	*x =  hl & 0x1F;
	
	hl &= 0x0FE0;
        y1 = (hl >>  8) & 7;
        y2 = (hl >>  5) & 7;
        y3 = (hl >> 11) & 3;

        *y = 2*(y1 + 8*y2 + 64*y3);
}

void JswGame::setMariaPos(int x, int y)
{
	int hl = m_mem->peek(0x955F) + 256 * m_mem->peek(0x9560);
	int y1, y2, y3;
	y &= 0xF0;
	int w = (2*y) + x;
	
	hl = (hl & ~(0xFFF)) | x;
	y1 = ((y >> 1) & 7) << 8;
	y2 = ((y >> 4) & 7) << 5; 
	y3 = ((y >> 7) & 3) << 11;
	hl |= (y1+y2+y3);

	m_mem->poke(0x955F, (hl & 0xFF));
	m_mem->poke(0x9560, (hl >> 8));

	hl = 0x5C00 + w;
        m_mem->poke(0x956d, (hl & 0xFF));
        m_mem->poke(0x956e, (hl >> 8));
	hl += 32;
        m_mem->poke(0x9573, (hl & 0xFF));
        m_mem->poke(0x9574, (hl >> 8));
}


void JswGame::getToiletPos(int *x, int *y)
{	
	*x = m_mem->peek(0x95B9) & 0x1F;
	*y = m_mem->peek(0x95B6);
}

void JswGame::setToiletPos(int x, int y)
{
        int hl;

	m_mem->poke(0x95B9, x);
	m_mem->poke(0x95B6, y);

	y *= 2;
	y &= 0x1E0;
	m_mem->poke(0x958E, x | (y & 0xE0));

        hl = 0x5C00 + y + x;
        m_mem->poke(0x95c2, (hl & 0xFF));
        m_mem->poke(0x95c3, (hl >> 8));
        hl += 32;
        m_mem->poke(0x95c5, (hl & 0xFF));
        m_mem->poke(0x95c6, (hl >> 8));

}


int JswGame::prevRoom(int room)
{
	int o = room;
	do
	{
		room--;
		if (room < 0) return o;
	} while (roomReserved(room));
	return room;
}

int JswGame::nextRoom(int room)
{
	int o = room;
	do
	{
		room++;
		if (room > 255) return o;
	} while (roomReserved(room));
	return room;
}




JswRoomList *JswGame::getRoomList(void)
{
        int n;
        jswByte *b;
        char rname[40];

        JswRoomList *s = new JswRoomList();

        for (n = 0; n < 256; n++) if (!roomReserved(n))
        {
                b = getRoom(n);
                sprintf(rname, "%03d %-32.32s", n, b + m_roomNameOffset);
                s->addString(rname);
        } 
        return s;
}


int JswGame::canTeleport(void)
{
	if (m_mem->peek(35696) == 205) return 1;
	return 0;	
}



jswByte *JswGame::getTeleportList(void)
{
	int addr;

	if (!canTeleport()) return NULL;

	addr = m_mem->peek(35698) * 256 + m_mem->peek(35697);
	addr = m_mem->peek(addr+6) * 256 + m_mem->peek(addr+5);

	return m_mem->memoryAt(addr);
}

int JswGame::teleUnload(void)
{
	m_mem->poke(35696, 0x01);
	m_mem->poke(35697, 0xFE);
	m_mem->poke(35698, 0xEF);
	return ZXE_CONTINUE;
}

int JswGame::teleLoad(int page)
{
	static jswByte tport[] = {
        	0xD5,			// PUSH DE
		0xDD,0xE5,		// PUSH IX
		0xDD,0x21,0x4A,0x01,	// LD IX,buffer
		0x11,0x04,0x00,		// LD DE,4
		0xDD,0x46,0x00,		// LD B,(IX+0)
		0xDD,0x23,		// INC IX
		0x78,			// LD A,B
       		0xB7,			// OR A
		0x28,0x30,		// JR Z,NOTEL
// LOOP:
		0x3A,0x20,0x84,		// LD A,(SHEET)
		0xDD,0xBE,0x00,		// CP (IX+0)
		0x20,0x24,		// JR NZ,NOTME
		0x3A,0xCF,0x85,		// LD A,(WILLYY)
		0xDD,0xBE,0x01,		// CP (IX+1)
		0x20,0x1C,		// JR NZ,NOTME
		0x3A,0xD3,0x85,		// LD A,(WILLYX)
		0xE6,0x1F,		// AND 1Fh
		0xDD,0xBE,0x02,		// CP (IX+2)
		0x20,0x12,		// JR NZ,NOTME
		0xDD,0x7E,0x03,		// LD A,(IX+3)
		0x32,0x20,0x84,		// LD (SHEET),A
		0xE5,			// PUSH HL
		0x00,0x00,0x00,		// Teleporter FX
		0xE1,			// POP HL
		0xDD,0xE1,		// POP IX
		0xD1,			// POP DE
		0xC1,			// POP BC
		0xC3,0x12,0x89,		// JP L8912
// NOTME:
 		0xDD,0x19,		// ADD IX,DE
		0x10,0xD0,		// DJNZ LOOP
// NOTEL:
		0xDD,0xE1,		// POP IX
		0xD1,			// POP DE
		0x01,0xFE,0xEF,		// LD BC,EFFEh
		0xC9,			// RET
		0x00 };

	memcpy(m_mem->memoryAt(page * 256), tport, 
		sizeof(tport));
	m_mem->poke(page * 256 + 6, page);
        m_mem->poke(35696, 0xCD);
        m_mem->poke(35697, 0x00);
        m_mem->poke(35698, page);
	return ZXE_CONTINUE;
}

int JswGame::teleLoad(void)
{
	int rv;

	JswRoomList *sl = getRoomList();	

	if(!sl) return ZXE_CONTINUE;

	sl->setTitle("Room to be replaced");
	rv = sl->doModal();	

	if (rv >= ZXE_QUIT) { delete sl; return rv; }
	if (rv != ZXE_OK)   { delete sl; return ZXE_CONTINUE; }

	teleLoad(sl->getSelectedRoom() + 0xC0);
	delete sl;

	return ZXE_CONTINUE;
}



int JswGame::toggleTeleporters(void)
{
	int rv;

	if (canTeleport())
	{
		VideoMenu vm("Delete all teleporters?", "Confirm", "Cancel",
				NULL);

		rv = vm.doModal(); 
		if (rv >= ZXE_QUIT) return rv;
		if (rv != ZXE_OK || vm.getSelected() != 0) return ZXE_CONTINUE;
		teleUnload();
		return rv;	
	}
	return teleLoad();
}



int JswGame::guardianEditor(int room, int guardian, jswByte *table, int x, 
		VideoBitmap *backg)
{
	jswByte theGuard[8];

	memcpy(theGuard, table + 8 * guardian, 8);

	GuardianEditor *g = newGuardianEditor(room, theGuard, backg, x);

	int rv = g->doModal();
	delete g;
	
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_OK) memcpy(table + 8 * guardian, theGuard, 8);
	return ZXE_CONTINUE;
}

GuardianEditor *JswGame::newGuardianEditor(int room, jswByte *guard, VideoBitmap *b, int x)
{
	return new Jsw128GuardEditor(room, this, guard, b, x);
}


int JswGame::supportsExtGuards(void)
{
	return 0;
}


jswByte *JswGame::getFont(int *len)
{
	if (len) *len = 0; 
	return NULL;
}



int JswGame::clearRooms(void)
{
	jswByte *b;
	VideoMenu vm("This is destructive!", 
                     "Delete all rooms", "Cancel", NULL);

	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK || vm.getSelected() != 0) return ZXE_CONTINUE;

	for (int n = 0; n < 256; n++)
	{			 
		char buf[33];
		sprintf(buf, "           Room %03d             ",n);
		Room *rc = getRoomClass(n);
		if (rc)
		{
			rc->clear();
			rc->setTitle(buf);
		}
	}
	// Delete all items
	m_mem->poke(m_objectCount, 0xFF);
	return ZXE_CONTINUE;
}


int JswGame::clearSprites(void)
{
       VideoMenu vm("This is destructive!",
                     "Delete all sprites", "Cancel", NULL);

        int rv = vm.doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv != ZXE_OK || vm.getSelected() != 0) return ZXE_CONTINUE;

	deleteSprites();
	return ZXE_CONTINUE;
}


int JswGame::clearGuardians(void)
{
	char tbuf[40];
	int gt;
	int rv  = chooseGuardTable(&gt);
	jswByte *b;
	
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK)   return ZXE_CONTINUE;

	sprintf(tbuf, "Clear guardian table %04x", gt);
        VideoMenu vm("This is destructive!",
                     tbuf, "Cancel", NULL);
	rv = vm.doModal();
        if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK)   return ZXE_CONTINUE;

	b = m_mem->memoryAt(gt);
	memset(b, 0, 1023);
	b[1016] = 0xFF;	
	return ZXE_CONTINUE;
}


int JswGame::getItemCount()
{
	return m_mem->peek(m_objectCount);
}

jswByte *JswGame::getTitleAttrs(void)
{
        return m_mem->memoryAt(0x9800);
}        


jswByte JswGame::getWillySpritePage(void) 
{
	return 0x9D;
}


JswGuardian *JswGame::newGuardian(jswByte *guard, int x, int bg)
{
	return new Jsw128Guard(guard, x, m_mem, bg);
}

void JswGame::onChangeMemmap(void)
{
}


typedef Room *RoomPtr;

Room *JswGame::getRoomClass(int room)
{
	Room *r, **rc;
	int newsize, n;

	if (room < 0 || room >= getRoomCount()) return NULL;

	if (room < m_roomCacheSize)
	{
		if (m_roomCache[room]) return m_roomCache[room];
		r = newRoom(room);
		m_roomCache[room] = r;
		return r;
	}

	r = newRoom(room);
	if (m_roomCacheSize == 0) newsize = 64;
	else			  newsize = 2 * m_roomCacheSize;

	/* XXX Ensure that the new size will take the room in question! */
	while (newsize < room) newsize *= 2;

	rc = new RoomPtr[newsize];
	for (n = 0; n < newsize; n++) rc[n] = NULL;
	if (m_roomCache)
	{
		for (n = 0; n < m_roomCacheSize; n++) rc[n] = m_roomCache[n];
		delete m_roomCache;
	}
	m_roomCache = rc;
	m_roomCacheSize = newsize;
	rc[room] = r;
	return r;
}


/* The standard game engine doesn't work if Maria and the toilet are in the
 * same room. Patch it dynamically if they end up there. */
void JswGame::checkMariaToilet()
{
	if (getMariaRoom() == getToiletRoom())
	{
		m_mem->poke(0x9539, 0x18);
		m_mem->poke(0x9567, 0xB6);
		m_mem->poke(0x959A, 0xC0);
		m_mem->poke(0x959B, 0xCD);
		m_mem->poke(0x959C, 0x3B);
		m_mem->poke(0x959D, 0x95);
		m_mem->poke(0x959E, 0x00);
		m_mem->poke(0x959F, 0x00);
	}
	else
	{
		m_mem->poke(0x9539, 0x20);
		m_mem->poke(0x9567, 0xB7);
		m_mem->poke(0x959A, 0x3A);
		m_mem->poke(0x959B, 0x20);
		m_mem->poke(0x959C, 0x84);
		m_mem->poke(0x959D, 0xFE);
		m_mem->poke(0x959E, getToiletRoom());
		m_mem->poke(0x959F, 0xC0);
	}
}
