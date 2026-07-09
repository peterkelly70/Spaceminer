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
#include "manicportalpage.hxx"
#include "manicfixedguard.hxx"
#include "manicgfxform.hxx"
#include "room.hxx"
#include "tick.xbm"
#include "left.xbm"
#include "right.xbm"

#define WP_X 24

ManicPortalPage::ManicPortalPage(ManicGame *game) : ManicRoomNav(game)
{
	int x,y,w,h;

	h = (3 * CHAR_H + CHAR_SH);
	y = 18 * CHAR_H + CHAR_SH;

	m_cx = m_cy = 0;
	m_ocx = m_ocy = -1;

	addHotspot(new Hotspot('W', (WP_X + 1) * CHAR_W, y, 32, h, 'w','W'));
        addHotspot(new Hotspot('X', (WP_X + 4) * CHAR_W, y, 32, h, 'x','X'));
	addHotspot(new Hotspot('G', 63 * CHAR_SW, 18 * CHAR_H, 17 * CHAR_SW,
					5 * CHAR_SH, 'G', 'g'));

// Portal colour
        x = VIDEO_W - 16 * CHAR_W;
        y = 24 * CHAR_H + CHAR_SH;
        w = 4 * CHAR_W;
        h = 2 * CHAR_H;

        addHotspot(new Hotspot('I', x, y, w, h, 'I', 'i')); x += w;
        addHotspot(new Hotspot('P', x, y, w, h, 'P', 'p')); x += w;
        addHotspot(new Hotspot('B', x, y, w, h, 'B', 'b')); x += w;
        addHotspot(new Hotspot('F', x, y, w, h, 'F', 'f'));

        x = CHAR_W;
        addHotspot(new Hotspot('D', x, y, w, h, 'D', 'd')); x += w;
        addHotspot(new Hotspot('N', x, y, w, h, 'N', 'n')); x += w;

        x = 17 * CHAR_W;
        y = 19 * CHAR_H;
	w = 2 * CHAR_W;
	h = CHAR_H;
        addHotspot(new Hotspot('E', x, y, w, h, 'E', 'e')); y += CHAR_H;
        addHotspot(new Hotspot('K', x, y, w, h, 'K', 'k')); y += CHAR_H;
        addHotspot(new Hotspot('S', x, y, w, h, 'S', 's')); y += CHAR_H;
        addHotspot(new Hotspot('V', x, y, w, h, 'V', 'v')); y += CHAR_H;
        addHotspot(new Hotspot('Y', x, y, w, h, 'Y', 'y')); y += CHAR_H;

	setTitle("Portals");

	SpectrumMemory *mem = game->getMem();

	int adrw = 256 * game->getWillySpritePage();
                                           
	m_bmWilly = ::bitmapFromSprite(16,16, mem->memoryAt(adrw), 1, 
			ZX_BLACK, ZX_BRWHITE);
	m_bmPortal= ::bitmapFromSprite(16,16, game->getRoom(m_room)+656, 1,
			ZX_BLACK, ZX_BRWHITE);
        m_bmTick = bitmapFromXbm(tick_width, tick_height, tick_bits, 0,
                                ZX_BRBLUE, ZX_BRWHITE);
        m_bmLeft = bitmapFromXbm(left_width, left_height, left_bits, 0,
                                ZX_BRBLUE, ZX_BRWHITE);
        m_bmRight= bitmapFromXbm(right_width, right_height, right_bits, 0,
                                ZX_BRBLUE, ZX_BRWHITE);
	m_mode = 0;
	m_prevMode = -1;
}

ManicPortalPage::~ManicPortalPage()
{
	delete m_bmWilly;
	delete m_bmPortal;
	delete m_bmTick;
}



