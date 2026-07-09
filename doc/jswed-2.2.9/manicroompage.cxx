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
#include "manicgame.hxx"
#include "manicitemform.hxx"
#include "manicroompage.hxx"
#include "manicroomform.hxx"
#include "cellattrform.hxx"
#include "cellform.hxx"
#include "room.hxx"

static jswByte gl_clipBuffer[1024];
static void xy(int ctype, int *x, int *y, int *w);


ManicRoomPage::ManicRoomPage(ManicGame *game) : ManicRoomNav(game)
{
	m_ot   = NULL;
	m_cx = m_cy = m_acyminor = 0;
	m_octype = -1;
	m_oft = -1;
	m_mb = 0;
	m_ocx = m_ocy = -1;
	m_cdir = 0;
	m_attr = 0;

	// Active areas
	int x, y, w, h;
	h = CHAR_H;

	xy(0, &x, &y, &w); addHotspot(new Hotspot('a', x, y, w, h, 'A', 'a')); 
	xy(1, &x, &y, &w); addHotspot(new Hotspot('w', x, y, w, h, 'w', 'w')); 
	xy(2, &x, &y, &w); addHotspot(new Hotspot('r', x, y, w, h, 'R', 'r')); 
	xy(3, &x, &y, &w); addHotspot(new Hotspot('e', x, y, w, h, 'E', 'e')); 
	xy(4, &x, &y, &w); addHotspot(new Hotspot('c', x, y, w, h, 'c', 'c')); 
	xy(5, &x, &y, &w); addHotspot(new Hotspot('f', x, y, w, h, 'f', 'f')); 
	xy(6, &x, &y, &w); addHotspot(new Hotspot('F', x, y, w, h, 'F', 'F')); 
	xy(7, &x, &y, &w); addHotspot(new Hotspot('W', x, y, w, h, 'W', 'W')); 
	xy(8, &x, &y, &w); addHotspot(new Hotspot('i', x, y, w, h, 'I', 'i')); 
	xy(9, &x, &y, &w); addHotspot(new Hotspot('?', x, y, w, h, '?', '?')); 
	xy(10,&x, &y, &w); addHotspot(new Hotspot('C', x, y, w, h, 'C', 'C')); 

/*	w = 6 * CHAR_W;
	y = 256 + 2*CHAR_H;
	x = VIDEO_W - 19 * CHAR_W;

        addHotspot(new Hotspot('1', x, y, w, h, '1')); y += CHAR_H;
        addHotspot(new Hotspot('2', x, y, w, h, '2')); y += CHAR_H;
        addHotspot(new Hotspot('3', x, y, w, h, '3')); y += CHAR_H;
        addHotspot(new Hotspot('4', x, y, w, h, '4')); y += CHAR_H;
        addHotspot(new Hotspot('5', x, y, w, h, '5')); y += CHAR_H;
        addHotspot(new Hotspot('6', x, y, w, h, '6')); y += CHAR_H;
        addHotspot(new Hotspot('7', x, y, w, h, '7')); y += CHAR_H;
        addHotspot(new Hotspot('8', x, y, w, h, '8')); y += CHAR_H;

	x += w;
	w = 2 * CHAR_W;
        y = 256 + 2*CHAR_H;

        addHotspot(new Hotspot(ZXK_ALT | '1', x,y,w,h,ZXK_ALT | '1')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '2', x,y,w,h,ZXK_ALT | '2')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '3', x,y,w,h,ZXK_ALT | '3')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '4', x,y,w,h,ZXK_ALT | '4')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '5', x,y,w,h,ZXK_ALT | '5')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '6', x,y,w,h,ZXK_ALT | '6')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '7', x,y,w,h,ZXK_ALT | '7')); y += CHAR_H;
        addHotspot(new Hotspot(ZXK_ALT | '8', x,y,w,h,ZXK_ALT | '8')); y += CHAR_H;
*/	
        x = CHAR_W * 8;
        y = 256 + (2 * CHAR_H);
	h = CHAR_H;
	w = 11 * CHAR_W;

	addHotspot(new Hotspot('B', x, y,         w, h, 'B', 'b'));
	addHotspot(new Hotspot('O', x, y +     h, w, h, 'O', 'o'));
	addHotspot(new Hotspot('S', x, y + 2 * h, w, h, 'S', 's'));
	//addHotspot(new Hotspot('G', x, y + 2 * h, w, h, 'G', 'g'));
	addHotspot(new Hotspot('I' | ZXK_ALT, x, y+3*h, w, h, 'I' | ZXK_ALT,
				'i' | ZXK_ALT));
	addHotspot(new Hotspot('C' | ZXK_ALT, x, y+4*h, w, h, 'C' | ZXK_ALT,
				'c' | ZXK_ALT));

	setTitle("Rooms");
	setCellType(0);
}

