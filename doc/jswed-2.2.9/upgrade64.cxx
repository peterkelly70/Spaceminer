/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004,2006  John Elliott <jce@seasip.demon.co.uk>

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
#include "snapio.hxx"
#include "j128game.hxx"
#include "j64game.hxx"
#include "patch.hxx"
#include "room.hxx"

extern Patch patchDeathless;

static int y = 0;
extern VideoBitmap *bm;

extern jswByte jsw_bank_0[], patch_tune2[], patch_tune[];
extern jswByte title64[];
extern jswByte memmap64[];
extern jswByte patch_8ff8[];
extern jswByte patch_8d53[];
extern jswByte patch_8dc5[];
extern jswByte patch_90ca_11[];	
extern jswByte patch_91c8[];
extern jswByte patch_ec66[];
extern jswByte patch_fefc[]; 


static void startLog(char *s)
{
        bm = newVideoBitmap(0, 0, VIDEO_W, VIDEO_H);
	y = 0;
	videoScreen->cls();
	videoScreen->bottomBar(s);
}

static void stopLog(char *s, int ok = 0)
{
	videoScreen->drawText(0, VIDEO_H - CHAR_H, s,
				ZX_BLACK, ZX_WHITE);
	videoWaitForKey(ZXE_CANCEL);		
	if (!ok) bm->toScreen(0, 0);	
	return;
}

static void doLog(char *s)
{
	videoScreen->drawText(0, y, s, ZX_BLACK, ZX_WHITE);
	y += CHAR_H;
	if (y >= (VIDEO_H - 3 * CHAR_H)) 
	{
		y = 0;
		videoScreen->drawText(0, VIDEO_H - CHAR_H, "[More]", ZX_BLACK,
					ZX_WHITE);
		videoWaitForKey(ZXE_CANCEL);
		videoScreen->fillBox(0, VIDEO_H - CHAR_H, VIDEO_W, CHAR_H,
					ZX_WHITE);
	}
	videoScreen->fillBox(0, y, VIDEO_W, CHAR_H, ZX_WHITE);
}



static void upgrade(char *s, SpectrumMemory *m, unsigned short *patch)
{
	doLog(s);
        int n = 0;
	unsigned short a,v;

        do {
                a = patch[n++];
                v = patch[n++];

                if (a) m->poke(a,v);

        } while(a);
}
char *getFtail(char variant)
{
	char *ftail;
	switch(variant)
	{
		case 'V': ftail = "v"; break;
		case 'W': ftail = "w"; break;
		case 'X': ftail = "x"; break;
		case 'Y': ftail = "y"; break;
		case 'Z': ftail = "z"; break;
		case '[': ftail = "yy"; break;
		default:  return NULL;
	}
	return ftail;
}



