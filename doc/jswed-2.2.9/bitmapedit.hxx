/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005  John Elliott <jce@seasip.demon.co.uk>

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

class BitmapEditListener;


class BitmapEditor: public VideoControl, TickListener
{
public:
	BitmapEditor(int x, int y, int w, int h, int previewBelow = 0);
	virtual ~BitmapEditor();
	void setBitmap(jswByte *bitmap);	
	jswByte *getBitmap(void);
	virtual void drawFocus(int focused);
	virtual void redraw(void);
	void setForeground(int fg);
	void setBackground(int bg);
	void setAttribute(jswByte attr);

	void drawCell(VideoSurface *s, int x0, int y0,
				int cx, int cy, int fg = -1, int bg = -1);
	void drawCursor(VideoSurface *s, int x0, int y0, int draw = 1);
	void togglePixel(int x, int y);
	void setPixel   (int x, int y, int set, int draw = 1);
	int  getPixel	(int x, int y);

	int doContextMenu(void);
	void flipH(void);
	void flipV(void);
	void moveU(void);
	void moveD(void);
	void moveL(void);
	void moveR(void);
	void rotate90(void);
	void invert(void);
	void zap(void);
	void clipStore(void);
	void clipSwap(void);
	void clipPaste(void);
	// Overrides
	virtual void onTick(void);	
	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
	virtual int onButtonUp(int x, int y, int button);
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
	inline void setListener(BitmapEditListener *e) { m_listener = e; }
	inline BitmapEditListener *getListener() { return m_listener; }
protected:
	jswByte *m_bitmap;
	BitmapEditListener *m_listener;
	int	m_bw, m_bh;
	int 	m_fg, m_bg, m_surround;
	int	m_cx, m_cy;
        TickListener *m_ot;
        int m_haveTimer;
	int m_tcount;
	int m_dragging;
	int m_inContextMenu;
	int m_previewBelow;

	int doReflectMenu(void);
	int doRotateMenu(void);
	int doMoveMenu(void);
	int doClipMenu(void);
};

class BitmapEditListener
{
public:
	virtual void onBitmapUpdate(BitmapEditor *caller, int x, int y, int pixel) = 0;
};

