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
#include "manicitemform.hxx"
#include "manicguardpage.hxx"
#include "manicguardedit.hxx"
#include "manicroomform.hxx"
#include "cellattrform.hxx"
#include "cellform.hxx"

static jswByte gl_clipBuffer[256];


ManicGuardPage::ManicGuardPage(ManicGame *game) : ManicRoomNav(game)
{
	m_ot   = NULL;
	m_cx = m_cy = m_cyminor = 0;
	m_octype = -1;
	m_oft = -1;
	m_mb = 0;
	m_ocx = m_ocy = -1;
	m_cdir = 0;
	m_attr = 0;

	// Active areas
	int x, y, w, h;
	h = CHAR_H;
	w = 16 * CHAR_W;
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
/*
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
/*
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
*/
	setTitle("Guards");
	setCellType(0);
}

ManicGuardPage::~ManicGuardPage()
{
}



void ManicGuardPage::drawFixed(void)
{
	int x,y,h;

	videoScreen->bottomBar("Guardian editor");

        videoScreen->fillBox(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
                                16 * CHAR_W, 9 * CHAR_H, ZX_BRWHITE);
        videoScreen->box(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H + CHAR_SH,
                                16 * CHAR_W, 9 * CHAR_H, ZX_BLACK);

        videoScreen->drawSmallText(VIDEO_W - 19 * CHAR_W, 256 + CHAR_H,
                              "Guards                      ", 
				ZX_BRWHITE, ZX_BLACK);
        videoScreen->smallZxLogo(VIDEO_W - 12 * CHAR_SW, 
				256 + CHAR_H);
	videoScreen->fillBox(VIDEO_W - 5 * CHAR_W, 256 + CHAR_H + CHAR_SH, 1, 
				9 * CHAR_H, ZX_BLACK);

/*
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
	videoScreen->drawSmallText(x, y, "Keyboard: ^Z or ^U to undo; CTRL+cursors", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "or J,<,> change room; ALT+1-8 set guardian", ZX_BLACK, ZX_WHITE);
	y += CHAR_SH;
	videoScreen->drawSmallText(x, y, "start frame; cursor keys and SPACE to draw", ZX_BLACK, ZX_WHITE);
*/
	ManicRoomNav::drawFixed();
}


void ManicGuardPage::setCellType(int ct)
{
	jswByte *data = m_game->getRoom(m_room);

	m_ctype = ct;
	if (ct < 4)
	{
		m_attr = data[702 + 7 * ct];
	}
	else 
	{
		m_attr = data[733 + 7 * ct];
	}
	drawCellType();
}


void ManicGuardPage::drawCellType()
{
	int nt;

	if ((m_octype == m_ctype) || !m_visible || !m_drawnFixed) return;

	int x = VIDEO_W - 19 * CHAR_W;
	int y = 256 + ((m_ctype + 2) * CHAR_H);
	int oy= 256 + ((m_octype + 2) * CHAR_H);
	int w = 18 * CHAR_W;

	if (m_octype >= 0) videoScreen->fillReplace (x, oy, w, CHAR_H,
                                        ZX_BRCYAN,
                                        ZX_BRWHITE);
	videoScreen->fillReplace(x, y, w, CHAR_H, ZX_BRWHITE,
                                        ZX_BRCYAN);
	m_octype = m_ctype;

}


void ManicGuardPage::flashColours(void)
{
	int n;
	int objcount = 5;
	jswByte *data = m_game->getRoom(m_room);

	if (!m_visible) return;
        ManicRoomNav::flashColours();

	/* Draw guardian thumbnails  */
	for (n = 0; n < 4; n++)
	{
        	if (m_hguards[n]) m_hguards[n]->drawThumb(videoScreen,
                                                        VIDEO_W - 4 * CHAR_W
                                                        - CHAR_SW,
                                                        256 + (2 +n)*CHAR_H);

                else videoScreen->fillBox(VIDEO_W - 4 * CHAR_W - CHAR_SW,
                                                     256 + (2+n) * CHAR_H,
                                                     CHAR_W, CHAR_H,
                                                     ZX_BRWHITE);
	}
	for (; n < 8; n++)
	{
        	if (m_vguards[n-4]) m_vguards[n-4]->drawThumb(videoScreen,
                                                        VIDEO_W - 4 * CHAR_W
                                                        - CHAR_SW,
                                                        256 + (2 +n)*CHAR_H);

                else videoScreen->fillBox(VIDEO_W - 4 * CHAR_W - CHAR_SW,
                                                     256 + (2+n) * CHAR_H,
                                                     CHAR_W, CHAR_H,
                                                     ZX_BRWHITE);
	}

	/* Draw animation frames 
        if (m_ct >= 10 && m_ct < 20 && m_guards[m_ct % 10])
        {
                m_guards[m_ct % 10]->drawFrames(videoScreen, 80, VIDEO_H - 32);
        }
        else    videoScreen->fillBox(80, VIDEO_H - 32, 480, 32, ZX_WHITE);
	*/
}




void ManicGuardPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;
	ManicGuardian *g = NULL;;


	if (m_ctype >= 4) g = m_vguards[m_ctype - 4];
	else		  g = m_hguards[m_ctype];

	if (g) 
	{
		if (m_cx > 30) m_cx = 30;
		if (m_cy > 14) m_cy = 14;
                if (m_ctype >= 4) g->drawBounds(s,i,m_cx,m_cy, m_cyminor);
		else		  g->drawBounds(s,i,m_cx,m_cy);
	}
	else	// Non-existent guardian
	{	
		if (m_cx > 30) m_cx = 30;
                if (m_cy > 14) m_cy = 14;              
		s->box(m_cx * 16, m_cy * 16, 32, 32, i);
	}
}




void ManicGuardPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];

       		sprintf(s, "Guardian editor [%d]", m_room);
	        videoScreen->bottomBar(s);
	}
	ManicRoomNav::drawData(flashonly);
}



void ManicGuardPage::drawGuardList(void)
{
        jswByte *room = getRoom(m_room);
	jswByte *guard;
	int x, y, n, p, stop;
	char buf[20];
	bool bVertis = m_game->roomHasFeature(m_room, FEAT_VGUARDS) ||
		       m_game->roomHasFeature(m_room, FEAT_SKYLABS);
	bool bSkylab = m_game->roomHasFeature(m_room, FEAT_SKYLABS);
	y = 256 + 2 * CHAR_H;
	x = VIDEO_W - 18 * CHAR_W;
	
	stop = 0;
	for (n = 0; n < 4; n++)
	{
		guard = room + 702 + 7 * n;
		if (guard[0] == 0 || guard[0] == 0xFF)
		{
			sprintf(buf, "%d: Blank     ", n + 1);
		}
		else 
		{
			sprintf(buf, "%d: Horizontal", n + 1);
		}
		if (n == m_ctype)	p = ZX_BRCYAN;
		else			p = ZX_BRWHITE;

		videoScreen->drawText(x, y, buf, ZX_BLACK, p);
		y += CHAR_H;
	} 
	for (n = 0; n < 4; n++)
	{
		guard = room + 733 + 7 * n;
		if (!bVertis)
		{
			sprintf(buf, "             ");
		}
		else if (guard[0] == 0 || guard[0] == 0xFF)
		{
			sprintf(buf, "%d: Blank     ", n + 5);
		}
		else if (!bSkylab)
		{
			sprintf(buf, "%d: Vertical  ", n + 5);
		}
		else
		{
			sprintf(buf, "%d: Skylab    ", n + 5);
		}
		if ((n+4) == m_ctype)	p = ZX_BRCYAN;
		else			p = ZX_BRWHITE;
		videoScreen->drawText(x, y, buf, ZX_BLACK, p);
		y += CHAR_H;
	}
}





void ManicGuardPage::putCell()
{
	jswByte *room = getRoom(m_room);
	int it, n;
	jswByte *gbody;
	ManicGuardian *g;

	if (m_ctype >= 4)
	{
		g = m_vguards[m_ctype - 4];
		gbody = room + 733 + 7 * (m_ctype - 4);
		gbody[2] = m_cy * 8 + (m_cyminor & 7);
		gbody[3] = m_cx & 0x1F;
	}
	else
	{
		g = m_hguards[m_ctype];
		gbody = room + 702 + 7 * (m_ctype);
		gbody[1] = (m_cx & 0x1F) | ((m_cy << 5) & 0xE0);
		gbody[2] = 0x5C | ((m_cy >> 3) & 1);
		gbody[3] = 0x60 | (m_cy & 8);
		gbody[5] = (gbody[5] & 0x1F) | ((m_cy << 5) & 0xE0);
		gbody[6] = (gbody[6] & 0x1F) | ((m_cy << 5) & 0xE0);
	}
	g->setX(m_cx);
	g->setY((m_cy * 8) + (m_cyminor & 7));
}