ManicRoomPage::~ManicRoomPage()
{
}



void ManicRoomPage::drawFixed(void)
{
	int x,y,h;

	videoScreen->bottomBar("Room editor");

	videoScreen->fillBox(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				19 * CHAR_W, 8 * CHAR_H, ZX_BRWHITE);
        videoScreen->box(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
                                19 * CHAR_W, 8 * CHAR_H, ZX_BLACK);
        videoScreen->box(VIDEO_W - 19 * CHAR_SW, 256 + CHAR_H + CHAR_SH,
                                19 * CHAR_SW, 8 * CHAR_H, ZX_BLACK);

	videoScreen->drawSmallText(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H,
			      "Cells                                   ", 
				ZX_BRWHITE, ZX_BLACK);
	videoScreen->smallZxLogo(VIDEO_W - 8 * CHAR_SH, 256 + CHAR_H);

	x = VIDEO_W - 19 * CHAR_W + CHAR_SH;
	y = 256 + 2 * CHAR_H;
	h = CHAR_H;
	videoScreen->drawText(x,y,    "a:Air", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+h,  "w:Water", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + 19*CHAR_SW - 4 , y+h,"W:Water 2", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+2*h,"r:cRumbly", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+3*h,"e:Earth", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+4*h,"c:Convey", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + 19 * CHAR_SW - 4,y+4*h,"C:Conv 2", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+5*h,"f:Fire 1", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + 19*CHAR_SW - 4, y+5*h,"F:Fire 2", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+6*h,"I:Item", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + 19*CHAR_SW - 4,y+6*h,"?:Other", ZX_BLACK, ZX_BRWHITE);

/*        videoScreen->drawSmallText(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H,
                              "Guards          ", 
				ZX_BRWHITE, ZX_BLACK);
        videoScreen->smallZxLogo(VIDEO_W - 11 * CHAR_H - 6 * CHAR_SH, 
				256 + CHAR_H);
	videoScreen->fillBox(VIDEO_W - 13 * CHAR_W, 256 + CHAR_H + CHAR_SH, 1, 
				9 * CHAR_H, ZX_BLACK);
*/
	x = CHAR_W * 8;
	y = 256 + CHAR_H;
	h = CHAR_H;

	videoScreen->fillBox(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 6 * CHAR_H,
			ZX_BRWHITE);
        videoScreen->box(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 6 * CHAR_H,
                        ZX_BLACK);

	videoScreen->drawSmallText(x - CHAR_SW,y,
			"Other options            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->drawText(x,y + h, "B:clipBoard", ZX_BLACK, ZX_BRWHITE);
//	videoScreen->drawText(x,y+4*h, "G:Guardians", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+2*h, "O:Options  ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+3*h, "S:Shapes   ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+4*h, "AltI:Items ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+5*h, "AltC:Convey", ZX_BLACK, ZX_BRWHITE);
	videoScreen->smallZxLogo(x + 18 * CHAR_SW, y);

	x = 0;
	y = 256 + 9 * CHAR_H;
	videoScreen->drawSmallText(x, y, "Keyboard: ^Z or ^U to undo;", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, " J,<,> change room; ", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "cursor keys and SPACE to draw", ZX_BLACK, ZX_WHITE);

	ManicRoomNav::drawFixed();
}


void ManicRoomPage::setCellType(int ct)
{
	jswByte *data = m_game->getRoom(m_room);

	m_ctype = ct;
	if (ct < 8)
	{
		m_attr = data[544 + 9 * m_ctype];
	}
	else if (ct == 10)
	{
		m_attr = data[580];
	}
	drawCellType();
}

static void xy(int ctype, int *x, int *y, int *w)
{
	*x = VIDEO_W - 19 * CHAR_W;
	*y = 256 + 2 * CHAR_H;
	*w = 19 * CHAR_SW;

	switch(ctype)
	{
		case 0: return;
		case 1: *y +=   CHAR_H; return;
		case 2: *y += 2*CHAR_H; return;
		case 3: *y += 3*CHAR_H; return;
		case 4: *y += 4*CHAR_H; return;
		case 5: *y += 5*CHAR_H; return;
		case 6: *y += 5*CHAR_H; *x += 19 * CHAR_SW; return;
		case 7: *y +=   CHAR_H; *x += 19 * CHAR_SW; return;
		case 8: *y += 6*CHAR_H; return;
		case 9: *y += 6*CHAR_H; *x += 19 * CHAR_SW; return;
		case 10:*y += 4*CHAR_H; *x += 19 * CHAR_SW; return;
	}
}

void ManicRoomPage::drawCellType()
{
	int nt;

	if ((m_octype == m_ctype) || !m_visible || !m_drawnFixed) return;

	int x, y, ox, oy, w, ow;

	xy(m_ctype,  &x, &y, &w);	
	xy(m_octype, &ox, &oy, &ow);	

	if (m_octype >= 0) videoScreen->fillReplace (ox, oy, ow, CHAR_H,
                                        ZX_BRCYAN,
                                        ZX_BRWHITE);
	videoScreen->fillReplace(x, y, w, CHAR_H, ZX_BRWHITE,
                                        ZX_BRCYAN);
	m_octype = m_ctype;
}


void ManicRoomPage::flashColours(void)
{
	int n;
	int objcount = 5;
	jswByte *data = m_game->getRoom(m_room);

	if (!m_visible) return;
	for (n = 0; n < 5; n++)
	{
		if (data[n*5+629] == 0xFF) break;	
		if (data[n*5+629] != 0) --objcount;	
	}

	if (m_oft != objcount && m_drawnFixed)
	{
		char buf[20];

		sprintf(buf, "%03d item%s", objcount, 
				(objcount == 1) ? " free " : "s free" );
		videoScreen->drawSmallText(VIDEO_W - 8 * CHAR_W, 
					   VIDEO_H - 4 * CHAR_H, buf, 
					   ZX_BLACK, ZX_WHITE);

		m_oft = objcount;
	}
        ManicRoomNav::flashColours();

	/* Draw guardian thumbnails 
	for (int n = 0; n < 8; n++)
	{
        	if (m_guards[n]) m_guards[n]->drawThumb(videoScreen,
                                                        VIDEO_W - 12 * CHAR_W
                                                        - CHAR_SW,
                                                        256 + (2 +n)*CHAR_H);

                else videoScreen->fillBox(VIDEO_W - 12 * CHAR_W - CHAR_SW,
                                                     256 + (2+n) * CHAR_H,
                                                     CHAR_W, CHAR_H * (8-n),
                                                     ZX_BRWHITE);
	}
	*/
	/* Draw animation frames 
        if (m_ct >= 10 && m_ct < 20 && m_guards[m_ct % 10])
        {
                m_guards[m_ct % 10]->drawFrames(videoScreen, 80, VIDEO_H - 32);
        }
        else    videoScreen->fillBox(80, VIDEO_H - 32, 480, 32, ZX_WHITE);
	*/
}



void ManicRoomPage::drawXCursor(VideoSurface *s, int x, int y, int i)
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

void ManicRoomPage::drawConveyorCursor(VideoSurface *s, int i)
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



void ManicRoomPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	if (m_ocx != -1)
	{
		if (m_ctype == 4) drawConveyorCursor(s, i);
	}
	else 
	{
		if (m_ctype >= 11)
		{
	/*		if (m_guards[m_ct % 10])
			{
				JswGuardian *g = m_guards[m_ct % 10];

				if (g->isArrow()) g->drawBounds(s,i,m_cx, m_cy, m_acyminor);	
				else
				{
					if (m_cx > 30) m_cx = 30;
					if (m_cy > 14) m_cy = 14;
			                g->drawBounds(s,i,m_cx,m_cy);
				} 
			}
			else	// Non-existent guardian
			{	
				if (m_cx > 30) m_cx = 30;
                                if (m_cy > 14) m_cy = 14;              
				s->box(m_cx * 16, m_cy * 16, 32, 32, i);
			} */
		}		
		else s->box(m_cx * 16, m_cy * 16, 16, 16, i);
	}
}




void ManicRoomPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];

       		sprintf(s, "Room editor [%d]", m_room);
	        videoScreen->bottomBar(s);
	}
	ManicRoomNav::drawData(flashonly);
}







void ManicRoomPage::putCell()
{
	jswByte *room = getRoom(m_room);
	jswByte *cell = room + m_cy * 32 + m_cx;
	int it, n;

/*	if (m_ct >= 11)
	{
		JswGuardian *g = m_guards[m_ct % 10];
		
		if (g) 
		{
			if (g->isArrow())
				room[0xF1 + 2*(m_ct % 10)] = g->setPos(m_acyminor, m_cy);
			else    room[0xF1 + 2*(m_ct % 10)] = g->setPos(m_cx, m_cy);
		}
		return;	
	}
*/
	switch(m_ctype)
	{
		case 8: // Item
			it = itemAt(m_cx, m_cy);
			if (it == -1)	itemAdd(m_cx, m_cy);
			else		itemDel(it);
			break;

		case 4: // Conveyor
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
				cpos = ((cy << 5) | cx) + 0x7000;

				room[623] = (m_cdir < 0) ? 0 : 1;
				room[624] = ((cx & 0x1F) | (cy << 5) & 0xE0);
				room[625] = 0x70 | (cy & 8);
				room[626] = clen;
// Draw a conveyor 'under' the animation 
				for (n = 0; n < clen; n++)
				{
					room[cy * 32 + cx + n] = m_attr;
				}
				m_ocx = -1;
				m_ocy = -1;
				m_cdir = 0;
				drawBasic(0);
			}
			break;
		default:
		*cell = m_attr;
		getRoom()->drawCell(m_bmRoom, m_cx, m_cy, false, false);
//		drawCell(room, m_cx, m_cy, 0);
		break;
	}
}







