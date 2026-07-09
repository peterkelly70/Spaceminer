/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-5  John Elliott <jce@seasip.demon.co.uk>

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
#include "manicgame.hxx"
#include "manicroomdraw.hxx"
#include "manicfixedguard.hxx"
#include "room.hxx"


ManicRoomDraw::ManicRoomDraw(ManicGame *game)
{	
	int n;

	m_game = game;
	m_ticker = 0;
	m_room = game->getStartRoom();
	m_ot   = NULL;
	m_haveTimer = 0;
	m_visible = 0;

	for (n = 0; n < HG_MAX; n++) m_hguards[n] = NULL;
	for (n = 0; n < VG_MAX; n++) m_vguards[n] = NULL;
	m_vgwilly = m_vgeugene = m_vgkong = NULL;
	loadRoom(m_room);

	m_bmItem  = NULL;
	m_bmRoom  = newVideoBitmap(512, 256);
	m_bmAnim  = newVideoBitmap(512, 256);

	m_drawnFixed = 0;
	m_prevRoom = -1;
}

#define IFDEL(x) if (x) delete x; x = NULL;

ManicRoomDraw::~ManicRoomDraw()
{	
	int n;
        IFDEL(m_bmItem)
	delete m_bmRoom;
	delete m_bmAnim;
	for (n = 0; n < HG_MAX; n++) 
	{
		if (m_hguards[n]) delete m_hguards[n]; 
		m_hguards[n] = NULL;
	}
	for (n = 0; n < VG_MAX; n++)
	{
		 if (m_vguards[n]) delete m_vguards[n]; 
		m_vguards[n] = NULL;
	}
	if (m_vgwilly) delete m_vgwilly;
	if (m_vgeugene) delete m_vgeugene;
	if (m_vgkong) delete m_vgkong;
        if (m_haveTimer) setTickListener(m_ot);
}




void ManicRoomDraw::drawFixed(void)
{
	m_drawnFixed = 1;
}

VideoBitmap *ManicRoomDraw::bitmapFromSprite(jswByte *sprite)
{
	int ink   = sprite[0] & 7;
	int paper = (sprite[0] >> 3) & 7;

	if (sprite[0] & 64)  { ink |= ZX_BRIGHT; paper |= ZX_BRIGHT; }
	return ::bitmapFromSprite(8, 8, sprite+1, 1, ink, paper);
}

#define CONVEYORANIM 4	/* 2 or even 1 look nicer, but 4 matches JSW best */

/* Overridables */
void ManicRoomDraw::drawCellType() { }
void ManicRoomDraw::drawCursor(void) { }
void ManicRoomDraw::drawGuardList(void) { }


#define RR(bm) \
if (bm) { \
bm->getSurface()->rotateR(0, 2*animY,     16, 2, CONVEYORANIM); \
bm->getSurface()->rotateL(0, 2*animY + 4, 16, 2, CONVEYORANIM); \
}

#define RL(bm) \
if (bm) { \
bm->getSurface()->rotateL(0, 2*animY,     16, 2, CONVEYORANIM); \
bm->getSurface()->rotateR(0, 2*animY + 4, 16, 2, CONVEYORANIM); \
}

void ManicRoomDraw::flashColours(void)
{
	int n;

	++m_ticker;

	if (!m_visible) return;
	
	drawCellType();

	if ((m_ticker % 20)== 0)   drawData(1);

	if ( (m_ticker % 5)== 0)
	{
		getRoom()->animateConveyor();
		getRoom()->drawOneOffs(m_bmRoom, false, true);
		m_bmRoom->toSurface(m_bmAnim, 0, 0);
		for (n = 0; n < HG_MAX; n++)
		{	
			if (m_hguards[n])
			{
				m_hguards[n]->move();
// (horizontal guards don't flash)
//				m_hguards[n]->setTicker(m_ticker);
				m_hguards[n]->draw(m_bmAnim);
			}
		}
		for (n = 0; n < VG_MAX; n++)
		{	
			if (m_vguards[n])
			{
				m_vguards[n]->move();
				m_vguards[n]->setTicker(m_ticker);
				m_vguards[n]->draw(m_bmAnim);
			}
		}
		m_vgwilly->setTicker(m_ticker);
		m_vgwilly->draw(m_bmAnim);
		if (m_vgeugene) 
		{
			m_vgeugene->move();
			m_vgeugene->setTicker(m_ticker);
			m_vgeugene->draw(m_bmAnim);
		}
		if (m_vgkong) 
		{
			m_vgkong->move();
			m_vgkong->setTicker(m_ticker);
			m_vgkong->draw(m_bmAnim);
		}
		drawObjects();
		drawCursor();
	        m_bmAnim->toScreen(m_x, m_y);
	}

}







