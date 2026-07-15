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
#include "j48roomdraw.hxx"
#include "room.hxx"


Jsw48RoomDraw::Jsw48RoomDraw(JswGame *game)
{
	m_game = game;
	m_ticker = 0;
	m_room = game->getStartRoom();
	m_ot   = NULL;
	m_haveTimer = 0;
	m_visible = 0;

	for (int n = 0; n < (GUARD_MAX + FG_MAX); n++) m_guards[n] = NULL;
	loadRoom(m_room);

	m_bmRoom  = newVideoBitmap(512, 256);
	m_bmAnim  = newVideoBitmap(512, 256);

	m_drawnFixed = 0;
	m_prevRoom = -1;
}

Jsw48RoomDraw::~Jsw48RoomDraw()
{
	delete m_bmRoom;
	delete m_bmAnim;
	for (int n = 0; n < (GUARD_MAX + FG_MAX); n++) 
		if (m_guards[n]) delete m_guards[n]; 

        if (m_haveTimer) setTickListener(m_ot);
}




void Jsw48RoomDraw::drawFixed(void)
{
	m_drawnFixed = 1;
}

VideoBitmap *Jsw48RoomDraw::bitmapFromSprite(jswByte *sprite)
{
	int ink   = sprite[0] & 7;
	int paper = (sprite[0] >> 3) & 7;

	if (sprite[0] & 64)  { ink |= ZX_BRIGHT; paper |= ZX_BRIGHT; }
	return ::bitmapFromSprite(8, 8, sprite+1, 1, ink, paper);
}

#define CONVEYORANIM 4	// 2 or even 1 look nicer, but 4 matches JSW best 

// Overridables 
void Jsw48RoomDraw::drawCellType() { }
void Jsw48RoomDraw::drawCursor(void) { }
void Jsw48RoomDraw::drawGuardList(void) { }

void Jsw48RoomDraw::flashColours(void)
{
	++m_ticker;

	if (!m_visible) return;
	
	drawCellType();

	if ((m_ticker % 20)== 0)   drawData(1);

	if ( (m_ticker % 5)== 0)
	{
		getRoom()->animateConveyor();
		getRoom()->drawOneOffs(m_bmRoom, false);
		m_bmRoom->toSurface(m_bmAnim, 0, 0);
		for (int n = 0; n < (GUARD_MAX + FG_MAX); n++)
		{	
			if (m_guards[n])
			{
				m_guards[n]->move();
				m_guards[n]->draw(m_bmAnim);
			}
			else if (n < GUARD_MAX) n = GUARD_MAX - 1; 
			else break;
		}
		drawItems();
		drawCursor();
	        m_bmAnim->toScreen(m_x, m_y);
	}

}







void Jsw48RoomDraw::flashSprite(VideoBitmap *bmp, jswByte attr)
{
        int ink   = attr & 7;
        int paper = (attr >> 3) & 7;
	int im;

	if (!(attr & 128)) return; 

        if (attr & 64)  { ink |= ZX_BRIGHT; paper |= ZX_BRIGHT; }

	for (im = 1; im < 15; im++)
	{
		if (im != paper && im != ink && im != 8) break;
	}
	bmp->getSurface()->fillReplace(0, 0, 16, 16, paper, im);
	bmp->getSurface()->fillReplace(0, 0, 16, 16, ink, paper);
	bmp->getSurface()->fillReplace(0, 0, 16, 16, im, ink);
}

void Jsw48RoomDraw::drawData(int flashonly)
{
	jswByte *DEBUGDATA = getRoom()->getGuardianBuffer() + 8;

	getRoom()->createCellPatterns();

	if (flashonly)
	{
		getRoom()->flashCells();
	}

	drawBasic(flashonly);
	if (!flashonly)
	{
		char sbuf[33];
		int n;

		getRoom()->drawOneOffs(m_bmRoom, false);
		//drawConveyor();
		drawGuardList();	
		getRoom()->getTitle(sbuf);
		videoScreen->drawText(m_x, m_y + 256, sbuf, ZX_BLACK,
			ZX_BRWHITE);
	        m_bmRoom->toSurface(m_bmAnim, 0, 0);

 		for (n = 0; n < (GUARD_MAX + FG_MAX); n++)
                {
                        if (m_guards[n]) 
			{
                        	m_guards[n]->draw(m_bmAnim);
			}
			else if (n < GUARD_MAX) n = GUARD_MAX - 1; 
                } 
		drawItems();
		m_bmAnim->toScreen(m_x, m_y);
	}
}


