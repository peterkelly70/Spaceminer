
/************************************************************************

    JSWED 2.2.3 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-1,2004-2005  John Elliott <jce@seasip.demon.co.uk>

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
#include "j64guardpage.hxx"
#include "guardedit.hxx"
#include "j128gedit.hxx"
#include "j64gedit.hxx"
#include "j64gtableform.hxx"
#include "room.hxx"

static char guardChars[] = "0123456789ABCDEFGHIKLMNOPQRSTUVWXYZ****************";	
static jswByte gl_clipBuf[8];

Jsw64GuardPage::Jsw64GuardPage(JswGame *game) : Jsw48RoomNav(game)
{
	m_ot   = NULL;
	m_cx = m_cy = m_acyminor = 0;
	m_octype = -1;
	m_ctype  = 0;
	m_oft = -1;
	m_mb = 0;
	m_ocx = m_ocy = -1;
	m_cdir = 0;

	// Active areas
	int x, y, w, h;
	x = 8 * CHAR_W;
	y = 256 + 2 * CHAR_H;
	w = 8 * CHAR_W;
	h = CHAR_H;


	for (int n = 0; n < 32; n++)
	{
		addHotspot(new Hotspot(guardChars[n], 
				x + (8 * CHAR_W * (n/8)), 
				y + (CHAR_H * (n % 8)), w, h, 
			toupper(guardChars[n]), tolower(guardChars[n]))); 
	}
	setTitle("Guards");
}

Jsw64GuardPage::~Jsw64GuardPage()
{
}


void Jsw64GuardPage::drawFixed(void)
{
	int x,y,h,n,m,max;

	videoScreen->bottomBar("Guardian editor");

	videoScreen->fillBox(8 * CHAR_W, 256 + CHAR_H + CHAR_SH,
				32 * CHAR_W, 9 * CHAR_H + CHAR_SH, ZX_BRWHITE);
	videoScreen->box(8 * CHAR_W, 256 + CHAR_H + CHAR_SH - 1,
			32 * CHAR_W, 9 * CHAR_H + CHAR_SH + 1, ZX_BLACK);
	videoScreen->box(16 * CHAR_W, 256 + CHAR_H + CHAR_SH - 1,
			  8 * CHAR_W, 9 * CHAR_H + CHAR_SH + 1, ZX_BLACK);
	videoScreen->box(16 * CHAR_W, 256 + CHAR_H + CHAR_SH - 1,
			 16 * CHAR_W, 9 * CHAR_H + CHAR_SH + 1, ZX_BLACK);

	videoScreen->drawSmallText(8 * CHAR_W, 256 + CHAR_H,
	"Guardians                                                       ",
				ZX_BRWHITE, ZX_BLACK);

	videoScreen->smallZxLogo(VIDEO_W - 8 * CHAR_SH, 256 + CHAR_H);
	Jsw48RoomNav::drawFixed();
}


void Jsw64GuardPage::setCellType(int ct)
{
	if (ct >= getRoom()->getMaxGuards() || ct < 0) return;
	m_ctype = ct;
	drawCellType();
}

void Jsw64GuardPage::drawCellType()
{
	if ((m_octype == m_ctype) || !m_visible || !m_drawnFixed) return;

	int w  = 8 * CHAR_W;
	int x =  8 * CHAR_W;
	int y  = 256 + 2 * CHAR_H;
	int ox = x;
	int ow = w;
	int oy = y;

	y += (m_ctype % 8) * CHAR_H;
	x += (m_ctype / 8) * 8 * CHAR_W;
	oy += (m_octype % 8) * CHAR_H;
	ox += (m_octype / 8) * 8 * CHAR_W;

	if (m_octype >= 0) videoScreen->fillReplace (ox, oy, ow, CHAR_H,
                                        ZX_BRCYAN,
                                        ZX_BRWHITE);
	if (m_ctype >= 0) videoScreen->fillReplace(x, y, w, CHAR_H, ZX_BRWHITE,
                                        ZX_BRCYAN);
	m_octype = m_ctype;

}


void Jsw64GuardPage::flashColours(void)
{
	if (!m_visible) return;

        Jsw48RoomNav::flashColours();

	/* Draw guardian thumbnails */
	for (int n = 0; n < 32; n++)
	{
		int x = ((8*(n / 8)) + 14) * CHAR_W;
		int y = ((n % 8) + 18) * CHAR_H;

        	if (m_guards[n]) m_guards[n]->drawThumb(videoScreen, x, y);
                else videoScreen->fillBox(x, y, CHAR_W, CHAR_H, ZX_BRWHITE);
	}

	/* Draw animation frames */
        if (m_guards[m_ctype])
        {
                m_guards[m_ctype]->drawFrames(videoScreen, 80, VIDEO_H - 32);
        }
        else    videoScreen->fillBox(80, VIDEO_H - 32, 480, 32, ZX_WHITE);

}



