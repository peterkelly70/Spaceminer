/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005-6  John Elliott <jce@seasip.demon.co.uk>

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

static int gl_clipW = 0, gl_clipH = 0;
static jswByte *gl_clipData = NULL;

BitmapEditor::BitmapEditor(int x, int y, int w, int h, int previewBelow) :
		VideoControl(x, y, (previewBelow ? 16 : 20) * w + 4, 
			           (previewBelow ? 20 : 16) * h + 4)
{
	m_bw = w;
	m_bh = h;
	m_fg = ZX_WHITE;
	m_bg = ZX_BLACK;
	m_surround = ZX_BLACK;
	m_bitmap = NULL;
	m_ot = NULL;
	m_haveTimer = 0;
	m_cx = m_cy = 0;
	m_dragging = 0;
	m_inContextMenu = 0;
	m_previewBelow = previewBelow;
	m_listener = NULL;
	m_tcount = 0;
}

BitmapEditor::~BitmapEditor()
{
	if (m_haveTimer) setTickListener(m_ot);
}

void BitmapEditor::setForeground(int fg)
{
	if (fg != m_fg)
	{
		m_fg = fg;
		redraw();
	}
}

void BitmapEditor::setBackground(int bg)
{
	if (bg != m_bg)
	{
		m_bg = bg;
		redraw();
	}
}

void BitmapEditor::setAttribute(jswByte b)
{
	int fg = (b & 7);
	int bg = (b & 0x38) >> 3;

	if (b & 0x40) 
	{
		fg |= 8;
		bg |= 8;
	}
	if (fg != m_fg || bg != m_bg)
	{
		m_fg = fg;
		m_bg = bg;
		redraw();
	}
}


jswByte *BitmapEditor::getBitmap()
{
	return m_bitmap;
}

void BitmapEditor::setBitmap(jswByte *bitmap)
{
	m_bitmap = bitmap;
	redraw();
}

void BitmapEditor::drawFocus(int focused)
{
	if (focused)
        {
                if (!m_haveTimer)
                {
                        m_ot = setTickListener(this);
                        m_tcount = 0;
                        m_haveTimer = 1;
                }
        }
        else if (m_haveTimer)
        {
                setTickListener(m_ot);
                m_ot = NULL;
                m_haveTimer = 0;
        }


        VideoControl::drawFocus(focused);
        drawCursor(videoScreen, m_x, m_y);
}

void BitmapEditor::onTick(void)
{
        ++m_tcount;
        if (!m_inContextMenu) drawCursor(videoScreen, m_x, m_y);
	if (m_ot) m_ot->onTick();
}

void BitmapEditor::redraw()
{
	if (!m_visible) return;

	VideoBitmap *bmp = newVideoBitmap(m_w, m_h);
	VideoSurface *s  = bmp->getSurface();

	s->fillBox(0, 0, m_w, m_h, m_surround);

	for (int x = 0; x < m_bw; x++) for (int y = 0; y < m_bh; y++)
	{
		drawCell(s, 0, 0, x, y);
	}
	drawCursor(s, 0, 0);
	bmp->toScreen(m_x, m_y); //, 0, 0, m_w, m_h);
	delete bmp;
}


void BitmapEditor::drawCell(VideoSurface *s, 
			int x0, int y0, int x, int y, int fg, int bg)
{
	int sx, sy, pixel;

	if (fg == -1) fg = ZX_BLACK;
	if (bg == -1) bg = ((x ^ y) & 1) ? ZX_BRWHITE : ZX_BRYELLOW;

	pixel = getPixel(x, y);

	sx = x0 + 2 + 16 * x;
	sy = y0 + 2 + 16 * y;
	if (pixel) s->shadedBox(sx+1, sy+1, 14, 14, fg, bg);
	else	   s->fillBox  (sx+1, sy+1, 14, 14, bg);
	s->box(sx,sy,16,16,fg);

	sx = x0 + 4 + 2 * x;
	sy = y0 + 4 + 2 * y;

	if (m_previewBelow) sy += 16 * m_bh;
	else		    sx += 16 * m_bw;

	s->fillBox(sx, sy, 2, 2, pixel ? m_fg : m_bg);	
}



