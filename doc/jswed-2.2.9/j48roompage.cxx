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
#include "j48roompage.hxx"
#include "j128roomform.hxx"
#include "j48udgsform.hxx"
#include "j48clrsform.hxx"
#include "room.hxx"

static jswByte gl_clipBuffer[256];


Jsw48RoomPage::Jsw48RoomPage(JswGame *game) : Jsw48RoomNav(game)
{
	m_ot   = NULL;
	m_cx = m_cy = m_acyminor = 0;
	m_oct = -1;
	m_oft = -1;
	m_mb = 0;
	m_ocx = m_ocy = -1;
	m_cdir = 0;

	// Active areas
	int x, y, w, h;
	x = VIDEO_W - 10 * CHAR_W;
	y = 256 + 2*CHAR_H;
	w = 10 * CHAR_W;
	h = CHAR_H;

	addHotspot(new Hotspot('A', x, y, w, h, 'A', 'a')); y += CHAR_H;
	addHotspot(new Hotspot('W', x, y, w, h, 'W', 'w')); y += CHAR_H;
	addHotspot(new Hotspot('E', x, y, w, h, 'E', 'e')); y += CHAR_H;
	addHotspot(new Hotspot('F', x, y, w, h, 'F', 'f')); y += CHAR_H;
        addHotspot(new Hotspot('R', x, y, w, h, 'R', 'r')); y += CHAR_H;
        addHotspot(new Hotspot('C', x, y, w, h, 'C', 'c')); y += CHAR_H;
        addHotspot(new Hotspot('I', x, y, w, h, 'I', 'i')); y += CHAR_H;

	w = 6 * CHAR_W;
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
	
        x = CHAR_W * 8;
        y = 256 + (2 * CHAR_H);
	h = CHAR_H;
	w = 11 * CHAR_W;

	addHotspot(new Hotspot('X', x, y,         w, h, 'X', 'x'));
	addHotspot(new Hotspot('B', x, y +     h, w, h, 'B', 'b'));
	addHotspot(new Hotspot('O', x, y + 2 * h, w, h, 'O', 'o'));
	addHotspot(new Hotspot('G', x, y + 3 * h, w, h, 'G', 'g'));
	addHotspot(new Hotspot('C' | ZXK_ALT, x, y+4*h, w, h, 'C' | ZXK_ALT,
				'c' | ZXK_ALT));

	setTitle("Rooms");
	setCellType(0);
}

Jsw48RoomPage::~Jsw48RoomPage()
{
}



