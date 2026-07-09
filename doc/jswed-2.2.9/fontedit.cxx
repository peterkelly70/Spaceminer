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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "fontedit.hxx"
#include "letterchooser.hxx"



FontEditPage::FontEditPage(JswGame *g)
{
	m_font = g->getFont(&m_fontLen);

	m_chooser = new LetterChooser(11 * CHAR_W, 16 * CHAR_H, 18 * CHAR_W,
				       8 * CHAR_H, m_font, m_fontLen);
		
	m_editor = new BitmapEditor(13 * CHAR_W, CHAR_H, 8, 8);
	addChild(m_editor);
	addChild(m_chooser);
	setTitle("Font");
	m_chooser->setListener(this);
	m_editor->setListener(this);
	setFocus(m_chooser);
	onChangeSprite(m_font);
}

FontEditPage::~FontEditPage()
{
	delete m_editor;
	delete m_chooser;
}


int FontEditPage::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);

	if (whichRectangle & VPR_RIGHT)
	{
		videoScreen->bottomBar("Font editor");

		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H,
			"Keyboard hints: Select a char. using cu"
			"rsor keys and ENTER; then press TAB to", 
			ZX_BLACK, ZX_WHITE);
		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H + CHAR_SH,
			"switch to the editor. ^Z or ^U to undo", 
			ZX_BLACK, ZX_WHITE);
	}
	return rv;
}

void FontEditPage::onBitmapUpdate(BitmapEditor *caller, int x, int y, int pixel)
{
	m_chooser->redrawSelection();
}




int FontEditPage::onReveal(int rect)
{
	int r = VideoNotePage::onReveal(rect);

	return r;
}

void FontEditPage::onChangeSprite(jswByte *newBitmap)
{
	m_editor->setBitmap(newBitmap);
	memcpy(m_undoBuf, newBitmap, 8);
}

void FontEditPage::undo(void)
{
	jswByte buf[8];
	jswByte *bm = m_editor->getBitmap();
	
	memcpy(buf, bm, 8);
	memcpy(bm, m_undoBuf, 8);
	memcpy(m_undoBuf, buf, 8);
	m_editor->setBitmap(bm);	// Make it redraw
}

int FontEditPage::preContextMenu(void)
{
	return m_editor->doContextMenu();
}


int FontEditPage::onKeyDown(int keysym)
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
