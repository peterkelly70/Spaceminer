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

class VideoNotebook : public KeyListener, 
		      public ButtonListener,
                      public MouseListener
{
private:
	PVPAGE	*m_pages;
	int m_pcount, m_pmax;
	int m_x, m_y, m_w, m_h, m_pw;
	int m_active;
	char *m_title;
protected:
	inline PVPAGE *getPages() { return m_pages; }
	inline PVPAGE getActive() { return m_pages[m_active]; }
	void draw(void);
	void drawFixed(void);
public:	
	void setTitle(char *s);
	inline char *getTitle(void) { return m_title; }

	VideoNotebook(int x, int y, int w, int h);
	virtual ~VideoNotebook();		

	int addPage(VideoNotePage *p);
	void selectPage(int n);
	int doModal();

	// Events
        virtual int onButtonDown(int x, int y, int button);
	virtual int onButtonUp(int x, int y, int button);
        virtual int onKeyDown(int keysym);
        virtual int onMouseMove(int x, int y, int xrel, int yrel);
};
