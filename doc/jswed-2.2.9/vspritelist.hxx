/************************************************************************

    JSWED 2.00 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001  John Elliott <jce@seasip.demon.co.uk>

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

/* Up/Down scrolling list box for sprites */

class SpectrumMemory;

class VSpriteList : public ScrollingList
{
public:
	VSpriteList(SpectrumMemory *mem);
	virtual ~VSpriteList();

	void addSprite(int id);
	void setSelected(int id);
	int  getSelected(void);
protected:
	SpectrumMemory *m_mem;

	// Draw an item. Note that index may be greater than the
	// number of items present in the control; in this case,
	// draw a blank (so to speak).
	// background is white/cyan, depending on selection.
	virtual void drawItem(int x, int y, int index, int background);

	// Draw the caption of this control.
	virtual void drawTitle(int x, int y);

	
};