void Jsw48RoomPage::drawFixed(void)
{
	int x,y,h;

	videoScreen->bottomBar("Room editor");

	videoScreen->fillBox(VIDEO_W - 10 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				10 * CHAR_W, 8 * CHAR_H, ZX_BRWHITE);
        videoScreen->box(VIDEO_W - 10 * CHAR_W, 256 + CHAR_H + CHAR_SH,
                                10 * CHAR_W, 8 * CHAR_H, ZX_BLACK);

	videoScreen->fillBox(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				8 * CHAR_W, 9 * CHAR_H, ZX_BRWHITE);
	videoScreen->box(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				8 * CHAR_W, 9 * CHAR_H, ZX_BLACK);

	videoScreen->drawSmallText(VIDEO_W - 10 * CHAR_W, 256 + CHAR_H,
			      "Cells                   ", 
				ZX_BRWHITE, ZX_BLACK);
	videoScreen->smallZxLogo(VIDEO_W - 6 * CHAR_SH, 256 + CHAR_H);

	x = VIDEO_W - 10 * CHAR_W + CHAR_SH;
	y = 256 + 2 * CHAR_H;
	h = CHAR_H;
	videoScreen->drawText(x,y,  "A:Air", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+h,"W:Water", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+2*h,"E:Earth", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+3*h,"F:Fire", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+4*h,"R:Ramp", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+5*h,"C:Convey", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x,y+6*h,"I:Item", ZX_BLACK, ZX_BRWHITE);

        videoScreen->drawSmallText(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H,
                              "Guards          ", 
				ZX_BRWHITE, ZX_BLACK);
        videoScreen->smallZxLogo(VIDEO_W - 11 * CHAR_H - 6 * CHAR_SH, 
				256 + CHAR_H);
	videoScreen->fillBox(VIDEO_W - 13 * CHAR_W, 256 + CHAR_H + CHAR_SH, 1, 
				9 * CHAR_H, ZX_BLACK);

	x = CHAR_W * 8;
	y = 256 + CHAR_H;
	h = CHAR_H;

	videoScreen->fillBox(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 7 * CHAR_H,
			ZX_BRWHITE);
        videoScreen->box(x - CHAR_SW, y + CHAR_SH, 25 * CHAR_SW, 7 * CHAR_H,
                        ZX_BLACK);

	videoScreen->drawSmallText(x - CHAR_SW,y,
			"Other options            ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->drawText(x,y + h, "X:set eXits", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+2*h, "B:clipBoard", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+3*h, "O:Options  ", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+4*h, "G:Guardians", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawText(x,y+5*h, "AltC:Convey", ZX_BLACK, ZX_BRWHITE);
	videoScreen->smallZxLogo(x + 18 * CHAR_SW, y);

	x = 0;
	y = 256 + 9 * CHAR_H;
	videoScreen->drawSmallText(x, y, "Keyboard: ^Z or ^U to undo; CTRL+cursors", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "or J,<,> change room; ALT+1-8 set guardian", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "start frame; cursor keys and SPACE to draw", ZX_BLACK, ZX_WHITE);

	Jsw48RoomNav::drawFixed();
}


void Jsw48RoomPage::setCellType(int ct)
{
	m_ct = ct;
	drawCellType();
}

void Jsw48RoomPage::drawCellType()
{
	if ((m_oct == m_ct) || !m_visible || !m_drawnFixed) return;

	int w  = 10 * CHAR_W;
	int x  = VIDEO_W - w;
	int y  = 256 + 2 * CHAR_H;
	int ox = x;
	int ow = w;
	int oy = y;

	y  += (m_ct  % 10) * CHAR_H;
	oy += (m_oct % 10) * CHAR_H;

	if (m_ct  > 6) {  x -= 9 * CHAR_W;  w = 8 * CHAR_W; }
	if (m_oct > 6) { ox -= 9 * CHAR_W; ow = 8 * CHAR_W; } 

	if (m_oct >= 0) videoScreen->fillReplace (ox, oy, ow, CHAR_H,
                                        ZX_BRCYAN,
                                        ZX_BRWHITE);
	videoScreen->fillReplace(x, y, w, CHAR_H, ZX_BRWHITE,
                                        ZX_BRCYAN);
	m_oct = m_ct;
}


void Jsw48RoomPage::flashColours(void)
{
	int objcount = m_game->getItemCount();

	if (!m_visible) return;


	if (m_oft != objcount && m_drawnFixed)
	{
		char buf[20];

		sprintf(buf, "%03d item%s", objcount, 
				(objcount == 1) ? " free " : "s free" );
		videoScreen->drawSmallText(VIDEO_W - 10 * CHAR_W, 
					   VIDEO_H - 4 * CHAR_H, buf, 
					   ZX_BLACK, ZX_WHITE);

		m_oft = objcount;
	}
        Jsw48RoomNav::flashColours();

	/* Draw guardian thumbnails */
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

	/* Draw animation frames */
        if (m_ct >= 10 && m_ct < 20 && m_guards[m_ct % 10])
        {
                m_guards[m_ct % 10]->drawFrames(videoScreen, 80, VIDEO_H - 32);
        }
        else    videoScreen->fillBox(80, VIDEO_H - 32, 480, 32, ZX_WHITE);

}



void Jsw48RoomPage::drawXCursor(VideoSurface *s, int x, int y, int i)
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

void Jsw48RoomPage::drawRampCursor(VideoSurface *s, int i)
{
        int x, y, p, q, l;

        if      (m_cx < m_ocx) m_cdir = -1;
        else if (m_cx > m_ocx) m_cdir = 1;

        /* Note that if m-cx != m_ocx, we don't touch m_cdir */

	l = abs(m_cx - m_ocx) + 1;
        x = m_ocx;
	y = m_ocy;
        for (q = 0; q < l; q++)
        {
		s->box(x * 16, y * 16, 16, 16, i);
                if (m_cdir)
		{
			for (p = 0; p < 16; p++)
			{
                            if (m_cdir < 0)
                                 s->plot(x * 16 + p, y * 16 + p, i);
                            else s->plot(x * 16 + p, y * 16 + (16-p), i);
                        }
                }
                else drawXCursor(s, x * 16, m_ocy * 16, i);
		x += m_cdir; 
      		y--; 	
		if (y < 0) 
		{
			m_cx = x;
			break;
		}
	}

}


void Jsw48RoomPage::drawConveyorCursor(VideoSurface *s, int i)
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



void Jsw48RoomPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	if (m_ocx != -1)
	{
		if (m_ct == 5) drawConveyorCursor(s, i);
		else if (m_ct == 4) drawRampCursor(s, i);
	}
	else 
	{
		if (m_ct >= 10)
		{
			if (m_guards[m_ct % 10])
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
			}
		}		
		else s->box(m_cx * 16, m_cy * 16, 16, 16, i);
	}
}




