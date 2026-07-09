/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-6  John Elliott <jce@seasip.demon.co.uk>

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
#include "screenedit.hxx"
#include "tick.xbm"
#include "pencil.xbm"
#include "png.h"

extern "C" {

#include <SDL.h>
#include <SDL_image.h>

};

#define MD_BITMAP 0
#define MD_ATTRIB 1

#define GR_NONE  0
#define GR_PIXEL 1
#define GR_ATTR  2

#define GRID_C1  0x28
#define GRID_C2  0x79

ScreenEditorPage::ScreenEditorPage()
{
	int x, y, w, h;

	m_bDraw = false;
	m_bmData = m_bmAttr1 = m_bmAttr2 = m_bmPixGrid = NULL;
	m_tcount = 0;
	m_haveTimer = 0;
	m_ot = NULL;
	m_pcx = m_pcy = 0;
	m_acx = m_acy = 0;
	m_visible = 0;
	m_attrib = 0x47;
	m_buttonDown = 0;
	m_grid = GR_NONE;	
	m_mode = MD_BITMAP;
	m_undoBits = NULL;
	m_undoAttrs = NULL;

	m_bmTick = bitmapFromXbm(tick_width, tick_height, tick_bits, 0,
				ZX_BRBLUE, ZX_BRWHITE);
	m_bmPencil = bitmapFromXbm(pencil_width, pencil_height, pencil_bits, 0,
				ZX_BRBLUE, ZX_BRWHITE);
	m_bmColours = newVideoBitmap(16, 16);
	for (y = 0; y < 16; y++) for (x = 0; x < 16; x++)
	{
		m_bmColours->getSurface()->plot(x, y, (x^y));
	}
	m_bmCgrid = newVideoBitmap(16,16);
	m_bmPgrid = newVideoBitmap(16,16);
	for (y = 0; y < 16; y += 4) for (x = 0; x < 16; x += 4) 
	{
		int c = ((x ^ y) & 4) ? ZX_BRWHITE : ZX_CYAN;
		m_bmCgrid->getSurface()->fillBox(x, y, 4, 4, c);

		m_bmPgrid->getSurface()->fillBox(x, y, 4, 4, ZX_BRWHITE);
		m_bmPgrid->getSurface()->plot(x, y, ZX_BLACK);
	}

        x = VIDEO_W - 16 * CHAR_W;
        y = 24 * CHAR_H + CHAR_SH;
	w = 4 * CHAR_W;
	h = 2 * CHAR_H;

	addHotspot(new Hotspot('I', x, y, w, h, 'I', 'i')); x += w; 
	addHotspot(new Hotspot('P', x, y, w, h, 'P', 'p')); x += w;
	addHotspot(new Hotspot('B', x, y, w, h, 'B', 'b')); x += w; 
	addHotspot(new Hotspot('F', x, y, w, h, 'F', 'f'));

	x = 0;
	addHotspot(new Hotspot('M', x, y, w, h, 'M', 'm'));

	x = 5 * CHAR_W;
	addHotspot(new Hotspot('G', x, y, w, h, 'G', 'g'));
	
	x = 10 * CHAR_W;
	w /= 2;
	addHotspot(new Hotspot('<', x, y, w, h, '<')); x += w;
	addHotspot(new Hotspot('>', x, y, w, h, '>'));
}


ScreenEditorPage::~ScreenEditorPage()
{
	deleteBitmaps();	
	delete m_bmTick;
	delete m_bmColours;
	delete m_bmPgrid;
	delete m_bmCgrid;
	delete m_bmPencil;
	if (m_haveTimer) setTickListener(m_ot);
}

void ScreenEditorPage::deleteBitmaps(bool withUndo)
{
	if (withUndo)
	{
		if (m_undoBits)  delete [] m_undoBits;
		if (m_undoAttrs) delete [] m_undoAttrs;
		m_undoBits = m_undoAttrs = NULL;
	}
	if (m_bmData)    delete m_bmData;
	if (m_bmAttr1)   delete m_bmAttr1;
	if (m_bmAttr2)   delete m_bmAttr2;
	if (m_bmPixGrid) delete m_bmPixGrid;
        m_bmData = m_bmAttr1 = m_bmAttr2 = m_bmPixGrid = NULL;
}