Jsw64Game *JswGame::upgrade64(char variant)
{
	int n;
	int andy_mode = 0, geoff_mode = 0, strx_mode = 0;
	char *ftail;
	char rt7buf[20];
	char hexbuf[20];
	FILE *fprt7;
	char *hexfile;
	jswByte objTable[512];

	ftail = getFtail(variant);
	if (!ftail) return NULL;
	sprintf(rt7buf, "rtime7%s.bin", ftail);
	sprintf(hexbuf, "room_%s.hex", ftail);
	
	startLog("Upgrade to JSW64");
	doLog("Examining game");
	if (m_mem->peek(0x892E) == 0) 
	{
		strx_mode = 1;
		   //  1...5...10...15...20...25...30...35...40
		doLog("Softricks engine. This'll never work.");
	}	
	else switch (m_mem->peek(0x9650))
	{
		case 0x5F: doLog("Original JSW engine."); break;
		case 0x08: doLog("Geoff Mode JSW. Cannot upgrade."); 
			   geoff_mode = 1;
			   stopLog("A Cannot convert Geoff Mode JSW");
			   return NULL;
		case 0xEE: doLog("Andrew Mode JSW. This'll never work.");
			   andy_mode = 1;
			   break;
		default:   doLog("Unknown engine. Cannot upgrade.");
			   stopLog("2 Unknown JSW Engine");
			   return NULL;
	}
	SpectrumMemory *mem = new SpectrumMemory(m_mem, 1);
	
	if (!mem) 
	{
		stopLog("4 Out of memory");
		return NULL;
	}
	/* mem is now a 128k memory image */

	/* Copy the start message to bank 7 */

	doLog("Generating title screen");
	memcpy(mem->memoryAt(0xC000, 7), title64, 0x1600);
	memcpy(mem->memoryAt(0xD600, 7), m_mem->memoryAt(0x8454), 256);
	mem->poke(0xD700, 0, 7);

	/* Load rtime.js7 */
	fprt7 = openResource(rt7buf, "rb");
	if (!fprt7)
	{
		stopLog("F Failed to open RTIME*.JS7");
		return NULL;
	}
	if (fread(mem->memoryAt(0xDB00, 7), 1, 7000, fprt7) < 7000)
	{
		stopLog("R Failed to load RTIME*.JS7");
		return NULL;
	}
	fclose(fprt7);
	/* Title tune */
	memcpy(mem->memoryAt(0xF52A, 7), jsw_bank_0 + 0x2C2A, 2774);

	/* Reverse any patches that don't come over */
	if (imcMode())
	{	      
		doLog("Reversing infinite death patch");
		int n;
		for (n = 0; n < patchDeathless.lineCount; n++)
		{
			mem->poke(patchDeathless.lines[n].address,
					patchDeathless.lines[n].oldVal);		
		}	
	}


	/* Convert the room database */
	
	if (strx_mode)
	{
        	memcpy(mem->memoryAt(0xC000, 1), m_mem->memoryAt(0xC000), 0x3E00);
		memcpy(objTable, m_mem->memoryAt(0xFE00), 0x200);
	}
	else 
	{
		memcpy(mem->memoryAt(0xC000, 1), m_mem->memoryAt(0xC000), 0x4000);
		memcpy(objTable, m_mem->memoryAt(0xA400), 0x200);
	}
	/* Copy item count */
	m_mem->poke(0x85CA, m_mem->peek(0xA3FF));

	/* Generate the JSW64 bank 0. Start by copying the JSW128 
	 * bank 0, which has the music code. Then do the HL9 upgrades
	 * on it. */
	memcpy(mem->memoryAt(0xC000, 0), jsw_bank_0, 0x4000);
	memcpy(mem->memoryAt(0xFEC0, 0), patch_tune, 64);
	memcpy(mem->memoryAt(0xF902, 0), patch_tune2, 245);
        /* Convert the item location table */
	doLog("Converting item table");
	if (strx_mode) 
	{
		for (n = 0; n < 256; n++)
		{
			mem->poke(0xC000+n, objTable[n] & 0x3F);
			mem->poke(0xC500+n, objTable[n]);
			mem->poke(0xC600+n, objTable[n + 0x100]);	
		}
		doLog("Softrix-specific fixes");
		mem->poke(0x8640+0x2D, 0x80);
		mem->poke(0x8640+0x2E, 0x80);
		mem->poke(0x8640+0x0C, 0x41);
                mem->poke(0x8640+0x0D, 0x41);
                mem->poke(0x8640+0x0E, 0x41);
		mem->poke(0x8640+0x0F, 0x41);
		mem->poke(0x8801, 0xC5);
		mem->poke(0x93D2, 0xC5);
		mem->poke(0x88F3, 0xE0);
		mem->poke(0x86A1, 0xFB);
		mem->poke(0x86A5, 0xFB);
	}
        else for (n = 0; n < 256; n++)
        {
                mem->poke(0xC000+n, m_mem->peek(0xA400 + n) & 0x3F);
        }

	// Apply the patches...
	// XXX The jswed 2.2 source tree does this after item import. 
	// The 2.3 tree did it before. Which is correct?
	char *boo = mem->loadHexPatch(getResourceName(hexbuf, "r"));
	if (boo)
	{
		doLog(boo);
		return NULL;	
	}

	Jsw64Game *g;
/*	if (strx_mode) g = new Softricks128Game(mem);
	else           */g = new Jsw64Game(mem);

	// Migrate the rooms
	doLog("Converting rooms");
	for (int nroom = 0; nroom < 64; nroom++)
	{
		int x, y, ct;
		Cell cell[16];
		Room *room48 = getRoomClass(nroom);
		Room *room64 = g->getRoomClass(nroom);
		jswByte *room64bits = g->getRoom(nroom);
		jswByte *room48bits = getRoom(nroom);

		room64->clear();
		if (!room48) continue;
		// Copy cell patterns
		for (x = 0; x < 16; x++)
		{
			cell[x].behaviour = x % 10;
			cell[x].pattern[0] = 0xFF;
		}
		memcpy(cell[0].pattern, room48bits + 0xA0, 9);
		memcpy(cell[1].pattern, room48bits + 0xA9, 9);
		memcpy(cell[2].pattern, room48bits + 0xB2, 9);
		memcpy(cell[3].pattern, room48bits + 0xBB, 9);
		if (room48bits[0xDA] == 1)
			memcpy(cell[5].pattern, room48bits + 0xC4, 9);
		else	memcpy(cell[4].pattern, room48bits + 0xC4, 9);
		if (room48bits[0xD6] & 1)
			memcpy(cell[7].pattern, room48bits + 0xCD, 9);
		else	memcpy(cell[6].pattern, room48bits + 0xCD, 9);
		room64->setCells(cell);
		room64->setTitle((char *)room48bits + 0x80);

		for (y = 0; y < 16; y++)
		{
			for (x = 0; x < 32; x++)
			{
				room64->setCell(x, y, room48->getCellAt(x,y));
			}
		}
		// Copy conveyor rules
		room64bits[0xD6] = room48bits[0xD6];
		room64bits[0xD7] = room48bits[0xD7];
		room64bits[0xD8] = room48bits[0xD8];
		room64bits[0xD9] = room48bits[0xD9];

		// Draw conveyor 
	        int len, step;
	        int w = room48bits[0xD7] + 256 * room48bits[0xD8];
		x = (w & 0x1F);
	        y = (w >> 5) & 0x0F;

		len = room48bits[0xD9] & 0x1F;
		for (n = 0; n < len; n++)
		{
			if (y >= 0 && y < 16 && x >= 0 && x < 32)
			{
				room64->setCell(x, y, 6 + (room48bits[0xD6] & 1));
				x++;
			}
		}
		// Draw ramp
		w = room48bits[0xDB] + (256*room48bits[0xDC]);
		x = (w & 0x1F);
		y = ((w >> 5) & 0x0F);
		len = room48bits[0xDD] & 0x1F;
		if (room48bits[0xDA] != 1) len = -len;
		if (len < 0) { len = -len; step = -1; } else step = 1;
		for (n = 0; n < len; n++)
		{
			if (y >= 0 && y < 16 && x >= 0 && x < 32)
			{
				room64->setCell(x, y, 4 + (step == 1 ? 1 : 0));
				x+= step;
				--y;
			}
		
		}

		// If old room had escalator...
		if (room48bits[0xC4] == room48bits[0xCD])
		{
			// If conveyor direction = ramp direction, escalator
			// is going up.
			if ((room48bits[0xD6] & 1) == (room48bits[0xDA] & 1))
				room64bits[0xFF] |= 4;
			else	room64bits[0xFF] |= 6;

		}

		for (x = 0xF0, y = 0; x < 0x100; x += 2)
		{
			if (room48bits[x] == 0xFF) break;
			++y;
		}
		// y = count of guardians

		if (variant == '[' && y > 4)
		{
			room64->setGuardianTable(0x81FF - (8 * y));
		}
		else	room64->setGuardianTable(0x8000);

		// Guardian table
		memcpy(room64->getGuardianBuffer(), 
		       room48->getGuardianBuffer(), (8 * y) + 1);
		// Arrows
		jswByte *buf = room64->getGuardianBuffer();
		for (n = 0; n < y; n++)
		{
			if ((buf[n*8] & 0x0F) == 4)	// Arrow
			{
				buf[n*8+1] = 7;	// White
				if (buf[n*8] & 0x80) buf[n*8+3] = 0x7F;
				else		     buf[n*8+3] = 0xFE;	
			}
		}
		// No solar power
		room64bits[0xDC] = 0;
		room64bits[0xDD] = 0;
		// Border etc.
		room64bits[0xDE] = (room48bits[0xDE] & 7) | 0x38;
		// Air
		room64bits[0xDF] = 0xFF;
		room64bits[0xE0] = 0xFF;
		// Item
		memcpy(room64bits+0xE1, room48bits+0xE1, 9+4);
		// Willy's sprite
		if (!andy_mode)
		{
			if (n == 29) room64bits[0xED] = 0xB6;
			else room64bits[0xED] = 0;
		}
		else	room64bits[0xED] = room48bits[0xED];
	}

	// V and W have an item table at C000. Others don't.	
	if (variant == 'V' || variant == 'W')	memmap64[40] = 128;
	else					memmap64[40] = 0;
	
	memcpy(mem->memoryAt(g->getMemmap()), memmap64, g->getMapsize());	

	// Transfer teleporters
	if (canTeleport())
	{
		doLog("Transferring teleports");
		jswByte *b = getTeleportList();
		g->teleLoad();
		jswByte *c = g->getTeleportList();
		memcpy(c, b, 4 * b[0] + 1);		

	}
	if (mem->peek(0x8C0C) == 0x95)
	{
		doLog("Upgrading Softricks death patch");
		mem->poke(0x8C04, 0x6E);
		mem->poke(0x8C0A, 0x21);
		mem->poke(0x881B, 0x19);
		mem->poke(0x8C0C, 0x6E);
	}
	
	stopLog("0 OK", 1);

	g->upgrade128();	
	return g;
}


