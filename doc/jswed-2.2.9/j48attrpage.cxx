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
#include "j48attrpage.hxx"

#define BM_BOTTOMSCR 0
#define BM_TITLE48   1
#define BM_LOADSCR   2

Jsw48AttrPage::Jsw48AttrPage(JswGame *g)
{
	m_game = g;

	setTitle("Screens");

	setup(BM_BOTTOMSCR);
}


Jsw48AttrPage::~Jsw48AttrPage()
{
}

#include "bottomscr.xbm"

static jswByte jswscr_bits[] =
{
#include "jswscr.hxx"
};

void Jsw48AttrPage::setup(int bitmap)
{	
	jswByte *ttl = m_game->getTitleAttrs();

	m_nBitmap = bitmap;
	switch(bitmap)
	{
		case BM_BOTTOMSCR:	m_bits   = bottomscr_bits;
					m_attrs  = ttl + 0x200;
					m_format = FMT_XBM;
					m_w      = bottomscr_width;
					m_h      = bottomscr_height;
					break;
		case BM_TITLE48:	m_bits   = jswscr_bits;
					m_attrs  = ttl;
					m_format = FMT_ROSCRN;
					m_w      = 256;
					m_h      = 128;
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


int Jsw48AttrPage::redraw(int whichRectangle)
{
	char b[40];
	int rv = ScreenEditorPage::redraw(whichRectangle);

	sprintf(b, "Screen-size graphics [%d]", m_nBitmap + 1);
	videoScreen->bottomBar(b);
	return rv;
}



int Jsw48AttrPage::onNext()
{
	++m_nBitmap;
	if (m_nBitmap == BM_LOADSCR/* + 1 */) m_nBitmap = BM_BOTTOMSCR;
	setup(m_nBitmap);
	redraw(VPR_RIGHT);
	return ZXE_CONTINUE;
}

int Jsw48AttrPage::onPrev()
{
        --m_nBitmap;
        if (m_nBitmap < 0) m_nBitmap = /*BM_LOADSCR */ BM_TITLE48;
        setup(m_nBitmap);
	redraw(VPR_RIGHT);
        return ZXE_CONTINUE;
}