void Jsw48RoomPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];

       		sprintf(s, "Room editor [%d]", m_room);
	        videoScreen->bottomBar(s);
	}
	Jsw48RoomNav::drawData(flashonly);
}



void Jsw48RoomPage::drawGuardList(void)
{
        jswByte *room = getRoom(m_room);
	int x, y, n, p, stop;
	char buf[8];

	y = 256 + 2 * CHAR_H;
	x = VIDEO_W - 18 * CHAR_W;
	
	stop = 0;
	for (n = 0; n < 8; n++)
	{
		jswByte mask = m_game->getGuardianMask();
		if (stop || (room[0xF0 + 2*n] & mask) == mask)
		{
			stop = 1;
			strcpy(buf, "     ");
		}
		else sprintf(buf, "%d:G%02x", n + 1, (room[0xF0 + 2*n] & mask));

		if (n + 10 == m_ct)	p = ZX_BRCYAN;
		else			p = ZX_BRWHITE;

		videoScreen->drawText(x, y, buf, ZX_BLACK, p);
		y += CHAR_H;
	}
}





void Jsw48RoomPage::putCell(int shifted)
{
	int dx, andmask, ormask, ob;
	jswByte *room = getRoom(m_room);
	jswByte *cell = room + (m_cy * 8 + (m_cx / 4));

	if (m_ct >= 10)
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

	switch(m_ct)
	{
		case 6: // Item
			ob = m_game->objectAt(m_room, m_cx, m_cy * 8);

			if (ob == -1 || shifted) 
				m_game->objectInsert(m_room,m_cx,m_cy*8);
			else	m_game->objectDelete(ob);
			break;

		case 5: if (m_ocx == -1)	/* Start conveyor */
			{
				m_ocx = m_cx;
				m_ocy = m_cy;
				m_cdir = 0;
			}
			else			/* End conveyor */
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

                case 4: if (m_ocx == -1)        /* Start ramp */
                        {
                                m_ocx = m_cx;
                                m_ocy = m_cy;
                                m_cdir = 0;
                        }
                        else                    /* End ramp */
                        {

                                int rlen = abs(m_cx - m_ocx) + 1;
                                if (!m_cdir) rlen = 0;
                                int cx, cy, cpos;

                                cx = m_ocx;
                                cy = m_ocy;
                                cpos = ((cy << 5) | cx) + 0x5E00;

                                room[0xDA] = (m_cdir < 0) ? 0 : 1;
                                room[0xDB] = (cpos & 0xFF);
                                room[0xDC] = (cpos >> 8);
                                room[0xDD] = rlen; 
                                m_ocx = -1; 
                                m_ocy = -1;
                                m_cdir = 0;
				drawBasic(0);
			}
			break;

		default:
		getRoom()->setCell(m_cx, m_cy, m_ct);
		getRoom()->drawCell(m_bmRoom, m_cx, m_cy, false, false);
		break;
	}
}






