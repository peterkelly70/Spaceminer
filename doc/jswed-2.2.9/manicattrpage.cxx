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
#include "manicgame.hxx"
#include "manicattrpage.hxx"

#define BM_BOTTOMSCR 0
#define BM_TITLE1    1
#define BM_TITLE2    2
#define BM_LOADSCR   3

ManicAttrPage::ManicAttrPage(ManicGame *g)
{
	m_game = g;

	setTitle("Screens");

	setup(BM_BOTTOMSCR);
}


ManicAttrPage::~ManicAttrPage()
{
}

#include "maniclives.xbm"


void ManicAttrPage::setup(int bitmap)
{	
	m_nBitmap = bitmap;
	switch(bitmap)
	{
		case BM_BOTTOMSCR:	m_bits   = maniclives_bits;
					m_attrs  = m_game->getMem()->memoryAt(40704);
					m_format = FMT_XBM;
					m_w      = maniclives_width;
					m_h      = maniclives_height;
					break;
		case BM_TITLE1:		m_bits   = m_game->getMem()->memoryAt(40960);
					m_attrs  = m_game->getMem()->memoryAt(64512);
					m_format = FMT_SCREEN;
					m_w      = 256;
					m_h      = 64;
					break;	
                case BM_TITLE2:         m_bits   = m_game->getMem()->memoryAt(43008);
                                        m_attrs  = m_game->getMem()->memoryAt(40448);
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


int ManicAttrPage::redraw(int whichRectangle)
{
	char b[40];
	int rv = ScreenEditorPage::redraw(whichRectangle);

	sprintf(b, "Screen-size graphics [%d]", m_nBitmap + 1);
	videoScreen->bottomBar(b);
	return rv;
}



int ManicAttrPage::onNext()
{
	++m_nBitmap;
	if (m_nBitmap == BM_LOADSCR/* + 1 */) m_nBitmap = BM_BOTTOMSCR;
	setup(m_nBitmap);
	redraw(VPR_RIGHT);
	return ZXE_CONTINUE;
}

int ManicAttrPage::onPrev()
{
        --m_nBitmap;
        if (m_nBitmap < 0) m_nBitmap = /*BM_LOADSCR */ BM_TITLE2;
        setup(m_nBitmap);
	redraw(VPR_RIGHT);
        return ZXE_CONTINUE;
}



