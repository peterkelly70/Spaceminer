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

class WaitPrompt : public KeyListener,
		   public ButtonListener
{
	public:
        virtual int onButtonDown(int x, int y, int button);
        virtual int onKeyDown(int keysym);
};

int videoWaitForKey(int defcode)
{
	WaitPrompt wp;

	KeyListener    *kl = setKeyListener(&wp);
	ButtonListener *bl = setButtonListener(&wp);
	int rv = videoEvent();
	setKeyListener(kl);
	setButtonListener(bl);
	if (!rv) return defcode;
	return rv;
}

int WaitPrompt::onKeyDown(int k)
{
	switch (k)
	{
		case ZXK_BREAK:	return ZXE_CANCEL;
		case ZXK_ENTER:
		case ZXK_EDIT:	return ZXE_OK;
	}
	return KeyListener::onKeyDown(k);
}


int WaitPrompt::onButtonDown(int x, int y, int b)
{
	return ZXE_OK;
}


