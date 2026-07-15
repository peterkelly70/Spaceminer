/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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

class JswGame;

class Jsw128MemPage : public VideoNotePage
{
protected:
	void drawFixed(void);
	void drawData(void);
	void drawDatum(int x, int y, int datum);
	void drawFocus(int draw = 1);
	void getXY(int n, int *x, int *y);
	int  getN(int x, int y);
	void toggle(int n);
	bool guardsPossible(int n);
	int  guardTableId(void);
	int m_focus;
	int m_gpages;
	jswByte *m_map;
	JswGame *m_game;
	unsigned m_memmap;
	unsigned m_mapsize;
	bool m_allowGuards;
public:
	Jsw128MemPage(JswGame *game, bool allowGuards);
	virtual ~Jsw128MemPage();

        virtual int onReveal(int whichRectangle);
        virtual int onConceal();
        virtual int redraw(int whichRectangle);
	virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);
};


