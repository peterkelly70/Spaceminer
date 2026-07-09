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

#include "jswed.hxx"
#include "spritepage.hxx"
#include "spriteform.hxx"

SpritePage::SpritePage(SpectrumMemory *mem)
{
	m_list = NULL;
	m_mem  = mem;
	m_editor = new BitmapEditor(9 * CHAR_W, CHAR_H, 16, 16);
	setTitle("Sprites");
}

SpritePage::~SpritePage()
{
	delete m_editor;
	delete m_list;
}


int SpritePage::redraw(int whichRectangle)
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


/*
int SpritePage::onReveal(int rect)
{
	int r = VideoNotePage::onReveal(rect);

	if (m_reload)
	{
		m_list->emptyBuffer();
		m_reload->fillSprites(-1, m_list);
		m_list->redraw();
		m_reload = 0;
	} 
	return r;
}
*/

void SpritePage::onChangeSprite(jswByte *newBitmap)
{
	m_editor->setBitmap(newBitmap);
	memcpy(m_undoBuf, newBitmap, 32);
}


void SpritePage::undo(void)
{
	jswByte buf[32];
	jswByte *bm = m_editor->getBitmap();
	
	memcpy(buf, bm, 32);
	memcpy(bm, m_undoBuf, 32);
	memcpy(m_undoBuf, buf, 32);
	m_editor->setBitmap(bm);	// Make it redraw
}

int SpritePage::preContextMenu(void)
{
	VideoMenu vm("Options", "Editor",
				"Bitmap",
				"Import",
				"Export",
				"Exit",
				NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
	switch(vm.getSelected())
	{
		case 0: return ZXE_CONTINUE;
		case 1: return m_editor->doContextMenu();
		case 2: return doImport();
		case 3: return doExport();
		case 4: return ZXE_OK;
	}	
	return ZXE_OK;
}

int SpritePage::doImport()
{
	SpriteForm sf(0, m_mem, m_list);

	int rv = sf.doModal();
	if (rv >= ZXE_QUIT) return rv;	
	if (rv == ZXE_OK)
	{
		jswByte *bm = m_editor->getBitmap();
		m_editor->setBitmap(bm);	
		m_list->redraw();
	}
	return ZXE_CONTINUE;
}

int SpritePage::doExport()
{
	int rv;
	SpriteForm sf(1, m_mem, m_list);

	rv = sf.doModal();
	if (rv >= ZXE_QUIT) return rv;	

	return ZXE_CONTINUE;
}

int SpritePage::onKeyDown(int keysym)
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
