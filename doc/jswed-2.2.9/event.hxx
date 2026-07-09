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

class TickListener
{
public:
	virtual void onTick(void);
};

class MouseListener
{
public:
	virtual int onMouseMove(int x, int y, int xrel, int yrel);
};

class ButtonListener
{
public:
	virtual int onButtonUp  (int x, int y, int button);
	virtual int onButtonDown(int x, int y, int button);
};

class KeyListener
{	
public:
        virtual int onKeyUp  (int keysym);
        virtual int onKeyDown(int keysym);
};

bool shiftPressed();
bool ctrlPressed();
bool altPressed();

int videoEvent();
KeyListener *setKeyListener(KeyListener *k);
KeyListener *getKeyListener();
KeyListener *setAppKeyFilter(KeyListener *k);
KeyListener *getAppKeyFilter();

ButtonListener *setButtonListener(ButtonListener *b);
ButtonListener *getButtonListener();
MouseListener *setMouseListener(MouseListener *m);
MouseListener *getMouseListener();
TickListener  *setTickListener(TickListener *t);
TickListener  *getTickListener();

#define ZXE_CONTINUE 0
#define ZXE_OK       1
#define ZXE_CANCEL   2
#define ZXE_QUIT     3