void ScreenEditorPage::setup(bool withUndo)
{	
	int x, y;

	deleteBitmaps(withUndo);

	switch(m_format)
	{
		case FMT_XBM:   m_mode   = MD_BITMAP;
				m_bmData = bitmapFromXbm(m_w, m_h, m_bits, 
					1, ZX_ORANGE, ZX_BRORANGE); break;
		case FMT_ROSCRN:
		case FMT_SCREEN: m_bmData = bitmapFromScreen(m_h, m_bits,
					1, ZX_ORANGE, ZX_BRORANGE); break;
		case FMT_SPRITE: m_bmData = bitmapFromSprite(m_w, m_h, m_bits, 
					1, ZX_ORANGE, ZX_BRORANGE); break;
	}
	m_bmPixGrid = newVideoBitmap(m_w * 2, m_h * 2);
	m_bmPixGrid->getSurface()->fillBox(0, 0, m_w * 2, m_h * 2, ZX_BRORANGE);
	for(x = 6; x < 2*m_w; x += 16) for (y = 6; y < 2*m_h; y+= 16)
	{
		m_bmPixGrid->getSurface()->box(x, y, 4, 4, ZX_ORANGE);
	} 
	if (withUndo)
	{
		m_undoBits = new jswByte[m_w * m_h / 8];	
		memcpy(m_undoBits, m_bits, m_w * m_h / 8);
	}
	m_bmAttr1 = newVideoBitmap(m_w*2, m_h*2);
        m_bmAttr2 = newVideoBitmap(m_w*2, m_h*2);

	m_aw = (m_w + 7) / 8;	
	m_ah = (m_h + 7) / 8;

	if (m_acx >= m_aw) m_acx = m_aw - 1;
	if (m_acy >= m_ah) m_acy = m_ah - 1;
	if (m_pcx >= m_w)  m_pcx = m_w - 1;
	if (m_pcy >= m_h)  m_pcy = m_h - 1;

	if (m_attrs)
	{
		for (x = 0; x < m_aw; x++) for (y = 0; y < m_ah; y++) 
		{
			setCellAttribute(x, y, m_attrs[y*m_aw + x]);
		}
		if (withUndo)
		{
			m_undoAttrs = new jswByte[m_aw * m_ah];
			memcpy(m_undoAttrs, m_attrs, m_aw * m_ah);
		}
	}
	else
	{
		for (x = 0; x < m_aw; x++) for (y = 0; y < m_ah; y++)
		{
			setCellAttribute(x, y, 0x46);
		}
		if (withUndo) m_undoAttrs = NULL;
	}
	if (m_format == FMT_XBM || m_format == FMT_ROSCRN)
		m_mode = MD_ATTRIB;
	if (!m_attrs) m_mode = MD_BITMAP;
}