void ManicPortalPage::drawFixed(void)
{
        int x,y,w,h;
	videoScreen->bottomBar("Portal");
	
	videoScreen->fillBox((WP_X) * CHAR_W, 18 * CHAR_H, 7 * CHAR_W, 4 * CHAR_H,
				ZX_BRWHITE);
        videoScreen->box    ((WP_X) * CHAR_W, 18 * CHAR_H, 7 * CHAR_W, 4 * CHAR_H,
                                ZX_BLACK);
	videoScreen->drawSmallText((WP_X) * CHAR_W, 18 * CHAR_H, 
				   "Special Chars ", ZX_BRWHITE, ZX_BLACK);

	m_bmWilly->toScreen ((WP_X + 1) * CHAR_W, 19 * CHAR_H);
	m_bmPortal->toScreen ((WP_X + 4) * CHAR_W, 19 * CHAR_H);

	y = 21 * CHAR_H + (CHAR_SH / 2);

	videoScreen->drawSmallText((WP_X + 1) * CHAR_W, y, "W", ZX_BLACK, ZX_BRWHITE);
	videoScreen->drawSmallText((WP_X + 4) * CHAR_W, y, "X", ZX_BLACK, ZX_BRWHITE);

        x = VIDEO_W - 16 * CHAR_W;
        y = 24 * CHAR_H;
        w = 4 * CHAR_W;
        h = 2 * CHAR_H;
        videoScreen->fillBox(x, y, 16 * CHAR_W, 5*CHAR_SH, ZX_BRWHITE);
        videoScreen->box(x, y + CHAR_SH, 16 * CHAR_W, h, ZX_BLACK);
        videoScreen->drawSmallText(x, y, "Portal attributes               ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "I:* P:* B:* F:"
                        , ZX_BLACK, ZX_BRWHITE);
        videoScreen->smallZxLogo(VIDEO_W - 6 * CHAR_SW, y);
        videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
        videoScreen->box(x, y + CHAR_SH, w + 4*CHAR_W, h, ZX_BLACK);
        videoScreen->box(x, y + CHAR_SH, w + 8*CHAR_W, h, ZX_BLACK);

        x = 0;
        y = 24 * CHAR_H;
        w = 4 * CHAR_W;
        h = 2 * CHAR_H;
        videoScreen->fillBox(x, y, 8 * CHAR_W, 5*CHAR_SH, ZX_BRWHITE);
        videoScreen->box(x, y + CHAR_SH, 8 * CHAR_W, h, ZX_BLACK);
        videoScreen->drawSmallText(x, y, "Willy           ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "D:* N:*"
                        , ZX_BLACK, ZX_BRWHITE);
        videoScreen->smallZxLogo(x + 10 * CHAR_SW, y);
        videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
        videoScreen->box(x, y + CHAR_SH, w + 4*CHAR_W, h, ZX_BLACK);

	/* Flags for the special beasties */
        x = 9 * CHAR_W;
        y = 18 * CHAR_H;
        w = 8 * CHAR_W;
        h = 13 * CHAR_SH;
        videoScreen->fillBox(x, y, 10 * CHAR_W, h, ZX_BRWHITE);
        videoScreen->box(x, y, 8 * CHAR_W, h, ZX_BLACK);
        videoScreen->box(x, y + CHAR_SH, 10 * CHAR_W, h - CHAR_SH, ZX_BLACK);
        videoScreen->drawSmallText(x, y, "Specials            ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(x + CHAR_SW, y + 2*CHAR_SH, "Eugene ", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + CHAR_SW, y + 4*CHAR_SH, "Kong   ", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + CHAR_SW, y + 6*CHAR_SH, "Solar  ", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + CHAR_SW, y + 8*CHAR_SH, "Vguards", ZX_BLACK, ZX_BRWHITE);
        videoScreen->drawText(x + CHAR_SW, y + 10*CHAR_SH, "skYlabs", ZX_BLACK, ZX_BRWHITE);
        videoScreen->smallZxLogo(x + 14 * CHAR_SW, y);

	videoScreen->fillBox(63 * CHAR_SW, 18 * CHAR_H, 17 * CHAR_SW, 
				5 * CHAR_SH, ZX_BRWHITE);
        videoScreen->box    (63 * CHAR_SW, 18 * CHAR_H, 17 * CHAR_SW, 
				5 * CHAR_SH, ZX_BLACK);
	videoScreen->drawSmallText(63 * CHAR_SW, 18 * CHAR_H, 
				   "                   ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->smallZxLogo(74 * CHAR_SW, 18 * CHAR_H);
        videoScreen->drawText(32 * CHAR_W - 2, 19 * CHAR_H, "Graphics", ZX_BLACK, ZX_BRWHITE);

	ManicRoomNav::drawFixed();
}





void ManicPortalPage::drawCursor()
{
	VideoSurface *s = m_bmAnim->getSurface();
	int i = ZX_BRWHITE;
	if ((m_ticker % 20) >= 10) i = ZX_BRBLUE;

	int w = 32, h = 32;
	if (m_cx == 31) w = 16;
	if (m_cy == 15) h = 16;
	s->box(m_cx * 16, m_cy * 16, w, h, i);
}




void ManicPortalPage::drawData(int flashonly)
{	
	int n, x, y;
	char s[30];

	if (!flashonly)
	{
       		sprintf(s, "Portal [%d]", m_room);
	        videoScreen->bottomBar(s);
		showWilly();
		showSpecials();
	}
	if (m_mode != m_prevMode) 
	{
		for (n = 0; n < 3; n++)
		{
			int x = ((3 * n) + 1 + WP_X) * CHAR_W;
			int y = 18 * CHAR_H + CHAR_SH;
			int w = 2 * CHAR_W;
			int h = 3 * CHAR_H + CHAR_SH;

			if (n == m_mode) videoScreen->fillReplace( x, y, w, h, 
				ZX_BRWHITE, ZX_BRCYAN);
			if (n == m_prevMode) videoScreen->fillReplace( x,y,w,h,
				ZX_BRCYAN, ZX_BRWHITE);
		}
		m_prevMode = m_mode;	
	}
	showAttributes();

	ManicRoomNav::drawData(flashonly);
}

void ManicPortalPage::drawGuardList(void)
{
}



int ManicPortalPage::onHotspot(int id)
{
	jswByte *data = getRoom(m_room);
	jswByte oa = data[655];
	jswByte bg = (data[544] >> 3) & 0xFF;

	switch(id)
	{
		case 'G':
			  {
				jswByte roomCopy[1024];
				memcpy(roomCopy, data, 1024);
				intPreContextMenu();
				ManicGraphicsForm mg(roomCopy, m_hasvguards || m_skylabs);
				int rv = mg.doModal();
			  	postContextMenu();
				if (rv >= ZXE_QUIT) return rv;
				if (rv == ZXE_OK)
				{
					memcpy(data, roomCopy, 1024);
			  		getRoom()->createCellPatterns();
					if (m_vgeugene)
					{
						delete m_vgeugene;
						m_vgeugene = new ManicEugene(data, bg);
					}
					if (m_bmPortal) delete m_bmPortal;
					m_bmPortal = ::bitmapFromSprite(
						16, 16, data + 656, 1, ZX_BLACK, ZX_BRWHITE);
					m_bmPortal->toScreen( (WP_X + 4) * CHAR_W, 19 * CHAR_H);
					if (1 == m_mode) videoScreen->fillReplace( (WP_X + 4) * CHAR_W, 19 * CHAR_H, 
						32, 32, ZX_BRWHITE, ZX_BRCYAN);
				}	
			  }
			  return ZXE_CONTINUE;
		case 'W': m_mode = 0; drawData(); return ZXE_CONTINUE;
                case 'X': m_mode = 1; drawData(); return ZXE_CONTINUE;
		case 'I': data[655] &= (~7);
			  data[655] |= (oa + 1) & 7;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  return ZXE_CONTINUE; 
		case 'P': data[655] &= (~0x38);
			  data[655] |= (oa + 8) & 0x38;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  return ZXE_CONTINUE; 
		case 'F': data[655] ^= 0x80;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  return ZXE_CONTINUE; 
		case 'B': data[655] ^= 0x40;
			  showAttributes();
			  getRoom()->createCellPatterns();
			  return ZXE_CONTINUE; 
		case 'D': data[618] = data[618] ? 0 : 1;
			  showWilly();
			  delete m_vgwilly;
		          m_vgwilly = new ManicFixedGuard(MFG_WILLY, m_game, data, bg);
			  return ZXE_CONTINUE; 
		case 'N': data[617] = (data[617] + 1) % 4;
			  showWilly();
			  delete m_vgwilly;
		          m_vgwilly = new ManicFixedGuard(MFG_WILLY, m_game, data, bg);
			  return ZXE_CONTINUE; 
	}
	if (m_game->featuresSettableByRoom()) switch(id)
	{
		case 'E': data[627] ^= FEAT_EUGENE;  gotoRoom(m_room); break; 
		case 'K': data[627] ^= FEAT_KONG;    gotoRoom(m_room); break; 
		case 'S': data[627] ^= FEAT_SOLAR;   gotoRoom(m_room); break; 
		case 'V': data[627] ^= FEAT_VGUARDS; gotoRoom(m_room); break; 
		case 'Y': data[627] ^= FEAT_SKYLABS; gotoRoom(m_room); break; 
	}
	return ManicRoomNav::onHotspot(id);
}

int ManicPortalPage::onSelect(void)
{
	jswByte *data = getRoom(m_room);

	switch(m_mode)
	{
		case 0:	data[616] = m_cy * 16;
			data[620] = (m_cx & 0x1F) | ((m_cy << 5) & 0xE0);
			data[621] = 0x5C | ((m_cy & 8) >> 3);
			gotoRoom(m_room);
			break;
		case 1: data[688] = (m_cx & 0x1F) | ((m_cy << 5) & 0xE0);
			data[689] = 0x5C | ((m_cy & 8) >> 3);
			data[690] = (m_cx & 0x1F) | ((m_cy << 5) & 0xE0);
			data[691] = 0x60 | (m_cy & 8);
			gotoRoom(m_room);
			break;
	}
	return ZXE_CONTINUE;
}


int ManicPortalPage::onKeyDown(int keysym)
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
	return ManicRoomNav::onKeyDown(keysym);
}


int ManicPortalPage::onButtonDown(int x, int y, int button)
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

	return ManicRoomNav::onButtonDown(x, y, button);
}