int Jsw64Game::upgrade128()
{
	int l = 0;
	startLog("Upgrading JSW64 engine");

	if (m_mem->peek(0x93F7) == 0xDD &&
	    m_mem->peek(0x93F8) == 0x34)
	{
		l = 9;
		doLog("HL9 to HL10 upgrade");

		// Bonus rooms
		m_mem->poke(0x93F7, 0xCD);
		m_mem->poke(0x93F8, 0xF8);
		m_mem->poke(0x93F9, 0x8F);

		memcpy(m_mem->memoryAt(0x8FF8), patch_8ff8, 18);

		// Escalators
		m_mem->poke(0x8D94, 0xCD);
		m_mem->poke(0x8D95, 0x53);
		m_mem->poke(0x8D96, 0x8D);
		m_mem->poke(0x8D9B, 0xCD);
		m_mem->poke(0x8D9C, 0x53);
		m_mem->poke(0x8D9D, 0x8D);
		m_mem->poke(0x8DA2, 0xCD);
		m_mem->poke(0x8DA3, 0x57);
		m_mem->poke(0x8DA4, 0x8D);
		m_mem->poke(0x8DA9, 0xCD);
		m_mem->poke(0x8DAA, 0x57);
		m_mem->poke(0x8DAB, 0x8D);
		memcpy(m_mem->memoryAt(0x8D53), patch_8d53, 23);

		// Diagonal guardians bugfix
		m_mem->poke(0x8530, 0x18);
		m_mem->poke(0x8531, 0xF2);
		m_mem->poke(0x8532, 0x00);
		m_mem->poke(0x8533, 0x00);

		char rt7buf[20];
		FILE *fprt7;

		char *ftail = getFtail(getVariant());
		if (ftail) 
		{
			sprintf(rt7buf, "rtime7%s.bin", ftail);
			fprt7 = openResource(rt7buf, "rb");
			if (!fprt7)
			{
				stopLog("F Failed to open RTIME*.JS7");
				return ZXE_CANCEL;
			}
// Don't read all of RTIME*.JS7 -- lose the last few bytes, because they
// may contain user-edited data. But overlay with the latest code.
			if (fread(getMem()->memoryAt(0xDB00, 7), 1, 0x19FF, fprt7) < 0x19FF)
			{
				stopLog("R Failed to load RTIME*.JS7");
				return ZXE_CANCEL;
			}
			fclose(fprt7);
		}
	}
	/* check for HL10 or less */
	if (m_mem->peek(0x90CA) == 0xE6 &&
	    m_mem->peek(0x90CB) == 0x0F)
	{
		l = 10;
		doLog("HL10 to HL11 upgrade");

		m_mem->poke(0x84e2, 0x8b);	// Move a type-8 guardian
		m_mem->poke(0x84e3, 0xec);
		memcpy(m_mem->memoryAt(0x8DC5), patch_8dc5, 14);
		memcpy(m_mem->memoryAt(0x90CA), patch_90ca_11, 9);	
		memcpy(m_mem->memoryAt(0x91C8), patch_91c8, 6);
		memcpy(m_mem->memoryAt(0xEC66), patch_ec66, 0x95);
		memcpy(m_mem->memoryAt(0xFEFC), patch_fefc, 0x104);

		// Reload RTIME7 again
		char rt7buf[20];
		FILE *fprt7;

		char *ftail = getFtail(getVariant());
		if (ftail) 
		{
			sprintf(rt7buf, "rtime7%s.bin", ftail);
			fprt7 = openResource(rt7buf, "rb");
			if (!fprt7)
			{
				stopLog("F Failed to open RTIME*.JS7");
				return ZXE_CANCEL;
			}
// Don't read all of RTIME*.JS7 -- lose the last few bytes, because they
// may contain user-edited data. But overlay with the latest code.
			if (fread(getMem()->memoryAt(0xDB00, 7), 1, 0x19FF, fprt7) < 0x19FF)
			{
				stopLog("R Failed to load RTIME*.JS7");
				return ZXE_CANCEL;
			}
			fclose(fprt7);
		}
//
// Fixups for funnies in the JSW48 -> 64 upgrade process. Version 2.1.6 did
// a spectacularly bad job here...
//
	}
	if (m_mem->peek(0xEC6C) == 0xA0)
	{
		l = 11;
		doLog("Corrected rope guardian control");
		m_mem->poke(0xEC6C, 0xA4);
	}
	if (m_mem->peek(0xEC6E) == 0xD6)
	{
		l = 11;
		doLog("Corrected arrow guardian control");
		m_mem->poke(0xEC6E, 0x37);
		m_mem->poke(0xEC6F, 0x92);
	}
	if (m_mem->peek(0xFFCC) != 0xCB)
	{
		l = 11;
		doLog("Corrected opening wall function");
		memcpy(m_mem->memoryAt(0xFEFC), patch_fefc, 0x104);
	}
	if (m_mem->peek(0xFF3B) != 0xCD)
	{
		l = 11;
		doLog("Corrected upward skylabs");
		m_mem->poke(0x841B, 0xDD);
		m_mem->poke(0x841C, 0xBE);
		m_mem->poke(0x841D, 0x07);
		m_mem->poke(0x841E, 0x3F);
		m_mem->poke(0x841F, 0xC9);
		m_mem->poke(0xFF3B, 0xCD);
		m_mem->poke(0xFF3C, 0x1B);
		m_mem->poke(0xFF3D, 0x84);
	}
	if (m_mem->peek(0x87A7) == 0xE6 && m_mem->peek(0x87A8) != 0x18)
	{
		l = 11;
		doLog("Corrected 128-room WRITETYPER");
		m_mem->poke(0x87A8, 0x18);
		m_mem->poke(0x87AA, 0x18);
		m_mem->poke(0x87AD, 0xC9);
	}
	char variant = m_mem->peek(0x85C9);
	if (variant != 'V' && variant != 'W' && m_mem->peek(0x93D7) == 0xC3)
	{
		l = 11;
		doLog("Corrected JSW128-to-64 conversion");
		m_mem->poke(0x93D7, 0x4E);
		m_mem->poke(0x93D8, 0xCB);
		m_mem->poke(0x93D9, 0xB9);
		m_mem->poke(0x93DA, 0x3A);
		m_mem->poke(0x93DB, 0x20);
		m_mem->poke(0x93DC, 0x84);
		m_mem->poke(0x93DD, 0xF6);
		m_mem->poke(0x93DE, 0x40);
		m_mem->poke(0x93DF, 0xB9);
	}
	if (m_mem->peek(0xEC19) != 0xEC)
	{
		l = 12;
		doLog("Corrected stationary ropes");
		m_mem->poke(0xEBEA, 0xC2);
		m_mem->poke(0xEBEB, 0xB6);
		m_mem->poke(0xEBEC, 0x91);
		m_mem->poke(0xEBED, 0xC3);
		m_mem->poke(0xEBEE, 0xD9);
		m_mem->poke(0xEBEF, 0x90);
		m_mem->poke(0xEB90, 0x00);
		m_mem->poke(0xEC18, 0xFB);
		m_mem->poke(0xEC19, 0xEC);
		m_mem->poke(0xECFB, 0xED);
		m_mem->poke(0xECFC, 0x78);
		m_mem->poke(0xECFD, 0xC3);
		m_mem->poke(0xECFE, 0xF1);
		m_mem->poke(0xECFF, 0xEB);

	}
	/* Add support for 'trap' cells */
	if (m_mem->peek(0x8FF1) != 0xFE)
	{
		doLog("HL11 to HL12 upgrade");
		l = 12;
		// Fix the memory map
		m_mem->poke(0x8646, 0x80);	// Page 0x9E
		m_mem->poke(0x8647, 0x80);	// Page 0x9F

		m_mem->poke(0x8FF1, 0xFE);
		m_mem->poke(0x8FF2, 0x03);	// CP 3
		m_mem->poke(0x8FF3, 0xC8);	// RET Z
		m_mem->poke(0x8FF4, 0xFE);	// CP 10
		m_mem->poke(0x8FF5, 0x0A);	
		m_mem->poke(0x8FF6, 0xC9);	// RET

		m_mem->poke(0x962C, 0xCD);	// CALL EC38
		m_mem->poke(0x962D, 0x38);
		m_mem->poke(0x962E, 0xEC);
		m_mem->poke(0x962F, 0xC0);	// RET NZ
		m_mem->poke(0x9630, 0xFE);
		m_mem->poke(0x9631, 0x03);	// CP 3
		m_mem->poke(0x9632, 0xC0);	// RET NZ
		m_mem->poke(0x9633, 0xC3);	// JP 90B6
		m_mem->poke(0x9634, 0xB6);
		m_mem->poke(0x9635, 0x90);
	}
        if (l) stopLog("0 OK", 1);
        else bm->toScreen(0, 0);

        return ZXE_CONTINUE;
}