int ScreenEditorPage::redraw(int whichRectangle)
{
	m_visible = 1;
	int rv = VideoNotePage::redraw(whichRectangle);

	int x,y,w,h;
	x = VIDEO_W - 16 * CHAR_W;
	y = 24 * CHAR_H;
	w = 4 * CHAR_W;
	h = 2 * CHAR_H;
	videoScreen->fillBox(x, y, 16 * CHAR_W, 5*CHAR_SH, ZX_BRWHITE);
	videoScreen->box(x, y + CHAR_SH, 16 * CHAR_W, h, ZX_BLACK);
	videoScreen->drawSmallText(x, y, "Attributes                      ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "I:* P:* B:* F:"
			, ZX_BLACK, ZX_BRWHITE);
	videoScreen->smallZxLogo(VIDEO_W - 6 * CHAR_SW, y);
	videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
	videoScreen->box(x, y + CHAR_SH, w + 4*CHAR_W, h, ZX_BLACK);
	videoScreen->box(x, y + CHAR_SH, w + 8*CHAR_W, h, ZX_BLACK); 

	x = 0;
	videoScreen->fillBox(x, y, w, 5 * CHAR_SH, ZX_BRWHITE);
	videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
	videoScreen->drawSmallText(x, y, "Mode    ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "M:*", ZX_BLACK, ZX_BRWHITE);

	x = 5 * CHAR_W;
        videoScreen->fillBox(x, y, w, 5 * CHAR_SH, ZX_BRWHITE);
        videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
        videoScreen->drawSmallText(x, y, "Grid    ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "G:*", ZX_BLACK, ZX_BRWHITE);

	x = 10 * CHAR_W;
        videoScreen->fillBox(x, y, w, 5 * CHAR_SH, ZX_BRWHITE);
        videoScreen->box(x, y + CHAR_SH, w, h, ZX_BLACK);
        videoScreen->drawSmallText(x, y, "Go to   ", ZX_BRWHITE, ZX_BLACK);
        videoScreen->drawText(x + CHAR_SW, y + CHAR_H, "< >", ZX_BLACK, ZX_BRWHITE);
	videoScreen->box(x, y+ CHAR_SH, w/2, h, ZX_BLACK);

	if (m_h < 192)
	{
		videoScreen->fillBox(m_x, m_y + 2 * m_h, 512, 384 - 2 * m_h, ZX_WHITE);
	}
	if (m_w < 256)
	{
                videoScreen->fillBox(m_x + 2 * m_w, 0, 512 - 2 * m_w, 384, ZX_WHITE);
	}

	drawData(0);

	return rv;
}


void ScreenEditorPage::drawData(int flashOnly)
{
	int t = m_tcount % 40;

	if (t >= 20 && m_bmAttr2) m_bmAttr2->toScreen(m_x, m_y);
        else if       (m_bmAttr1) m_bmAttr1->toScreen(m_x, m_y);

	drawCursor();
	
	if (!flashOnly)
	{
		showMode();
		showGrid();
		showAttributes();
		drawZoom();
	}
	else drawZoomCursor(videoScreen);

}

void ScreenEditorPage::drawCursor(void)
{
	int t = m_tcount % 40;
	int ci;

	if (t >= 20) ci = ZX_BRBLUE;
	else	     ci = ZX_BRWHITE;

	if (m_mode == MD_ATTRIB) 
	{
		videoScreen->box(m_x + 16 * m_acx, m_y + 16 * m_acy, 16, 16, ci);	
	}
	else if (t < 10 || t >= 30)
	{
		drawPixel(m_pcx, m_pcy);
		videoScreen->plot(m_x + 2 * m_pcx,   m_y + 2*m_pcy,   ci);
		videoScreen->plot(m_x + 2 * m_pcx+1, m_y + 2*m_pcy+1, ci);
	}
	else
	{
		drawPixel(m_pcx, m_pcy);
                videoScreen->plot(m_x + 2 * m_pcx+1, m_y + 2*m_pcy,   ci);
                videoScreen->plot(m_x + 2 * m_pcx,   m_y + 2*m_pcy+1, ci);
	}
}


void ScreenEditorPage::drawCell(int x, int y)
{
        int t = m_tcount % 40;
	int sx = x * 16;
	int sy = y * 16;

        if (t >= 20 && m_bmAttr2) m_bmAttr2->toScreen(m_x+sx, m_y + sy,
				    sx, sy, 16, 16);
        else if       (m_bmAttr1) m_bmAttr1->toScreen(m_x+sx, m_y + sy,
				    sx, sy, 16, 16);
}

void ScreenEditorPage::drawPixel(int x, int y)
{
        int t = m_tcount % 40;
        int sx = x * 2;
        int sy = y * 2;

        if (t >= 20 && m_bmAttr2) m_bmAttr2->toScreen(m_x+sx, m_y + sy,
                                    sx, sy, 2, 2);
        else if       (m_bmAttr1) m_bmAttr1->toScreen(m_x+sx, m_y + sy,
                                    sx, sy, 2, 2);

}

void ScreenEditorPage::showMode(void)
{
        int x = CHAR_SW + 2 * CHAR_W;
        int y = 25 * CHAR_H;

	switch(m_mode)
	{
		case MD_BITMAP: m_bmPencil->toScreen(x, y); break;
		case MD_ATTRIB: m_bmColours->toScreen(x, y); break;	
	}
}

void ScreenEditorPage::showGrid(void)
{
	int x = 7 * CHAR_W + CHAR_SW;
	int y = 25 * CHAR_H;

	switch(m_grid)
	{
		case GR_NONE:	videoScreen->fillBox(x, y, 16, 16, ZX_BRWHITE);
				break;
		case GR_ATTR:	m_bmCgrid->toScreen(x, y);  break;
		case GR_PIXEL:	m_bmPgrid->toScreen(x, y);  break;
	}
}


//
// Show the currently selected attributes
//
void ScreenEditorPage::showAttributes(void)
{
        int x,y;
	int ifg, ibg, pfg, pbg;

	ibg = (m_attrib & 7) | ZX_BRIGHT;		// Current ink
	pbg = ((m_attrib >> 3) & 7) | ZX_BRIGHT;	// Current paper

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
	if (m_attrib & 0x40) m_bmTick->toScreen(x +  8 * CHAR_W + 2, y+2);
        if (m_attrib & 0x80) m_bmTick->toScreen(x + 12 * CHAR_W + 2, y+2);
	
}

void ScreenEditorPage::onTick(void)
{
	if (!m_visible) return;
	++m_tcount;
	if ((m_tcount % 20) == 0)
	{
		drawData(1);
	}
	else if ((m_tcount % 20) == 10) 
	{
		drawCursor();
		drawZoomCursor(videoScreen);
	}

}


void ScreenEditorPage::intPreContextMenu(void)
{
	if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_tcount = 0;
                m_haveTimer = 0;
        }
}


int ScreenEditorPage::preContextMenu(void)
{
	intPreContextMenu();	

	VideoMenu vm("Options", "Editor", "Import image", "Export image",
			"Exit", NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL)
	{
		postContextMenu();
		return ZXE_CONTINUE;
	}
	switch(vm.getSelected())
	{
		case 0: postContextMenu();
			return ZXE_CONTINUE;

		case 1: rv = doImport();
			postContextMenu();
			return rv;

		case 2: rv = doExport();
			postContextMenu();
			return rv;

		case 3: postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return ZXE_CONTINUE;
}


void ScreenEditorPage::postContextMenu(void)
{
        if (!m_haveTimer)
        {
                m_ot = setTickListener(this);
                m_tcount = 0;
                m_haveTimer = 1;
        }
	VideoNotePage::postContextMenu();
}

void ScreenEditorPage::setCellAttribute(int x, int y, jswByte b)
{
	//
	// If there is an attribute grid, then the colours are fixed
	//
	if (m_grid == GR_ATTR) 
	{
		if ((x ^ y) & 1) b = GRID_C1;
		else		 b = GRID_C2; 
	}

	jswByte fg = (b & 7), bg = (b >> 3) & 7;

	if (b & 64) { fg |= ZX_BRIGHT; bg |= ZX_BRIGHT; }

	VideoBitmap *bm = (m_grid == GR_PIXEL) ? m_bmPixGrid : m_bmData;

        bm->toSurface(m_bmAttr1->getSurface(), x*16, y*16, x*16, y*16, 16, 16);
        bm->toSurface(m_bmAttr2->getSurface(), x*16, y*16, x*16, y*16, 16, 16);

	m_bmAttr1->getSurface()->fillReplace(x*16,y*16,16,16,ZX_ORANGE,  fg);
	m_bmAttr1->getSurface()->fillReplace(x*16,y*16,16,16,ZX_BRORANGE,bg);

	if (b & 128) { b = fg; fg = bg; bg = b; } 

        m_bmAttr2->getSurface()->fillReplace(x*16,y*16,16,16,ZX_ORANGE,  fg);
        m_bmAttr2->getSurface()->fillReplace(x*16,y*16,16,16,ZX_BRORANGE,bg);
}


void ScreenEditorPage::togglePixel(int x, int y)
{
	bool b = getPixel(x, y);
	setPixel(x, y, !b);
}

int ScreenEditorPage::onReveal(int whichRectangle)
{
        if (!m_haveTimer)
        {
                m_ot = setTickListener(this);
                m_tcount = 0;
                m_haveTimer = 1;
        }
        return VideoNotePage::onReveal();
}


int ScreenEditorPage::onConceal(void)
{
        if (m_haveTimer)
        {
		setTickListener(m_ot);
		m_tcount = 0;
                m_haveTimer = 0;
        }
        return VideoNotePage::onConceal();
}


int  ScreenEditorPage::onKeyDown(int keysym)
{
	int rv = onKey(keysym);
	if (rv != -1) return rv;

	if (keysym == 'Z' - '@' || keysym == 'U' - '@') return onUndo();

	if (m_mode == MD_ATTRIB) switch(keysym)
	{
		case ZXK_UP:    moveCursor(m_acx, m_acy - 1); return ZXE_CONTINUE;
                case ZXK_DOWN:  moveCursor(m_acx, m_acy + 1); return ZXE_CONTINUE;
                case ZXK_LEFT:  moveCursor(m_acx - 1, m_acy); return ZXE_CONTINUE;
                case ZXK_RIGHT: moveCursor(m_acx + 1, m_acy); return ZXE_CONTINUE;
		case ' ':	setCellAttribute(m_acx, m_acy, m_attrib);
       				m_attrs[m_acy*m_aw + m_acx] = m_attrib;
				drawCell(m_acx, m_acy);
			        drawCursor();
				drawZoomCursor(videoScreen);
				break;

	}
	else switch(keysym)
	{
                case ZXK_UP:    moveCursor(m_pcx, m_pcy - 1); return ZXE_CONTINUE;
                case ZXK_DOWN:  moveCursor(m_pcx, m_pcy + 1); return ZXE_CONTINUE;
                case ZXK_LEFT:  moveCursor(m_pcx - 1, m_pcy); return ZXE_CONTINUE;
                case ZXK_RIGHT: moveCursor(m_pcx + 1, m_pcy); return ZXE_CONTINUE;
                case ' ':       togglePixel(m_pcx, m_pcy);
                                drawPixel(m_pcx, m_pcy);
                                drawCursor();
				drawZoomCursor(videoScreen);
                                break;
	}
	return VideoNotePage::onKeyDown(keysym);
}

int ScreenEditorPage::onMouseMove(int x, int y, int xrel, int yrel)
{
	if (x >= m_x && x < (m_x + (16*m_aw)) &&
	    y >= m_y && y < (m_y + (16*m_ah)))
	{
		if (m_mode == MD_ATTRIB)
		{
			x = (x - m_x) / 16;
			y = (y - m_y) / 16;
			if (m_buttonDown)
			{
				setCellAttribute(x, y, m_attrib);
				m_attrs[m_acy*m_aw + m_acx] = m_attrib;
				drawCell(x, y);		
			}
			moveCursor(x, y);
			return ZXE_CONTINUE;		
		}
		else
		{
			x = (x - m_x) / 2;
			y = (y - m_y) / 2;
			if (m_buttonDown)
			{	
				setPixel(x, y, m_bDraw);
			}
			moveCursor(x, y);
			return ZXE_CONTINUE;
		}
	}
	return VideoNotePage::onMouseMove(x, y, xrel, yrel);
}



int ScreenEditorPage::onButtonDown(int x, int y, int button)
{
        int rv = onButton(x, y, button);
        if (rv != -1) return rv;

	if (x >= m_x && y >= m_y && x < (m_x + 2 * m_w) && y < (m_y + 2*m_h))
	{
		if (m_mode == MD_ATTRIB)
		{
			m_buttonDown = 1;
			setCellAttribute(m_acx, m_acy, m_attrib);
                        m_attrs[m_acy*m_aw + m_acx] = m_attrib;
			drawCell(m_acx, m_acy);
			drawCursor();
			drawZoomCursor(videoScreen);
		}
		else
		{
			m_buttonDown = 1;
			m_bDraw = !getPixel(m_pcx, m_pcy);
			setPixel(m_pcx, m_pcy, m_bDraw);
			drawCursor();
			drawZoom();
		}
	}
	return VideoNotePage::onButtonDown(x, y, button);
}


int ScreenEditorPage::onButtonUp  (int x, int y, int button)
{
	m_buttonDown = 0;
	return VideoNotePage::onButtonUp(x, y, button);
}



void ScreenEditorPage::moveCursor(int x, int y)
{
	if (m_mode == MD_ATTRIB)
	{
		if (x < 0 || y < 0 || x >= m_aw || y >= m_ah) return;

		drawCell(m_acx, m_acy);
		m_acx = x;
		m_acy = y;
		drawZoom();
	}
	else
	{
		if (x < 0 || y < 0 || x >= m_w || y >= m_h) return;
		drawPixel(m_pcx, m_pcy);
		m_pcx = x;
		m_pcy = y;
		drawZoom();
	}
	drawCursor();	
}

void ScreenEditorPage::drawZoom(void)
{
	int x, y;
	int x1, y1;
	if (m_mode == MD_ATTRIB) { x = m_acx;     y = m_acy;     }
	else			 { x = m_pcx / 8; y = m_pcy / 8; } 

	VideoBitmap *bm = newVideoBitmap(128, 128);
	for (x1 = 0; x1 < 8; x1++) for (y1 = 0; y1 < 8; y1++)	
	{
		int pixel = m_bmAttr1->getSurface()->pointZx(16 * x + 2*x1, 
							     16 * y + 2*y1);

		bm->getSurface()->fillBox(16 * x1, 16 * y1, 16, 16, pixel);
	}
	drawZoomCursor(bm->getSurface());
	bm->toScreen(m_lx, m_ly);
	delete bm;
}

void ScreenEditorPage::drawZoomCursor(VideoSurface *s)
{
	
	if (m_mode != MD_ATTRIB)
	{
		int t = m_tcount % 40;
		int ci, x1, y1;

	        if (t >= 20) ci = ZX_BRBLUE;
		else         ci = ZX_BRWHITE;

		x1 = (m_pcx % 8); y1 = (m_pcy % 8);
                s->box(m_lx + 16 * x1, m_ly + 16 * y1, 16, 16, ci);
	}
}

int ScreenEditorPage::onHotspot(int id)
{
	int oa = m_attrib;

	switch(id)
	{
		case 'I':
			m_attrib &= (~7);
			m_attrib |= (oa + 1) & 7;
			showAttributes();
			break;
		case 'P':
			m_attrib &= (~0x38);
			m_attrib |= (oa + 8) & 0x38;
			showAttributes();
			break;
		case 'B': 
			m_attrib ^= 0x40;
			showAttributes();
			break;
		case 'F':
			m_attrib ^= 0x80;
			showAttributes();
			break;
		case 'M': return changeMode();
		case 'G': return changeGrid();
		case '<': return onPrev();
		case '>': return onNext();
	}
	return ZXE_CONTINUE;
}


jswByte* ScreenEditorPage::lookupPixel(int x, int y, jswByte *mask)
{
	switch(m_format)
	{
		case FMT_ROSCRN:
                case FMT_SCREEN:
                *mask = 0x80 >> (x & 7);

                return m_bits + (x / 8) + ((y & (~63)) * 32) + ((y & 56)*4) 
				+ (y & 7) * 256;

                case FMT_SPRITE:
                *mask = 0x80 >> (x & 7);
                return m_bits + (y * ((m_w + 7)/8)) + (x/8);

		case FMT_XBM:
		*mask = 1 << (x & 7);
		return m_bits + (y * ((m_w + 7)/8)) + (x/8);

	}
	return NULL;
}


bool ScreenEditorPage::getPixel(int x, int y)
{
	jswByte mask;
	jswByte *b   = lookupPixel(x, y, &mask);

	return ((*b) & mask) ? true : false;
}


void ScreenEditorPage::setPixel(int x, int y, bool draw)
{
	// Read-only bitmaps
	if (m_format == FMT_XBM || m_format == FMT_ROSCRN) return;
	
	jswByte mask;
        jswByte *b   = lookupPixel(x, y, &mask);

	if (draw) *b |= mask;
	else	  *b &= ~mask;	

	m_bmData->getSurface()->fillBox(x*2,y*2, 2, 2, 
				(draw ? ZX_ORANGE : ZX_BRORANGE));

	if (m_attrs) mask = m_attrs[(y/8)*m_aw + (x/8)];
	else         mask = 0x46;

        //
        //        // If there is an attribute grid, then the colours are fixed

	if (m_grid == GR_ATTR) 
        {
        	if ((x ^ y) & 8) mask = GRID_C1;
        	else             mask = GRID_C2;
	}
        jswByte fg = (mask & 7), bg = (mask >> 3) & 7;

        if (mask & 64) { fg |= ZX_BRIGHT; bg |= ZX_BRIGHT; }

        VideoBitmap *bm = (m_grid == GR_PIXEL) ? m_bmPixGrid : m_bmData;

        bm->toSurface(m_bmAttr1->getSurface(), x*2, y*2, x*2, y*2, 2, 2);
        bm->toSurface(m_bmAttr2->getSurface(), x*2, y*2, x*2, y*2, 2, 2);

        m_bmAttr1->getSurface()->fillReplace(x*2,y*2,2,2,ZX_ORANGE,  fg);
        m_bmAttr1->getSurface()->fillReplace(x*2,y*2,2,2,ZX_BRORANGE,bg);

        if (mask & 128) { mask = fg; fg = bg; bg = mask; }

        m_bmAttr2->getSurface()->fillReplace(x*2,y*2,2,2,ZX_ORANGE,  fg);
        m_bmAttr2->getSurface()->fillReplace(x*2,y*2,2,2,ZX_BRORANGE,bg);

	drawPixel(x, y);
	drawZoom();
}


int ScreenEditorPage::changeMode(void)
{
	if (m_mode == MD_BITMAP && m_attrs) 
	{
		m_mode = MD_ATTRIB;
		showMode();
		return ZXE_CONTINUE;
	}
	if (m_mode == MD_ATTRIB && m_format != FMT_XBM && m_format != FMT_ROSCRN)
	{
		m_mode = MD_BITMAP;
		showMode();
		return ZXE_CONTINUE;
	}
	return ZXE_CONTINUE;
}

int ScreenEditorPage::changeGrid(void)
{
	int og = m_grid;

	switch(m_grid)
	{
		case GR_NONE:
			if (m_attrs) m_grid = GR_PIXEL;
			else if (m_format != FMT_XBM && m_format != FMT_ROSCRN)
				     m_grid = GR_ATTR;
			break;
		case GR_PIXEL:
			if (m_format != FMT_XBM && m_format != FMT_ROSCRN)
				m_grid = GR_ATTR;
			else	m_grid = GR_NONE;
			break;
		case GR_ATTR:
			m_grid = GR_NONE;
			break;
	}
	if (m_grid != og)
	{
		setup(false);
		drawData(0);		
	}
	return ZXE_CONTINUE;
}


int ScreenEditorPage::onUndo(void)
{
	int n;
	jswByte s;

	if (m_format != FMT_XBM && m_format != FMT_ROSCRN) for (n = (m_w * m_h / 8) - 1; n >= 0; n--)
	{
		s = m_bits[n];
		m_bits[n] = m_undoBits[n];
		m_undoBits[n] = s;
	}
	if (m_attrs) for (n = m_aw * m_ah - 1; n >= 0; n--)
	{
		s = m_attrs[n];
		m_attrs[n] = m_undoAttrs[n];
		m_undoAttrs[n] = s;
	}
	setup(false);
	drawData(0);
	return ZXE_CONTINUE;
}





int ScreenEditorPage::doExport()
{
	VideoMenu vm("Export format ", "Screen$", "PNG", "Cancel", NULL);

	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch (vm.getSelected())
	{
		case 0: return exportScreen();
		case 1: return exportPNG();
	}
	return ZXE_CONTINUE;
}

static unsigned char scrHeader[128] = 
{
	 'P',  'L',  'U',  'S',  '3',  'D',  'O',  'S',
	0x1A, 0x01, 0x00, 0x80, 0x1B, 0x00, 0x00, 0x03,
	0x00, 0x1B, 0x00, 0x40
};

int ScreenEditorPage::exportScreen()
{
        char filename[PATH_MAX];
        JswFileForm ff;
        int rv;
	int x, y;
	jswByte *row;
	unsigned char scrBuf[0x1B80];

        filename[0] = 0;
        rv = ff.doModal(filename);
        if (rv != ZXE_OK) return rv;

	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

		if ( (rv = vm.doModal()) != ZXE_OK) return ZXE_CONTINUE;
		if (vm.getSelected()) return ZXE_CONTINUE;
	}
	// Generate screen bitmap
	memset(scrBuf, 0, sizeof(scrBuf));
	for (y = 0; y < 192; y++)
	{
		row = scrBuf + 0x80 + ((y & 0xC0) * 0x20) + 
			              ((y & 0x38) * 0x04) +
			              ((y & 0x07) * 0x100);
		for (x = 0; x < 256; x += 8)
		{
			bool pix = false;
			jswByte m2;
		        jswByte *b   = lookupPixel(x, y, &m2);
		
			row[0] = *b;	
			++row;
		}
	}
	// Attributes
	for (y = 0; y < 24; y++)
	{
		row = scrBuf + 0x1880 + 32 * y;
		for (x = 0; x < 32; x++)
		{
			if (y < m_ah && x < m_aw)
			{
       				row[x] = m_attrs[y*m_aw + x];
			}
			else	row[x] = 0x38;
		}
	}	
	// +3DOS header
	memcpy(scrBuf, scrHeader, 128);
	for (x = 0; x < 127; x++)
	{
		scrBuf[127] += scrBuf[x];
	}
	fp = fopen(filename, "wb");
	if (fp)
	{
		if ((fwrite(scrBuf, 1, 0x1B80, fp) < 0x1B80) ||
		    (fclose(fp) == EOF))
		{
			alert("Write failed", "Cancel");
		}
	}
	else
	{
		alert("File open failed", "Cancel");
	}
	return ZXE_CONTINUE;
}

// Define png_jumpbuf() in case we are using a pre-1.0.6 version of libpng 
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) png_ptr->jmpbuf
#endif

