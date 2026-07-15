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

#include "screenedit.hxx"

class Jsw128AttrPage : public ScreenEditorPage
{
protected:
	JswGame       *m_game;

	int m_nBitmap;
	void setup(int bitmap);
public:
	Jsw128AttrPage(JswGame *g);
	virtual ~Jsw128AttrPage();

	virtual int onNext(void);
	virtual int onPrev(void);
        virtual int redraw(int whichRectangle);
};


