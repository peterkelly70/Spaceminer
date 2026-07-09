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
#include "jswgame.hxx"
#include "henrymsg.hxx"
#include "henrygame.hxx"

HenryMessagePage::HenryMessagePage(HenryGame *g)
{
	m_game = g;

	int x0 = (VIDEO_W - 32 * CHAR_W - 4) / 2;
	int y0 = VIDEO_H - 12 * CHAR_H;
	int x1 = m_x + 2 + 8 * CHAR_W;	
	int y1 = m_y + 6 + CHAR_H;
	int x2 = m_x + 2;
	int n;
	SpectrumMemory *mem = g->getMem();

	m_label[0] = new VideoLabel(x1, m_y, "Game over rhyme:");
	m_edit[0] = new VideoTextEdit(x1, y1, 28 * CHAR_W + 4, CHAR_H + 4);
	m_edit[0]->setLimit(27);
	m_edit[0]->bindFixed((char *)mem->memoryAt(0xF683));	
        m_edit[1] = new VideoTextEdit(x1, y1+2*CHAR_H, 22 * CHAR_W + 4, CHAR_H + 4);
        m_edit[1]->setLimit(21);
        m_edit[1]->bindFixed((char *)mem->memoryAt(0xF69E));
        m_edit[2] = new VideoTextEdit(x1, y1+4*CHAR_H, 18 * CHAR_W + 4, CHAR_H + 4);
        m_edit[2]->setLimit(17);
        m_edit[2]->bindFixed((char *)mem->memoryAt(0xF6B3));
        m_edit[3] = new VideoTextEdit(x1, y1+6*CHAR_H, 27 * CHAR_W + 4, CHAR_H + 4);
        m_edit[3]->setLimit(26);
        m_edit[3]->bindFixed((char *)mem->memoryAt(0xF6C4));

	m_label[1] = new VideoLabel(x2, y0 - 8 * CHAR_H, "Game Over message");

	m_edit[4] = new VideoTextEdit( x0, y0 - 6 * CHAR_H, 
					11 * CHAR_W + 4, CHAR_H + 4);
	m_edit[4]->setLimit(10);
	m_edit[4]->bindFixed((char *)mem->memoryAt(0xF679));

	m_label[2] = new VideoLabel(x2, y0 - 4 * CHAR_H, "Game title ");


	m_edit[5] = new VideoTextEdit( x0, y0 - 2 * CHAR_H, 
					15 * CHAR_W + 4, CHAR_H + 4);	
	m_edit[5]->setLimit(14);
	m_edit[5]->bindFixed((char *)mem->memoryAt(0xF6DE));

	m_label[3] = new VideoLabel(x2, y0, "Scrolling title message");

	g->getScrolly(m_buf);

	m_edit[6] = new VideoTextEdit( x0, y0 + 2 * CHAR_H, 32*CHAR_W + 4, 
						CHAR_H + 4);
	if (g->isLater()) m_edit[6]->setLimit(869);
	else		  m_edit[6]->setLimit(684);

	m_edit[6]->bind(m_buf);

//	m_label[4] = new VideoLabel(x2, y0 + 4 * CHAR_H, 
//	m_edit[7] = new VideoTextEdit(x0, y0 + 6 * CHAR_H, 

	for (n = 0; n < 7; n++) addChild(m_edit[n]);
	for (n = 0; n < 4; n++) addChild(m_label[n]);
	setTitle("Message");
	setFocus(m_edit[0]);
}

HenryMessagePage::~HenryMessagePage()
{
	int n;
	for (n = 0; n < 7; n++) delete m_edit[n];
	for (n = 0; n < 4; n++) delete m_label[n];
}



int HenryMessagePage::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);

	videoScreen->bottomBar("Game messages");
	return rv;
}



void HenryMessagePage::onEditChange(VideoControl *v)
{
	if (v == m_edit[6]) m_game->setScrolly(m_buf);
}
