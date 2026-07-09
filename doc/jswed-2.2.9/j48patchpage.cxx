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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "j48patchpage.hxx"
#include "patch.hxx"
#include "j128game.hxx"
#include "j64game.hxx"
#include "filetypelist.hxx"

extern Patch patchSoftricksAttract, patchDeathless;

Jsw48PatchPage::Jsw48PatchPage(JswGame *g) : JswPatchPage(g)
{
	m_btnUpgrade = new VideoButton(101, 10 * CHAR_W, 24 * CHAR_H,
					"Upgrade to 128k");
	m_btnUpgrade->setListener(this);	
	addChild(m_btnUpgrade);

        int x = patchSoftricksAttract.label->getX();
        int y = patchSoftricksAttract.label->getY();

        y += CHAR_H + 4;
        patchDeathless.label = new VideoLabel(x, y, "No infinite death");
        patchDeathless.checkbox=new VideoCheckbox(x+21*CHAR_W, y);
        addPatch(&patchDeathless);

        updatePatches();
}

Jsw48PatchPage::~Jsw48PatchPage()
{	
	delete m_btnUpgrade;
}

int Jsw48PatchPage::onCheckboxSelect(VideoCheckbox *b, int value)
{
	return JswPatchPage::onCheckboxSelect(b, value);
}

int Jsw48PatchPage::onButtonSelect(VideoButton *b)
{
	if (b == m_btnUpgrade)
	{
		VideoMenu vm("Upgrade to 128k", 
				"JSW128 engine",
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
			case 0: m_game->m_upgrade = m_game->upgrade();
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 1: m_game->m_upgrade = m_game->upgrade64('V');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 2: m_game->m_upgrade = m_game->upgrade64('W');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 3: m_game->m_upgrade = m_game->upgrade64('X');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 4: m_game->m_upgrade = m_game->upgrade64('Y');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 5: m_game->m_upgrade = m_game->upgrade64('Z');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			case 6: m_game->m_upgrade = m_game->upgrade64('[');
				if (!m_game->m_upgrade) return ZXE_CONTINUE;
				return ZXE_OK + 101;
			default: return ZXE_CONTINUE;
		}
	}
	return ZXE_CONTINUE;
}