int ManicRoomPage::roomProperties(void)
{
        jswByte cimage[1024];
	int rv;
	
	memcpy(cimage, getRoom(m_room), 1024);

	ManicRoomForm rf(m_room, m_game, cimage);
	rv = rf.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_OK) 
	{
		memcpy(getRoom(m_room), cimage, 1024);
		drawData();
	} 
	return ZXE_CONTINUE;
}


#define IFDEL(x) if (x) delete x; x = NULL;



int ManicRoomPage::roomCells(void)
{
        jswByte cimage[1024], *data;
        int rv;

        memcpy(cimage, data = getRoom(m_room), 1024);

	CellForm cf(cimage + 544, 8,
			"Air",
			"Water",
			"Crumbly",
			"Earth",
			"Conveyor",
			"Fire 1",
			"Fire 2",
			"Water 2");

        rv = cf.doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_OK)
        {
		int n, m;
		for (n = 0; n < 512; n++)
		{
			for (m = 544; m <= 608; m += 9)
			{
/* Regenerate cell attributes. If the old map used cell type m, set the
 * new map to have cell type m. */
				if (data[n] == data[m]) 
				{
					cimage[n] = cimage[m];
					break;
				}
			}
		}
		getRoom()->createCellPatterns();
	        IFDEL(m_bmItem)

                memcpy(data, cimage, 1024);
                drawData();
        } 
	return ZXE_CONTINUE;
}


int ManicRoomPage::conveyorMenu(void)
{
        jswByte *room = getRoom(m_room);
	VideoMenu vmc("Conveyor", "Off", "Sticky", 
			"Animation position 1",
			"Animation position 2",
			"Animation position 3",
			"Animation position 4",
			"Animation position 5", 
			"Animation position 6", NULL);
	int rv = vmc.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	switch(vmc.getSelected())
	{
		case 0:	room[623] = 2; return ZXE_CONTINUE;
		case 1: room[623] = 3; return ZXE_CONTINUE;
		case 2: room[625] = (room[625] & 0xF8);     return ZXE_CONTINUE;
		case 3: room[625] = (room[625] & 0xF8) | 1; return ZXE_CONTINUE;
		case 4: room[625] = (room[625] & 0xF8) | 2; return ZXE_CONTINUE;
		case 5: room[625] = (room[625] & 0xF8) | 3; return ZXE_CONTINUE;
		case 6: room[625] = (room[625] & 0xF8) | 4; return ZXE_CONTINUE;
		case 7: room[625] = (room[625] & 0xF8) | 5; return ZXE_CONTINUE;
	}
	return rv;
}