int ManicPortalPage::onMouseMove (int x, int y, int xrel, int yrel)
{
        if (x >= m_x && x < (m_x + 512) &&
            y >= m_y && y < (m_y + 256))
        {
                m_cx = (x - m_x)/16;
                m_cy = (y - m_y)/16;
        }
	return ManicRoomNav::onMouseMove(x,y,xrel,yrel);
}


void ManicPortalPage::loadRoom(int room, int reload)
{
	ManicRoomNav::loadRoom(room, reload);

	delete m_bmPortal;
	m_bmPortal= ::bitmapFromSprite(16,16, m_game->getRoom(room)+656, 1,
			ZX_BLACK, ZX_BRWHITE);
	m_bmPortal->toScreen ((WP_X + 4) * CHAR_W, 19 * CHAR_H);
	if (1 == m_mode) videoScreen->fillReplace( (WP_X + 4) * CHAR_W, 19 * CHAR_H, 
		32, 32, ZX_BRWHITE, ZX_BRCYAN);
}

int ManicPortalPage::preContextMenu(void)
{
	intPreContextMenu();
	return ManicRoomNav::preContextMenu();
}


void ManicPortalPage::showAttributes()
{
        int x,y;
        int ifg, ibg, pfg, pbg;

	jswByte *data = getRoom(m_room);

        ibg = (data[655] & 7) | ZX_BRIGHT;               // Current ink
        pbg = ((data[655] >> 3) & 7) | ZX_BRIGHT;        // Current paper
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
        if (data[655] & 0x40) m_bmTick->toScreen(x +  8 * CHAR_W + 2, y+2);
        if (data[655] & 0x80) m_bmTick->toScreen(x + 12 * CHAR_W + 2, y+2);
}



