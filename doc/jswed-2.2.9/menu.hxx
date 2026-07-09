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

class VideoMenu : public KeyListener,
		  public ButtonListener,
		  public MouseListener 
{
// Data
protected:
        VideoBitmap *m_bitmap;  // Bitmap behind the menu
        pChar *m_options;       // Options
        pChar m_title;          // Title
        int m_x, m_y, m_w, m_h; // Menu area
        int m_count;            // Number of options
        int m_selected;         // Current option

public:
	VideoMenu(char *s, ...);
	VideoMenu(int count, char *title, char **options);
	void construct(int count, char *title, char **options);
	virtual ~VideoMenu();

	virtual int doModal();
	inline int getSelected() { return m_selected; }
	void setSelected(int);

	// Events
        virtual int onButtonDown(int x, int y, int button);
        virtual int onKeyDown(int keysym);
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
protected:
	virtual void draw();
	virtual void drawFixed();
};


