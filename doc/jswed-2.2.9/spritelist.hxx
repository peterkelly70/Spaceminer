/************************************************************************

    JSWED 2.2.2 - Editor for Jet Set Willy and derivatives

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

#ifndef SPRITELIST_HXX_INCLUDED
#define SPRITELIST_HXX_INCLUDED 

/* Side-scrolling list box for sprites */

class SpectrumMemory;

#include "slistener.hxx"

struct SpriteEntry
{
	int zxAddress;
	jswByte *bitmap;
};

class SpriteList : public VideoControl
{
private:	
	void construct(SpectrumMemory *mem);
public:
	SpriteList(SpectrumMemory *mem, int x, int y, int w, int h);
	SpriteList(SpriteList *other, int x, int y, int w, int h);
	virtual ~SpriteList();

	inline void setListener(SpriteListListener *l) { m_listener = l; }
	inline SpriteListListener *getListener() { return m_listener; }

	// Overrides
	virtual int onKeyDown(int key);
        virtual int onButtonDown(int x, int y, int button);
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
	inline void setTitle(char *s) { m_title = s; }
	inline char *getTitle()       { return m_title; } 

	inline void setSelectAction(int s) { m_selectAction = s; }
	inline int getSelectAction()       { return m_selectAction; }

	inline int   getItemCount() { return m_itemCount;   }
	inline SpriteEntry getItem(int n) { return m_itemList[n]; }
	void  sortItems(int (*compar)(const void *, const void *) = NULL);
	virtual int doModal();
	void addSprite(int address);
	void addSprite(int address, jswByte *realImage);
	void emptyBuffer(void);
	virtual void redraw(void);
	void setSelected(int address);
	int getSelected(jswByte **bitmap = NULL);
protected:
	VideoBitmap *m_bitmap;
	SpectrumMemory *m_mem;
	SpriteListListener *m_listener;

	pChar  m_title;
	SpriteEntry *m_itemList;
	int    m_itemCount;
	int    m_itemMax;
	int    m_topRow;
	int    m_oldTopRow;
	int    m_selected;
	int    m_selectAction;
	int    m_oldSelected;
	int    m_oldNotified;
	int    m_fcW, m_fcH;
// Check that the selection pointer is valid.
	void checkLimits(void);
	void notify(void);
// Overridables
	virtual void drawFixed(void);
	virtual void draw(int reDraw);
	// Called when an item is selected. Return ZXE_CONTINUE if the
	// item is disabled, or the listbox is reloaded, etc.
	virtual int  onSelect(void);

	// Draw an item. Note that index may be greater than the
	// number of items present in the control; in this case,
	// draw a blank (so to speak).
	// background is white/cyan, depending on selection.
	virtual void drawItem(int x, int y, int index, int background);

	// Draw the caption of this control.
	virtual void drawTitle(int x, int y);

};

#endif // ndef SPRITELIST_HXX_INCLUDED