/* Draw Willy's initial direction and frame */	
void ManicPortalPage::showWilly()
{
	int x, y;
	char s[5];

	jswByte *data = getRoom(m_room);
	x = 0;
	y = 25 * CHAR_H;
	if (data[618]) m_bmLeft ->toScreen(x + 5 * CHAR_SW + 2, y+2);
	else	       m_bmRight->toScreen(x + 5 * CHAR_SW + 2, y+2);		

	sprintf(s, "%d", data[617] % 4);
        videoScreen->drawText(x + 13 * CHAR_SW, y, s, ZX_BLACK, ZX_BRWHITE);
}


void ManicPortalPage::showSpecials()
{
	int x,y,w,h;
        x = 17 * CHAR_W + CHAR_SW;
        y = 19 * CHAR_H;

        videoScreen->fillBox(x, y, CHAR_W, 5 * CHAR_H, ZX_BRWHITE);
	if (m_game->roomHasFeature(m_room, FEAT_EUGENE))
		m_bmTick->toScreen(x + 2, y + 2);
	if (m_game->roomHasFeature(m_room, FEAT_KONG))
		m_bmTick->toScreen(x + 2, y + CHAR_H + 2);
	if (m_game->roomHasFeature(m_room, FEAT_SOLAR))
		m_bmTick->toScreen(x + 2, y + 2*CHAR_H + 2);
	if (m_game->roomHasFeature(m_room, FEAT_VGUARDS))
		m_bmTick->toScreen(x + 2, y + 3*CHAR_H + 2);
	if (m_game->roomHasFeature(m_room, FEAT_SKYLABS))
		m_bmTick->toScreen(x + 2, y + 4*CHAR_H + 2);
}