int Jsw48RoomPage::roomExits(void)
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
	sprintf(str[0], "Left  [%03d %-24.24s]", room[0xE9], rl+128);
	sprintf(str[1], "Right [%03d %-24.24s]", room[0xEA], rr+128);
	sprintf(str[2], "Up    [%03d %-24.24s]", room[0xEB], ru+128);
	sprintf(str[3], "Down  [%03d %-24.24s]", room[0xEC], rd+128);

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



int Jsw48RoomPage::roomProperties(void)
{
	VideoMenu vm("Room properties", "General", "Shapes", 
			"Colours", NULL);
        jswByte cimage[256];
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0:
		{
			memcpy(cimage, getRoom(m_room), 256);

			Jsw128RoomForm rf(m_room, m_game, cimage);
			rv = rf.doModal();
			if (rv >= ZXE_QUIT) return rv;
			if (rv == ZXE_OK) 
			{
				memcpy(getRoom(m_room), cimage, 256);
				drawData();
			}
		}
		break;

		case 1: return roomUdgs();
		case 2: return roomClrs();

	}
	return ZXE_CONTINUE;
}


int Jsw48RoomPage::roomUdgs(void)
{
        jswByte cimage[256];
        int rv;

        memcpy(cimage, getRoom(m_room), 256);

        Jsw48UdgsForm rf(cimage);
        rv = rf.doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_OK)
        {
		getRoom()->createCellPatterns();

                memcpy(getRoom(m_room), cimage, 256);
                drawData();
        }
        return ZXE_CONTINUE;
}



int Jsw48RoomPage::roomClrs(void)
{
        jswByte cimage[256];
        int rv;

        memcpy(cimage, getRoom(m_room), 256);

        Jsw48ClrsForm rf(cimage);
        rv = rf.doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_OK)
        {
		getRoom()->createCellPatterns();

                memcpy(getRoom(m_room), cimage, 256);
                drawData();
        }
	return ZXE_CONTINUE;
}

int Jsw48RoomPage::guardianMenu(void)
{
        jswByte *room = getRoom(m_room);
        VideoMenu vmg1("Guardian", "Cancel", "Add", "Edit", "Delete", "Table", NULL);
	VideoMenu vmg2("Guardian", "Cancel", "Add", "Edit", "Delete", NULL);
        VideoMenu vmg3("Guardian", "Cancel", "Add", "Table", NULL);
        VideoMenu vmg4("Guardian", "Cancel", "Add", NULL);

	VideoMenu *vm;

	int x, rv, ng, gi, bg;
	jswByte *g;

	if (m_game->guardTableCount() > 1) vm = (m_ct < 10) ? &vmg3 : &vmg1;
	else				   vm = (m_ct < 10) ? &vmg4 : &vmg2;

	
	
	bg = (room[0xA0] >> 3) & 0x0F;

	rv = vm->doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	int ns = vm->getSelected();

	if (m_ct < 10 && ns > 1) ns += 2;

        switch(ns)
	{
		case 1: if (m_guards[7]) return ZXE_CONTINUE;
					/* All guardians taken */


			rv = m_game->chooseGuard(m_bmRoom, 
						m_game->getGuards(m_room), 
						&ng, bg);
                        if (rv >= ZXE_QUIT) return rv;
                        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

			g = m_game->getGuards(m_room) + 8 * ng;
			for (gi = 0; gi < 8; gi++) if (!m_guards[gi]) break;

	                switch(g[0] & 0x0F)
                	{
                       		case 1: case 5:  case 6:
                        	case 9: case 13: case 14: x = g[6]; break;

                        	case 4: x = 64; break;

                       		default: x = 15; break;
                	}
			room[0xF0 + 2 * gi] = ng;
			room[0xF1 + 2 * gi] = x;
			if (gi < 7) room[0xF2 + 2 * gi] = 0xFF;

			m_guards[gi] = m_game->newGuardian(g, x, bg);
    			drawData(); 
			return ZXE_CONTINUE;
		case 2: // Edit guardian
			if (m_ct < 10) return ZXE_CONTINUE;
			ng = m_ct % 10;
			gi = room[0xF0 + 2 * ng] & m_game->getGuardianMask();
			x  = room[0xF1 + 2 * ng];
			rv = m_game->guardianEditor(m_room, gi,
				m_game->getGuards(m_room), 
				x, m_bmRoom); 
			if (rv != ZXE_OK) return rv;
			delete m_guards[ng];
			g = m_game->getGuards(m_room) + 8 * gi;
			m_guards[ng] = m_game->newGuardian(g, x, bg);
			return ZXE_CONTINUE;

		case 3:	// Delete guardian
                        if (m_ct < 10) return ZXE_CONTINUE;
			ng = m_ct % 10;

			if (ng == 7)
			{
				room[0xFE] = room[0xFF] = 0xFF;
				delete m_guards[7]; m_guards[7] = NULL;
				drawGuardList();
				return ZXE_CONTINUE;
			}
			delete m_guards[ng];
			for (; ng < 7; ng++)
			{
				room[0xF0 + 2 * ng] = room[0xF2 + 2 * ng];
				room[0xF1 + 2 * ng] = room[0xF3 + 2 * ng];
				m_guards[ng] = m_guards[ng+1];
			}
			room[0xFE] = room[0xFF] = 0;
			m_guards[7] = NULL;
			drawGuardList();
			return ZXE_CONTINUE;	
		case 4: ng = room[0xE0] * 256 + room[0xDF];
			rv = m_game->chooseGuardTable(&ng);
			if (rv >= ZXE_QUIT) return rv;
			if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
			room[0xDF] = (ng & 0xFF);
			room[0xE0] = (ng >> 8);
		        loadRoom(m_room, 1);
			drawData();
			return ZXE_CONTINUE;
	}
	return ZXE_CONTINUE;
}

