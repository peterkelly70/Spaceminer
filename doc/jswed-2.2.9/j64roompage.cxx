/************************************************************************

    JSWED 2.1.6 - Editor for Jet Set Willy and derivatives

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
#include "j64roompage.hxx"
#include "j64roomform.hxx"
#include "j64cellform.hxx"
#include "spritelist.hxx"
#include "j64gfxform.hxx"
#include "j64itemform.hxx"
#include "j64fbarrierform.hxx"
#include "cellattrform.hxx"
#include "room.hxx"

static jswByte gl_clipBuffer[1024];
static char cellChars[] = "0123456789ABCDEF";	


Jsw64RoomPage::Jsw64RoomPage(JswGame *game) : Jsw48RoomNav(game)
{
	m_ot   = NULL;
	m_cx = m_cy = 0;
	m_octype = -1;
	m_ctype  = 0;
	m_oft = -1;
	m_mb = 0;
	m_ocx = m_ocy = -1;
	m_cdir = 0;

	// Active areas
	int x, y, w, h;
	x = VIDEO_W - 20 * CHAR_W;
	y = 256 + CHAR_H + CHAR_SH;
	w = 10 * CHAR_W;
	h = CHAR_H;

	char buf[8];

	for (int n = 0; n < 16; n++)
	{
		sprintf(buf, "%X:", n);
		addHotspot(new Hotspot(buf[0], x + (10 * CHAR_W * (n/8)), 
			y + (CHAR_H * (n % 8)), w, h, 
			toupper(buf[0]), tolower(buf[0]))); 
	}
	addHotspot(new Hotspot('I', x, y + 8 * CHAR_H, w, h, 'I', 'i'));
	addHotspot(new Hotspot('S', x + 10 * CHAR_W, y + 8 * CHAR_H, w, h, 'S', 's'));
			
        x = CHAR_W * 8;
        y = 256 + (2 * CHAR_H);
	h = CHAR_H;
	w = 11 * CHAR_W;

	addHotspot(new Hotspot('X', x, y,         w, h, 'X', 'x'));
	addHotspot(new Hotspot('L', x, y +     h, w, h, 'B', 'b'));
	addHotspot(new Hotspot('O', x, y + 2 * h, w, h, 'O', 'o'));
	addHotspot(new Hotspot('C' | ZXK_ALT, x, y+3*h, w, h, 'C' | ZXK_ALT,
				'c' | ZXK_ALT));

	addHotspot(new Hotspot('?', x, y + 5 * h + CHAR_SH, w, h, '?', '?'));
	setTitle("Rooms");
//	setCellType(0);
}

Jsw64RoomPage::~Jsw64RoomPage()
{
}


void Jsw64RoomPage::drawFixed(void)
{
	int x,y,h,n,m,max;

	videoScreen->bottomBar("Room editor");

	videoScreen->fillBox(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				19 * CHAR_W, 9 * CHAR_H + CHAR_SH, ZX_BRWHITE);
	videoScreen->box(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH-1,
				19 * CHAR_W, 9 * CHAR_H + CHAR_SH+1, ZX_BLACK);
	videoScreen->box(VIDEO_W - 19 * CHAR_SW, 256 + CHAR_H + CHAR_SH-1,
				19 * CHAR_SW, 9 * CHAR_H + CHAR_SH+1, ZX_BLACK);

	videoScreen->drawSmallText(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H,
				"Cells                                   ",
				ZX_BRWHITE, ZX_BLACK);

	videoScreen->smallZxLogo(VIDEO_W - 8 * CHAR_SH, 256 + CHAR_H);

	x = VIDEO_W - 19 * CHAR_W + CHAR_SW;
	y = 256 + CHAR_H + CHAR_SH;
	h = CHAR_H;
	int w = 9 * CHAR_W;

	char buf[8];

	max = getRoom()->getCellTypeCount();
	for (n = 0; n < max; n++)
	{
		sprintf(buf, "%X:", n);
		videoScreen->drawText(x + (n/8) * (9 * CHAR_W + CHAR_SW),
				      y + (n%8) * h, buf, ZX_BLACK, ZX_BRWHITE);
	}
	videoScreen->drawText(x, y + 8*h, "I:Item", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x + 9*CHAR_W + CHAR_SW, 
			y + 8*h, "S:Solar", ZX_BLACK, ZX_BRWHITE);

	x = CHAR_W * 8;
	y = 256 + CHAR_H;
	h = CHAR_H;

	videoScreen->fillBox(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 5 * CHAR_H,
			ZX_BRWHITE);
        videoScreen->box(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 5 * CHAR_H,
                        ZX_BLACK);

	videoScreen->drawSmallText(x - CHAR_SW,y,
			"Other options            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->drawText(x,y + h, "X:set eXits", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+2*h, "L:cLipboard", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+3*h, "O:Options  ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+4*h, "AltC:Convey", ZX_BLACK, ZX_BRWHITE);
	videoScreen->smallZxLogo(x + 18 * CHAR_SW, y);

	if (((Jsw64Game *)m_game)->getVariant() == 'Z')
	{
		videoScreen->fillBox(x - CHAR_SW, y + 6 * CHAR_H, 25 * CHAR_SW,
			       	2 * CHAR_H, ZX_BRWHITE);
	        videoScreen->box(x - CHAR_SW, y + 6 * CHAR_H, 25 * CHAR_SW, 
				2 * CHAR_H, ZX_BLACK);
		videoScreen->drawText(x,y+6*h + CHAR_SH, "?:Other    ", ZX_BLACK, ZX_BRWHITE);
		videoScreen->drawSmallText(x - CHAR_SW,y + 6 * CHAR_H,
			"Custom cell attributes   ", ZX_BRWHITE, ZX_BLACK);
	}
	x = 0;
	y = 256 + 9 * CHAR_H;
	videoScreen->drawSmallText(x, y, "Keyboard: ^Z or ^U to undo; CTRL+cursors", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "or J,<,> change room; cursor keys and", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "SPACE to draw", ZX_BLACK, ZX_WHITE);

	Jsw48RoomNav::drawFixed();
}


void Jsw64RoomPage::setCellType(int ct)
{
	if (ct >= getRoom()->getCellTypeCount() && ct < 16)
	{
		return;	
	}
	m_ctype = ct;
	drawCellType();
	drawData();
}

void Jsw64RoomPage::drawCellType()
{
	if ((m_octype == m_ctype) || !m_visible || !m_drawnFixed) return;

	int w  = 10 * CHAR_W + CHAR_SW;
	int x = VIDEO_W - 20 * CHAR_W;
	int y  = 256 + CHAR_H + CHAR_SH;
	int ox = x;
	int ow = w;
	int oy = y;

	if (m_ctype >= 0x100 && m_ctype <= 0x1FF)
	{
        	x = CHAR_W * 8 - CHAR_SW;
	        y = 256 + (7 * CHAR_H) + CHAR_SH;
		w = 12 * CHAR_W + CHAR_SW;
	}
	else switch(m_ctype)
	{
		case CT_CONV:	// Conveyor
		x = CHAR_W * 7;
		y = 256 + 5 * CHAR_H;
		w = 13 * CHAR_W; 
		break;


		case CT_ITEM:	/* item */
		y += 8 * CHAR_H;
		break;

		case CT_SOLAR:	/* solar */
		y += 8 * CHAR_H;
		x += 10 * CHAR_W + CHAR_SW;
		w -= CHAR_W;
		break;

		default:
		if (m_ctype >= 8)
		{
			x  += 10 * CHAR_W + CHAR_SW;
			w  -= CHAR_W;
		}
		y  += (m_ctype  % 8) * CHAR_H;
		break;
	}
	if (m_octype >= 0x100 && m_octype <= 0x1FF)
	{
        	ox = CHAR_W * 8 - CHAR_SW;
	        oy = 256 + (7 * CHAR_H) + CHAR_SH;
		ow = 12 * CHAR_W + CHAR_SW;
	}
	else switch(m_octype)
	{
		case CT_CONV:
		ox = CHAR_W * 7;
		oy = 256 + 5 * CHAR_H;
		ow = 13 * CHAR_W; 
		break;

		case CT_ITEM:	/* item */
		oy += 8 * CHAR_H;
		break;

		case CT_SOLAR:	/* solar */
		oy += 8 * CHAR_H;
		ox += 10 * CHAR_W + CHAR_SW;
		ow -= CHAR_W;
		break;

		default:
		if (m_octype >= 8)
		{
			ox += (m_octype / 8) * (10 * CHAR_W + CHAR_SW);
			ow -= CHAR_W;
		}
		oy += (m_octype % 8) * CHAR_H;
		break;
	}
	

	if (m_octype >= 0) videoScreen->fillReplace (ox, oy, ow, CHAR_H,
                                        ZX_BRCYAN,
                                        ZX_BRWHITE);
	if (m_ctype >= 0) videoScreen->fillReplace(x, y, w, CHAR_H, ZX_BRWHITE,
                                        ZX_BRCYAN);
	m_octype = m_ctype;

}


