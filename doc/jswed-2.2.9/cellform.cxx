/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004,2005  John Elliott <jce@seasip.demon.co.uk>

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
#include "cellform.hxx"

typedef char *pchar;

CellForm::CellForm(jswByte *cells, int count, ...) : VideoForm(10 * CHAR_W + CHAR_SW, CHAR_H,
						    CHAR_W * 20, CHAR_H * 24)
{
	int n, x, y;

	va_list ap;
	va_start(ap, count);

	m_count = count;
	m_cells = cells;
	m_captions = new pchar[count];
	for (n = 0; n < count; n++)
	{
		m_captions[n] = va_arg(ap, pchar);
	}
	va_end(ap);

	m_prev   = new VideoButton('<',        m_x + 2  * CHAR_W, m_y + 8 * CHAR_H, " < ");
	m_next   = new VideoButton('>',        m_x + 16 * CHAR_W, m_y + 8 * CHAR_H, " > ");
	m_cancel = new VideoButton(ZXE_CANCEL, m_x + CHAR_W, 23 * CHAR_H, "Cancel");
	m_ok     = new VideoButton(ZXE_OK,     m_x + 12 * CHAR_W, 23 * CHAR_H, "  OK  ");

	m_prev->setListener(this);
	m_next->setListener(this);

	x = m_x + 6 * CHAR_W;
	y = m_y + 2 * CHAR_H + CHAR_SH;
	m_edit   = new BitmapEditor(x, y, 8, 8, 1);
	m_ink    = new VideoColourChooser(x, y + 11 * CHAR_H);
	m_paper  = new VideoColourChooser(x, y + 13 * CHAR_H);
	m_bright = new VideoCheckbox(x + 7 * CHAR_W, y + 15 * CHAR_H);
	m_flash  = new VideoCheckbox(x + 7 * CHAR_W, y + 17 * CHAR_H);
	m_curcell = 0;
        m_ink->setListener(this);
	m_paper->setListener(this);
	m_bright->setListener(this);
}

CellForm::~CellForm()
{
	delete m_ink;
	delete m_paper;
	delete m_bright;
	delete m_flash;
	delete m_edit;
	delete m_captions;
	delete m_ok;
	delete m_cancel;
	delete m_prev;
	delete m_next;
}

void CellForm::redraw(void)
{
	VideoForm::redraw();
	
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 13 * CHAR_H, "Ink", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 15 * CHAR_H, "Paper", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 17 * CHAR_H, "Bright", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 19 * CHAR_H, "Flash", ZX_BLACK, ZX_WHITE);

	videoScreen->drawText(m_x + 6 * CHAR_W, m_y + CHAR_H + CHAR_SH / 2, m_captions[m_curcell], ZX_BLACK, ZX_WHITE);
	redrawBorder("Room graphics");
}



int CellForm::doModal()
{
	return VideoForm::doModal();
}

#define SCOL(y) \
	m_ink   ->setColour (m_cells[y] & 7);       \
	m_paper ->setColour((m_cells[y] >> 3) & 7); \
	m_bright->setCheck  (m_cells[y] & 64); \
	m_flash ->setCheck  (m_cells[y] & 128);

#define GCOL(y) m_cells[y] = \
	 m_ink   ->getColour()           | \
	(m_paper ->getColour() << 3)     | \
	(m_bright->getCheck() ?  64 : 0) | \
	(m_flash ->getCheck() ? 128 : 0);


void CellForm::onChooseColour(VideoColourChooser *c)
{
	int clr;

	if (c == m_ink) 
	{
		clr = m_ink->getColour();
		if (m_bright->getCheck()) clr |= 8;
		m_edit->setForeground(clr);
	}
	if (c == m_paper)
	{
		clr = m_paper->getColour();
		if (m_bright->getCheck()) clr |= 8;
		m_edit->setBackground(clr);
	}
}


int CellForm::onCheckboxSelect(VideoCheckbox *cb, int ticked)
{
	if (cb == m_bright)
	{
		int clr;

		clr = m_ink->getColour();
		if (ticked) clr |= 8;
		m_edit->setForeground(clr);

		clr = m_paper->getColour();
		if (ticked) clr |= 8;
		m_edit->setBackground(clr);
	}
	return ZXE_OK;
}



void CellForm::showChildren(int redraw)
{
        int n;
	addChild(m_ink);
	addChild(m_paper);
	addChild(m_bright);
	addChild(m_flash);
	addChild(m_edit);
	m_edit->setBitmap(m_cells + 9*m_curcell + 1);
	m_edit->setAttribute(m_cells[9 * m_curcell]);
	SCOL(9 * m_curcell);
        addChild(m_ok);
        addChild(m_cancel);
        addChild(m_prev);
        addChild(m_next);
	VideoForm::showChildren(redraw);
}


void CellForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
	GCOL(9 * m_curcell);

	removeChild(m_ink);
	removeChild(m_paper);
	removeChild(m_bright);
	removeChild(m_flash);
	removeChild(m_edit);

        removeChild(m_ok);
        removeChild(m_cancel);
        removeChild(m_next);
        removeChild(m_prev);

}


int CellForm::onButtonSelect(VideoButton *b)
{
	int move = 0;
	switch(b->getId())
	{
		case '<': move = -1; break;
		case '>': move = 1;  break;
		default: return b->getId();
	}
	GCOL(9 * m_curcell);

	m_curcell += move;
	if (m_curcell >= m_count) m_curcell = 0;
	if (m_curcell < 0) m_curcell = m_count - 1;	

	m_edit->setBitmap(m_cells + 9*m_curcell + 1);
	m_edit->setAttribute(m_cells[9 * m_curcell]);
	SCOL(9 * m_curcell);

	redraw();
	return ZXE_CONTINUE;
}