int Jsw48RoomPage::conveyorMenu(void)
{
        jswByte *room = getRoom(m_room);
	VideoMenu vmc("Conveyor", "Ramp Left", "Ramp Right", "Off", "Sticky", 
			NULL);
	int rv = vmc.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	switch(vmc.getSelected())
	{
		case 0: room[0xD6] = 0;
			room[0xD9] = 0;
			room[0xCD] = room[0xC4];
/*			delete m_bmConvey;
			m_bmConvey = bitmapFromSprite(room + 0xCD); */
			drawBasic(0);	
			return ZXE_CONTINUE;
		case 1: room[0xD6] = 1;
                        room[0xD9] = 0;
                        room[0xCD] = room[0xC4];
/*                        delete m_bmConvey;
                        m_bmConvey = bitmapFromSprite(room + 0xCD); */
			drawBasic(0);
                        return ZXE_CONTINUE;
		case 2:	room[0xD6] = 2;
			return ZXE_CONTINUE;
		case 3: room[0xD6] = 3;
			return ZXE_CONTINUE;
	}
	return rv;
}



int Jsw48RoomPage::preContextMenu(void)
{
	intPreContextMenu();
	VideoMenu vm("Options", "Editor", "Conveyor", "Guardian", 
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
		case 2: rv = guardianMenu();
			postContextMenu();
			return rv;
		case 3: rv = roomProperties();
			postContextMenu();
			return rv;
		case 4: rv = roomExits();
			postContextMenu();
			return rv;
		case 5: rv = roomClipboard();
			postContextMenu();
			return rv;
		case 6: postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return Jsw48RoomNav::preContextMenu();
}







void Jsw48RoomPage::undo(void)
{
	jswByte buf[256];
	jswByte *b = m_game->getRoom(m_room);

	memcpy(buf, b, 256);
        memcpy(b, undoBuffer, 256);
	memcpy(undoBuffer, buf, 256);

	if (m_visible) drawData();
}


void Jsw48RoomPage::loadRoom(int room, int reload)
{
        jswByte *b = m_game->getRoom(room);

	if (!reload) memcpy(undoBuffer, b, 256);

	Jsw48RoomNav::loadRoom(room, reload);
}



int Jsw48RoomPage::onHotspot(int id)
{
	jswByte *room = getRoom(m_room);
	int rv;

	if ((id >= ('1' | ZXK_ALT)) && (id <= ('8' | ZXK_ALT)))
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
	}
	switch(id)
	{
                case 'A':
                        setCellType(0); return ZXE_CONTINUE;
                case 'W':
                        setCellType(1); return ZXE_CONTINUE;
                case 'E':
                        setCellType(2); return ZXE_CONTINUE;
                case 'F':
                        setCellType(3); return ZXE_CONTINUE;
                case 'R':
                        setCellType(4); return ZXE_CONTINUE;
                case 'C':
                        setCellType(5); return ZXE_CONTINUE;
                case 'I':
                        setCellType(6); return ZXE_CONTINUE;

		case 'X': // Exits
			intPreContextMenu();
			rv = roomExits();
			postContextMenu();
			return rv;
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
		case 'G': // Guardians
			intPreContextMenu();
			rv = guardianMenu();
			postContextMenu();
			return rv;
		case ('C' | ZXK_ALT): // Conveyor extra
			intPreContextMenu();
			rv = conveyorMenu();
			postContextMenu();
			return rv;
	}
	return Jsw48RoomNav::onHotspot(id);
}