void Jsw64GuardPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;


	if (m_guards[m_ctype])
	{
		JswGuardian *g = m_guards[m_ctype];

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



void Jsw64GuardPage::drawData(int flashonly)
{
	if (!flashonly)
	{
	        char s[30];
		int x,y,fg;

       		sprintf(s, "Guardian editor [%d]", m_room);
	        videoScreen->bottomBar(s);
	}
	Jsw48RoomNav::drawData(flashonly);
}



void Jsw64GuardPage::drawGuardList(void)
{
	int x, y, n, p, stop;
	char buf[8];

	y = 256 + 2 * CHAR_H;
	x = 8 * CHAR_W + CHAR_SW;

	m_maxGuards = getRoom()->getMaxGuards();
	stop = 0;
	for (n = 0; n < m_maxGuards; n++)
	{
		if (m_guards[n])
		{
			sprintf(buf, "%c:%-3.3s  ", guardChars[n], 
							guardType(n));
		}
		else
		{
			sprintf(buf, "%c: --  ", guardChars[n]);
		}
		if (n == m_ctype)	p = ZX_BRCYAN;
		else			p = ZX_BRWHITE;

		videoScreen->drawText(x + ((n/8)*8) * CHAR_W, 
				      y + (n%8) * CHAR_H, buf, ZX_BLACK, p);
	}
	for (; n < 32; n++)
	{
		videoScreen->drawText(x + ((n/8)*8) * CHAR_W, 
				      y + (n%8) * CHAR_H, "       ", 
				      ZX_BLACK, ZX_BRWHITE);
	}
}


const char *Jsw64GuardPage::guardType(int ng)
{
	jswByte *buf = getRoom()->getGuardianBuffer() + (8 * ng);

	switch(buf[0] & 0x0f)
	{
		case 0:  return "---";
		case 1:  
		case 9:  return "Hor";
		case 2: 
		case 7: 
		case 10:
		case 15: return "Ver";
		case 3:  return "Rop";
		case 4:  return "Arr";	
		case 5: 
		case 6:  
		case 13: 
		case 14: return "Dia";
		case 12: return "UD ";
		case 8:  switch(buf[0] >> 4)
		{
			case 0:   return "Sky";
			case 1:   return "Eu1";
			case 2:   return "EuC";
			case 3:   return "Eug";
			case 5:   return "Lif";
			case 6:   return "Sce";
			case 8:   return "Tri";
			case 9:   return "Sw ";
			case 10:  return "Wal";
			case 11:  return "Stp";
			case 12:  return "Stp";
			case 13:  return "TrW";
			default:  return "S??";
		}
		default: return "???";
	}
}



void Jsw64GuardPage::putCell()
{
	jswByte *buf = getRoom()->getGuardianBuffer() + (8 * m_ctype);
	JswGuardian *g = m_guards[m_ctype];
		
//	printf("Put cell. Before edit, guardian = "
//		"%02x %02x %02x %02x "
//		"%02x %02x %02x %02x\n", 
//		buf[0], buf[1], buf[2], buf[3],
//		buf[4], buf[5], buf[6], buf[7]);
	if (g) 
	{
		int type = buf[0] & 0x0F;
		if (type == 8) type = buf[0];

		switch(type)
		{
			case 1: case 9:	// Horizontals
			case 2: case 10: // Verticals
			case 7: case 15:
			case 5: case 6:	// Diagonals
			case 13: case 14:
			case 0x08: case 0x18:	// Skylabs & Eugenes
			case 0x28: case 0x38:
			case 0x58: case 0x68:   // Lift, Scenery
			case 0x98:		// Switch
			buf[2] = g->setPos(m_cx, m_cy);
			buf[3] = m_cy * 16;
			break;

			case 3: // Oops. Forgot the rope.
			buf[2] = g->setPos(m_cx, m_cy);
			break;

			case 4: // Arrow
			buf[2] = g->setPos(m_acyminor, m_cy);
			break;

			case 0xA8:	// Opening wall
			g->setPos(m_cx, m_cy);
			buf[6] = m_cx;
			buf[7] = m_cy * 16;
			break;
		}
	}
//	printf("End put cell. Before edit, guardian = "
//		"%02x %02x %02x %02x "
//		"%02x %02x %02x %02x\n", 
//		buf[0], buf[1], buf[2], buf[3],
//		buf[4], buf[5], buf[6], buf[7]);
}




int Jsw64GuardPage::guardianMenu(void)
{
        VideoMenu vmg1("Guardian", "Editor", "Clipboard", "Add", 
			"Edit", "Delete", "Table", "Exit", NULL);
        VideoMenu vmg2("Guardian", "Editor", "Clipboard", "Edit", 
			"Delete", "Table", "Exit", NULL);
        VideoMenu vmg3("Guardian", "Editor", "Clipboard", "Add", 
			"Table", "Exit", NULL);
        VideoMenu vmg4("Guardian", "Editor", "Clipboard", "Table", 
			"Exit", NULL);

	VideoMenu *vm;
	static jswByte gTemplate[8] = { 0x12, 0x64, 0x0f, 0x80, 
					0xfe, 0xac, 0x70, 0xa0 };

        jswByte *DEBUGDATA = getRoom()->getGuardianBuffer() + 8;

	int x, rv, ng, gi, bg;
	jswByte *g;
	int spares = -1;

	for (ng = 0; ng < m_maxGuards; ng++)
	{
		if (m_guards[ng] == NULL)	// [2.1.3] Inverted logic... 
		{
			spares = ng;
			break;
		}
	}
	if (spares >= 0)
	{
		if (m_guards[m_ctype])	vm = &vmg1;
		else			vm = &vmg3;
	}	
	else
	{
		if (m_guards[m_ctype])	vm = &vmg2;
		else			vm = &vmg4;
	}
	rv = vm->doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	int ns = vm->getSelected();

	if (vm == &vmg2 && ns > 1)  ns += 1;
	if (vm == &vmg3 && ns > 2)  ns += 2;
	if (vm == &vmg4 && ns > 1)  ns += 3;
	// 0 = Editor
	// 1 = Clipboard
	// 2 = Add
	// 3 = Edit
	// 4 = Delete
	// 5 = Table
	// 6 = Exit
	bg = getRoom()->getBackground();
        switch(ns)
	{
		case 1:	return clipMenu();
		case 2: if (spares < 0)	 return ZXE_CONTINUE;
			g = getRoom()->getGuardianBuffer() + 8 * spares;
			if (g[0] == 0xFF) g[8] = 0xFF;
			memcpy(g, gTemplate, 8);
			{
				Jsw64GuardEditor ge(m_room, m_game, g, m_bmRoom);
				rv = ge.doModal();

				if (rv >= ZXE_QUIT) return rv;
				if (rv != ZXE_OK) return ZXE_CONTINUE;
				delete m_guards[spares];
				m_guards[spares] = m_game->newGuardian(g, 
								g[2], bg); 
			}
    			drawData();  
			return ZXE_CONTINUE;
		case 3: // Edit guardian
			if (m_guards[m_ctype] == NULL) return ZXE_CONTINUE;
			g = getRoom()->getGuardianBuffer() + 8 * m_ctype;
			{
				Jsw64GuardEditor ge(m_room, m_game, g, m_bmRoom);
				
				rv = ge.doModal();

				if (rv >= ZXE_QUIT) return rv;
				if (rv != ZXE_OK) return ZXE_CONTINUE;
				delete m_guards[m_ctype];
				m_guards[m_ctype] = m_game->newGuardian(g, 
								g[2], bg); 

			}	
			return ZXE_CONTINUE;

		case 4:	// Delete guardian
			if (m_guards[m_ctype] == NULL) return ZXE_CONTINUE;
			g = getRoom()->getGuardianBuffer() + 8 * m_ctype;
			delete m_guards[m_ctype];
			for (ng = m_ctype; ng < m_maxGuards - 1; ng++)
			{
				memcpy(g, g+8, 8);
				g += 8;
				m_guards[ng] = m_guards[ng+1];
			}
			m_guards[ng] = NULL;
			g[0] = 0xFF;
			drawData();
			return ZXE_CONTINUE;
		// Choose guardian table	
		case 5: 
			{
				Jsw64GuardianTableForm jgtf(getRoom());
				rv = jgtf.doModal();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
			}	
		        loadRoom(m_room, 1); 
			drawData();
			return ZXE_CONTINUE;
	}
	return ZXE_CONTINUE;
}


int Jsw64GuardPage::preContextMenu(void)
{
	intPreContextMenu();
	int rv = guardianMenu();
	postContextMenu();
	return rv;
}







void Jsw64GuardPage::undo(void)
{
	jswByte buf[1024];
	jswByte *b = m_game->getRoom(m_room);
	int roomLen = getRoom()->getRoomLength();

	memcpy(buf, b, roomLen);
        memcpy(b, undoBuffer, roomLen);
	memcpy(undoBuffer, buf, roomLen);

	if (m_visible) drawData();
}


void Jsw64GuardPage::loadRoom(int room, int reload)
{
        jswByte *b = m_game->getRoom(room);
	int roomLen = getRoom()->getRoomLength();
	m_maxGuards = getRoom()->getMaxGuards();
	if (m_ctype >= m_maxGuards) setCellType(0);

	if (!reload) memcpy(undoBuffer, b, roomLen);
	switch(((Jsw64Game *)m_game)->getVariant())
	{
		case 'V': case 'W': 
			memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x200);
			break;
		default: 
			memcpy(m_game->getMem()->memoryAt(0x8000), b, 0x400);
			break;
	}

	Jsw48RoomNav::loadRoom(room, reload);
}