int ManicRoomPage::preContextMenu(void)
{
	intPreContextMenu();
	VideoMenu vm("Options", "Editor", "Conveyor",  
                     "Room options", "Clipboard etc.", 
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
		case 3: rv = roomClipboard();
			postContextMenu();
			return rv;
		case 4: postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return ManicRoomNav::preContextMenu();
}







void ManicRoomPage::undo(void)
{
	jswByte buf[256];
	jswByte *b = m_game->getRoom(m_room);

	memcpy(buf, b, 256);
        memcpy(b, undoBuffer, 256);
	memcpy(undoBuffer, buf, 256);

	if (m_visible) drawData();
}


void ManicRoomPage::loadRoom(int room, int reload)
{
        jswByte *b = m_game->getRoom(room);

	if (!reload) memcpy(undoBuffer, b, 256);

	ManicRoomNav::loadRoom(room, reload);
}



int ManicRoomPage::onHotspot(int id)
{
	jswByte *room = getRoom(m_room);
	int rv;

/*	if ((id >= ('1' | ZXK_ALT)) && (id <= ('8' | ZXK_ALT)))
	{
		int n = (id - ('1')) & ~ZXK_ALT;
		if (m_guards[n])
		{
			room[0xF1 + 2 * n] &= 0x1F;
			room[0xF1 + 2 * n] |= m_guards[n]->bumpFrame();
			return ZXE_CONTINUE;	
		}
		return ZXE_CONTINUE;
	}
	if (id >= '1' && id <= '8')
	{
		if (m_guards[(id-'1')]) setCellType(id - '1' + 10);
		return ZXE_CONTINUE;
	} */
	switch(id)
	{
                case 'a': setCellType(0); return ZXE_CONTINUE;
                case 'w': setCellType(1); return ZXE_CONTINUE;
                case 'r': setCellType(2); return ZXE_CONTINUE;
                case 'e': setCellType(3); return ZXE_CONTINUE;
                case 'c': setCellType(4); return ZXE_CONTINUE;
                case 'f': setCellType(5); return ZXE_CONTINUE;
                case 'F': setCellType(6); return ZXE_CONTINUE;
                case 'W': setCellType(7); return ZXE_CONTINUE;
                case 'i': setCellType(8); return ZXE_CONTINUE;
                case 'C': setCellType(10); return ZXE_CONTINUE;
                case '?': 
			intPreContextMenu();
			{
				CellAttrForm cf(&m_attr);
				rv = cf.doModal();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_OK)
				{
					postContextMenu();
					setCellType(9); 
				}
				else	postContextMenu();
			}
			return ZXE_CONTINUE;

		case 'B': // Clipboard
			intPreContextMenu();
			rv = roomClipboard();
			postContextMenu();
			return rv;	
		case 'O': // Options
			intPreContextMenu();
			rv = roomProperties();
			postContextMenu();
			return rv;
		case 'S': // Shapes
			intPreContextMenu();
			rv = roomCells();
			postContextMenu();
			return rv;
		case ('C' | ZXK_ALT): // Conveyor extra
			intPreContextMenu();
			rv = conveyorMenu();
			postContextMenu();
			return rv;
		case ('I' | ZXK_ALT): // Items extra
			{
				jswByte roomCopy[1024];
				intPreContextMenu();
				memcpy(roomCopy, room, 1024);
				ManicItemForm mif(roomCopy + 629);
				rv = mif.doModal();
	
				if (rv == ZXE_OK)
				{
					memcpy(room, roomCopy, 1024);	
	        			IFDEL(m_bmItem)
                			drawData();
				}
				if (rv < ZXE_QUIT) rv = ZXE_CONTINUE;
				postContextMenu();
			}
			return rv;
	}
	return ManicRoomNav::onHotspot(id);
}


int ManicRoomPage::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_BREAK:
			m_ocx = m_ocy = -1;
			m_cdir  = -1;
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
			 putCell(); return ZXE_CONTINUE;
	}
	return ManicRoomNav::onKeyDown(keysym);
}

int ManicRoomPage::onButtonDown(int x, int y, int button)
{
	if (y < 256)
	{
		m_mb = 1;
		return onMouseMove(x, y, 0, 0);
	}
	return ManicRoomNav::onButtonDown(x, y, button);
}

int ManicRoomPage::onButtonUp(int x, int y, int button)
{
	m_mb = 0;
	return ManicRoomNav::onButtonUp(x, y, button);
}