void Jsw64RoomPage::flashColours(void)
{
	int objcount = m_game->getItemCount();

	if (!m_visible) return;


	if (m_oft != objcount && m_drawnFixed)
	{
		char buf[20];

		sprintf(buf, "%03d item%s", objcount, 
				(objcount == 1) ? " free " : "s free" );
		videoScreen->drawSmallText(13 * CHAR_W, 
					   VIDEO_H - 4 * CHAR_H, buf, 
					   ZX_BLACK, ZX_WHITE);

		m_oft = objcount;
	}
        Jsw48RoomNav::flashColours();

        videoScreen->fillBox(80, VIDEO_H - 32, 480, 32, ZX_WHITE);

}



void Jsw64RoomPage::drawXCursor(VideoSurface *s, int x, int y, int i)
{
	s->plot(x + 8, y + 8, i);
        s->plot(x + 7, y + 7, i);
        s->plot(x + 7, y + 9, i);
        s->plot(x + 6, y + 6, i);
        s->plot(x + 6, y +10, i);
        s->plot(x + 9, y + 7, i);
        s->plot(x + 9, y + 9, i);
        s->plot(x +10, y + 6, i);
        s->plot(x +10, y +10, i);
}


void Jsw64RoomPage::drawConveyorCursor(VideoSurface *s, int i)
{
	int x, w;

	if      (m_cx < m_ocx) m_cdir = -1;
	else if (m_cx > m_ocx) m_cdir = 1;

	/* Note that if m-cx != m_ocx, we don't touch m_cdir */

	x = (m_cdir < 0) ? m_cx : m_ocx;
	w = (m_cdir > 0) ? m_cx : m_ocx;

	w -= (x-1);

	s->box(x * 16, m_ocy * 16, w * 16, 16, i);

	for (int x1 = x; x1 < (x + w); x1++)
	{
		if (m_cdir)
		{
			s->box(x1 * 16 + 2, m_ocy * 16 + 8, 12, 1, i);
			if (m_cdir < 0)
			{
				s->plot(x1 * 16 + 3, m_ocy * 16 + 7, i);
				s->plot(x1 * 16 + 3, m_ocy * 16 + 9, i);
				s->plot(x1 * 16 + 4, m_ocy * 16 + 6, i);
				s->plot(x1 * 16 + 4, m_ocy * 16 +10, i);
			}
			else
			{
				s->plot(x1 * 16 +13, m_ocy * 16 + 7, i);
				s->plot(x1 * 16 +13, m_ocy * 16 + 9, i);
				s->plot(x1 * 16 +12, m_ocy * 16 + 6, i);
				s->plot(x1 * 16 +12, m_ocy * 16 +10, i);
			}
		}
		else drawXCursor(s, x1 * 16, m_ocy * 16, i);
	}
}



