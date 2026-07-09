/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

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

class VideoForm : public KeyListener,
                  public ButtonListener,
                  public MouseListener

{
public:
	VideoForm(int x, int y, int w, int h);
	virtual ~VideoForm();
	virtual int doModal();
// Event routing
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
	virtual int onButtonUp  (int x, int y, int button);
	virtual int onButtonDown(int x, int y, int button);
        virtual int onKeyUp  (int keysym);
        virtual int onKeyDown(int keysym);
protected:
	virtual void showChildren(int redraw = 1);
	virtual void hideChildren(void);
	virtual void redraw(void);
	void redrawBorder(char *title);
	VideoControl *getFocus();
	int  setFocus(VideoControl *child);
	void drawFocus(void);
	int addChild(VideoControl *child);
	int removeChild(VideoControl *child);
	int m_x, m_y, m_w, m_h;

protected:
	PVCONTROL *m_controls;
	int	   m_ccount, m_cmax;
	int	   m_focus;
};


