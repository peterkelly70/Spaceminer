/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2006  John Elliott <jce@seasip.demon.co.uk>

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

#include "slistener.hxx"

class LetterChooser : public VideoControl
{
public:
        LetterChooser(int x, int y, int w, int h, jswByte *font, int count);
        ~LetterChooser();

        virtual void redraw(void);
        virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);

	inline void setListener(SpriteListListener *l) { m_listener = l;    }
	inline SpriteListListener *getListener(void)   { return m_listener; }

	void redrawSelection(void);

protected:
	void setSelected(int nSel);
	void getCharPos(int ch, int *x, int *y);

        jswByte *m_font;
        int m_count;
        int m_selected;
	SpriteListListener *m_listener;
};