void Jsw64RoomPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	if (m_ocx != -1)
	{
		if (m_ctype == CT_CONV) drawConveyorCursor(s, i);
	}
	else 
	{
		if (m_ctype == CT_SOLAR)
		{
			if (m_cy > 7) m_cy = 7;
		}
		s->box(m_cx * 16, m_cy * 16, 16, 16, i); 
	}
}




void Jsw64RoomPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];
		int x,y,fg;

       		sprintf(s, "Room editor [%d]", m_room);
	        videoScreen->bottomBar(s);

		int max = getRoom()->getCellTypeCount();

		for (int n = 0; n < max; n++)
		{
			x = VIDEO_W - 12 * CHAR_W + CHAR_SW;
			y = 256 + CHAR_SH + (1 + (n % 8)) * CHAR_H;

		       	if (n >= 8) x += (CHAR_W * 9) + CHAR_SW;
			getRoom()->drawCell(x + CHAR_SW, y, n); 
			x -= 5 * CHAR_W;
			if (m_ctype == n) fg = ZX_BRCYAN;
			else		  fg = ZX_BRWHITE;
			switch(getRoom()->getCellBehaviour(n))
			{
				case CB_AIR:	videoScreen->drawText (x, y, "air  ", ZX_BLACK, fg); break;
				case CB_WATER:	videoScreen->drawText (x, y, "water", ZX_BLACK, fg); break;
				case CB_EARTH:	videoScreen->drawText (x, y, "earth", ZX_BLACK, fg); break;
				case CB_FIRE:	videoScreen->drawText (x, y, "fire ", ZX_BLACK, fg); break;
				case CB_LRAMP:	videoScreen->drawText (x, y, "\\ramp", ZX_BLACK, fg); break;
				case CB_RRAMP:	videoScreen->drawText (x, y, "/ramp", ZX_BLACK, fg); break;
				case CB_LCONV:	videoScreen->drawText (x, y, "<conV", ZX_BLACK, fg); break;
				case CB_RCONV:	videoScreen->drawText (x, y, ">conV", ZX_BLACK, fg); break;
				case CB_CRUMBLY:videoScreen->drawText (x, y, "crumb", ZX_BLACK, fg); break;
				case CB_TRAMP:	videoScreen->drawText (x, y, "tramp", ZX_BLACK, fg); break;
				case CB_TRAP:	videoScreen->drawText (x, y, "trap ", ZX_BLACK, fg); break;
			}
		}
		x = VIDEO_W - 12 * CHAR_W + CHAR_SW;
		y = 256 + CHAR_SH + 9 * CHAR_H;
		videoScreen->fillBox(x + CHAR_SW, y, 16, 16, ZX_BLACK);
		getRoom()->drawCell(x + CHAR_SW, y, CT_ITEM); 
		x += (CHAR_W * 9) + CHAR_SW;
		getRoom()->drawCell(x + CHAR_SW, y, CT_SOLAR); 
	}
	Jsw48RoomNav::drawData(flashonly);
}