#define IFDEL(x) if (x) delete x; x = NULL;




int ManicGuardPage::guardianMenu(void)
{
        jswByte *room = getRoom(m_room);
	VideoMenu vmg1("Options", "Editor", "Add guardian", "Edit guardian", "Delete guardian", "Exit", NULL);
        VideoMenu vmg2("Options", "Editor", "Add guardian", "Exit", NULL);
        VideoMenu vmg3("Add guardian", "Horizontal", "Vertical", NULL);
        VideoMenu vmg4("Add guardian", "Horizontal", "Skylab", NULL);
	VideoMenu *vm, *vmf;
	ManicGuardian **guard;
	bool freeH, freeV, uni;
	int freetype;

	int x, rv, ng, gi, bg;
	jswByte *g, *data;

	data = m_game->getRoom(m_room);
	freeH = false;
	freeV = false;
	uni = m_kong || m_skylabs || m_hasvguards;

	// See if we can add extra H or V guardians.
	for (ng = 0; ng < 4; ng++)
	{
		if (m_hguards[ng] == NULL) freeH = true;
		if (m_vguards[ng] == NULL) freeV = true;
	}
	if (m_skylabs == false && m_hasvguards == false) freeV = false;

	if (m_ctype < 4) 
	{
		guard = (ManicGuardian **)&m_hguards[m_ctype];
		g     = data + 702 + 7 * m_ctype;
	}
	else
	{
		guard = (ManicGuardian **)&m_vguards[m_ctype - 4];
		g     = data + 733 + 7 * (m_ctype - 4);	
	}

	if (*guard) vm = &vmg1;
	else	    vm = &vmg2;
	
	bg = (room[544] >> 3) & 0x0F;

	rv = vm->doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	int ns = vm->getSelected();

	if (vm == &vmg2 && ns == 2) ns = 4; 

        switch(ns)
	{
		case 1: 
			if (freeH == false && freeV == false)
				return ZXE_CONTINUE;
			if (freeH && freeV)
			{
				if (m_skylabs) 	vmf = &vmg4;
				else 		vmf = &vmg3;
				rv = vmf->doModal();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
				freetype = vmf->getSelected();
			}
			else if (freeH) freetype = 0;
			else		freetype = 1;

			for (ng = 0; ng < 4; ng++)
			{
				if (freetype == 1 && m_vguards[ng] == NULL) 
				{
					guard = (ManicGuardian **)&m_vguards[ng];
					g     = data + 733 + 7 * ng;	
					g[0] = data[544] ^ 5;
					g[1] = 0;
					g[2] = 0x38;
					g[3] = 0x0F;
					g[4] = 1;
					g[5] = g[2] - 8;
					g[6] = g[2] + 8;
					break;
				}
				else if (freetype == 0 && m_hguards[ng] == NULL) 
				{
					guard = (ManicGuardian **)&m_hguards[ng];
					g     = data + 702 + 7 * ng;
					g[0] = data[544] ^ 6;
					g[1] = 0xEF;
					g[2] = 0x5C;
					g[3] = 0x60;
					g[4] = 0;
					g[5] = g[1] - 1;
					g[6] = g[1] + 1;
					break;
				}
			}
			if (ng >= 4) return ZXE_CONTINUE;
			{
				ManicGuardianEditor mge((freetype == 1),
							m_skylabs,
							!uni,
							m_game,
							data,
							g, 
							m_bmRoom);
				
				rv = mge.doModal();	
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_OK)
				{
					if (freetype == 0)
						*guard = new ManicHGuardian(!uni, data, g);
					else if (m_skylabs)
						*guard = new ManicSkylab(data, g);
					else	*guard = new ManicVGuardian(data, g);
				}
			}
			drawGuardList();
			return ZXE_CONTINUE;
		case 2: // Edit guardian
			{
				ManicGuardianEditor mge(m_ctype >= 4,
							m_skylabs,
							!uni,
							m_game,
							data,
							g, 
							m_bmRoom);
				
				rv = mge.doModal();	
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_OK)
				{
					delete (*guard);
					if (m_ctype < 4)
						*guard = new ManicHGuardian(!uni, data, g);
					else if (m_skylabs)
						*guard = new ManicSkylab(data, g);
					else	*guard = new ManicVGuardian(data, g);
				}
				return ZXE_CONTINUE;
			}
		case 3:	// Delete guardian
			if (m_ctype < 4)
			{
				delete m_hguards[m_ctype];
				m_hguards[m_ctype] = NULL;
				g[0] = 0;
			}
			else
			{
				delete m_vguards[m_ctype - 4];
				for (ng = m_ctype - 4; ng < 3; ng++)
				{
					memcpy(data + 736 + 7 * ng,
					       data + 743 + 7 * ng, 7);
					m_vguards[ng] = m_vguards[ng + 1];
				}
				m_vguards[ng] = NULL;	
				data[754] = 0xff;
			}
			drawGuardList();
			return ZXE_CONTINUE;
		case 4:	return ZXE_OK;	
	}
	return ZXE_CONTINUE;
}


