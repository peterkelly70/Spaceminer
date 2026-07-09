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
#include "manicgame.hxx"
#include "manicroomnav.hxx"


ManicRoomNav::ManicRoomNav(ManicGame *game) : ManicRoomDraw(game)
{
	int x,y,w,h;

	x = CHAR_W;
	y = 256 + CHAR_H + CHAR_SH + (CHAR_SH / 4);
	w =   5 * CHAR_W;
	h =       CHAR_H + (CHAR_SH / 2);

	addHotspot(new Hotspot('J',x,    y  ,w,  h, 'J', 'j'));
        addHotspot(new Hotspot('<',x,    y+h,w/2,h, '<'));
        addHotspot(new Hotspot('>',x+w/2,y+h,w/2,h, '>'));
}


ManicRoomNav::~ManicRoomNav()
{
}



void ManicRoomNav::drawFixed(void)
{
	int x,y,h;
        videoScreen->fillBox(CHAR_SW, 256 + CHAR_H + CHAR_SH,
                                6 * CHAR_W, 2 * CHAR_H + CHAR_SH, ZX_BRWHITE);
        videoScreen->box(CHAR_SW, 256 + CHAR_H + CHAR_SH,
                                6 * CHAR_W, 2 * CHAR_H + CHAR_SH, ZX_BLACK);

        videoScreen->drawSmallText(CHAR_SW, 256 + CHAR_H, "Go to       ",
                                ZX_BRWHITE, ZX_BLACK);
	videoScreen->smallZxLogo(7 * CHAR_SW, 256 + CHAR_H); 
	x = CHAR_W;
	y = 256 + CHAR_H + CHAR_SH;
	h = CHAR_H + (CHAR_SH / 2);
	videoScreen->drawSmallText(CHAR_W, y + CHAR_SH, 
				"Enter no.", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x, y+h, " < > ", ZX_BLACK, ZX_BRWHITE);
	y = 256 + CHAR_H + CHAR_SH;
	videoScreen->box(CHAR_SW, y+h, 3 * CHAR_W, h, ZX_BLACK);
	videoScreen->box(CHAR_SW, y+h, 6 * CHAR_W, h, ZX_BLACK);
	
	ManicRoomDraw::drawFixed();
}

int ManicRoomNav::onHotspot(int id)
{
	jswByte *room = getRoom(m_room);

	switch(id)
	{
		case 'J': return jumpRoom();
		case '<': gotoRoom(m_game->prevRoom(m_room)); return ZXE_CONTINUE;
		case '>': gotoRoom(m_game->nextRoom(m_room)); return ZXE_CONTINUE;
	}
	return ZXE_CONTINUE;
}


int ManicRoomNav::onKeyDown(int keysym)
{
	int rv, rm;

	rv = HotspotManager::onKey(keysym); 
	if (rv != -1) return rv;
	switch(keysym)
	{
		case ZXK_F8:	return screenshotAll();
		case ZXK_CTRLA:
		case ZXK_CTRLJ: intPreContextMenu();
				rv = jumpRoomNumber(&rm);
				postContextMenu();
				if (rv >= ZXE_QUIT) return rv;
				if (rm >= 0) gotoRoom(rm);
				return ZXE_CONTINUE;

		case ZXK_CTRLL:	gotoRoom(m_room); return ZXE_CONTINUE;
		case ZXK_CTRLP:	if (m_prevRoom >= 0)
				{
					gotoRoom(m_prevRoom);
					return ZXE_CONTINUE;
				}
				break;
	}
	return ManicRoomDraw::onKeyDown(keysym);
}

int ManicRoomNav::onButtonDown(int x, int y, int button)
{
        int rv;

	rv = HotspotManager::onButton(x, y, button);
	if (rv != -1) return rv;
	return ManicRoomDraw::onButtonDown(x, y, button);
}

int ManicRoomNav::jumpRoom(void)
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

int ManicRoomNav::onReveal(int rect)
{
        /* Room we are working on has become unavailable? */
        if (m_game->getRoom(m_room) == NULL)
        {
                int l = m_game->prevRoom(m_room);
		if (l == m_room) l = m_game->nextRoom(m_room);

		gotoRoom(l);
        }
	else	gotoRoom(m_room);
	return ManicRoomDraw::onReveal(rect);
}




int ManicRoomNav::screenshotAll()
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
