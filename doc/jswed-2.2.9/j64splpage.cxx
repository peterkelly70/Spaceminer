/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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
#include "j64game.hxx"
#include "j48splpage.hxx"
#include "j64splpage.hxx"
#include "j64targetform.hxx"
#include "room.hxx"
#include "spritelist.hxx"

#include "tick.xbm"

Jsw64SplPage::Jsw64SplPage(JswGame *game) : Jsw48SplPage(game)
{
	int x, y, w, h;

	Portal p;
	getRoom()->getPortal(p);
	if (p.sprite)
		m_bmPortal = ::bitmapFromSprite(16, 16, p.sprite, 1, ZX_BLACK, ZX_BRWHITE);
	else 	m_bmPortal = NULL;
	m_bmTick = bitmapFromXbm(tick_width, tick_height, tick_bits, 0, 
			ZX_BRBLUE, ZX_BRWHITE);

	h = (3 * CHAR_H + CHAR_SH);
	y = 18 * CHAR_H + CHAR_SH;

	addHotspot(new Hotspot('X', 17 * CHAR_W, y, 32, h, 'x','X'));
        x = VIDEO_W - 16 * CHAR_W;
	y = 24 * CHAR_H + CHAR_SH;
        w = 4 * CHAR_W;
        h = 2 * CHAR_H;

        addHotspot(new Hotspot('I', x, y, w, h, 'I', 'i')); x += w;
        addHotspot(new Hotspot('P', x, y, w, h, 'P', 'p')); x += w;
        addHotspot(new Hotspot('B', x, y, w, h, 'B', 'b')); x += w;
        addHotspot(new Hotspot('F', x, y, w, h, 'F', 'f'));

	x = 8 * CHAR_W;
	y = 23 * CHAR_H;
	w = 14 * CHAR_W;
	h = CHAR_H;

	addHotspot(new Hotspot('G', x, y +   CHAR_H, w, h, 'G', 'g'));
	addHotspot(new Hotspot('D', x, y + 2*CHAR_H, w, h, 'D', 'd'));

	setTitle("Portals");
}

Jsw64SplPage::~Jsw64SplPage()
{
	delete m_bmTick;
	if (m_bmPortal) delete m_bmPortal;
}


