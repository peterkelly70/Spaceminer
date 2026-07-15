/************************************************************************

    JSWED 2.2.1 - Editor for Jet Set Willy and derivatives

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
#include "jswgame.hxx"
#include "j64game.hxx"
#include "j128patchpage.hxx"
#include "patch.hxx"
#include "filetypelist.hxx"

extern PatchLine plSoftricksDeath[];

PatchLine plSoftricksDeath128[] = {
        { 0x8C02, 0x47, 0xC9},
        { 0x8C03, 0x21, 0x32},
        { 0x8C04, 0x00, 0x6E},
        { 0x8C05, 0x58, 0x88},
        { 0x8C06, 0x11, 0xCD},
        { 0x8C07, 0x01, 0x13},
        { 0x8C08, 0x58, 0x88},
        { 0x8C09, 0x01, 0x3E},
        { 0x8C0A, 0xFF, 0x21},
        { 0x8C0B, 0x01, 0x32},
        { 0x8C0C, 0x77, 0x6E},
        { 0x8C0D, 0xED, 0x88},
        { 0x8C0E, 0xB0, 0x06},
        { 0x8C0F, 0x5F, 0x32},
        { 0x8C10, 0x2F, 0xC5},
        { 0x8C11, 0xE6, 0x78},
        { 0x8C12, 0x07, 0xCD},
        { 0x8C13, 0x07, 0xDE},
        { 0x8C14, 0x07, 0x96},
        { 0x8C15, 0x07, 0xC1},
        { 0x8C16, 0xF6, 0x10},
        { 0x8C17, 0x07, 0xF8},
        { 0x8C18, 0x57, 0xC3},
        { 0x8C19, 0x4B, 0x33},
        { 0x8C1A, 0xCB, 0x8C},
        { 0x8C1B, 0x09, 0x00},
        { 0x8C1C, 0xCB, 0x00},
        { 0x8C1D, 0x09, 0x00},
        { 0x8C1E, 0xCB, 0x00},
        { 0x8C1F, 0x09, 0x00},
        { 0x8C20, 0xF6, 0x00},
        { 0x8C21, 0x10, 0x00},
        { 0x8C22, 0xAF, 0x00},
        { 0x8C23, 0xD3, 0x00},
        { 0x8C24, 0xFE, 0x00},
        { 0x8C25, 0xEE, 0x00},
        { 0x8C26, 0x18, 0x00},
        { 0x8C27, 0x42, 0x00},  // 0x26 lines
	{ 0x881B, 0x1A, 0x19},
};



Jsw128PatchPage::Jsw128PatchPage(JswGame *g) : JswPatchPage(g)
{
        m_btnUpgrade = new VideoButton(101, 10 * CHAR_W, 24 * CHAR_H,
				"Convert to JSW64");
        m_btnUpgrade->setListener(this);
        addChild(m_btnUpgrade);

	for (int n = 0; n < m_patchCount; n++)
	{
		if (m_patchData[n].lines == plSoftricksDeath)
		{
			m_patchData[n].lines = plSoftricksDeath128;
			m_patchData[n].lineCount = 0x27;
		}
	}
	updatePatches();	
}

Jsw128PatchPage::~Jsw128PatchPage()
{	
}

int Jsw128PatchPage::onCheckboxSelect(VideoCheckbox *b, int value)
{
	return JswPatchPage::onCheckboxSelect(b, value);
}

int Jsw128PatchPage::onButtonSelect(VideoButton *b)
{
	if (b == m_btnUpgrade)
	{
		VideoMenu vm("Convert to JSW64", 
				"JSW64 variant V",
				"JSW64 variant W",
				"JSW64 variant X",
				"JSW64 variant Y",
				"JSW64 variant Z",
				"JSW64 variant [",
				"Cancel", NULL);
		int rv = vm.doModal();
		
		if (rv >= ZXE_QUIT) return rv;
		switch (vm.getSelected())
		{
			case 0: m_game->m_upgrade = m_game->upgrade64('V');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 1: m_game->m_upgrade = m_game->upgrade64('W');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 2: m_game->m_upgrade = m_game->upgrade64('X');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 3: m_game->m_upgrade = m_game->upgrade64('Y');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 4: m_game->m_upgrade = m_game->upgrade64('Z');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 5: m_game->m_upgrade = m_game->upgrade64('[');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			default: return ZXE_CONTINUE;
		}
	}
	return ZXE_CONTINUE;
}