static void se_png_error(png_structp png_ptr, png_const_charp error_msg)
{
	alert((char *)error_msg, "Cancel");
	longjmp(png_ptr->jmpbuf, 1);
}

static void se_png_warn(png_structp png_ptr, png_const_charp warning_msg)
{
	fprintf(stderr, "LibPNG warning: %s\n", warning_msg);
}


extern int gl_b[], gl_r[], gl_g[];




int ScreenEditorPage::exportPNG()
{
        char filename[PATH_MAX];
        JswFileForm ff;
        int rv;
	int x, y, n;
	png_byte  rows[128 * 192];
	png_bytep rowPtr[192];
	jswByte mask;
	png_color pngPalette[16];

	memset(rows, 0, sizeof(rows));
	for (y = 0; y < 192; y++)
	{
		rowPtr[y] = &rows[128 * y];
	}
	for (n = 0; n < 16; n++)
	{
		pngPalette[n].red   = gl_r[n];
		pngPalette[n].green = gl_g[n];
		pngPalette[n].blue  = gl_b[n];
	}

	png_structp pPng  = NULL;
      	png_infop   pInfo = NULL; 

	pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp)this, se_png_error, se_png_warn);
	if (pPng)
	{
		pInfo = png_create_info_struct(pPng);
	}
	if (!pPng || !pInfo)
	{
		if (pPng) png_destroy_write_struct(&pPng, NULL);
		rv = alert("Out of memory", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;	
	}
	if (setjmp(png_jmpbuf(pPng)))
	{
		png_destroy_write_struct(&pPng, &pInfo);
		return ZXE_CONTINUE;	
	}
        filename[0] = 0;
        rv = ff.doModal(filename);
        if (rv != ZXE_OK) 
	{
		png_destroy_write_struct(&pPng, &pInfo);
		return rv;
	}

	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

		if ( (rv = vm.doModal()) != ZXE_OK) 
		{
			png_destroy_write_struct(&pPng, &pInfo);
			return ZXE_CONTINUE;
		}
		if (vm.getSelected()) 
		{
			png_destroy_write_struct(&pPng, &pInfo);
			return ZXE_CONTINUE;
		}
	}
	fp = fopen(filename, "wb");
	if (!fp)
	{
		png_destroy_write_struct(&pPng, &pInfo);
		rv = alert("File create failed", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;	
	}
	// Let's go.
	png_init_io(pPng, fp);
	png_set_IHDR(pPng, pInfo, m_w, m_h, 4, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
	png_set_PLTE(pPng, pInfo, pngPalette, 16);

	// Generate the image.
	for (y = 0; y < 192; y++)
	{
		mask = 0x80;
		for (x = 0; x < 256; x++)
		{
			bool pix = false;
			if (x < m_w && y < m_h) pix = getPixel(x, y);
		
			jswByte attr = m_attrs[(y/8)*m_aw + (x/8)];
			
			if (pix) attr = (attr & 7) | ((attr & 0x40) >> 3);
			else	 attr = (attr & 0x78) >> 3;

			if (x & 1) 
			{
				rowPtr[y][x/2] |= (attr & 0x0F);
			}
			else
			{
				rowPtr[y][x/2] |= ((attr & 0x0F) << 4);
			}
		}
	}
	png_set_rows(pPng, pInfo, rowPtr);
	png_write_png(pPng, pInfo, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&pPng, &pInfo);
	fclose(fp);

	return ZXE_CONTINUE;
}

int ScreenEditorPage::doImport()
{
        char filename[PATH_MAX];
        JswFileForm ff;
	int rv, n;
	FILE *fp;
	jswByte magic[128], sum;

        filename[0] = 0;
	rv = ff.doModal(filename);

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK)   return ZXE_CONTINUE;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		rv = alert("Could not open file", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;
	}
	// Work out what we have here
	memset(magic, 0, sizeof(magic));
	fread(magic, 1, sizeof(magic), fp);
	if (!png_sig_cmp(magic, 0, 8))	// Importing PNG
	{
		fclose(fp);
		return importImage(filename);
	}
	if (!memcmp(magic, "BM", 2))	// Importing BMP
	{
		fclose(fp);
		return importImage(filename);
	}
	if (!memcmp(magic, "\377\330\377", 3)) // Importing JPG
	{
		fclose(fp);
		return importImage(filename);
	}
	if (!memcmp(magic, scrHeader, 14))
	{
		sum = 0;
		for (n = 0; n < 127; n++)
		{
			sum += magic[n];
		}
		if (sum == magic[n]) /* PLUS3DOS magic found */
		{
			return importScreen(fp);
		}
	}
	// No magic number. Treat as raw Spectrum screen.
	fseek(fp, 0, SEEK_SET);
	return importScreen(fp);
}