int Jsw64GuardPage::onHotspot(int id)
{
	char *pos = strchr(guardChars, id);
	if (pos)
	{
		int id = pos - guardChars;
		setCellType(id);
		return ZXE_CONTINUE;	
	}
	return Jsw48RoomNav::onHotspot(id);

}


int Jsw64GuardPage::onKeyDown(int keysym)
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
			if (m_guards[m_ctype]->isArrow())
			{
				if (m_acyminor > 0) { m_acyminor--; return ZXE_CONTINUE; }
				if (m_cy > 0)       { m_acyminor = 7; --m_cy; return ZXE_CONTINUE; }
			} 
			if (m_cy > 0) --m_cy;
			return ZXE_CONTINUE;
		case ZXK_DOWN:
			if (m_guards[m_ctype]->isArrow())
                        {
				if (m_cy == 14 && m_acyminor >= 6) return ZXE_CONTINUE;
                                if (m_acyminor < 7) { m_acyminor++; return ZXE_CONTINUE; }
                                if (m_cy < 15)      { m_acyminor = 0; ++m_cy; return ZXE_CONTINUE; }
                        } 
			if (m_cy < 15) ++m_cy;
			return ZXE_CONTINUE;
		case ZXK_ENTER:
		case ' ': 
			 putCell(); return ZXE_CONTINUE;
	} 
	return Jsw48RoomNav::onKeyDown(keysym);
}