void Jsw64RoomPage::putCell(int shifted)
{
	int sx, sy, ob;
	jswByte *room = getRoom(m_room);

	switch(m_ctype)
	{
		case CT_ITEM: // Item
			ob = m_game->objectAt(m_room, m_cx, m_cy * 8);

			if (ob == -1 || shifted) 
				m_game->objectInsert(m_room,m_cx,m_cy*8);
			else	m_game->objectDelete(ob);
			break;
		case CT_SOLAR:
			getRoom()->getSolarPos(&sx, &sy);
			getRoom()->setSolarPos(m_cx, m_cy);	
			getRoom()->drawCell(m_bmRoom, sx, sy, false, false);
			getRoom()->drawCell(m_bmRoom, m_cx, m_cy, false, false);
			break;

		case CT_CONV:
			if (m_ocx == -1)	// Start conveyor 
			{
				m_ocx = m_cx;
				m_ocy = m_cy;
				m_cdir = 0;
			}
			else			// End conveyor 
			{
				int clen = abs(m_cx - m_ocx) + 1; 
				if (!m_cdir) clen = 0;
				int cx, cy, cpos;

				cx = (m_ocx < m_cx) ? m_ocx : m_cx;
				cy = m_ocy;
				cpos = ((cy << 5) | cx) + 0x5E00;

				room[0xD6] = (m_cdir < 0) ? 0 : 1;
				room[0xD7] = (cpos & 0xFF);
				room[0xD8] = (cpos >> 8);	
				room[0xD9] = clen;
				m_ocx = -1;
				m_ocy = -1;
				m_cdir = 0;
				drawBasic(0);
			}
			break;

		default: 
		getRoom()->setCell(m_cx, m_cy, m_ctype);
		getRoom()->drawCell(m_bmRoom, m_cx, m_cy, false, false);
		break;
	}
}