void BitmapEditor::drawCursor(VideoSurface *s, int x0, int y0, int draw)
{
        int cfg = ZX_BRBLUE;
        int cbg = ZX_BRWHITE;

	if (!m_visible) return;

        if (m_tcount & 16)       { int a = cfg; cfg = cbg; cbg = a; }
        if (!m_focused || !draw) 
	{ 
		cfg = cbg = -1;
	}
	
	drawCell(s, x0, y0, m_cx, m_cy, cfg, cbg);
}


int BitmapEditor::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_EDIT:
			return doContextMenu();

		case ZXK_LEFT:
			if (m_cx > 0)
			{
				drawCell(videoScreen, m_x, m_y, m_cx, m_cy);
				--m_cx;
				drawCursor(videoScreen, m_x, m_y);
			}
			return ZXE_CONTINUE;

		case ZXK_RIGHT:
                        if (m_cx < m_bw - 1)
                        {
                                drawCell(videoScreen, m_x, m_y, m_cx, m_cy);
                                ++m_cx;
                                drawCursor(videoScreen, m_x, m_y);
                        }
                        return ZXE_CONTINUE;
                case ZXK_UP:
                        if (m_cy > 0)
                        {
                                drawCell(videoScreen, m_x, m_y, m_cx, m_cy);
                                --m_cy;
                                drawCursor(videoScreen, m_x, m_y);
                        }
                        return ZXE_CONTINUE;

                case ZXK_DOWN:
                        if (m_cy < m_bh - 1)
                        {
                                drawCell(videoScreen,m_x, m_y, m_cx, m_cy);
                                ++m_cy;
                                drawCursor(videoScreen, m_x, m_y);
                        }
                        return ZXE_CONTINUE;
		case ' ':
			togglePixel(m_cx, m_cy);
			return ZXE_CONTINUE;

	}
	return KeyListener::onKeyDown(keysym);
}

int BitmapEditor::onButtonDown(int x, int y, int button)
{
        if (button == gl_rbutton) return doContextMenu();

        int x1 = (x - m_x - 2) / 16;
        int y1 = (y - m_y - 2) / 16;

        if (x1 >=0 && x1 < m_bw && y1 >= 0 && y1 < m_bh)
        {
                drawCell(videoScreen, m_x, m_y, m_cx, m_cy);
                m_cx = x1;
                m_cy = y1;
		togglePixel(m_cx, m_cy);
		m_dragging = 1 + getPixel(m_cx, m_cy);
                return ZXE_CONTINUE;
        }
	return ButtonListener::onButtonDown(x,y,button);
}

int BitmapEditor::onButtonUp(int x, int y, int button)
{
	m_dragging = 0;
	return ButtonListener::onButtonUp(x,y,button);
}



int BitmapEditor::onMouseMove(int x, int y, int xrel, int yrel)
{
	int x1 = (x - m_x - 2) / 16;
	int y1 = (y - m_y - 2) / 16;

	if (x1 >=0 && x1 < m_bw && y1 >= 0 && y1 < m_bh)
	{
		drawCell(videoScreen, m_x, m_y, m_cx, m_cy);
		m_cx = x1;
		m_cy = y1;
		if (m_dragging) setPixel(x1, y1, m_dragging - 1);
		else		drawCursor(videoScreen, m_x, m_y);
		return ZXE_CONTINUE;	
	}

	return MouseListener::onMouseMove(x,y,xrel,yrel);
}


int BitmapEditor::getPixel(int x, int y)
{
	int pixel, offset;

	if (!m_bitmap) return 0;

	offset  = ((m_bw + 7) / 8) * y;
	offset += (x >> 3);

	pixel = m_bitmap[offset] & (0x80 >> (x & 7));

	return pixel ? 1 : 0;
}



void BitmapEditor::togglePixel(int x, int y)
{
	setPixel(x, y, 1 - getPixel(x, y));
}

