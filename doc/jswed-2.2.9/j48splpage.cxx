/************************************************************************

    JSWED 2.00 - Editor for Jet Set Willy and derivatives

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
#include "j48splpage.hxx"


Jsw48SplPage::Jsw48SplPage(JswGame *game) : Jsw48RoomNav(game)
{
	int h = (3 * CHAR_H + CHAR_SH);
	int y = 18 * CHAR_H + CHAR_SH;

	m_cx = m_cy = 0;
	m_ocx = m_ocy = -1;

	m_imcmode = game->imcMode();
	addHotspot(new Hotspot('W', 20 * CHAR_W, y, 32, h, 'w','W'));
        addHotspot(new Hotspot('M', 23 * CHAR_W, y, 32, h, 'm','M'));
        addHotspot(new Hotspot('T', 26 * CHAR_W, y, 32, h, 't','T'));
	addHotspot(new Hotspot('R', 29 * CHAR_W, y, 32, h, 'r','R'));
	setTitle("Start");

	SpectrumMemory *mem = game->getMem();

	int adrw = 256 * game->getWillySpritePage();
	int adrm = 256 * mem->peek(0x955D) + 0x80;
        int adrt = 256 * mem->peek(0x95B3);
                                           
	m_bmWilly = ::bitmapFromSprite(16,16, mem->memoryAt(adrw), 1, 
			ZX_BLACK, ZX_BRWHITE);
	m_bmToilet= ::bitmapFromSprite(16,16, mem->memoryAt(adrt), 1,
			ZX_BLACK, ZX_BRWHITE);
	m_bmMaria = ::bitmapFromSprite(16,16, mem->memoryAt(adrm), 1,
			ZX_BLACK, ZX_BRWHITE);
	m_bmRestart = ::bitmapFromSprite(16,16, mem->memoryAt(adrw + 0xE0), 1, 
			ZX_BLACK, ZX_BRWHITE);
	m_mode = 0;
	m_prevMode = -1;
}

Jsw48SplPage::~Jsw48SplPage()
{
	delete m_bmMaria;
	delete m_bmWilly;
	delete m_bmToilet;
	delete m_bmRestart;
}



void Jsw48SplPage::drawFixed(void)
{
	int w = 10;
	
	if (m_game->imcMode()) 
	{
		w = 13;
	}
	videoScreen->bottomBar("Start position etc.");
	
	videoScreen->fillBox(19 * CHAR_W, 18 * CHAR_H, w * CHAR_W, 4 * CHAR_H,
				ZX_BRWHITE);
        videoScreen->box    (19 * CHAR_W, 18 * CHAR_H, w * CHAR_W, 4 * CHAR_H,
                                ZX_BLACK);
	videoScreen->drawSmallText(19 * CHAR_W, 18 * CHAR_H, 
				   "Special Characters  ", ZX_BRWHITE, ZX_BLACK);

	m_bmWilly->toScreen (20 * CHAR_W, 19 * CHAR_H);
	m_bmMaria->toScreen (23 * CHAR_W, 19 * CHAR_H);
	m_bmToilet->toScreen(26 * CHAR_W, 19 * CHAR_H);
	if (m_game->imcMode())
	{
		videoScreen->drawSmallText(29 * CHAR_W, 18 * CHAR_H, 
					   "      ", ZX_BRWHITE, ZX_BLACK);
		videoScreen->smallZxLogo(29 * CHAR_W, 18 * CHAR_H);
		m_bmRestart->toScreen(29 * CHAR_W, 19 * CHAR_H);
	}
	else
	{
       		videoScreen->fillBox(29 * CHAR_W, 18 * CHAR_H, 3 * CHAR_W, 4 * CHAR_H,
                                ZX_WHITE);
	}
	int y = 21 * CHAR_H + (CHAR_SH / 2);

	videoScreen->drawSmallText(21 * CHAR_W, y, "W", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawSmallText(24 * CHAR_W, y, "M", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawSmallText(27 * CHAR_W, y, "T", ZX_BLACK, ZX_BRWHITE);
	if (m_game->imcMode())
	{
        	videoScreen->drawSmallText(30 * CHAR_W, y, "R", ZX_BLACK, ZX_BRWHITE);
	}
	m_prevMode = -1;	
	Jsw48RoomNav::drawFixed();
}




void Jsw48SplPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	int w = 32, h = 32;
	if (m_cx == 31) w = 16;
	if (m_cy == 15) h = 16;
	s->box(m_cx * 16, m_cy * 16, w, h, i);
}




void Jsw48SplPage::drawData(int flashonly)
{	
	int n, modes;

	if (m_game->imcMode() != m_imcmode)
	{
		m_imcmode = m_game->imcMode();
		drawFixed();
	}

	if (!flashonly)
	{
	        char s[30];

       		sprintf(s, "Start position etc. [%d]", m_room);
	        videoScreen->bottomBar(s);

	}
	modes = m_game->imcMode() ? 4 : 3;
		 
	if (m_mode != m_prevMode) 
	{
		for (n = 0; n < modes; n++)
		{
			int x = ((3 * n) + 20) * CHAR_W;
			int y = 18 * CHAR_H + CHAR_SH;
			int w = 2 * CHAR_W;
			int h = 3 * CHAR_H + CHAR_SH;

			if (n == m_mode) 
			{
				videoScreen->fillReplace( x, y, w, h, 
					ZX_BRWHITE, ZX_BRCYAN);
			}
			if (n == m_prevMode) 
			{
				videoScreen->fillReplace( x,y,w,h,
					ZX_BRCYAN, ZX_BRWHITE);
			}
		}
		m_prevMode = m_mode;	
	}
	Jsw48RoomNav::drawData(flashonly);


}

void Jsw48SplPage::drawGuardList(void)
{
}



int Jsw48SplPage::onHotspot(int id)
{
	switch(id)
	{
		case 'W': m_mode = 0; drawData(); return ZXE_CONTINUE;
                case 'M': if (m_game->getMariaRoom() == -1) return ZXE_CONTINUE;
			  m_mode = 1; drawData(); return ZXE_CONTINUE;
                case 'T': if (m_game->getToiletRoom()== -1) return ZXE_CONTINUE;
			  m_mode = 2; drawData(); return ZXE_CONTINUE;
		case 'R': if (!m_game->imcMode()) return ZXE_CONTINUE;
			  m_mode = 3; drawData(); return ZXE_CONTINUE;

	}
	return Jsw48RoomNav::onHotspot(id);
}




int Jsw48SplPage::onSelect(void)
{
	switch(m_mode)
	{
		case 0:	
			m_game->setStartRoom(m_room);
			m_game->setStartPos(m_cx, m_cy * 16);
			gotoRoom(m_room);
			break;
		case 1: m_game->setMariaRoom(m_room);
			m_game->setMariaPos(m_cx, m_cy * 16);
			gotoRoom(m_room);
			break;
		case 2: m_game->setToiletRoom(m_room);
			m_game->setToiletPos(m_cx, m_cy * 16);
			gotoRoom(m_room);
			break;
		case 3:	if (m_game->imcMode())
			{
				jswByte *data = m_game->getRoom(m_room);
				data[0xED] = m_cy * 16;
				data[0xEE] = m_cx + ((m_cy << 5) & 0xE0);	
				data[0xEF] = ((m_cy >> 3) & 1) | 0x5C;
				gotoRoom(m_room);
			}
			break;
			
	}
	return ZXE_CONTINUE;
}


int Jsw48SplPage::onKeyDown(int keysym)
{
	int rv;

	switch(keysym)
	{
		case ZXK_LEFT:
			if (m_cx > 0) --m_cx;
			return ZXE_CONTINUE;
		case ZXK_RIGHT:
			if (m_cx < 31) ++m_cx;
			return ZXE_CONTINUE;
		case ZXK_UP:
			if (m_cy > 0) --m_cy;
			return ZXE_CONTINUE;
		case ZXK_DOWN:
			if (m_cy < 15) ++m_cy;
			return ZXE_CONTINUE;
		case ZXK_ENTER:
		case ' ':
			intPreContextMenu();
			rv = onSelect();
			postContextMenu();
			return rv;
	}
	return Jsw48RoomNav::onKeyDown(keysym);
}


int Jsw48SplPage::onButtonDown(int x, int y, int button)
{
	int n = onMouseMove(x, y, 0, 0);
	if (n == ZXE_CONTINUE) 
	{
		if (x >= m_x && x < (m_x + 512) &&
            	    y >= m_y && y < (m_y + 256))
		{
			intPreContextMenu();
			n = onSelect();
			postContextMenu();
		}
	}
	if (n != ZXE_CONTINUE) return n;

	return Jsw48RoomNav::onButtonDown(x, y, button);
}

int Jsw48SplPage::onMouseMove (int x, int y, int xrel, int yrel)
{
        if (x >= m_x && x < (m_x + 512) &&
            y >= m_y && y < (m_y + 256))
        {
                m_cx = (x - m_x)/16;
                m_cy = (y - m_y)/16;
        }
	return Jsw48RoomNav::onMouseMove(x,y,xrel,yrel);
}


int Jsw48SplPage::preContextMenu(void)
{
	intPreContextMenu();
	return Jsw48RoomNav::preContextMenu();
}


