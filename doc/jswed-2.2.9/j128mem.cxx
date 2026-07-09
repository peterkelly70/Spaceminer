/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "j128game.hxx"
#include "j128mem.hxx"


Jsw128MemPage::Jsw128MemPage(JswGame *game, bool allowGuards)
{
	m_allowGuards = allowGuards;
	setTitle("Memory");
	m_focus = 0;
	m_memmap = game->getMemmap();
	m_map = game->getMem()->memoryAt(m_memmap);
	m_mapsize = game->getMapsize();
	m_game = game;
	m_gpages = (game->getGuardianMask() + 1) / 32;
}

Jsw128MemPage::~Jsw128MemPage()
{
}


int Jsw128MemPage::onReveal(int whichRectangle)
{
	int rv = VideoNotePage::onReveal(whichRectangle);
	drawData();
	return rv;
}


int Jsw128MemPage::onConceal()
{
	return VideoNotePage::onConceal();
}


int Jsw128MemPage::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);

	drawFixed();
	drawData();
	drawFocus();
	return rv;
}

void Jsw128MemPage::getXY(int n, int *x, int *y)
{
	*x = m_x + 1 + (8 * CHAR_W) * (n % 4);
	*y = m_y + 1 + (1 + CHAR_H) * (n / 4);
}

int Jsw128MemPage::getN(int x, int y)
{
	int n1 = (x - m_x - 1) / (8 * CHAR_W);
	int n2 = (y - m_y - 1) / (1 + CHAR_H);

	if (n1 < 0 || n1 > 4) return -1;

	n2 = (n2 * 4) + n1;
	if (n2 < 0 || n2 >= m_mapsize) return -1;
	return n2;
}