void ManicRoomDraw::flashSprite(VideoBitmap *bmp, jswByte attr)
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

void ManicRoomDraw::drawData(int flashonly)
{
	jswByte *room = getRoom(m_room);

	getRoom()->createCellPatterns();	

	if (!m_bmItem)
	{
		jswByte b = room[691];
		room[691] = 0x07;	/* White */
		m_bmItem = bitmapFromSprite(room + 691);
		room[691] = b;
	}
	drawBasic(flashonly);
	if (!flashonly)
	{
		char sbuf[33];
		int n;

		getRoom()->drawOneOffs(m_bmRoom, false, true);
		drawGuardList();	
		sprintf(sbuf, "%-32.32s", room + 512);
		for (n = 0; n < 32; n++)
		{
			if (sbuf[n] == ZXCH_COPY ) sbuf[n] = ZXK_COPY;
			if (sbuf[n] == ZXCH_POUND) sbuf[n] = ZXK_POUND;
		}
		videoScreen->drawText(m_x, m_y + 256, sbuf, ZX_BLACK,
			ZX_BRWHITE);
	        m_bmRoom->toSurface(m_bmAnim, 0, 0);
                for (n = 0; n < HG_MAX; n++)
                {
                        if (!m_hguards[n]) break;
                        m_hguards[n]->draw(m_bmAnim);
                }
                for (n = 0; n < VG_MAX; n++)
                {
                        if (!m_vguards[n]) break;
                        m_vguards[n]->draw(m_bmAnim);
                }
                m_vgwilly->draw(m_bmAnim);
		if (m_vgeugene)	m_vgeugene->draw(m_bmAnim);
		if (m_vgkong)	m_vgkong->draw(m_bmAnim);
		drawObjects();
		m_bmAnim->toScreen(m_x, m_y);
	}
}


void ManicRoomDraw::drawBasic(int flashonly)
{
        jswByte *room = getRoom(m_room);
        int x, y;

	for (y = 0; y < 16; y++) for (x = 0; x < 32; x++)
	{
	//	drawCell(room, x, y, flashonly);
		getRoom()->drawCell(m_bmRoom, x, y, flashonly, false);
	}
}






jswByte *ManicRoomDraw::getRoom(int room)
{
	return m_game->getRoom(room);
}


int ManicRoomDraw::onReveal(int whichRectangle)
{
	if (!m_haveTimer)
	{
		m_ot = setTickListener(this);
		m_ticker = 0;
		m_haveTimer = 1;
	}
	return VideoNotePage::onReveal();
}







void ManicRoomDraw::intPreContextMenu(void)
{
        if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_ot = NULL;
                m_haveTimer = 0;
        }
}


void ManicRoomDraw::postContextMenu(void)
{
	m_ot = setTickListener(this);	
	m_haveTimer = 1;
	VideoNotePage::postContextMenu();
}



int ManicRoomDraw::onConceal()
{
	if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_ot = NULL;
                m_haveTimer = 0;
        }

	return VideoNotePage::onConceal();
}


int ManicRoomDraw::redraw(int whichRectangle)
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