void Jsw64SplPage::drawFixed(void)
{
	int x,y,w,h;

	w = 13;
	
	if (m_game->imcMode()) 
	{
		w = 16;
	}
	videoScreen->bottomBar("Start position / Portal");
	
	videoScreen->fillBox(16 * CHAR_W, 18 * CHAR_H, w * CHAR_W, 4 * CHAR_H,
				ZX_BRWHITE);
        videoScreen->box    (16 * CHAR_W, 18 * CHAR_H, w * CHAR_W, 4 * CHAR_H,
                                ZX_BLACK);
	videoScreen->drawSmallText(16 * CHAR_W, 18 * CHAR_H, 
				   "Special Characters        ", ZX_BRWHITE, ZX_BLACK);
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
	y = 21 * CHAR_H + (CHAR_SH / 2);

	videoScreen->drawSmallText(18 * CHAR_W, y, "X", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawSmallText(21 * CHAR_W, y, "W", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawSmallText(24 * CHAR_W, y, "M", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawSmallText(27 * CHAR_W, y, "T", ZX_BLACK, ZX_BRWHITE);
	if (m_game->imcMode())
	{
        	videoScreen->drawSmallText(30 * CHAR_W, y, "R", ZX_BLACK, ZX_BRWHITE);
	}

	x = VIDEO_W - 16 * CHAR_W;
	y = 24 * CHAR_H;
	w = 4 * CHAR_W;
	h = 2 * CHAR_H;
	videoScreen->fillBox(x, y, 16 * CHAR_W, 5*CHAR_SH, ZX_BRWHITE);
	videoScreen->box(x, y + CHAR_SH, 16 * CHAR_W, h, ZX_BLACK);
	videoScreen->drawSmallText(x, y, "Portal attributes               ", 
			ZX_BRWHITE, ZX_BLACK);	
	videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "I:* P:* B:* F:"
			                        , ZX_BLACK, ZX_BRWHITE);
        videoScreen->smallZxLogo(VIDEO_W - 6 * CHAR_SW, y);
        videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK); 
        videoScreen->box(x, y + CHAR_SH, w + 4*CHAR_W, h, ZX_BLACK);
        videoScreen->box(x, y + CHAR_SH, w + 8*CHAR_W, h, ZX_BLACK);

	x = 8 * CHAR_W;
	y = 23 * CHAR_H;
	w = 14 * CHAR_W;
	h = 3 * CHAR_H;
	videoScreen->fillBox(x, y + CHAR_SH, w, h, ZX_BRWHITE);
	videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
	videoScreen->drawSmallText(x, y, "Portal options              ", 
			ZX_BRWHITE, ZX_BLACK);	
        videoScreen->smallZxLogo(x + 22 * CHAR_SW, y);
	videoScreen->drawText(x + CHAR_SW, y + CHAR_H,   "G:Graphic"
			                        , ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x + CHAR_SW, y + 2*CHAR_H, "D:Destination"
			                        , ZX_BLACK, ZX_BRWHITE);

	Jsw48RoomNav::drawFixed();
	m_prevMode = -1;
}



void Jsw64SplPage::drawData(int flashonly)
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

       		sprintf(s, "Start position and portals [%d]", m_room);
	        videoScreen->bottomBar(s);
		if (m_bmPortal) 
		{
			m_bmPortal->toScreen(17 * CHAR_W, 19 * CHAR_H);
		}
		else
		{
			videoScreen->fillBox(17 * CHAR_W, 19 * CHAR_H, 
					32, 32, ZX_BRWHITE);
		}
		if (4 == m_mode)
		{
			videoScreen->fillReplace(17 * CHAR_W, 19 * CHAR_H,
					32, 32, ZX_BRWHITE, ZX_BRCYAN);
		}
	}
	if (m_mode != m_prevMode) 
	{
		for (n = 0; n < 5; n++)
		{
			int x = ((3 * n) + 20) * CHAR_W;
			int y = 18 * CHAR_H + CHAR_SH;
			int w = 2 * CHAR_W;
			int h = 3 * CHAR_H + CHAR_SH;

			if (n == 4) x = 17 * CHAR_W;

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

	showAttributes();
	Jsw48RoomNav::drawData(flashonly);


}





int Jsw64SplPage::onHotspot(int id)
{
	int rv;
	jswByte *data = getRoom(m_room);
	jswByte oa = data[0xF4];

	switch(id)
	{
	  // Switch to Portal mode
	  	case 'G': intPreContextMenu();
			  rv = portalGraphic();
			  postContextMenu();
			  return rv;
	  	case 'D': intPreContextMenu();
			  rv = portalDestination();
			  postContextMenu();
			  return rv;
		case 'X': m_mode = 4; drawData(); return ZXE_CONTINUE;
		case 'I': data[0xF4] &= ~7;
			  data[0xF4] |= (oa + 1) & 7;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  drawData();
			  return ZXE_CONTINUE;
		case 'P': data[0xF4] &= ~0x38;
			  data[0xF4] |= (oa + 8) & 0x38;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  drawData();
			  return ZXE_CONTINUE;
		case 'F': data[0xF4] ^= 0x80;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  drawData();
			  return ZXE_CONTINUE;
		case 'B': data[0xF4] ^= 0x40;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  drawData();
			  return ZXE_CONTINUE;

	}
	return Jsw48SplPage::onHotspot(id);
}




int Jsw64SplPage::onSelect(void)
{
	jswByte *room = m_game->getRoom(m_room);

	switch(m_mode)
	{
		case 4:
		getRoom()->setPortalPos(m_cx, m_cy);
		drawData();
		return ZXE_CONTINUE;	
	}
	return Jsw48SplPage::onSelect();
}




int Jsw64SplPage::preContextMenu(void)
{
	intPreContextMenu();
	VideoMenu vm("Options", "Editor", "Portal graphic",
			"Portal destination", "Portal effect", "Exit", NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL)
	{
		postContextMenu();
		return ZXE_CONTINUE;
	}
	switch(vm.getSelected())
	{
		case 0:
			postContextMenu();
			return ZXE_CONTINUE;
		case 1:	rv = portalGraphic();
			postContextMenu();
			return rv;
		case 2:	rv = portalDestination();
			postContextMenu();
			return rv;
		case 3: rv = portalType();
			postContextMenu();
			return rv;
		case 4:	postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return Jsw48RoomNav::preContextMenu();
}



void Jsw64SplPage::showAttributes()
{
        int x,y;
        int ifg, ibg, pfg, pbg;

	jswByte *data = getRoom(m_room);

        ibg = (data[0xF4] & 7) | ZX_BRIGHT;               // Current ink
        pbg = ((data[0xF4] >> 3) & 7) | ZX_BRIGHT;        // Current paper
        //
        // "INK 9" - choose black/white ink for contrast
        //
        if (ibg >= ZX_BRGREEN) ifg = ZX_BLACK; else ifg = ZX_BRWHITE;
        if (pbg >= ZX_BRGREEN) pfg = ZX_BLACK; else pfg = ZX_BRWHITE;

        x = VIDEO_W - 14 * CHAR_W + CHAR_SW;
        y = 25 * CHAR_H;
        //
        // Display colours
        //
        videoScreen->drawText(x,          y, (ibg + '0' - ZX_BRIGHT), ifg, ibg);
        videoScreen->drawText(x+4*CHAR_W, y, (pbg + '0' - ZX_BRIGHT), pfg, pbg);
	//
	// Empty the bright/flash checkboxes
	//
        videoScreen->fillBox(x + 8*CHAR_W, y, CHAR_W, CHAR_H, ZX_BRWHITE);
        videoScreen->fillBox(x +12*CHAR_W, y, CHAR_W, CHAR_H, ZX_BRWHITE);
	//
	// And set them as appropriate.
	//
        if (data[0xF4] & 0x40) m_bmTick->toScreen(x +  8 * CHAR_W + 2, y+2);
        if (data[0xF4] & 0x80) m_bmTick->toScreen(x + 12 * CHAR_W + 2, y+2);
}


void Jsw64SplPage::loadRoom(int room, int reload)
{
	jswByte *b = m_game->getRoom(room);
	switch(((Jsw64Game *)m_game)->getVariant())
	{ 
		case 'V': case 'W': 
			memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x200); 
			break; 
		default: memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x400); 
			 break; 
	}
	Jsw48SplPage::loadRoom(room, reload);

	delete m_bmPortal;
	Portal p;
	getRoom()->getPortal(p);
	if (p.sprite)
		m_bmPortal = ::bitmapFromSprite(16, 16, p.sprite, 1, ZX_BLACK, ZX_BRWHITE);
	else	m_bmPortal = NULL;
	if (4 == m_mode) videoScreen->fillReplace(17 * CHAR_W, 19 * CHAR_H,
			32, 32, ZX_BRWHITE, ZX_BRCYAN);
}