int Jsw64RoomPage::roomExits(void)
{
	char str[4][48];
	jswByte dummy[161];
	jswByte *b = NULL;
	jswByte *room = getRoom(m_room);
	jswByte *rl   = getRoom(room[0xE9]),
	        *rr   = getRoom(room[0xEA]),
                *ru   = getRoom(room[0xEB]),
                *rd   = getRoom(room[0xEC]);

	sprintf((char *)dummy + 128, ":Out of range:");
	if (!rr) rr = dummy;
	if (!rl) rl = dummy;
	if (!ru) ru = dummy;
	if (!rd) rd = dummy;
	sprintf(str[0], "Left  [%03d %-24.24s]", room[0xE9], rl+0xB6);
	sprintf(str[1], "Right [%03d %-24.24s]", room[0xEA], rr+0xB6);
	sprintf(str[2], "Up    [%03d %-24.24s]", room[0xEB], ru+0xB6);
	sprintf(str[3], "Down  [%03d %-24.24s]", room[0xEC], rd+0xB6);

	VideoMenu vm("Set exits", str[0], str[1], str[2], str[3], NULL); 

	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	JswRoomList *s = m_game->getRoomList();
	if (s)
	{
		switch(vm.getSelected())
		{
			case 0: b = &(room[0xE9]); s->setTitle("Left exit");
				break;
			case 1: b = &(room[0xEA]); s->setTitle("Right exit");
				break;
			case 2: b = &(room[0xEB]); s->setTitle("Up exit");
				break;
			case 3: b = &(room[0xEC]); s->setTitle("Down exit");
				break;
		}
		s->setTitle("Exit destination");
		s->setSelectedRoom(*b);
		rv = s->doModal();
		if (rv != ZXE_QUIT && rv != ZXE_OK) rv = ZXE_CONTINUE;
		else if (rv == ZXE_OK)
		{
			*b = s->getSelectedRoom();
			rv = ZXE_CONTINUE;
		}
		delete s;
	}
	return rv;
}



int Jsw64RoomPage::roomProperties(void)
{
	VideoMenu vm("Room properties", "General", "Cells", "Item bitmap", 
			"Sprites", "Final Barrier", NULL); 
        jswByte cimage[1024];

	int rv = vm.doModal();
	int roomLen = getRoom()->getRoomLength();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0:
		{
			memcpy(cimage, getRoom(m_room), roomLen);

			Jsw64RoomForm rf(m_room, m_game, cimage);
			rv = rf.doModal();
			if (rv >= ZXE_QUIT) return rv;
			if (rv == ZXE_OK) 
			{
				memcpy(getRoom(m_room), cimage, roomLen);
				drawData();
			}
		}
		break;

		case 1: return roomCells();
		case 2:
		{
			memcpy(cimage, getRoom(m_room), roomLen);

			Jsw64ItemForm rf(cimage + 0xE1);
			rv = rf.doModal();
			if (rv >= ZXE_QUIT) return rv;
			if (rv == ZXE_OK) 
			{
				memcpy(getRoom(m_room), cimage, roomLen);
				drawData();
			}
		}
		break;
		case 3:
		{
			memcpy(cimage, getRoom(m_room), roomLen);

			Jsw64GraphicsForm jsg((Jsw64Game *)m_game, 
					getRoom(), cimage);
			rv = jsg.doModal();
			if (rv >= ZXE_QUIT) return rv;
			if (rv == ZXE_OK) 
			{
				memcpy(getRoom(m_room), cimage, roomLen);
				for (int n = 0; n < (GUARD_MAX + FG_MAX); n++)
				{
					if (m_guards[n])
					{
	/* Force a regen of sprite graphics */
						m_guards[n]->setSpritePage(
						  m_guards[n]->getSpritePage());
					}
				}
				drawData();
			}
		}
		break;

		case 4: // Bring up a Final Barrier form.
		{
			jswByte *rm  = getRoom(m_room);
			jswByte oldc = rm[0xD6]; 

			Jsw64BarrierForm jbf(rm, m_game, m_bmRoom);
			rv = jbf.doModal();
			if (rv >= ZXE_QUIT) return rv;
			if (rv != ZXE_OK) 
			{
				rm[0xD6] = oldc;
			}
		}
		break;
	}
	return ZXE_CONTINUE;
}





