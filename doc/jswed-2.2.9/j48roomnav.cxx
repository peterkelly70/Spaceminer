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
#include "j48roomnav.hxx"


Jsw48RoomNav::Jsw48RoomNav(JswGame *game) : Jsw48RoomDraw(game)
{
	int x,y,w,h;

	x = CHAR_W;
	y = 256 + CHAR_H + CHAR_SH + (CHAR_SH / 4);
	w =   5 * CHAR_W;
	h =       CHAR_H + (CHAR_SH / 2);

	addHotspot(new Hotspot(7,  x,    y,    w,  h,ZXK_UP    | ZXK_CONTROL));
	addHotspot(new Hotspot(6,  x,    y+2*h,w,  h,ZXK_DOWN  | ZXK_CONTROL));
	addHotspot(new Hotspot(5,  x,    y+h,  w/2,h,ZXK_LEFT  | ZXK_CONTROL));
	addHotspot(new Hotspot(8,  x+w/2,y+h,  w/2,h,ZXK_RIGHT | ZXK_CONTROL)); 
	addHotspot(new Hotspot('J',x,    y+3*h,w,  h, 'J', 'j'));
        addHotspot(new Hotspot('<',x,    y+4*h,w/2,h, '<'));
        addHotspot(new Hotspot('>',x+w/2,y+4*h,w/2,h, '>'));
	m_prevRoom = -1;
}


Jsw48RoomNav::~Jsw48RoomNav()
{
}



void Jsw48RoomNav::drawFixed(void)
{
	int x,y,h;
        videoScreen->fillBox(CHAR_SW, 256 + CHAR_H + CHAR_SH,
                                6 * CHAR_W, 7 * CHAR_H, ZX_BRWHITE);
        videoScreen->box(CHAR_SW, 256 + CHAR_H + CHAR_SH,
                                6 * CHAR_W, 7 * CHAR_H, ZX_BLACK);

        videoScreen->drawSmallText(CHAR_SW, 256 + CHAR_H, "Go to       ",
                                ZX_BRWHITE, ZX_BLACK);
	videoScreen->smallZxLogo(7 * CHAR_SW, 256 + CHAR_H); 
	x = CHAR_W;
	y = 256 + CHAR_H + CHAR_SH + (CHAR_SH / 2);
	h = CHAR_H + (CHAR_SH / 2);
        videoScreen->drawText(x, y, "  ^  ", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x, y+h, " \013 \014 ", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x, y+2*h, "  \011  ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawSmallText(CHAR_W, y+3*h + (CHAR_SH / 2), 
				"Enter no.", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x, y+4*h, " < > ", ZX_BLACK, ZX_BRWHITE);
	y = 256 + CHAR_H + CHAR_SH + (CHAR_SH / 4);
        videoScreen->box(CHAR_SW, y+h,   6 * CHAR_W, h, ZX_BLACK);
	videoScreen->box(CHAR_SW, y+h,   3 * CHAR_W, h, ZX_BLACK);
	videoScreen->box(CHAR_SW, y+3*h, 6 * CHAR_W, 1, ZX_BLACK);
        videoScreen->box(CHAR_SW, y+4*h, 6 * CHAR_W, h, ZX_BLACK);
        videoScreen->box(CHAR_SW, y+4*h, 3 * CHAR_W, h, ZX_BLACK);
	
	Jsw48RoomDraw::drawFixed();
}

int Jsw48RoomNav::onHotspot(int id)
{
	jswByte *room = getRoom(m_room);

	switch(id)
	{
		case 5: gotoRoom(room[0xE9]); return ZXE_CONTINUE;
		case 6: gotoRoom(room[0xEC]); return ZXE_CONTINUE;
		case 7: gotoRoom(room[0xEB]); return ZXE_CONTINUE;
		case 8: gotoRoom(room[0xEA]); return ZXE_CONTINUE;
		case 'J': return jumpRoom();
		case '<': gotoRoom(m_game->prevRoom(m_room)); return ZXE_CONTINUE;
		case '>': gotoRoom(m_game->nextRoom(m_room)); return ZXE_CONTINUE;
	}
	return ZXE_CONTINUE;
}


int Jsw48RoomNav::onKeyDown(int keysym)
{
	int rv, rm;

	rv = HotspotManager::onKey(keysym); 
	if (rv != -1) return rv;
	switch(keysym)
	{
		case ZXK_F8:    return screenshotAll();
		case ZXK_CTRLA:
		case ZXK_CTRLJ: intPreContextMenu();
				rv = jumpRoomNumber(&rm);
				postContextMenu();
				if (rv >= ZXE_QUIT) return rv;
				if (rm >= 0) gotoRoom(rm);
				return ZXE_CONTINUE;

		case ZXK_CTRLL: gotoRoom(m_room); return ZXE_CONTINUE;
		case ZXK_CTRLP: if (m_prevRoom >= 0)
				{
					gotoRoom(m_prevRoom);
					return ZXE_CONTINUE;
				}
				break;
	}
	return Jsw48RoomDraw::onKeyDown(keysym);
}

int Jsw48RoomNav::onButtonDown(int x, int y, int button)
{
        int rv;

	rv = HotspotManager::onButton(x, y, button);
	if (rv != -1) return rv;
	return Jsw48RoomDraw::onButtonDown(x, y, button);
}

int Jsw48RoomNav::jumpRoom(void)
{
	int rv = ZXE_CONTINUE;
	int rm;

	intPreContextMenu();

	JswRoomList *s = m_game->getRoomList();
	if (s)
	{
		s->setTitle("Jump to room");
		s->setSelectedRoom(m_room);
		rv = s->doModal();
		if (rv != ZXE_QUIT && rv != ZXE_OK) rv = ZXE_CONTINUE;
		else if (rv == ZXE_OK)
		{
			rm = s->getSelectedRoom();
			gotoRoom(rm);
		}
		delete s;
	}
	postContextMenu();
	return rv;
}

int Jsw48RoomNav::onReveal(int rect)
{
        /* Room we are working on has become unavailable? */
        if (m_game->getRoom(m_room) == NULL)
        {
                int l = m_game->prevRoom(m_room);
		if (l == m_room) l = m_game->nextRoom(m_room);

		gotoRoom(l);
        }
	else	gotoRoom(m_room);
	return Jsw48RoomDraw::onReveal(rect);
}


int Jsw48RoomNav::screenshotAll()
{
	/* Start at the beginning... */
	int rm, rm0, rv;

	rm0 = m_room;
	do
	{
		rm = m_room;
		m_room = m_game->prevRoom(m_room);
	}
	while (rm != m_room);

	/* Then continue to the end */
	do
	{
		char buf[20];

		gotoRoom(m_room);
		sprintf(buf, "%03d.png", m_room);
		rv = m_bmAnim->getSurface()->savePng(0, 0, 256, 128, buf, 2);
		if (rv >= ZXE_QUIT) return rv;
		rm = m_room;
		m_room = m_game->nextRoom(m_room);

	} while (rm != m_room);
	gotoRoom(rm0);
	return ZXE_CONTINUE;
}