Jsw64Game *Jsw128Game::upgrade64(char variant)
{
	int n, max;
	int andy_mode = 0, geoff_mode = 0, strx_mode = 0;
	char *ftail;
	char rt7buf[20];
	char hexbuf[20];
	FILE *fprt7;
	char *hexfile;

	if (variant == 'V' || variant == 'W') max = 128;
	else max = 64;

	ftail = getFtail(variant);
	if (!ftail) return NULL;
	sprintf(rt7buf, "rtime7%s.bin", ftail);
	sprintf(hexbuf, "room_%s.hex", ftail);
	
	startLog("Upgrade to JSW64");
	doLog("Examining game");
	if (m_mem->peek(0x892E) == 0) 
	{
		strx_mode = 1;
		   //  1...5...10...15...20...25...30...35...40
		doLog("Softricks engine. This'll never work.");
	}	
	else switch (m_mem->peek(0x9650))
	{
		case 0x5F: doLog("Original JSW engine."); break;
		case 0xEE: doLog("Andrew Mode JSW. This'll never work.");
			   andy_mode = 1;
			   break;
	}
	SpectrumMemory *mem = new SpectrumMemory(m_mem, 1);
	
	if (!mem) 
	{
		stopLog("4 Out of memory");
		return NULL;
	}
	/* mem is now a 128k memory image */

	/* Copy the start message to bank 7 */

	doLog("Copying title screen");
	memcpy(mem->memoryAt(0xC000, 7), m_mem->memoryAt(0xC000, 7), 0x1B00);

	/* Load rtime.js7 */
	fprt7 = openResource(rt7buf, "rb");
	if (!fprt7)
	{
		stopLog("F Failed to open RTIME*.JS7");
		return NULL;
	}
	if (fread(mem->memoryAt(0xDB00, 7), 1, 7000, fprt7) < 7000)
	{
		stopLog("R Failed to load RTIME*.JS7");
		return NULL;
	}
	fclose(fprt7);
	/* Title tune */
	memcpy(mem->memoryAt(0xF52A, 7), 
		m_mem->memoryAt(0xF52A, 7), 0xAD6);

	/* Copy item count */
	m_mem->poke(0x85CA, m_mem->peek(0xA3FF));

	/* Generate the JSW64 bank 0. Start by copying the JSW128 
	 * bank 0, which has the music code. Then do the HL9 upgrades
	 * on it. */
	memcpy(mem->memoryAt(0xC000, 0), m_mem->memoryAt(0xC000, 0), 0x4000);
	memcpy(mem->memoryAt(0xFEC0, 0), patch_tune, 64);
	memcpy(mem->memoryAt(0xF902, 0), patch_tune2, 245);

	// Apply the patches...
	char *boo = mem->loadHexPatch(getResourceName(hexbuf, "r"));
	if (boo)
	{
		doLog(boo);
		return NULL;	
	}
	// Copy the font in again; it was in the hex file.
	memcpy(mem->memoryAt(0xC100, 0), m_mem->memoryAt(0xC100, 0), 0x300);

	Jsw64Game *g;
	g = new Jsw64Game(mem);

	// Migrate the rooms
	doLog("Converting rooms");
	for (int nroom = 0; nroom < max; nroom++)
	{
		int x, y, ct;
		Cell cell[16];
		Room *room48 = getRoomClass(nroom);
		Room *room64 = g->getRoomClass(nroom);
		jswByte *room64bits = g->getRoom(nroom);
		jswByte *room48bits = getRoom(nroom);

		room64->clear();
		if (!room48) continue;
		// Copy cell patterns
		for (x = 0; x < 16; x++)
		{
			cell[x].behaviour = x % 10;
			cell[x].pattern[0] = 0xFF;
		}
		memcpy(cell[0].pattern, room48bits + 0xA0, 9);
		memcpy(cell[1].pattern, room48bits + 0xA9, 9);
		memcpy(cell[2].pattern, room48bits + 0xB2, 9);
		memcpy(cell[3].pattern, room48bits + 0xBB, 9);
		if (room48bits[0xDA] == 1)
			memcpy(cell[5].pattern, room48bits + 0xC4, 9);
		else	memcpy(cell[4].pattern, room48bits + 0xC4, 9);
		if (room48bits[0xD6] & 1)
			memcpy(cell[7].pattern, room48bits + 0xCD, 9);
		else	memcpy(cell[6].pattern, room48bits + 0xCD, 9);
		room64->setCells(cell);
		room64->setTitle((char *)room48bits + 0x80);

		for (y = 0; y < 16; y++)
		{
			for (x = 0; x < 32; x++)
			{
				room64->setCell(x, y, room48->getCellAt(x,y));
			}
		}
		// Copy conveyor rules
		room64bits[0xD6] = room48bits[0xD6];
		room64bits[0xD7] = room48bits[0xD7];
		room64bits[0xD8] = room48bits[0xD8];
		room64bits[0xD9] = room48bits[0xD9];

		// Draw conveyor 
	        int len, step;
	        int w = room48bits[0xD7] + 256 * room48bits[0xD8];
		x = (w & 0x1F);
	        y = (w >> 5) & 0x0F;

		len = room48bits[0xD9] & 0x1F;
		for (n = 0; n < len; n++)
		{
			if (y >= 0 && y < 16 && x >= 0 && x < 32)
			{
				room64->setCell(x, y, 6 + (room48bits[0xD6] & 1));
				x++;
			}
		}
		// Draw ramp
		w = room48bits[0xDB] + (256*room48bits[0xDC]);
		x = (w & 0x1F);
		y = ((w >> 5) & 0x0F);
		len = room48bits[0xDD] & 0x1F;
		if (room48bits[0xDA] != 1) len = -len;
		if (len < 0) { len = -len; step = -1; } else step = 1;
		for (n = 0; n < len; n++)
		{
			if (y >= 0 && y < 16 && x >= 0 && x < 32)
			{
				room64->setCell(x, y, 4 + (step == 1 ? 1 : 0));
				x+= step;
				--y;
			}
		
		}

		// If old room had escalator...
		if (room48bits[0xC4] == room48bits[0xCD])
		{
			// If conveyor direction = ramp direction, escalator
			// is going up.
			if ((room48bits[0xD6] & 1) == (room48bits[0xDA] & 1))
				room64bits[0xFF] |= 4;
			else	room64bits[0xFF] |= 6;

		}

		for (x = 0xF0, y = 0; x < 0x100; x += 2)
		{
			if (room48bits[x] == 0xFF) break;
			++y;
		}
		// y = count of guardians

		if (variant == '[' && y > 4)
		{
			room64->setGuardianTable(0x81FF - (8 * y));
		}
		else	room64->setGuardianTable(0x8000);

		// Guardian table
		memcpy(room64->getGuardianBuffer(), 
		       room48->getGuardianBuffer(), (8 * y) + 1);
		// Arrows
		jswByte *buf = room64->getGuardianBuffer();
		for (n = 0; n < y; n++)
		{
			if ((buf[n*8] & 0x0F) == 4)	// Arrow
			{
				buf[n*8+1] = 7;	// White
				if (buf[n*8] & 0x80) buf[n*8+3] = 0x7F;
				else		     buf[n*8+3] = 0xFE;	
			}
		}
		// No solar power
		room64bits[0xDC] = 0;
		room64bits[0xDD] = 0;
		// Border etc.
		room64bits[0xDE] = (room48bits[0xDE] & 0x87) | 0x38;
		// Air
		room64bits[0xDF] = 0xFF;
		room64bits[0xE0] = 0xFF;
		// Item
		memcpy(room64bits+0xE1, room48bits+0xE1, 9+4);
		// Willy's sprite
		room64bits[0xED] = room48bits[0xED];
	}
	// Reverse out a JSW128 patch not present in JSW64.
	mem->poke(0x8912, 0x3A);
	mem->poke(0x8913, 0x20);
	mem->poke(0x8914, 0x84);

	jswByte *memmap64a =   mem->memoryAt(g->getMemmap());
	jswByte *memmap128 = m_mem->memoryAt(getMemmap());

	memcpy(memmap64a, memmap64, g->getMapsize());	
	memcpy(memmap64a/* + 12*/, memmap128, getMapsize());

	// V and W have an item table at C000. Others don't.	
	if (variant == 'V' || variant == 'W')	memmap64a[40] = 128;
	else
	{
		// If converting to a variant other than V or W, the item
		// table needs to go back from C000 to A400.
		memmap64a[40] = 0;
		mem->poke(0x93D7, 0x4E);
		mem->poke(0x93D8, 0xCB);
		mem->poke(0x93D9, 0xB9);
		mem->poke(0x93DA, 0x3A);
		mem->poke(0x93DB, 0x20);
		mem->poke(0x93DC, 0x84);
		mem->poke(0x93DD, 0xF6);
		mem->poke(0x93DE, 0x40);
		mem->poke(0x93DF, 0xB9);
		memcpy(mem->memoryAt(0xA400), mem->memoryAt(0xC000), 256);
	}
	// Unmap the guardian tables and teleporters
	for (n = g->getMapsize() - 1; n >= 0; n--)
	{
		if ((memmap64a[n] >= 0x20 && memmap64a[n] < 0x60) ||
			memmap64a[n] == 0x61)
		{
			memmap64a[n] = 0;
		}
	}

	// Transfer teleporters
	if (canTeleport())
	{
		doLog("Transferring teleports");
		jswByte *b = getTeleportList();
		g->teleLoad();
		jswByte *c = g->getTeleportList();
		memcpy(c, b, 4 * b[0] + 1);		

	}
	stopLog("0 OK", 1);

	g->upgrade128();	
	return g;
}