void Jsw48RoomDraw::drawBasic(int flashonly)
{
        int x, y;

	for (y = 0; y < 16; y++) for (x = 0; x < 32; x++)
	{
		getRoom()->drawCell(m_bmRoom, x, y, flashonly, false);
	}
}





jswByte *Jsw48RoomDraw::getRoom(int room)
{
	return m_game->getRoom(room);
}


int Jsw48RoomDraw::onReveal(int whichRectangle)
{
	if (!m_haveTimer)
	{
		m_ot = setTickListener(this);
		m_ticker = 0;
		m_haveTimer = 1;
	}
	return VideoNotePage::onReveal();
}







void Jsw48RoomDraw::intPreContextMenu(void)
{
        if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_ot = NULL;
                m_haveTimer = 0;
        }
}


void Jsw48RoomDraw::postContextMenu(void)
{
	m_ot = setTickListener(this);	
	m_haveTimer = 1;
	VideoNotePage::postContextMenu();
}



int Jsw48RoomDraw::onConceal()
{
	if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_ot = NULL;
                m_haveTimer = 0;
        }

	return VideoNotePage::onConceal();
}


int Jsw48RoomDraw::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);
	if (whichRectangle & VPR_RIGHT)
	{
		drawFixed();
		drawData();
	}
	m_visible = 1;
	return rv;
}

void Jsw48RoomDraw::gotoRoom(int room)
{
	if (room != m_room) m_prevRoom = m_room;
	jswByte *b = m_game->getRoom(room);
	if (!b) return;

	getRoom()->createCellPatterns();

	m_room = room;
	loadRoom(m_room);

	drawData();
}




void Jsw48RoomDraw::loadRoom(int room, int reload)
{
/* Original JSW48 version...
	int ng, n, bg;
	jswByte *b = m_game->getRoom(room);

	bg = getRoom()->getBackground(); //(b[0xA0] >> 3) & 0x0F;
	b  += 0xF0;
	jswByte *p, *g = m_game->getGuards(room);

	for (n = 0; n < (GUARD_MAX + FG_MAX); n++)
	{
		if (m_guards[n]) delete m_guards[n];
		m_guards[n] = NULL;
	}
	for (n = 0; n < 8; n++)
	{	
		ng = (*b) & (m_game->getGuardianMask());
		if (ng == m_game->getGuardianMask()) break;
		++b;
		p = g + (8 * ng);
		if (*p == 0xFF) break;
		m_guards[n] = m_game->newGuardian(p, *b, bg);
		++b;	
	}
	checkSpecials(); */

	int ng, n, bg;
	jswByte *buf = getRoom()->getGuardianBuffer();
	m_maxGuards = getRoom()->getMaxGuards();
	bg = getRoom()->getBackground();

	for (n = 0; n < (GUARD_MAX + FG_MAX); n++)
	{
		if (m_guards[n]) delete m_guards[n];
		m_guards[n] = NULL;
	}
	for (n = 0; n < m_maxGuards; n++)
	{	
		if (buf[n * 8] == 0xFF) break;
		m_guards[n] = m_game->newGuardian(buf + n*8, buf[n*8+2], bg);
	}
	checkSpecials();
}



void Jsw48RoomDraw::checkSpecials()
{
	int n = GUARD_MAX;
	int bg;
	Room *r = getRoom();

        bg = r->getBackground();

	if (m_game->getStartRoom() == m_room)
	{
		m_guards[n++] = new JswFixedGuard(FG_WILLY, m_game, r, bg);
	}
	if (m_game->getMariaRoom() == m_room)
	{
		m_guards[n++] = new JswFixedGuard(FG_MARIA, m_game, r, bg);
	}
	if (m_game->getToiletRoom() == m_room)
	{
		m_guards[n++] = new JswFixedGuard(FG_TOILET, m_game, r, bg);
	}	
	if (m_game->imcMode())
	{
		m_guards[n++] = new JswFixedGuard(FG_RESET, m_game, r, bg);
	}

}


void Jsw48RoomDraw::drawItems(void)
{
	int ink = 3 + ((m_ticker / 5) % 4);

	getRoom()->drawItems(m_bmAnim, ink);
}                                                     



void Jsw48RoomDraw::onTick(void)
{
	flashColours();
}