void BitmapEditor::setPixel(int x, int y, int v, int draw)
{
	int pixel, offset;

	if (!m_bitmap) return;

	offset  = ((m_bw + 7) / 8) * y;
	offset += (x >> 3);

	pixel = (0x80 >> (x & 7));

	if (v) m_bitmap[offset] |= pixel;
	else   m_bitmap[offset] &= ~pixel;

	if (!draw) return;
	
	if (x == m_cx && y == m_cy) drawCursor(videoScreen, m_x, m_y);
	else			    drawCell(videoScreen, m_x, m_y, x, y);

	if (m_listener)
	{
		m_listener->onBitmapUpdate(this, x, y, v);
	}
}


int BitmapEditor::doContextMenu(void)
{
	VideoMenu vm("Options", "Editor",
				"Reflect",
				"Rotate",
				"Move",
				"Clipboard",
                                "Exit", NULL);

	m_inContextMenu = 1;
	int rv = vm.doModal();
	m_inContextMenu = 0;

        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
        switch (vm.getSelected())
	{
		case 0: return ZXE_CONTINUE;
	        case 1: return doReflectMenu();
		case 2: return doRotateMenu();
		case 3: return doMoveMenu();
		case 4: return doClipMenu();
		case 5: return ZXE_OK;	
        }
	return ZXE_CONTINUE;
}


int BitmapEditor::doReflectMenu(void)
{
	VideoMenu vm("Reflect", "Left/right", "Top/bottom", NULL);
        m_inContextMenu = 1;
        int rv = vm.doModal();
        m_inContextMenu = 0;
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

	if (vm.getSelected())	flipV();	// Vertical
	else			flipH();	

	return ZXE_CONTINUE;
}

int BitmapEditor::doRotateMenu(void)
{
	if (m_bw != m_bh) 
	{
		flipH();
		flipV();
		return ZXE_CONTINUE;
	}
	VideoMenu vm("Rotate", "90\242", "180\242", "270\242", NULL);
        m_inContextMenu = 1;
        int rv = vm.doModal();
        m_inContextMenu = 0;
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

        switch(vm.getSelected())
        {
		case 2: rotate90();	// Note: FALL THROUGH in all cases.
		case 1: rotate90();
		case 0:	rotate90();
	}
	redraw();
	return ZXE_CONTINUE;
}

int BitmapEditor::doMoveMenu(void)
{
        VideoMenu vm("Move", "Up", "Down", "Left", "Right", NULL);
        m_inContextMenu = 1;
        int rv = vm.doModal();
        m_inContextMenu = 0;
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: moveU(); break;
		case 1: moveD(); break;
		case 2: moveL(); break;
		case 3: moveR(); break;
	}
	redraw();
        return ZXE_CONTINUE;
}

int BitmapEditor::doClipMenu(void)
{
        VideoMenu vm("Misc", "Invert", "Clear", "Copy", "Swap", "Paste", NULL);
        m_inContextMenu = 1;
        int rv = vm.doModal();
        m_inContextMenu = 0;
        if (rv >= ZXE_QUIT) return rv;
        if (rv == ZXE_CANCEL) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: invert();    break;
		case 1: zap();       break;
		case 2: clipStore(); break;
		case 3: clipSwap();  break;
		case 4: clipPaste(); break;
	}
	return ZXE_CONTINUE;
}



void BitmapEditor::flipH(void)
{
	if (!m_bitmap) return;

        for (int y = 0; y < m_bh; y++)
        {
		for (int x = 0; x < m_bw / 2; x++)
                {
                	int pl = getPixel(x,            y);
                        int pr = getPixel(m_bw - x - 1, y);
                        setPixel(x,            y, pr, 0);
                        setPixel(m_bw - x - 1, y, pl, 0);
                }
        }
        redraw();
}


void BitmapEditor::flipV(void)
{
	if (!m_bitmap) return;

        int llb = ((m_bw + 7) / 8);
        jswByte *b   = m_bitmap;
        jswByte *b2  = m_bitmap + (llb * (m_bh - 1));

        for (int y = 0; y < m_bh / 2; y++)
        {
		for (int x = 0; x < llb; x++)
                {
                	jswByte s = b[0];
                        b[0] = b2[0];
                        b2[0] = s;
                        ++b;
                        ++b2;
                }
                b2 -= 2 * llb;
        }
        redraw();
}

