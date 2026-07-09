#ifndef FONTEDIT_HXX_INCLUDED
#define FONTEDIT_HXX_INCLUDED

/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

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

class LetterChooser;

#include "slistener.hxx"

class FontEditPage : public VideoNotePage,SpriteListListener,BitmapEditListener
{
// Data
protected:
	jswByte *m_font;
	int m_fontLen;	
	BitmapEditor *m_editor;
	LetterChooser *m_chooser;

	void undo(void);
public:
	FontEditPage(JswGame *g);
	virtual ~FontEditPage();

	jswByte m_undoBuf[8];

	virtual int preContextMenu(void);

	virtual int onReveal(int rect);
        virtual void onChangeSprite(jswByte *newBitmap);

	void reload(JswGame *g);
        virtual int redraw(int whichRectangle);

	virtual int onKeyDown(int keysym);

	virtual void onBitmapUpdate(BitmapEditor *caller, int x, int y, int pixel);
};


#endif	// def FONTEDIT_HXX_INCLUDED

