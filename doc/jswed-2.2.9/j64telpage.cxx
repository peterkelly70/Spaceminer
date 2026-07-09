/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2004  John Elliott <jce@seasip.demon.co.uk>

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
#include "j64telpage.hxx"


Jsw64TelPage::Jsw64TelPage(JswGame *game) : Jsw48TelPage(game)
{
}

Jsw64TelPage::~Jsw64TelPage()
{
}



int  Jsw64TelPage::preContextMenu()
{
	if (!m_game->canTeleport()) return Jsw48TelPage::preContextMenu();

	intPreContextMenu();

	VideoMenu vm("Options", 
		     "Editor",
		     "No special effect",
		     "Screen flashes",
		     "Air runs down",
		     "Both the above",
		     "Exit",
		     NULL);
	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv == ZXE_CANCEL)
	{
		postContextMenu();
		return ZXE_CONTINUE;
	}
	int tpBase      = m_game->getMem()->peek(35698) * 256;
	jswByte *tpCode = m_game->getMem()->memoryAt(tpBase);
	switch(vm.getSelected())
	{
		case 0:	postContextMenu();
			return ZXE_CONTINUE;
		case 1:	tpCode[0x34] = 0;
			tpCode[0x35] = 0;
			tpCode[0x36] = 0;
			break;
		case 2:	tpCode[0x34] = 0xCD;
			tpCode[0x35] = 0xA9;
			tpCode[0x36] = 0x86;
			break;
		case 3:	tpCode[0x34] = 0xCD;
			tpCode[0x35] = 0xAC;
			tpCode[0x36] = 0x86;
			break;
		case 4:	tpCode[0x34] = 0xCD;
			tpCode[0x35] = 0xAF;
			tpCode[0x36] = 0x86;
			break;
		case 5: postContextMenu();
			return ZXE_OK;
	}
	postContextMenu();
	return ZXE_CONTINUE;
}