int Jsw64GuardPage::onButtonDown(int x, int y, int button)
{
	if (y < 256)
	{
		m_mb = 1;
		return onMouseMove(x, y, 0, 0);
	}
	return Jsw48RoomNav::onButtonDown(x, y, button);
}



int Jsw64GuardPage::onButtonUp(int x, int y, int button)
{
	m_mb = 0;
	return Jsw48RoomNav::onButtonUp(x, y, button);
}



int Jsw64GuardPage::onMouseMove(int x, int y, int xrel, int yrel)
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


int Jsw64GuardPage::clipMenu()
{
	VideoMenu vmg1("Clipboard", "Cut", "Copy", "Paste", NULL);
	VideoMenu vmg2("Clipboard", "Paste", NULL);
	VideoMenu *vm;
	jswByte *g;
	int ng;

	if (m_guards[m_ctype])	vm = &vmg1;
	else			vm = &vmg2;

	int rv = vm->doModal();
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	int ns = vm->getSelected();
	if (vm == &vmg2) ns += 2;
	int bg = getRoom()->getBackground();
	switch(ns)
	{
		case 0: 
			if (m_guards[m_ctype] == NULL) return ZXE_CONTINUE;
			g = getRoom()->getGuardianBuffer() + 8 * m_ctype;
			memcpy(gl_clipBuf, g, 8);
			delete m_guards[m_ctype];
			for (ng = m_ctype; ng < m_maxGuards - 1; ng++)
			{
				memcpy(g, g+8, 8);
				g += 8;
				m_guards[ng] = m_guards[ng+1];
			}
			m_guards[ng] = NULL;
			g[0] = 0xFF;
			drawData();
			return ZXE_CONTINUE;
		case 1:
			if (m_guards[m_ctype] == NULL) return ZXE_CONTINUE;
			g = getRoom()->getGuardianBuffer() + 8 * m_ctype;
			memcpy(gl_clipBuf, g, 8);
			return ZXE_CONTINUE;
		case 2: 
			g = getRoom()->getGuardianBuffer() + 8 * m_ctype;
			if (g[0] == 0xFF) g[8] = 0xFF;
			memcpy(g, gl_clipBuf, 8);
			if (m_guards[m_ctype]) delete m_guards[m_ctype];
			m_guards[m_ctype] = m_game->newGuardian(g, g[2], bg); 
			drawData();
			return ZXE_CONTINUE;
	}

}


