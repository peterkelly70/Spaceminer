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

int JswNotebook::onButtonDown(int x, int y, int button)
{
	if (button == gl_rbutton)
	{
                VideoMenu vm("Options", "Editor", "Exit", NULL);
		
		int n = getActive()->preContextMenu();

		if (n >= ZXE_CONTINUE) return n;

                int rv = vm.doModal();

		getActive()->postContextMenu();

                if (rv >= ZXE_QUIT) return rv;
                if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
                if (vm.getSelected()) return ZXE_OK;
                return ZXE_CONTINUE;
	}

	int rv = VideoNotebook::onButtonDown(x,y,button);

	// Horrid hack for 48->128 upgrade
	if (rv == (ZXE_OK + 101)) return ZXE_OK;
	if (rv < ZXE_QUIT) return ZXE_CONTINUE;
	return rv;
}



int JswNotebook::onKeyDown(int keysym)
{
        int rv;

	if (keysym == ZXK_EDIT)
	{
		VideoMenu vm("Options", "Editor", "Exit", NULL);

                int n = getActive()->preContextMenu();

                if (n >= ZXE_CONTINUE) return n;
		rv = vm.doModal();
		getActive()->postContextMenu();

		if (rv >= ZXE_QUIT) return rv;
		if (rv == ZXE_CANCEL) return ZXE_CONTINUE;
		if (vm.getSelected()) return ZXE_OK;
		return ZXE_CONTINUE;
	}
	rv = VideoNotebook::onKeyDown(keysym);
        if (rv == (ZXE_OK + 101)) return ZXE_OK;
        if (rv < ZXE_QUIT) return ZXE_CONTINUE;
        return rv;
}


