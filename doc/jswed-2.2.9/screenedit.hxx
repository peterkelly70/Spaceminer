/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004 John Elliott <jce@seasip.demon.co.uk>

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

#include "hotspotmgr.hxx"

#define FMT_SPRITE 0
#define FMT_XBM    1
#define FMT_SCREEN 2
#define FMT_ROSCRN 3

struct SDL_Surface;

class ScreenEditorPage : public VideoNotePage, 
                         public TickListener, 
                         public HotspotManager
{
protected:
        VideoBitmap   *m_bmData, *m_bmAttr1, *m_bmAttr2, *m_bmTick,
		      *m_bmPencil, *m_bmColours, *m_bmCgrid, *m_bmPgrid,
		      *m_bmPixGrid;
	jswByte       *m_bits, *m_attrs, *m_undoBits, *m_undoAttrs;

	bool m_bDraw;
	int  m_buttonDown;
	jswByte m_attrib;
	int  m_w, m_h, m_aw, m_ah, m_format, m_acx, m_acy, m_visible;
	int  m_tcount, m_haveTimer, m_mode, m_pcx, m_pcy, m_grid;
	TickListener *m_ot;
	void intPreContextMenu();
	void setup(bool withUndo = true);
	void deleteBitmaps(bool withUndo = true);
	void setCellAttribute(int x, int y, jswByte attr);
	void togglePixel(int x, int y);
	void setPixel(int x, int y, bool ink);
	bool getPixel(int x, int y);
	void drawPixel(int x, int y);
	void drawData(int tickOnly);
	void drawCursor(void);
	void drawCell(int x, int y);
	void moveCursor(int x, int y);
	void showAttributes(void);
	void showMode(void);
	void showGrid(void);
	void drawZoom(void);
	void drawZoomCursor(VideoSurface *s);
	void onDraw(void);
	jswByte *lookupPixel(int x, int y, jswByte *mask);
	virtual int changeMode(void);
	virtual int changeGrid(void);
	virtual int onUndo(void);
	virtual int onNext(void) = 0;
	virtual int onPrev(void) = 0;	

	int doImport(void);
	int doExport(void);

	int importScreen(FILE *fp);
	void importCell(int x, int y, SDL_Surface *s0);
	int importImage(char *filename);

	int exportScreen(void);
	int exportPNG(void);
public:
	ScreenEditorPage();
	virtual ~ScreenEditorPage();
// Events
	virtual void onTick(void);
	virtual int  onKeyDown(int keysym);	
	virtual int  onMouseMove(int x, int y, int xrel, int yrel);
	virtual int  onButtonDown(int x, int y, int button);
	virtual int  onButtonUp  (int x, int y, int button);
// Overrides
        virtual int redraw(int whichRectangle);
	virtual int preContextMenu(void);
	virtual void postContextMenu(void);
	virtual int onReveal(int whichRectangle);
	virtual int onConceal(void);
	virtual int onHotspot(int id);
};