int Jsw64RoomPage::roomCells(void)
{
	Cell cells[16];
        int rv;
	int roomLen = getRoom()->getRoomLength();

	getRoom()->getCells(cells);

        Jsw64CellForm rf(cells, getRoom());
        rv = rf.doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_OK)
        {
		getRoom()->setCells(cells);
		getRoom()->createCellPatterns();
		
                drawData(0);
        }
	return ZXE_CONTINUE;
}



int Jsw64RoomPage::conveyorMenu(void)
{
        jswByte *room = getRoom(m_room);
	VideoMenu vmc("Conveyor", "Set animation", "Off", "Sticky", 
			NULL);
	int rv = vmc.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	switch(vmc.getSelected())
	{
		case 0: setCellType(CT_CONV);
			return ZXE_CONTINUE;
		case 1:	room[0xD6] = 2;
			return ZXE_CONTINUE;
		case 2: room[0xD6] = 3;
			return ZXE_CONTINUE;
	}
	return rv;
}



int Jsw64RoomPage::preContextMenu(void)
{
	intPreContextMenu();
	VideoMenu vm("Options", "Editor", "Conveyor", // "Guardian", 
                     "Room options", "Room exits", "Clipboard etc.", 
                     "Exit editor", NULL);
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) 
	{
		postContextMenu();
		return ZXE_CONTINUE;
	}
	switch (vm.getSelected())
	{
		case 0: postContextMenu();
		        return ZXE_CONTINUE;
		case 1: rv = conveyorMenu();
			postContextMenu();
			return rv;
		case 2: rv = roomProperties();
			postContextMenu();
			return rv;
		case 3: rv = roomExits();
			postContextMenu();
			return rv;
		case 4: rv = roomClipboard();
			postContextMenu();
			return rv;
		case 5: postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return Jsw48RoomNav::preContextMenu();
}







void Jsw64RoomPage::undo(void)
{
	jswByte buf[1024];
	jswByte *b = m_game->getRoom(m_room);
	int roomLen = getRoom()->getRoomLength();

	memcpy(buf, b, roomLen);
        memcpy(b, undoBuffer, roomLen);
	memcpy(undoBuffer, buf, roomLen);

	if (m_visible) drawData();
}


void Jsw64RoomPage::loadRoom(int room, int reload)
{
        jswByte *b = m_game->getRoom(room);
	int roomLen = getRoom()->getRoomLength();

	if (!reload) memcpy(undoBuffer, b, roomLen);

	switch(((Jsw64Game *)m_game)->getVariant())
	{ 
		case 'V': case 'W': 
			memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x200); 
			break; 
		default: memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x400); 
			 break; 
	}
	Jsw48RoomNav::loadRoom(room, reload);
}



