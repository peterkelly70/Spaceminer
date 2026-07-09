/************************************************************************

    JSWED 2.1.6 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "j64attrpage.hxx"

#define BM_BOTTOMSCR 0
#define BM_AIRBAR    1
#define BM_TITLE64a  2
#define BM_TITLE64b  3
#define BM_TITLE64c  4
#define BM_GAME1     5
#define BM_GAME2     6
#define BM_LOADSCR   7

Jsw64AttrPage::Jsw64AttrPage(JswGame *g)
{
	m_game = g;

	setTitle("Screens");

	setup(BM_BOTTOMSCR);
}


Jsw64AttrPage::~Jsw64AttrPage()
{
}

#include "air.xbm"
#include "bottomscr.xbm"

void Jsw64AttrPage::setup(int bitmap)
{	
	m_nBitmap = bitmap;
	switch(bitmap)
	{
		case BM_BOTTOMSCR:	m_bits   = bottomscr_bits;
					m_attrs  = m_game->getMem()->memoryAt(0x9A00);
					m_format = FMT_XBM;
					m_w      = bottomscr_width;
					m_h      = bottomscr_height;
					break;
		case BM_TITLE64a:	m_bits   = m_game->getMem()->memoryAt(0xC000, 7);
					m_attrs  = m_game->getMem()->memoryAt(0xD000, 7);
					m_format = FMT_SCREEN;
					m_w      = 256;
					m_h      = 128;
					break;	
		case BM_TITLE64b:	m_bits   = m_game->getMem()->memoryAt(0xC000, 7);
					m_attrs  = m_game->getMem()->memoryAt(0xD200, 7);
					m_format = FMT_ROSCRN;
					m_w      = 256;
					m_h      = 128;
					break;	
		case BM_TITLE64c:	m_bits   = m_game->getMem()->memoryAt(0xC000, 7);
					m_attrs  = m_game->getMem()->memoryAt(0xD400, 7);
					m_format = FMT_ROSCRN;
					m_w      = 256;
					m_h      = 128;
					break;
		case BM_AIRBAR:		m_bits   = air_bits;
					m_attrs  = m_game->getMem()->memoryAt(0x97E0);
					m_format = FMT_XBM;
					m_w      = air_width;
					m_h      = air_height;
					break;
		case BM_GAME1:		m_bits   = m_game->getMem()->memoryAt(0xE2FF, 7);
					m_attrs  = m_game->getMem()->memoryAt(0xEAFF, 7);
					m_format = FMT_SCREEN;
					m_w      = 256;
					m_h      = 64;
					break;	
		case BM_GAME2:		m_bits   = m_game->getMem()->memoryAt(0xEBFF, 7);
					m_attrs  = m_game->getMem()->memoryAt(0xF3FF, 7);
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


int Jsw64AttrPage::redraw(int whichRectangle)
{
	char b[40];
	int rv = ScreenEditorPage::redraw(whichRectangle);

	sprintf(b, "Screen-size graphics [%d]", m_nBitmap + 1);
	videoScreen->bottomBar(b);
	return rv;
}



int Jsw64AttrPage::onNext()
{
	++m_nBitmap;
	if (m_nBitmap == BM_LOADSCR/* + 1 */) m_nBitmap = BM_BOTTOMSCR;
	setup(m_nBitmap);
	redraw(VPR_RIGHT);
	return ZXE_CONTINUE;
}

int Jsw64AttrPage::onPrev()
{
        --m_nBitmap;
        if (m_nBitmap < 0) m_nBitmap = /*BM_LOADSCR */ BM_TITLE64c;
        setup(m_nBitmap);
	redraw(VPR_RIGHT);
        return ZXE_CONTINUE;
}



