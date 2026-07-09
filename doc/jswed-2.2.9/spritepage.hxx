#ifndef SPRITEPAGE_HXX_INCLUDED
#define SPRITEPAGE_HXX_INCLUDED

/************************************************************************

    JSWED 2.2.2 - Editor for Jet Set Willy and derivatives

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

#include "spritelist.hxx"

class SpritePage : public VideoNotePage, public SpriteListListener
{
// Data
protected:
	SpriteList *m_list;
	BitmapEditor *m_editor;
	SpectrumMemory *m_mem;

	int doExport();
	int doImport();
	void undo(void);
public:
	SpritePage(SpectrumMemory *mem);
	virtual ~SpritePage();

	jswByte m_undoBuf[32];

	virtual int preContextMenu(void);

	//virtual int onReveal(int rect);
        virtual void onChangeSprite(jswByte *newBitmap);

        virtual int redraw(int whichRectangle);

	virtual int onKeyDown(int keysym);
};


#endif	// def SPRITEPAGE_HXX_INCLUDED