int Jsw64RoomPage::onHotspot(int id)
{
	int rv;
	if (id >= '0' && id <= '9')
	{
		setCellType(id - '0');
		return ZXE_CONTINUE;
	}
	if (id >= 'A' && id <= 'F')
	{
		setCellType(id - 'A' + 10);
		return ZXE_CONTINUE;
	}


	switch(id)
	{
		case 'I': // Item
			setCellType(CT_ITEM);
			return ZXE_CONTINUE;
		case 'S': // Solar Power
			setCellType(CT_SOLAR);
			return ZXE_CONTINUE;
		case 'V': // ConVeyor
			setCellType(CT_SOLAR);
			return ZXE_CONTINUE;
		case 'X': // Exits
			intPreContextMenu();
			rv = roomExits();
			postContextMenu();
			return rv;
		case 'L': // Clipboard
			intPreContextMenu();
			rv = roomClipboard();
			postContextMenu();
			return rv;	
		case 'O': // Options
			intPreContextMenu();
			rv = roomProperties();
			postContextMenu();
			return rv;
		case ('C' | ZXK_ALT): // Conveyor extra
			intPreContextMenu();
			rv = conveyorMenu();
			postContextMenu();
			return rv;
		case '?':	// Custom cell type
			if (((Jsw64Game *)m_game)->getVariant() != 'Z')
				break;
			intPreContextMenu();
			{
				jswByte attr = m_ctype & 0xFF;	

				CellAttrForm cf(&attr);
				rv = cf.doModal();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_OK)
				{
					postContextMenu();
					setCellType(attr | 0x100);
				}
				else postContextMenu();
			}
			return ZXE_CONTINUE;
	}
	return Jsw48RoomNav::onHotspot(id);

}


int Jsw64RoomPage::onKeyDown(int keysym)
{
	
	switch(keysym)
	{
		case ZXK_BREAK:
			m_ocx = m_ocy = -1;
			m_cdir  = -1;
			return ZXE_CONTINUE;
		case 'V':
		case 'v':
			setCellType(CT_CONV);
			return ZXE_CONTINUE;
		case 'U' - '@':
		case 'Z' - '@':
			undo();
			return ZXE_CONTINUE;
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
			 putCell(0); return ZXE_CONTINUE;
		case ZXK_SHIFTSPACE: 
			 putCell(1); return ZXE_CONTINUE;
	} 
	return Jsw48RoomNav::onKeyDown(keysym);
}

int Jsw64RoomPage::onButtonDown(int x, int y, int button)
{
	if (y < 256)
	{
		m_mb = 1;
		return onMouseMove(x, y, 0, 0);
	}
	return Jsw48RoomNav::onButtonDown(x, y, button);
}

int Jsw64RoomPage::onButtonUp(int x, int y, int button)
{
	m_mb = 0;
	return Jsw48RoomNav::onButtonUp(x, y, button);
}

int Jsw64RoomPage::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + 512) &&
	    y >= m_y && y < (m_y + 256))
	{
		m_cx = (x - m_x)/16;
		m_cy = (y - m_y)/16;
		if (m_mb) putCell(shiftPressed());
	}
	return ZXE_CONTINUE;
}






int Jsw64RoomPage::roomClipboard(void)
{
	int roomLen = getRoom()->getRoomLength();
	jswByte *room = getRoom(m_room);
	jswByte fill;
	int y,x;

	VideoMenu vm("Clipboard etc.",
		     "Cut", "Copy", "Paste", "Clear room",
		     "Flip vertically", "Flip horizontally", NULL);
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: memcpy(gl_clipBuffer, room, roomLen);
			getRoom()->clear();
			if (m_visible) drawData();
			break;
		case 1: memcpy(gl_clipBuffer, room, roomLen);
			break;
		case 2: memcpy(room, gl_clipBuffer, roomLen);
			if (m_visible) drawData();
			break;
		case 3:	getRoom()->clear();
			if (m_visible) drawData();
			break;
		case 4: getRoom()->mirrorV();
			if (m_visible)
			{
				loadRoom(m_room);
				drawData();
			}
			break;
		case 5: getRoom()->mirrorH();
			if (m_visible)
			{
				loadRoom(m_room);
				drawData();
			}
			break;
	}
	return ZXE_CONTINUE;
}


