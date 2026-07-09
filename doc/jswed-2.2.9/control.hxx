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

class VideoControl : public KeyListener,
                     public ButtonListener,
                     public MouseListener

{
	friend class VideoNotePage;

public:
	VideoControl(int x, int y, int w, int h);
	virtual ~VideoControl();
	int hitTest(int x, int y);
	virtual void drawFocus(int focused);
	virtual void redraw(void) = 0;
	virtual int canFocus();
	void setVisible(int visible = 1, int redraw = 1);
	int  getVisible(void);	
	inline int getX() { return m_x; }
        inline int getY() { return m_y; }
        inline int getW() { return m_w; }
        inline int getH() { return m_h; }

protected:
	int m_x, m_y, m_w, m_h;
	int m_focused;
	int m_visible;
};

typedef VideoControl *PVCONTROL;