int ManicRoomPage::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + 512) &&
	    y >= m_y && y < (m_y + 256))
	{
		m_cx = (x - m_x)/16;
		m_cy = (y - m_y)/16;
		m_acyminor = ((y - m_y) % 16) / 2;
		if (m_mb) putCell();
	}
	return ZXE_CONTINUE;
}



static jswByte flip4(jswByte b)
{
	return ((b &  3) << 6) | ((b &  12) << 2) | 
	       ((b & 48) >> 2) | ((b & 192) >> 6);

}


int ManicRoomPage::roomClipboard(void)
{
	jswByte *room = getRoom(m_room);
	jswByte fill;
	int y,x;

	VideoMenu vm("Clipboard etc.",
		     "Cut", "Copy", "Paste", 
		     "Clear room",
		     "Flip vertically", 
		     "Flip horizontally", NULL);
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: memcpy(gl_clipBuffer, room, 1024);
			clearRoom();	// Cut
			break;
		case 1: memcpy(gl_clipBuffer, room, 1024); // Copy
			break;
		case 2: memcpy(room, gl_clipBuffer, 1024); // Paste
			if (m_visible) drawData();
			break;
		case 3:	clearRoom();	// Clear to air
			if (m_visible) drawData();
			break;

		case 4: mirrorVert();
			if (m_visible)
			{
				loadRoom(m_room);
				drawData();
			}
			break;

		case 5: mirrorHoriz();
			if (m_visible) 
			{
				loadRoom(m_room);
				drawData();
			}
			break;
	}
	return ZXE_CONTINUE;
}


int ManicRoomPage::itemAt(int x, int y)
{
	int n;
	jswByte *data = getRoom(m_room);
	
	for (n = 0; n < 5; n++)
	{
		int ix = data[n*5+630] & 0x1F;
		int iy = (data[n*5+632] & 8) | (data[n * 5 + 630] >> 5);
		if (data[n*5+629] == 0) continue;
		if (ix != x || iy != y) continue;
		if (data[n*5+629] == 0xFF) break;
		return n;	
	}
	return -1;
}


void ManicRoomPage::itemAdd(int x, int y)
{
	int n, m;
	jswByte *data = getRoom(m_room);

	for (n = 0; n < 5; n++)
	{
		if (data[n*5+629] == 0) break;
		if (data[n*5+629] == 0xFF) break;
	}
	if (n >= 5) return;

	// If this object was the last, move the 'last object' up one.	
	if (data[n*5+629] == 0xFF)
	{
		data[n*5+634] = 0xFF;
	}

	// Item attributes are autogenerated; use Alt+I to edit them manually.
	data[n*5+629] = data[544] & 0xF8 | (3 + (n%4));
	data[n*5+630] = (x & 0x1F) | ((y << 5) & 0xE0);
	data[n*5+631] = ((y & 8) >> 3) | 0x5C;
	data[n*5+632] =  (y & 8)       | 0x60;
	data[n*5+633] = 0xFF;
}


void ManicRoomPage::itemDel(int n)
{
	int m;
	jswByte *data = getRoom(m_room);

	for (m = n; m < 5; m++)
	{
		if (data[m*5+629] != 0 && data[m*5+629] != 0xFF) break;
	}
	// If there are no items after the one being deleted, insert 
	// end-of-list. Otherwise just zero out the attribute byte
	if (m >= 5) data[n*5 + 629] = 0xFF;
	else	    data[n*5 + 629] = 0;
}




void ManicRoomPage::clearRoom()
{
	getRoom()->clear();
}


void flip8(jswByte *data)
{
	int x, y, n;

	for (y = 0; y < 8; y++)
	{
		jswByte mask1 = 0x80, mask2 = 0x01;
		x = 0;
		for (n = 0; n < 8; n++)
		{
			if (data[y] & mask1) x |= mask2;;
			mask1 = mask1 >> 1;	
			mask2 = mask2 << 1;	
		}
		data[y] = x;
	}	
}


/* Flip a Manic Miner room horizontally. 
 *
 * This started just with swapping the cells over, but I got carried away... */
void ManicRoomPage::mirrorHoriz()
{
	getRoom()->mirrorH();
        IFDEL(m_bmItem)
}



void ManicRoomPage::mirrorVert()
{
	getRoom()->mirrorV();
        IFDEL(m_bmItem)
}

