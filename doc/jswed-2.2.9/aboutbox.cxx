/************************************************************************

    JSWED 2.1.00 - Editor for Jet Set Willy and derivatives

    Copyright (C) 1998,1999,2000-2001,2004-6 John Elliott 
    			<jce@seasip.demon.co.uk>

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

extern VideoBitmap *bmSplash;

static char *gl_about[] = 
{
//       1...5...10...15...20...25...30...35...40
    "\000",
    "\012              JSWED v" JSWED_VERSION,		// 0
    "\001  Editor for Jet Set Willy & clones  ",	// 1
    "\000",						// 2
    "\000  Copyright (C) 2000-2001, 2004-2006",		// 3
    "\000 John Elliott <jce@seasip.demon.co.uk>",	// 4
    "\000",
    "\001  Game types supported:",
    "\000  JSW48  original        Softricks 48k",
    "\000  JSW128 up to HL9       Softricks 128k",
    "\000  Henry's Hoard          Geoff Mode",
    "\000  JSW64 (variants V,W,X,Y,Z,[)",
    "\000  Manic Miner (1st and 2nd editions)",
    "\000  ",
    NULL
};

static char *gl_boilerplate[] = 
{
"",
"    This program is free software; you can redistribute it and/or modify",
"    it under the terms of the GNU General Public License as published by",
"    the Free Software Foundation; either version 2 of the License, or",
"    (at your option) any later version.",
"",
"    This program is distributed in the hope that it will be useful,",
"    but WITHOUT ANY WARRANTY; without even the implied warranty of",
"    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
"    GNU General Public License for more details.",
"",
"    You should have received a copy of the GNU General Public License",
"    along with this program; if not, write to the Free Software",
"    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.",
NULL
};

int aboutBox(void)
{
	int n, y;

	videoScreen->cls();
	bmSplash->toScreen(4 * CHAR_W, 0);
	y = bmSplash->getHeight();
	for (n = 0; gl_about[n]; n++, y += CHAR_H)
	{
		videoScreen->drawText(0, y, &(gl_about[n][1]), gl_about[n][0], 
				ZX_WHITE);
	}
	videoScreen->bottomBar("About the JSW editor");
	videoScreen->drawText(0, VIDEO_H - CHAR_H, 
                "Press ENTER or click mouse to continue.", ZX_BLACK, ZX_WHITE);

	int r = videoWaitForKey(ZXE_CONTINUE);
	if (r >= ZXE_QUIT) return r;

	videoScreen->cls();
	bmSplash->toScreen(4 * CHAR_W, 0);
	y = bmSplash->getHeight();
	for (n = 0; gl_boilerplate[n]; n++, y += CHAR_SH)
	{
		videoScreen->drawSmallText(0, y, gl_boilerplate[n], ZX_BLACK, ZX_WHITE);
	}

	videoScreen->bottomBar("About the JSW editor");
	videoScreen->drawText(0, VIDEO_H - CHAR_H, 
                "Press ENTER or click mouse to finish.", ZX_BLACK, ZX_WHITE);

	return videoWaitForKey(ZXE_CONTINUE);
}



