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

/* JSW files...
 * 
 * We have an abstract class: JswGame. This is a "document" class.
 *  
 * We have a function to load input files. This must detect:
 *
 * JSW48 (normal, Andy and Geoff).
 * JSW128
 * JSW2
 * Henry's Hoard
 * Softricks modified engine
 * Anything else I can think of.
 * 
 * and return the correct JswGame derivative. 
 *
 * NEW in 2.1.0: Beginnings of support for Manic Miner
 *
 */
#include "jswgame.hxx"
#include "j48game.hxx"
#include "henrygame.hxx"
#include "j128game.hxx"
#include "j64game.hxx"
#include "sftxgame.hxx"
#include "sftxgame128.hxx"
#include "geoffgame.hxx"

#include "manicgame.hxx"
#include "manicbbgame.hxx"
#include "manicspgame.hxx"

int loadFile(Game **gp)
{
        char filename[PATH_MAX];
	JswFileForm ff;

        videoScreen->cls();
        videoScreen->bottomBar("Load Jet Set Willy");

	filename[0] = 0;
	int rv = ff.doModal(filename);

        if (rv != ZXE_OK) return rv;
	
	return loadFile(filename, gp);
}


int loadFile(char *filename, Game **gp)
{
	static jswByte henrySig[4] = { 0x29, 0x29, 0x29, 0xD5 };

	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		videoScreen->drawText(0, VIDEO_H - CHAR_H, 
			"F Failed to open file", ZX_BLACK, ZX_WHITE);	
		return videoWaitForKey(ZXE_CANCEL);
	}
	SpectrumMemory *mem = new SpectrumMemory(filename, fp);		
	if (!mem)
	{
                videoScreen->drawText(0, VIDEO_H - CHAR_H, "4 Out of memory",
                                ZX_BLACK, ZX_WHITE);
                return videoWaitForKey(ZXE_CANCEL);
	}
	*gp = NULL;
	if (mem->is128()) 
	{
		int rv;
		Jsw128Game *g   = NULL;
		Jsw64Game  *g64 = NULL;

/* The JSW64 check is: to see if the jumpblock is present at 869Fh. */
		if      (mem->peek(0x869F) == 0xC9) 
		{
/* See if it has a suitable variant ID. Prereleases don't. */
			if (mem->peek(0x85C9) >= 'V' && 
			    mem->peek(0x85C9) <= '[')
			{
				g64 = new Jsw64Game(mem);
			}
			else
			{
				videoScreen->drawText(0, VIDEO_H - CHAR_H, 
				"A Unknown JSW64 variant", ZX_BLACK, ZX_WHITE);
				return videoWaitForKey(ZXE_CANCEL);
			}
		}
		else if (mem->peek(0x892E) == 0)    g   = new Softricks128Game(mem);	
		else				    g   = new Jsw128Game(mem);

		/* v2.1.2: Begin adding JSW64 support */
		if (g)
		{
			g->setFilename(filename);
			videoScreen->drawText(0, VIDEO_H - CHAR_H, "0 OK",
       	                         ZX_BLACK, ZX_WHITE);
			rv = videoWaitForKey(ZXE_OK);

			if (rv < ZXE_QUIT) g->upgrade128();
			*gp = g;
		}
		if (g64)
		{
			g64->setFilename(filename);
			videoScreen->drawText(0, VIDEO_H - CHAR_H, "0 OK",
       	                         ZX_BLACK, ZX_WHITE);
			rv = videoWaitForKey(ZXE_OK);

			if (rv < ZXE_QUIT) g64->upgrade128();
			*gp = g64;
		}
		return rv;
	}

	jswByte *data = mem->memoryAt(0x8450);

	if      (!memcmp(data, henrySig, 4)) *gp = new HenryGame(mem);
	else if (!memcmp(data+1, "AIR", 3))
	{ 
		if      (mem->peek(0x892E) == 0) *gp = new SoftricksGame(mem); 
	// The following check picks up "Willy takes a trip". This uses
	// Geoff-Mode patch vectors to position Maria and the toilet,
	// but doesn't have diagonal guardians or rearraged tables.
		else if (mem->peek(0x9650) == 8) *gp = new GeoffGame(mem);
		else			         *gp = new Jsw48Game(mem);	
	}
	else if (!memcmp(data+2, "oo", 2)) *gp = new Geoff2Game(mem);

	if (!(*gp))
	{
		data = mem->memoryAt(0x8418);
		if (!memcmp(data, "AIR", 3))
		{
			if (mem->peek(0x846E) == 0x50) *gp = new ManicBBGame(mem);
			if (mem->peek(0x846E) == 0xF0) *gp = new ManicBBGame(mem);
			if (mem->peek(0x846E) == 0x0F) *gp = new ManicSPGame(mem);
		}
	}

	if (!(*gp))
	{
		char buf[41];
		sprintf(buf, "%-40.40s", "A Unknown file type");
                videoScreen->drawText(0, VIDEO_H - CHAR_H, buf,
                                ZX_BLACK, ZX_WHITE);
                return videoWaitForKey(ZXE_CANCEL);
	}
	(*gp)->setFilename(filename);

	videoScreen->drawText(0, VIDEO_H - CHAR_H, "0 OK",
                                ZX_BLACK, ZX_WHITE);
        int rv  = videoWaitForKey(ZXE_OK);
	(*gp)->afterLoadFix();
	return rv;
}
