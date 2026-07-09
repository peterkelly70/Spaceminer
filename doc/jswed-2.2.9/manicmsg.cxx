/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "manicmsg.hxx"

ManicMessagePage::ManicMessagePage(SpectrumMemory *mem)
{
	int x0 = (VIDEO_W - 32 * CHAR_W - 4) / 2;
	int y0 = VIDEO_H - 10 * CHAR_H;
	int x2 = m_x + 2;
	int n;


	m_label[0] = new VideoLabel(x2, y0 - 8 * CHAR_H, "Game Over message");

	m_edit[0] = new VideoTextEdit( x0, y0 - 6 * CHAR_H, 
					5 * CHAR_W + 4, CHAR_H + 4);
	m_edit[0]->setLimit(4);
	m_edit[0]->bindFixed((char *)mem->memoryAt(0x844F));

	m_edit[1] = new VideoTextEdit( x0 + 6 * CHAR_W, y0 - 6 * CHAR_H,
					5 * CHAR_W + 4, CHAR_H + 4);
	m_edit[1]->setLimit(4);
	m_edit[1]->bindFixed((char *)mem->memoryAt(0x8453));

	m_label[1] = new VideoLabel(x2, y0 - 4 * CHAR_H, "Score "
							  "message");


	m_edit[2] = new VideoTextEdit( x0, y0 - 2 * CHAR_H, 
					32 * CHAR_W + 4, CHAR_H + 4);	
	m_edit[2]->setLimit(32);
	m_edit[2]->bindFixed((char *)mem->memoryAt(0x842f));

	m_label[2] = new VideoLabel(x2, y0, "Scrolling title message");

	m_edit[3] = new VideoTextEdit( x0, y0 + 2 * CHAR_H, 32*CHAR_W + 4, 
						CHAR_H + 4);

        m_edit[3]->setLimit(256);
        m_edit[3]->bindFixed((char *)mem->memoryAt(0x9D00));

	for (n = 0; n < 4; n++) addChild(m_edit[n]);
	for (n = 0; n < 3; n++) addChild(m_label[n]);
	setTitle("Message");
	setFocus(m_edit[0]);
}

ManicMessagePage::~ManicMessagePage()
{
	int n;
	for (n = 0; n < 4; n++) delete m_edit[n];
	for (n = 0; n < 3; n++) delete m_label[n];
}



int ManicMessagePage::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);

	videoScreen->bottomBar("Game messages");
	return rv;
}

