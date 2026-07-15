/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "manicgame.hxx"
#include "manicsprite.hxx"

ManicSpritePage::ManicSpritePage(ManicGame *g)
{
	m_list = new SpriteList(g->getMem(), 2, m_y + 20 * CHAR_H, VIDEO_W - 4,
				               CHAR_H + CHAR_SH + 40);
	
	g->fillSprites(-1, m_list);
	m_list->setTitle("Sprites");
	addChild(m_list);
	m_editor = new BitmapEditor(9 * CHAR_W, CHAR_H, 16, 16);
	addChild(m_editor);
	m_list->setListener(this);
	setTitle("Sprites");
	setFocus(m_list);
}

ManicSpritePage::~ManicSpritePage()
{
	delete m_editor;
	delete m_list;
}


int ManicSpritePage::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);

	if (whichRectangle & VPR_RIGHT)
	{
		videoScreen->bottomBar("Sprite bitmaps");

		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H,
			"Keyboard hints: Select a sprite using cu"
			"rsor keys and ENTER; then press TAB to", 
			ZX_BLACK, ZX_WHITE);
		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H + CHAR_SH,
			"switch to the editor. ^Z or ^U to undo", 
			ZX_BLACK, ZX_WHITE);
	}
	return rv;
}

int ManicSpritePage::onReveal(int rect)
{
	return VideoNotePage::onReveal(rect);
}

void ManicSpritePage::onChangeSprite(jswByte *newBitmap)
{
	m_editor->setBitmap(newBitmap);
	memcpy(m_undoBuf, newBitmap, 32);
}


void ManicSpritePage::undo(void)
{
	jswByte buf[32];
	jswByte *bm = m_editor->getBitmap();
	
	memcpy(buf, bm, 32);
	memcpy(bm, m_undoBuf, 32);
	memcpy(m_undoBuf, buf, 32);
	m_editor->setBitmap(bm);	// Make it redraw
}

int ManicSpritePage::preContextMenu(void)
{
	return m_editor->doContextMenu();
}


int ManicSpritePage::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case 'Z' - '@':
		case 'U' - '@':
			undo();
			return ZXE_CONTINUE;
	}
	return VideoNotePage::onKeyDown(keysym);
}
