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
#include "j128msg.hxx"

Jsw128MessagePage::Jsw128MessagePage(SpectrumMemory *mem) 
		: JswMessagePage(mem)
{
	m_edit[3]->setLimit(1279);
	m_edit[3]->bind((char *)mem->memoryAt(0xD600,7));

	// 2.0.2: Add controls to edit the static title screen message.
	int y0 = VIDEO_H - 10 * CHAR_H;
        int x0 = (VIDEO_W - 32 * CHAR_W - 4) / 2;

	m_msgLabel = new VideoLabel(0, y0 + 3 * CHAR_H + CHAR_SH, 
					"Fixed title screen messsage");

        m_msgEdit = new VideoTextEdit( x0, y0 + 5 * CHAR_H,
                                        33 * CHAR_W + 4, CHAR_H + 4);
        m_msgEdit->setLimit(32);
        m_msgEdit->bindFixed((char *)mem->memoryAt(0x8454));
	addChild(m_msgLabel);
	addChild(m_msgEdit);	
}


Jsw128MessagePage::~Jsw128MessagePage()
{
	// 2.0.2: Remove controls to edit the static title screen message.
	delete m_msgEdit;
	delete m_msgLabel;
}
