#ifndef JSW64TELPAGE_HXX_INCLUDED
#define JSW64TELPAGE_HXX_INCLUDED

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

#include "j48telpage.hxx"

class Jsw64TelPage : public Jsw48TelPage
{
public:
	Jsw64TelPage(JswGame *game);
	virtual ~Jsw64TelPage();

	virtual int preContextMenu(void);
};

#endif //ndef JSW64TELPAGE_HXX_INCLUDED
