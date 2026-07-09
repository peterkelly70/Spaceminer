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
#include "filetypelist.hxx"


FileTypeList::FileTypeList(jswByte *type, int x, int y, int w, int h)
		: ScrollingList(x,y,w,h)
{
	m_type = type;

	addString("+3DOS");
	addString("SNA");
	addString("TAP");
	addString("Z80");
	
	switch(*type)
	{
		case 'O': m_selected = 0; break;
		case 'N': m_selected = 1; break;
		case 'T': m_selected = 2; break;
		case 'Z': m_selected = 3; break;
	}

}

FileTypeList::~FileTypeList()
{
}



int FileTypeList::onSelect(void)
{
	switch(m_selected)
	{
		case 0: *m_type = 'O'; break;
                case 1: *m_type = 'N'; break;
                case 2: *m_type = 'T'; break;
                case 3: *m_type = 'Z'; break;
	}
	return ZXE_OK;
}

