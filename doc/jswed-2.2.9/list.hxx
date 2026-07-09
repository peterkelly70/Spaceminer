/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-5  John Elliott <jce@seasip.demon.co.uk>

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

/* General scrolling list box */

class ScrollingList;

struct ListSelectListener
{
	virtual void onSelect(ScrollingList *sender, int nSel) = 0;
};

class ScrollingList : public VideoControl
{
public:
	ScrollingList(int x, int y, int w, int h);
	virtual ~ScrollingList();

	void copyFrom(ScrollingList *sl);
	// Overrides
	virtual int onKeyDown(int key);
        virtual int onButtonDown(int x, int y, int button);
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
	inline void setTitle(char *s) { m_title = s; }
	inline char *getTitle()       { return m_title; } 

	inline void  requireClick(int c) { m_click = c; }
	inline int   getSelected()  { return m_selected;    }
	inline void  setSelected(int s) { m_selected = s;  }
	inline int   getItemCount() { return m_itemCount;   }
	inline char *getItem(int n) { if (n >= m_itemCount) return NULL;
					return m_itemList[n]; }
	void  sortItems(int (*compar)(const void *, const void *) = NULL);
	virtual int doModal();
        virtual void addString(char *s);
	inline void setSelectAction(int s) { m_selectAction = s; }
	inline int getSelectAction()       { return m_selectAction; }

	inline void setListener(ListSelectListener *lsl) { m_listener = lsl; }
	inline ListSelectListener *getListener() { return m_listener; }
protected:
	VideoBitmap *m_bitmap, *m_types;

	ListSelectListener *m_listener;
	pChar  m_title;
	pChar *m_itemList;
	int    m_itemCount;
	int    m_itemMax;
	int    m_topRow;
	int    m_oldTopRow;
	int    m_selected;
	int    m_oldSelected;
	int    m_fcW, m_fcH;
	int    m_click;
	int    m_selectAction;
	bool   m_ctrlQ;
// Check that the selection pointer is valid.
	void checkLimits(void);
// Overridables
	virtual void drawFixed(void);
	virtual void draw(int reDraw);
	virtual void emptyBuffer(void);
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
	// Redraw the list
	virtual void redraw(void);
};

