#ifndef JSW64GUARD_HXX_INCLUDED
#define JSW64GUARD_HXX_INCLUDED

/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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

#include "j128guard.hxx"

class JswGame;
class Room;

class Jsw64Guard : public Jsw128Guard
{
public:
	Jsw64Guard(jswByte *g, jswByte x, SpectrumMemory *mem, int bg);
	virtual ~Jsw64Guard();

	virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0);
	virtual int setPos(int cx, int cy);

	virtual void constructSprites(void);
	virtual void setInk(int ink);
};

#endif //ndef JSW64GUARD_HXX_INCLUDED