int ScreenEditorPage::importScreen(FILE *fp)
{
	int y,x;
	jswByte *row;
	jswByte scrBuf[6912];

	memset(scrBuf, 0, sizeof(scrBuf));
	fread(scrBuf, 1, sizeof(scrBuf), fp);
	fclose(fp);

	// Bitmap
	if (m_format != FMT_XBM && m_format != FMT_ROSCRN)
	{
		for (y = 0; y < 192; y++)
		{
			if (y >= m_h) break;
	
			row = scrBuf + ((y & 0xC0) * 0x20) + 
				       ((y & 0x38) * 0x04) +
				       ((y & 0x07) * 0x100);
			// For speed, copy 8 bits at a time.
			for (x = 0; x < 256; x+= 8)
			{
				if (x < m_w)
				{
					jswByte m2;
				        jswByte *b   = lookupPixel(x, y, &m2);
					*b = row[0];
				}	
				++row;
			}
		}
	}
	// Attributes
	for (y = 0; y < 24; y++)
	{
		row = scrBuf + 0x1800 + 32 * y;
		for (x = 0; x < 32; x++)
		{
			if (y < m_ah && x < m_aw)
			{
       				m_attrs[y*m_aw + x] = row[x];
			}
		}
	}	
	// Redraw
	setup(false);
	return ZXE_CONTINUE;
}

