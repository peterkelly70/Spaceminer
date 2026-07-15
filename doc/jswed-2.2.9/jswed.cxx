/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-6  John Elliott <jce@seasip.demon.co.uk>

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

Game *gl_game = NULL;

// Draws fixed 

VideoBitmap *bmSplash;

static void mainScreen(void)
{
	videoScreen->cls();
	videoScreen->drawText(0, VIDEO_H - CHAR_H, ZXK_COPY, ZX_BLACK, ZX_WHITE);
	videoScreen->drawText(CHAR_W, VIDEO_H - CHAR_H, 
			" 2000-2001,2004-2006 John Elliott",
			ZX_BLACK, ZX_WHITE);

	if (gl_game != NULL)
	{
		videoScreen->drawText(0, 0, gl_game->getID(), ZX_BLACK, ZX_BRWHITE);
	}
}

static int mainMenu(void)
{
	int rv;
        VideoMenu mnu("Main", "Load file", "Save file", "Edit game",
                        "Configure", "About JSWED", "Leave program", NULL);
/* for testing text output only 
	for (int n = 0; n < 255; n++)
	{
		videoScreen->drawText(CHAR_W * (n % 40), CHAR_H * (n / 40), n,
				ZX_BRWHITE, ZX_BLUE);
	}
*/
	while ( (rv = mnu.doModal()) != ZXE_QUIT)
	{
		if (rv == ZXE_CANCEL) continue;
		switch(mnu.getSelected())
		{
			case 0: // Load file
				rv = loadFile(&gl_game);
				mainScreen();
				if (rv == ZXE_QUIT) return 0;
				break;

			case 1: // Save file
				if (!gl_game) continue;
				rv = saveFile(&gl_game);
				mainScreen();
				if (rv == ZXE_QUIT) return 0;	
				break;

			case 2:	// Edit game
				if (!gl_game) continue;
				do
				{	
					gl_game->m_upgrade = NULL;
					rv = gl_game->editMain();
					if (gl_game->m_upgrade)
					{
						Game *g=gl_game->m_upgrade;
						delete gl_game;
						gl_game = g;
					}
				} while	(rv != ZXE_QUIT && gl_game->m_upgrade);
				mainScreen();
				if (rv == ZXE_QUIT) return 0;
				break;
			case 3: rv = configure();
				mainScreen();
				if (rv == ZXE_QUIT) return 0;
				break;
			case 4: // About JSWED
				rv = aboutBox();
				mainScreen();
				if (rv == ZXE_QUIT) return 0;
				break;

			case 5: // Quit
				return 0;
		}

	}
	return 0;
}



#ifdef WIN32
extern "C" int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	videoInit(getResourceName("jswed.png", "rb"), 
                                  "Jet Set Willy Editor v" JSWED_VERSION,
				  "JSW editor");
	bmSplash = newVideoBitmap(getResourceName("jswed_logo.png", "rb"));

	if (argc > 1)
	{
		videoScreen->cls();
		videoScreen->bottomBar("Load Jet Set Willy");
		int rv = loadFile(argv[1], &gl_game);
		if (rv == ZXE_QUIT) exit(0);	
	}
	mainScreen();
	int r = mainMenu();
	if (bmSplash) delete bmSplash;
	return r;
}

