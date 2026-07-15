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
#include "j48telpage.hxx"


Jsw48TelPage::Jsw48TelPage(JswGame *game) : Jsw48RoomNav(game)
{
	m_cx = m_cy = 0;
	m_ocx = m_ocy = -1;

	addHotspot(new Hotspot('L', 21 * CHAR_W, 256 + 3*CHAR_H,
				6 * CHAR_W, CHAR_H, 'l', 'L'));

	setTitle("Tele.");
}

Jsw48TelPage::~Jsw48TelPage()
{
}



void Jsw48TelPage::drawFixed(void)
{
	videoScreen->bottomBar("Teleport editor");

	Jsw48RoomNav::drawFixed();
}




void Jsw48TelPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	s->box(m_cx * 16, m_cy * 16, 16, 32, i);
}




void Jsw48TelPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];

       		sprintf(s, "Teleport editor [%d]", m_room);
	        videoScreen->bottomBar(s);

		videoScreen->box(21 * CHAR_W - 1, 255 + 3*CHAR_H,
				 6 * CHAR_W + 2, CHAR_H + 2, ZX_BLACK);
		if (m_game->canTeleport())
		{
                        videoScreen->drawText(8 * CHAR_W, 256 + CHAR_H, 
                            "Teleporter extension is loaded. ", 
			    ZX_BLACK, ZX_WHITE);

			videoScreen->drawText( 21 * CHAR_W, 256 + 3*CHAR_H,
				"unLoad", ZX_BLACK, ZX_BRWHITE);
		}
		else
		{
			videoScreen->drawText(8 * CHAR_W, 256 + CHAR_H, 
			    "Teleporter extension not loaded.",
			    ZX_BLACK, ZX_WHITE);
                        videoScreen->drawText( 21 * CHAR_W, 256 + 3*CHAR_H,
                                " Load ", ZX_BLACK, ZX_BRWHITE);

		}
	}
	Jsw48RoomNav::drawData(flashonly);


}

void Jsw48TelPage::drawGuardList(void)
{
	int count;
	int n, x, y;
	jswByte *list = m_game->getTeleportList();

	if (!list) return;
	count = list[0]; ++list;

	for (n = 0; n < count; n++)
	{
		if (*list == m_room)
		{
			y = list[1];
			x = list[2] * 16;
			drawTeleporter(x, y);
		}
		list += 4;
	}
}


void Jsw48TelPage::drawTeleporter(int x, int y)
{
	VideoSurface *surface = m_bmRoom->getSurface();
	int n;

	surface->fillBox(x, y, 16, 32, ZX_ORANGE);
	
	for (n = y + 1; n < y + 31; n++)
	{
		surface->plot(x + 9, n, ZX_BRORANGE);
		surface->plot(x + 8, n, ZX_BRYELLOW);
		surface->plot(x + 7, n, ZX_BRYELLOW);
		surface->plot(x + 6, n, ZX_BRORANGE);
	}
	n--;
        surface->plot(x + 8, n, ZX_BRORANGE);
        surface->plot(x + 7, n, ZX_BRORANGE);

	for (n = x + 1; n < x + 15; n++)
	{
		surface->plot(n, y + 1, ZX_BRORANGE);
		surface->plot(n, y + 2, ZX_BRYELLOW);
		surface->plot(n, y + 3, ZX_BRORANGE);
	}
        surface->plot(x+ 8, y+3, ZX_BRYELLOW);
        surface->plot(x+ 7, y+3, ZX_BRYELLOW);
	surface->plot(x+ 1, y+2, ZX_BRORANGE);
	surface->plot(x+14, y+2, ZX_BRORANGE);
}



int Jsw48TelPage::onHotspot(int id)
{
	int rv;

	switch(id)
	{
		case 'L': intPreContextMenu();
			  rv =  m_game->toggleTeleporters();
			  postContextMenu();
			  drawData();
			  return rv;
	}
	return Jsw48RoomNav::onHotspot(id);
}

int Jsw48TelPage::onSelect(void)
{
	int rv, count = 0, n, telno = -1;

	// 1. Find out if we clicked on a teleporter
        jswByte *list = m_game->getTeleportList();

        if (!list) return ZXE_CONTINUE;

        count = list[0]; ++list;

	for (n = 0; n < count; n++)
	{
		if ( list[0] == m_room && (list[1] / 16) == m_cy   &&
		     list[2] == m_cx)
		{
			telno = n; break;
		}
		list += 4;
	}
	if (telno < 0)	/* Adding teleporter */
	{
		if (count >= 45) 
		{
			VideoMenu vm("All teleporters used",
				     "Cancel", NULL);
			rv = vm.doModal();
			if (rv >= ZXE_QUIT) return rv;
			return ZXE_CONTINUE;
		}
                list = m_game->getTeleportList();

		telno = list[0];
		++list[0];
		++list;
		list += 4 * telno;
		list[0] = m_room;
		list[1] = m_cy * 16;
		list[2] = m_cx;
		list[3] = 0;
	} 
	else
	{
		VideoMenu vm("Edit teleporter", "Change destination", 
			     "Delete teleporter", NULL);
		rv = vm.doModal();
		if (rv >= ZXE_QUIT) return rv;
		if (rv != ZXE_OK)   return ZXE_CONTINUE;

		if (vm.getSelected() == 1)
		{
			for (n = telno*4; n < count*4; n++)
			{
				list[n] = list[n+4];
			}
			list = m_game->getTeleportList();
			--list[0];
			drawData();
			return ZXE_CONTINUE;
		}			
	}
	rv = ZXE_CONTINUE;
	/* Setting teleporter destination */
        JswRoomList *s = m_game->getRoomList();
        if (s)
        {
                s->setTitle("Set teleporter destination");
                s->setSelectedRoom(list[3]);
                rv = s->doModal();
                if (rv != ZXE_QUIT && rv != ZXE_OK) rv = ZXE_CONTINUE;
                else if (rv == ZXE_OK)
                {
                        list[3] = s->getSelectedRoom();
                }
                delete s;
        }
	drawData();
	return rv;
}


int Jsw48TelPage::onKeyDown(int keysym)
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
			if (m_cy < 14) ++m_cy;
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


int Jsw48TelPage::onButtonDown(int x, int y, int button)
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

int Jsw48TelPage::onMouseMove (int x, int y, int xrel, int yrel)
{
        if (x >= m_x && x < (m_x + 512) &&
            y >= m_y && y < (m_y + 256))
        {
                m_cx = (x - m_x)/16;
                m_cy = (y - m_y)/16;
		if (m_cy == 15) m_cy = 14;
        }
	return Jsw48RoomNav::onMouseMove(x,y,xrel,yrel);
}


int Jsw48TelPage::preContextMenu(void)
{
	intPreContextMenu();
	return Jsw48RoomNav::preContextMenu();
}