void ScreenEditorPage::importCell(int x, int y, SDL_Surface *s0)
{
	int x1, y1, n, ink, paper;
	int zxc[8][8];
	int frequency[16];
	Uint8 r,g,b;

	memset(frequency, 0, sizeof(frequency));
	// Initialise the colour array
	for (y1 = 0; y1 < 8; y1++)
		for (x1 = 0; x1 < 8; x1++)
			zxc[y1][x1] = -1;


	for (y1 = 0; y1 < 8; y1++)
	{
		if (y + y1 >= s0->h) break;
		for (x1 = 0; x1 < 8; x1++)
		{
			if (x + x1 >= s0->w) break;

			jswByte *pixel = (jswByte *)(s0->pixels);

			pixel += ((y + y1) * s0->pitch);
			pixel += ((x + x1) * s0->format->BytesPerPixel);
			Uint32 pixval = 0;
			switch(s0->format->BitsPerPixel)
			{
				case 8:  pixval = *(Uint8  *)pixel; break;
				case 16: pixval = *(Uint16 *)pixel; break;
				// XXX 24-bit pixels are nassty
				case 24: pixval = *(Uint32 *)pixel; break;
				case 32: pixval = *(Uint32 *)pixel; break;
			}
			SDL_GetRGB(pixval, s0->format, &r, &g, &b);
			zxc[y1][x1] = 0;
			if (b >= 192) zxc[y1][x1] |= 1;
			if (r >= 192) zxc[y1][x1] |= 2;
			if (g >= 192) zxc[y1][x1] |= 4;
			if (b >= 224) zxc[y1][x1] |= 8;
			if (r >= 224) zxc[y1][x1] |= 8;
			if (g >= 224) zxc[y1][x1] |= 8;
			frequency[zxc[y1][x1]]++;
		}
	}
	int max = 0;

	// Work out ink and paper
	for (n = 0; n < 16; n++)
	{
		if (frequency[n] > max) 
		{
			max = frequency[n];	
			ink = n;
		}
	}
	max = paper = 0;
	for (n = 0; n < 16; n++)
	{
		if (frequency[n] > max && n != ink) 
		{
			max = frequency[n];	
			paper = n;
		}
	}
	// OK. We have ink and paper.
	jswByte attr = (ink & 7) | ((paper & 7) << 3);
	if ((ink & 8) | (paper & 8)) attr |= 0x40;

	int ay = y / 8;
	int ax = x / 8;
	if (ay < m_ah && ax < m_aw)
	{
       		m_attrs[ay*m_aw + ax] = attr;
	}
	// If bitmaps can't be set, don't set them.
	if (m_format == FMT_XBM || m_format == FMT_ROSCRN) return;
	// Now plot the dots
	for (y1 = 0; y1 < 8; y1++)
	{
		if (y + y1 >= m_h) break;

		jswByte mask = 0x80;
	        jswByte *b   = lookupPixel(x, y + y1, &mask);
		for (x1 = 0; x1 < 8; x1++)
		{
			if (x + x1 >= m_w) break;
		
			if      (zxc[y1][x1] == -1)    (*b) &= ~mask;	
			else if (zxc[y1][x1] == ink)   (*b) |= mask;
			else if (zxc[y1][x1] == paper) (*b) &= ~mask;
			else
			{
				// OK. We've got another colour. 
				// Is it closer to ink or paper?
				int di = (ink & 7) - (zxc[y1][x1] & 7); 
				if (di < 0) di = -di;
				int dp = (paper & 7) - (zxc[y1][x1] & 7);
				if (dp < 0) dp = -dp;
				if (di <= dp) (*b) |= mask;
				else	      (*b) &= ~mask;
			}
			mask = mask >> 1;	
		}
	}
}

int ScreenEditorPage::importImage(char *filename)
{
        SDL_Surface *s0 = IMG_Load(filename);
	int rv;
	int x, y;

	if (!s0)
	{
		rv = alert("Failed to load image", "Cancel");
		if (rv >= ZXE_QUIT) return rv;
		return ZXE_CONTINUE;	
	}
	int imgw = s0->w;
	int imgh = s0->h;

	for (y = 0; y < imgh; y += 8)
	{
		for (x = 0; x < imgw; x += 8)
		{
			importCell(x, y, s0);
		}
	}
	SDL_FreeSurface(s0);
	// Redraw
	setup(false);
	return ZXE_CONTINUE;
}
