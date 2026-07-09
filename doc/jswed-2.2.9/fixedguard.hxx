#ifndef FIXEDGUARD_HXX_INCLUDED
#define FIXEDGUARD_HXX_INCLUDED

/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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


#define FG_WILLY  1	// Willy
#define FG_MARIA  2	// Maria
#define FG_TOILET 3	// Bathroom toilet
#define FG_RESET  4	// Willy's reset position
#define FG_MAX    4

class Room;

class JswFixedGuard : public Jsw128Guard
{
protected:
	jswByte m_dummyGuard[8];
public:
	JswFixedGuard(int type, JswGame *g, Room *r, int bg);
	int doConstruct(int type, JswGame *g, Room *r, int bg);
};

#endif //ndef FIXEDGUARD_HXX_INCLUDED