void ManicRoomDraw::gotoRoom(int room)
{
	int n;

	if (room != m_room) m_prevRoom = m_room;
	jswByte *b = m_game->getRoom(room);
	if (!b) return;
	getRoom()->createCellPatterns();	
        IFDEL(m_bmItem) 
	for (n = 0; n < HG_MAX; n++) 
	{
		if (m_hguards[n]) delete m_hguards[n]; 
		m_hguards[n] = NULL;
	}
	for (n = 0; n < VG_MAX; n++)
	{
		 if (m_vguards[n]) delete m_vguards[n]; 
		m_vguards[n] = NULL;
	}
	if (m_vgwilly) delete m_vgwilly;
	if (m_vgeugene) delete m_vgeugene;
	if (m_vgkong) delete m_vgkong;
	m_vgwilly = m_vgeugene = m_vgkong = NULL;

	m_room = room;
	loadRoom(m_room);

	drawData();
}




void ManicRoomDraw::loadRoom(int room, int reload)
{
	int ng, n, bg;
	jswByte *g, *b;
	bool uni;
// Initialise horizontal guardians

 	b = m_game->getRoom(room);
	m_hasvguards = m_game->roomHasFeature(room, FEAT_VGUARDS);
	m_skylabs = m_game->roomHasFeature(room, FEAT_SKYLABS);
	m_solar   = m_game->roomHasFeature(room, FEAT_SOLAR);
	m_kong    = m_game->roomHasFeature(room, FEAT_KONG);
	m_eugene  = m_game->roomHasFeature(room, FEAT_EUGENE);

	uni = m_kong || m_skylabs || m_hasvguards;

	for (n = 0; n < HG_MAX; n++)
	{
		g = b + 702 + 7 * n;
		if (g[0] == 0) continue;
		if (g[0] == 0xFF) break;
		m_hguards[n] = new ManicHGuardian(!uni, b, g);
	}
	if (m_hasvguards || m_skylabs) for (n = 0; n < VG_MAX; n++)
	{
		g = b + 733 + 7 * n;
		if (g[0] == 0xFF) break;
		if (m_skylabs)
			m_vguards[n] = new ManicSkylab(b, g);
		else	m_vguards[n] = new ManicVGuardian(b, g);
	}
	// Eugene, Kong, Willy and portals.
	checkSpecials();
}

void ManicRoomDraw::checkSpecials()
{
	int bg;

        jswByte *b = m_game->getRoom(m_room);
        bg = (b[544] >> 3) & 0x0F;	// Air colour 

	m_vgwilly = new ManicFixedGuard(MFG_WILLY,  m_game, b, bg);
	if (m_eugene) m_vgeugene = new ManicEugene(b, bg);
	else m_vgeugene = NULL;
	if (m_kong) m_vgkong = new ManicKong(b, bg);
	else m_vgkong = NULL;
}



void ManicRoomDraw::drawObject(int ink, int paper, int x, int y)
{
        jswByte *room = m_game->getRoom(m_room);

	m_bmItem->getSurface()->fillReplace(0, 0, 16, 16, ZX_WHITE, ink);
	m_bmItem->getSurface()->fillReplace(0, 0, 16, 16, ZX_BLACK, paper);
	m_bmItem->toSurface(m_bmAnim, x*16, y*16); 

// Regenerate the object in case foreground & background end up the same.
	jswByte b = room[691];
	room[691] = 0x07;	/* White */
	m_bmItem = bitmapFromSprite(room + 691);
	room[691] = b;
}

void ManicRoomDraw::drawObjects(void)
{
	int n,x,y, rm;
	jswByte *ob;
        int ink, paper;
	// = 3 + ((m_ticker / 5) % 4);
        jswByte *room = m_game->getRoom(m_room);

	for (n = 0; n < 5; n++)
	{
		ob = room + 629 + 5 * n;

		if (ob[0] == 0xFF || ob[0] == 0) continue;
			
		ink   = (ob[0] & 7) + ((m_ticker / 5) % 4);
		paper = (ob[0] >> 3) & 7;
		if (ink > 6) ink = 3;

		x = ob[1] & 0x1F;
		y = ((ob[1] >> 5) & 7)  | (ob[3] & 8);
		if (ob[0] & 0x40) { ink |= 8; paper |= 8; }
		drawObject(ink, paper, x, y);
	}
}                                                     



void ManicRoomDraw::onTick(void)
{
	flashColours();
}



