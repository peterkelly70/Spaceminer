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
#include "henryattrpage.hxx"

#define BM_TITLE2    0
#define BM_VICTORY   1
#define BM_LOADSCR   2

HenryAttrPage::HenryAttrPage(JswGame *g)
{
	m_game = g;

	setTitle("Screens");

	setup(BM_TITLE2);
}


HenryAttrPage::~HenryAttrPage()
{
}


void HenryAttrPage::setup(int bitmap)
{	
	m_nBitmap = bitmap;
	switch(bitmap)
	{
		case BM_TITLE2:		m_bits   = m_game->getMem()->memoryAt(0x6000);
					m_attrs  = m_game->getMem()->memoryAt(0x7800);
					m_format = FMT_SCREEN;
					m_w      = 256;
					m_h      = 192;
					break;
		case BM_VICTORY:	m_bits   = m_game->getMem()->memoryAt(0xF734);
					m_attrs  = NULL;
					m_format = FMT_SCREEN;
					m_w      = 256;
					m_h      = 64;
					break;	
		case BM_LOADSCR:	m_bits   = m_game->getMem()->memoryAt(0x4000);
					m_attrs  = m_game->getMem()->memoryAt(0x5800);
					m_format = FMT_SCREEN;
					m_w       = 256;
					m_h       = 192;
					break;
	}	
	ScreenEditorPage::setup();
}


int HenryAttrPage::redraw(int whichRectangle)
{
	char b[40];
	int rv = ScreenEditorPage::redraw(whichRectangle);

	sprintf(b, "Screen-size graphics [%d]", m_nBitmap + 1);
	videoScreen->bottomBar(b);
	return rv;
}



int HenryAttrPage::onNext()
{
	++m_nBitmap;
	if (m_nBitmap == BM_LOADSCR/* + 1 */) m_nBitmap = BM_TITLE2;
	setup(m_nBitmap);
	redraw(VPR_RIGHT);
	return ZXE_CONTINUE;
}

int HenryAttrPage::onPrev()
{
        --m_nBitmap;
        if (m_nBitmap < 0) m_nBitmap = /*BM_LOADSCR */ BM_VICTORY;
        setup(m_nBitmap);
	redraw(VPR_RIGHT);
        return ZXE_CONTINUE;
}



