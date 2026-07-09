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
#include "jswgame.hxx"
#include "snapio.hxx"
#include "j128game.hxx"
#include "patch.hxx"
#include "sftxgame128.hxx"
#include "upgrade.hxx"

extern Patch patchDeathless;

static int y = 0;
VideoBitmap *bm;

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


Jsw128Game *JswGame::upgrade(void)
{
	int n;
	jswByte objTable[0x200];
	int andy_mode = 0, geoff_mode = 0, strx_mode = 0;

	startLog("Upgrade to JSW128");
	doLog("Examining game");
	if (m_mem->peek(0x892E) == 0) 
	{
		strx_mode = 1;
		doLog("Softricks JSW engine");
	}	
	else switch (m_mem->peek(0x9650))
	{
		case 0x5F: doLog("Original JSW engine"); break;
		case 0x08: doLog("Geoff Mode JSW. Cannot upgrade."); 
			   geoff_mode = 1;
			   stopLog("A Cannot convert Geoff Mode JSW");
			   return NULL;
		case 0xEE: doLog("Andrew Mode JSW.");
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
	memcpy(mem->memoryAt(0xC000, 7), title, 0x1600);
	memcpy(mem->memoryAt(0xD600, 7), m_mem->memoryAt(0x8454), 256);
	mem->poke(0xD700, 0, 7);

	/* Apply the patches, and copy the low half of bank 2 */

	::upgrade("Scrolling message", mem, patch_1);
	::upgrade("256 rooms", mem, patch_2);
	::upgrade("128k music", mem, patch_3);
	::upgrade("256 rooms for objects", mem, patch_4);
	::upgrade("256 rooms for WRITETYPER", mem, patch_5);
	::upgrade("128k title screen", mem, patch_6);
	::upgrade("Superjump",         mem, patch_7);
	if (!geoff_mode) ::upgrade("multiple guardian tables", mem, patch_8);
	::upgrade("Boot in status line", mem, patch_9);
	::upgrade("Pause keys", mem, patch_10);
	if (!andy_mode) ::upgrade("Sprites set by room", mem, patch_11);
	::upgrade("Pause bug fix (safe)", mem, patch_12);
	::upgrade("New cheat mode", mem, patch_13);
	::upgrade("Custom font", mem, patch_14);
	::upgrade("Screen attributes", mem, patch_15);
//	::upgrade("Diagonal guardians", mem, patch_dguards);
	::upgrade("Memory map", mem,  patch_mmap);

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
	}
	/* Generate the JSW128 bank 0 */
	memcpy(mem->memoryAt(0xC000, 0), jsw_bank_0, 0x4000);
        /* Convert the object location table */
	doLog("Converting object table");
	if (strx_mode) 
	{
		for (n = 0; n < 256; n++)
		{
			mem->poke(0xC000+n, objTable[n] & 0x3F);
			mem->poke(0xC500+n, objTable[n]);
			mem->poke(0xC600+n, objTable[n + 0x100]);	
		}
		doLog("Softrix-specific fixes");
		mem->poke(0x864C+0x2D, 0x80);
		mem->poke(0x864C+0x2E, 0x80);
		mem->poke(0x864C+0x0C, 0x41);
                mem->poke(0x864C+0x0D, 0x41);
                mem->poke(0x864C+0x0E, 0x41);
		mem->poke(0x864C+0x0F, 0x41);
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
	
	doLog("Converting rooms");
	for (n = 0; n < 64; n++)
	{
		jswByte *room = mem->memoryAt(0xC000 + 256 * n, 1);
		room[0xDE] &= 0x7F;	/* Reset superjump flag */
		if (!geoff_mode)
		{
			room[0xDF] =  0;  
			room[0xE0] = 0xA0;	/* Guardian table at 0xA000 */
		}
		room[0xE9] &= 0x3F;
		room[0xEA] &= 0x3F;
		room[0xEB] &= 0x3F;
		room[0xEC] &= 0x3F;	/* Exits */
		if (!andy_mode) room[0xED] = 0;		/* Sprite override */
	}

	if (!andy_mode) mem->poke(0xDDED, 0xB6, 1);	/* Nightmare Room */

	// Upgrade guardian system to HL7
	memcpy(mem->memoryAt(0x84D2), patch_84d2, 122);
	memcpy(mem->memoryAt(0x90ca), patch_90ca, 15);
	memcpy(mem->memoryAt(0x9133), patch_9133, 131);


	Jsw128Game *g;
	if (strx_mode) g = new Softricks128Game(mem);
	else           g = new Jsw128Game(mem);

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


int Jsw128Game::upgrade128(void)
{
	int l = 0;
	startLog("Upgrading JSW128 engine");

	if (m_mem->peek(0x9650)	== 0x5F && m_mem->peek(0x9654) == 0x20)
	{
		l = 3;
		// HL3 -> HL4 is sprite-per-room.
		doLog("HL3 to HL4 upgrade");
		m_mem->poke(0x9654, 0xED);
		m_mem->poke(0x9655, 0x80);
                m_mem->poke(0x9657, 0x80);
                m_mem->poke(0x9658, 0x38);
                m_mem->poke(0x965A, 0x57);
                m_mem->poke(0x965B, 0x00);
		for (int n = 0; n < 256; n++)
		{
			// Set room sprites to defaults.
			jswByte *r = getRoom(n);
			if (n == 0x1D) r[0xED] = 0xB6;
			else	       r[0xED] = 0; 
		}
	}
	if (m_mem->peek(0x85C2) == 0xC3)
	{
		// HL4 -> HL5 is superjump.
		if (!l) l = 4;
		::upgrade("HL4 to HL5 upgrade", m_mem, patch_5);
		for (int n = 0; n < 256; n++)
                {
                        // Set room sprites to defaults.
                        jswByte *r = getRoom(n);
                        r[0xDE] &= 0x7F;
                }

	}
	if (m_mem->peek(0x84D2) == 0x4A)
	{
		if (!l) l = 5;
		doLog("HL5 to HL7 upgrade");
		// HL5 -> HL7 = diagonal guardians
		memcpy(m_mem->memoryAt(0x84D2), patch_84d2, 122);
                memcpy(m_mem->memoryAt(0x90ca), patch_90ca, 15);
                memcpy(m_mem->memoryAt(0x9133), patch_9133, 131);
	}
        if (m_mem->peek(0x84F3) == 0x32)
        {
                if (!l) l = 6;
                doLog("HL6 to HL7 upgrade");
                // HL6 -> HL7 = diagonal guardian bugfix
                memcpy(m_mem->memoryAt(0x84D2), patch_84d2, 98);
	}
	if (m_mem->peek(0x8F9E) == 0x32)
	{
                if (!l) l = 7;
		// HL7 -> HL8 = Superjump tidy
                ::upgrade("HL7 to HL8 upgrade", m_mem, patch_hl8);
	}
	if (m_mem->peek(0x8C4B) == 0x1E)
	{
                if (!l) l = 8;
		// HL8 -> HL9 = Multiple tunes
		memcpy(m_mem->memoryAt(0xFEC0), patch_tune,  64);
		memcpy(m_mem->memoryAt(0xF902), patch_tune2, 245);
		memcpy(m_mem->memoryAt(0xF52A, 7), 
		       m_mem->memoryAt(0xEC2A), 2774);

		memcpy(m_mem->memoryAt(0x885D), patch_tune3, 28);
                ::upgrade("HL8 to HL9 upgrade", m_mem, patch_hl9);
	}
//
// While making the JSW48 -> JSW128 upgrade, earlier versions of JSWED2 failed
// to upgrade the guardian code to support diagonals. Up to HL8, this didn't
// cause a problem (apart from diagonal guardians not working). But when the
// HL9 patch is applied, it ends up scrambling the guardian movement code.
// This section checks for the corrupt code and reapplies the diagonal guardian
// patch followed by the HL8->HL9 patch.
//
	if (m_mem->peek(0x9147) == 0xDD && m_mem->peek(0x9148) == 0x7E &&
	    m_mem->peek(0x9149) == 0xDD)
	{
                if (!l) l = 8;
		memcpy(m_mem->memoryAt(0x84D2), patch_84d2, 122);
                memcpy(m_mem->memoryAt(0x90ca), patch_90ca, 15);
                memcpy(m_mem->memoryAt(0x9133), patch_9133, 131);
                ::upgrade("Fix faulty 48->128 upgrade", m_mem, patch_hl9);
	}
	if (l) stopLog("0 OK", 1);
	else bm->toScreen(0, 0);

	return ZXE_CONTINUE;
}


// v2.1.0: Renamed from fixRhodes()
int JswGame::afterLoadFix(void)
{
        int n;

	if (m_mem->peek(0x8201) == 0x40)
	{
		startLog("Import from editor");
		doLog("Importing from Paul Rhodes' editor");
		for (n = 1; n < 256; n += 2)
		{
			m_mem->poke (0x8200 + n, m_mem->peek(0x8200 + n) + 0x20);
		}
		stopLog("0 OK", 1);
	}
	if (m_mem->peek(0x8D39) == 0x58)
	{
		startLog("Import from editor");
		doLog("Importing from the Softricks editor");
		m_mem->poke(0x8D39, 0x5E);
		m_mem->poke(0x8D3B, 0x70);
		m_mem->poke(0x8D45, 0x5F);
		m_mem->poke(0x8D47, 0x78);
		m_mem->poke(0x8D71, 0x5E);
		m_mem->poke(0x9503, 0x70);
		stopLog("0 OK", 1);
	}
	return ZXE_CONTINUE;
}