int ManicGuardPage::preContextMenu(void)
{
	int rv;

	intPreContextMenu();
	rv = guardianMenu();
	postContextMenu();
	return rv;
}







void ManicGuardPage::undo(void)
{
	jswByte buf[256];
	jswByte *b = m_game->getRoom(m_room);

	memcpy(buf, b, 256);
        memcpy(b, undoBuffer, 256);
	memcpy(undoBuffer, buf, 256);

	if (m_visible) drawData();
}


void ManicGuardPage::loadRoom(int room, int reload)
{
        jswByte *b = m_game->getRoom(room);

	if (!reload) memcpy(undoBuffer, b, 256);

	ManicRoomNav::loadRoom(room, reload);

	if (m_hasvguards) 
	{
		b[761] = 0xFF;	// Enforce guardian end marker
	}
}



int ManicGuardPage::onHotspot(int id)
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
	*/
	if (id >= '1' && id <= '4')
	{
		if (m_hguards[(id-'1')]) setCellType(id - '1');
		return ZXE_CONTINUE;
	}
	if (id >= '5' && id <= '8')
	{
		if (m_vguards[(id-'5')]) setCellType(id - '1');
		return ZXE_CONTINUE;
	}
	return ManicRoomNav::onHotspot(id);
}


int ManicGuardPage::onKeyDown(int keysym)
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
			if (m_ctype >= 4)
			{
				if (m_cyminor > 0)  { m_cyminor--; return ZXE_CONTINUE; }
				if (m_cy > 0)       { m_cyminor = 7; --m_cy; return ZXE_CONTINUE; }
			}
			if (m_cy > 0) --m_cy; 
			return ZXE_CONTINUE;
		case ZXK_DOWN:
			if (m_ctype >= 4)
                        {
				if (m_cy == 14 && m_cyminor >= 6) return ZXE_CONTINUE;
                                if (m_cyminor < 7) { m_cyminor++; return ZXE_CONTINUE; }
                                if (m_cy < 15)     { m_cyminor = 0; ++m_cy; return ZXE_CONTINUE; }
                        }
			if (m_cy < 15) ++m_cy; 
			return ZXE_CONTINUE;
		case ZXK_ENTER:
		case ' ': 
			 putCell(); return ZXE_CONTINUE;
	}
	return ManicRoomNav::onKeyDown(keysym);
}

int ManicGuardPage::onButtonDown(int x, int y, int button)
{
	if (y < 256)
	{
		m_mb = 1;
		return onMouseMove(x, y, 0, 0);
	}
	return ManicRoomNav::onButtonDown(x, y, button);
}

int ManicGuardPage::onButtonUp(int x, int y, int button)
{
	m_mb = 0;
	return ManicRoomNav::onButtonUp(x, y, button);
}

int ManicGuardPage::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + 512) &&
	    y >= m_y && y < (m_y + 256))
	{
		m_cx = (x - m_x)/16;
		m_cy = (y - m_y)/16;
		m_cyminor = ((y - m_y) % 16) / 2;
		if (m_mb) putCell();
	}
	return ZXE_CONTINUE;
}