void BitmapEditor::moveU(void)
{
	int y, x, llb;

	if (!m_bitmap) return;

        llb = ((m_bw + 7) / 8);
        jswByte *b   = m_bitmap;
        jswByte *b2  = m_bitmap + llb;

	for (y = 1; y < m_bh; y++)
		for (x = 0; x < llb; x++)
			*(b++) = *(b2++);
	for (x = 0; x < llb; x++) *(b++) = 0;
	redraw();
}


void BitmapEditor::moveD(void)
{
	int x, y, llb;

        if (!m_bitmap) return;

        llb = ((m_bw + 7) / 8);
        jswByte *b   = m_bitmap + (m_bh    ) * llb;
        jswByte *b2  = m_bitmap + (m_bh - 1) * llb;

        for (y = 1; y < m_bh; y++)
                for (x = 0; x < llb; x++)
                        *(--b) = *(--b2);
	for (x = 0; x < llb; x++) *(--b) = 0;
        redraw();
}

void BitmapEditor::moveL(void)
{
	if (!m_bitmap) return;
	
	int y, x;

	for (y = 0; y < m_bh; y++)
	{
		for (x = 1; x < m_bw; x++)
			setPixel(x-1, y, getPixel(x,y), 0);
		setPixel(x-1, y, 0, 0);
	}
	redraw();
}

void BitmapEditor::moveR(void)
{
        if (!m_bitmap) return;

        int y, x;

        for (y = 0; y < m_bh; y++)
        {               
                for (x = m_bw; x > 1; x--)
                        setPixel(x-1, y, getPixel(x-2,y), 0);
                setPixel(x-1, y, 0, 0);
        }
        redraw();

}


void BitmapEditor::rotate90(void)
{
	if (!m_bitmap) return;

	int x1, y1, x2, y2, x, y;
	
	for (y = 0; y < m_bh / 2; y++)
	{
		for (x = 0; x < m_bw / 2; x++)
		{
			x1 = x;
			y1 = y;
			x2 = m_bw - 1 - x;
			y2 = m_bh - 1 - y;
			int p1  = getPixel(x1, y1);
			int p2  = getPixel(y1, x2);
			int p3  = getPixel(x2, y2);
			int p4  = getPixel(y2, x1);

			setPixel(y1, x2, p1, 0);
			setPixel(x2, y2, p2, 0);
			setPixel(y2, x1, p3, 0);
			setPixel(x1, y1, p4, 0);

		}	
	}
}


void BitmapEditor::invert(void)
{
	if (!m_bitmap) return;

        int bwb = (m_bw + 7) / 8;
        jswByte *b = m_bitmap;

        for (int x = 0; x < m_bh * bwb; x++)
        {
                *b = ~(*b);
                ++b;
        }
	redraw();
}

void BitmapEditor::zap(void)
{
        int bwb = (m_bw + 7) / 8;
	if (!m_bitmap) return;

	memset(m_bitmap, 0, m_bh * bwb);
	redraw();
}

void BitmapEditor::clipStore(void)
{
        int bwb = (m_bw + 7) / 8;
        if (!m_bitmap) return;

	if (gl_clipData) delete gl_clipData;

	gl_clipData = new jswByte[m_bh * bwb];

	memcpy(gl_clipData, m_bitmap, m_bh * bwb);
	gl_clipW = m_bw;
	gl_clipH = m_bh;
}


void BitmapEditor::clipSwap(void)
{
        int bwb = (m_bw + 7) / 8;
	jswByte b;
	int n;

        if (!m_bitmap) return;

        if (gl_clipW != m_bw || gl_clipH != m_bh) return;

	for (n = 0; n < m_bh * bwb; n++)
	{
		b = gl_clipData[n];
		gl_clipData[n] = m_bitmap[n];
		m_bitmap[n] = b;
	}
	redraw();
}


void BitmapEditor::clipPaste(void)
{
        int bwb = (m_bw + 7) / 8;

        if (!m_bitmap) return;

        if (gl_clipW != m_bw || gl_clipH != m_bh) return;

	memcpy(m_bitmap, gl_clipData, m_bh * bwb);

        redraw();
}