void Jsw128MemPage::drawFixed(void)
{
	int n, x, y;
	char s[8];

	videoScreen->bottomBar("Memory map");
	for (n = 0; n < m_mapsize; n++)
	{
		sprintf(s, "%02x00", n + 0x98);
		getXY(n, &x, &y);
		videoScreen->drawText(x, y, s, ZX_BLACK, ZX_WHITE);
	}
	
	// Draw legend at the bottom
	videoScreen->drawText(0, VIDEO_H - 2 * CHAR_H,
		"** Free   ** Sprites   **,** Reserved  ", ZX_BLACK, ZX_WHITE);
	if (m_allowGuards)
	{
		videoScreen->drawText(0, VIDEO_H - CHAR_H, 
		"** Teleporters         **,** Guardians", ZX_BLACK, ZX_WHITE);
	}
	else
	{
		videoScreen->drawText(0, VIDEO_H - CHAR_H, 
		"** Teleporters                        ", ZX_BLACK, ZX_WHITE);
	}
	videoScreen->drawText(0,            VIDEO_H - 2 * CHAR_H, "  ", 
					ZX_BLACK, ZX_GREEN);
	videoScreen->drawText(10 * CHAR_W,  VIDEO_H - 2 * CHAR_H, " S", 
					ZX_BLACK, ZX_CYAN);
        videoScreen->drawText(23 * CHAR_W,  VIDEO_H - 2 * CHAR_H, " R",
                                        ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(26 * CHAR_W,  VIDEO_H - 2 * CHAR_H, " r",
                                        ZX_WHITE, ZX_BLACK);

	videoScreen->drawText(0,            VIDEO_H - 1 * CHAR_H, " T",
					ZX_BLACK, ZX_YELLOW);
	if (m_allowGuards)
	{
	        videoScreen->drawText(23 * CHAR_W,  VIDEO_H - 1 * CHAR_H, "Gn",
                                        ZX_BRWHITE, ZX_RED);
       		videoScreen->drawText(26 * CHAR_W,  VIDEO_H - 1 * CHAR_H, "gn",
                                        ZX_WHITE, ZX_RED);
	}	
}


void Jsw128MemPage::drawData(void)
{
	int n, x, y;

        for (n = 0; n < m_mapsize; n++)
        {
                getXY(n, &x, &y);

		drawDatum(x, y, n);
        }

}

void Jsw128MemPage::drawDatum(int x, int y, int n)
{
	char s[4];
	int c = m_map[n];
	int fg, bg;

	sprintf(s, "%02x", c);

	x += (9 * CHAR_W) / 2;

	fg = ZX_BRRED;  
	bg = ZX_BRYELLOW;

	switch(c)
	{
		case 0:    fg = ZX_BLACK; 	// Free
			   bg = ZX_GREEN; 
			   strcpy(s, "  "); 
			   break;
		case 0x60: fg = ZX_BLACK;	// Sprites
			   bg = ZX_CYAN;
			   strcpy(s, " S"); 
			   break;
		case 0x61: fg = ZX_BLACK;	// Teleports
			   bg = ZX_YELLOW;
			   strcpy(s, " T");
			   break;
		case 0x80: fg = ZX_BRWHITE; 	// Reserved
			   bg = ZX_BLACK;
			   strcpy(s, " R");
			   break;
		default:   if (c >= 0x80) 
			   {
				strcpy(s, " r");
				fg = ZX_WHITE;
				bg = ZX_BLACK;
                           }
			   else if (c >= 0x40 && c < 0x60)
			   {
				sprintf(s, "g%c", (c - 0x10));
				fg = ZX_WHITE;
				bg = ZX_RED;
                           }
			   else if (c >= 0x20 && c < 0x40)
			   {
                                sprintf(s, "G%c", (c + 0x10));
                                fg = ZX_BRWHITE;
                                bg = ZX_RED;
                           }
			   break;	
	}

	videoScreen->drawText(x, y, s, fg, bg);
}

// Returns 1 if a guardian table can be put in the memory map at index "n".
bool Jsw128MemPage::guardsPossible(int n)
{
	unsigned char *data = m_map + n;

	if (!m_allowGuards) return false;
	if (n > m_mapsize - m_gpages) return false;

	for (int u = 1; u < m_gpages; u++) if (data[u] != 0) return false;
	return 1; 
}


int Jsw128MemPage::guardTableId(void)
{
	int n, map[32];

	memset(map, 0, sizeof(map));
	for (n = 0; n < m_mapsize; n++) if (m_map[n] < 0x40 && m_map[n] >= 0x20)
			map[m_map[n] & 0x1F] = 1;

	for (n = 1; n < 32; n++) if (!map[n]) return n;

	return -1;	
}


void Jsw128MemPage::toggle(int n)
{
	int x, y, i;
	int drawall = 0;
        unsigned char *data = m_map + n;
	unsigned char *da2  = data;

	if      (*data  > 0x80) *data = 0;
	else if (*data == 0x60) *data = 0x81;
	else if (*data >= 0x20 && *data < 0x60)
	{
		while (*da2 >= 0x40) --da2;

		for (int u = 0; u < m_gpages; u++) da2[u] = 0;
		*data = 0x60;

		drawall = 1;	
	}
	else if (*data == 0)
	{
		if (guardsPossible(n) && (i = guardTableId()) > 0)
		{
			data[0] = 0x20 + i;
			for (int u = 1; u < m_gpages; u++) data[u] = 0x40 + i;
			drawall = 1;	
		}
		else *data = 0x60;
	}
	if (drawall)
	{
		drawData();
	}
	else
	{	
		getXY(n, &x, &y);
		drawDatum(x, y, n);
	}
	m_game->onChangeMemmap();
}

void Jsw128MemPage::drawFocus(int draw)
{
	int x,y;
	
	getXY(m_focus, &x, &y);
        videoScreen->dottedBox(x - 1, y - 1, 7 * CHAR_W, CHAR_H + 2,
                        draw ? ZX_BRBLUE : ZX_WHITE);
	
}

int Jsw128MemPage::onButtonDown(int x, int y, int button)
{
	int nc = getN(x, y);
	
	if (nc < 0) return ZXE_CONTINUE;

	drawFocus(0);
	m_focus = nc;	
	toggle(nc);
	drawFocus(1);
	return ZXE_CONTINUE;	
}



int Jsw128MemPage::onKeyDown(int keysym)
{
	int fo = 0;

	switch (keysym)
	{
		case ZXK_RIGHT:
		case ZXK_TAB:		fo = 1;  break;
		case ZXK_LEFT:
		case ZXK_BACKTAB:	fo = -1; break;
		case ZXK_UP:		fo = -4; break;
		case ZXK_DOWN:		fo = 4;  break;
	}
	if (fo)
	{
		drawFocus(0);
		m_focus += fo;
		if (m_focus >= m_mapsize) m_focus = 0;
		else if (m_focus <=  0    ) m_focus = m_mapsize - 1;
		drawFocus(1);
		return ZXE_CONTINUE;
	}
	if (keysym == ' ')
	{
		toggle(m_focus);
	}
	return VideoNotePage::onKeyDown(keysym);
}