int Jsw48RoomPage::onKeyDown(int keysym)
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
			if (m_ct >= 10 && m_guards[m_ct % 10]->isArrow())
			{
				if (m_acyminor > 0) { m_acyminor--; return ZXE_CONTINUE; }
				if (m_cy > 0)       { m_acyminor = 7; --m_cy; return ZXE_CONTINUE; }
			}
			if (m_cy > 0) --m_cy;
			return ZXE_CONTINUE;
		case ZXK_DOWN:
                        if (m_ct >= 10 && m_guards[m_ct % 10]->isArrow())
                        {
				if (m_cy == 14 && m_acyminor >= 6) return ZXE_CONTINUE;
                                if (m_acyminor < 7) { m_acyminor++; return ZXE_CONTINUE; }
                                if (m_cy < 15)      { m_acyminor = 0; ++m_cy; return ZXE_CONTINUE; }
                        }
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

int Jsw48RoomPage::onButtonDown(int x, int y, int button)
{
	if (y < 256)
	{
		m_mb = 1;
		return onMouseMove(x, y, 0, 0);
	}
	return Jsw48RoomNav::onButtonDown(x, y, button);
}

int Jsw48RoomPage::onButtonUp(int x, int y, int button)
{
	m_mb = 0;
	return Jsw48RoomNav::onButtonUp(x, y, button);
}

int Jsw48RoomPage::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + 512) &&
	    y >= m_y && y < (m_y + 256))
	{
		m_cx = (x - m_x)/16;
		m_cy = (y - m_y)/16;
		m_acyminor = ((y - m_y) % 16) / 2;
		if (m_mb) putCell(shiftPressed());
	}
	return ZXE_CONTINUE;
}





int Jsw48RoomPage::roomClipboard(void)
{
	jswByte *room = getRoom(m_room);
	jswByte fill;
	int y,x;

	VideoMenu vm("Clipboard etc.",
		     "Cut", "Copy", "Paste", "Clear to Air",
		     "Clear to Earth", "Clear to Water", "Clear to Fire",
		     "Flip vertically", "Flip horizontally", NULL);
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: memcpy(gl_clipBuffer, room, 256);
			// FALL THROUGH to
		case 3:	getRoom()->clear();
			if (m_visible) drawData();
			break;
		case 1: memcpy(gl_clipBuffer, room, 256);
			break;
		case 2: memcpy(room, gl_clipBuffer, 256);
			if (m_visible) drawData();
			break;
		case 4: 
		case 5:
		case 6:
			fill = (vm.getSelected() - 3); // 1 2 or 3
			fill += (fill * 4) + (fill * 16) + (fill * 64);
			getRoom()->clear();
			memset(room, fill, 0x80);
                        if (m_visible) drawData();
                        break;

		case 7: getRoom()->mirrorV();
			if (m_visible) drawData();
			break;

		case 8: getRoom()->mirrorH();
			if (m_visible) drawData();
			break;
	}
	return ZXE_CONTINUE;
}