int Jsw64SplPage::portalGraphic()
{
	int spr;
	jswByte *data = getRoom(m_room);
	VideoMenu vm("Portal graphic", "No portal", "Choose sprite",
			"Cancel", NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: data[0xEE] = data[0xEF] = 0;
			loadRoom(m_room, 1);
			return ZXE_CONTINUE;

		case 1:	
			{
				SpriteList sl(m_game->getMem(),
						2, 12 * CHAR_H, VIDEO_W - 4,
						CHAR_H + CHAR_SH + 40);
				m_game->fillSprites(m_room, &sl);
				rv = sl.doModal();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
				spr = sl.getSelected();
				data[0xEE] = spr & 0xFF;
				data[0xEF] = (spr >> 8) & 0xFF;
				loadRoom(m_room, 1);
			}
			break;
	}
	return ZXE_CONTINUE;	
}



int Jsw64SplPage::portalDestination()
{
	jswByte *room = getRoom(m_room);
	int rv;

	JswRoomList *s = m_game->getRoomList();
	if (s)
	{
		s->setSelectedRoom(room[0xF5]);
		s->setTitle("Portal destination");
		rv = s->doModal();
		if (rv >= ZXE_QUIT) return rv;
		if (rv != ZXE_OK) return ZXE_CONTINUE;
		room[0xF5] = s->getSelectedRoom();
		delete s;
	}
	Jsw64TargetForm tf(m_game, m_game->getRoomClass(room[0xF5]), room);
	rv = tf.doModal();
	if (rv >= ZXE_QUIT) return rv;
	return ZXE_CONTINUE;
}


int Jsw64SplPage::portalType()
{
	VideoMenu vm("Portal effect",
		     "No effect",
		     "Screen flashes",
		     "Screen flash + air", NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

	jswByte *data = getRoom(m_room);
	switch(vm.getSelected())
	{
		case 0: data[0xF9] = 0xA6; data[0xFA] = 0x86; break;
		case 1: data[0xF9] = 0xA3; data[0xFA] = 0x86; break;
		case 2: data[0xF9] = 0xA0; data[0xFA] = 0x86; break;
	}
	return ZXE_CONTINUE;
}
