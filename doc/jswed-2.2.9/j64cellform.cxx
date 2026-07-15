/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-6  John Elliott <jce@seasip.demon.co.uk>

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
#include "j64cellform.hxx"
#include "room.hxx"

typedef char *pchar;

Jsw64CellForm::Jsw64CellForm(Cell *cells, Room *r) : 
VideoForm(3 * CHAR_W + CHAR_SW, CHAR_H, CHAR_W * 34, CHAR_H * 24)
{
	int n, x, y;

	m_room = r; 
	m_count = r->getCellTypeCount();
	m_cells = cells;
	m_captions = new pchar[m_count];
	setupCaptions();

	m_prev   = new VideoButton('<',        m_x + CHAR_W, m_y + 8 * CHAR_H, " < ");
	m_next   = new VideoButton('>',        m_x + 30 * CHAR_W, m_y + 8 * CHAR_H, " > ");
	m_cancel = new VideoButton(ZXE_CANCEL, m_x + CHAR_W, 23 * CHAR_H, "Cancel");
	m_ok     = new VideoButton(ZXE_OK,     m_x + 27 * CHAR_W, 23 * CHAR_H, "  OK  ");

	m_prev->setListener(this);
	m_next->setListener(this);

	x = m_x + 6 * CHAR_W;
	y = m_y + 2 * CHAR_H + CHAR_SH;
	m_edit   = new BitmapEditor(x, y, 8, 8, 1);
	m_list 	 = new ScrollingList(x + 10 * CHAR_W, y, 12 * CHAR_W, 15 * CHAR_H);
	m_ink    = new VideoColourChooser(x, y + 11 * CHAR_H);
	m_paper  = new VideoColourChooser(x, y + 13 * CHAR_H);
	m_bright = new VideoCheckbox(x + 7 * CHAR_W, y + 15 * CHAR_H);
	m_flash  = new VideoCheckbox(x + 7 * CHAR_W, y + 17 * CHAR_H);
	m_curcell = 0;
	m_list->setTitle("Type");
	m_list->addString("Air");
	m_list->addString("Water");
	m_list->addString("Earth");
	m_list->addString("Fire");
	m_list->addString("Ramp \\");
	m_list->addString("Ramp /");
	m_list->addString("Convey <<");
	m_list->addString("Convey >>");
	m_list->addString("Crumbling");
	m_list->addString("Trampoline");
	m_list->addString("Trap");	// 2.2.9
	m_list->setSelected(cells[0].behaviour);
	m_list->setSelectAction(ZXE_CONTINUE);
	m_list->requireClick(true);
	m_ink->setListener(this);
	m_paper->setListener(this);
	m_bright->setListener(this);
}

Jsw64CellForm::~Jsw64CellForm()
{
	delete m_list;
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

void Jsw64CellForm::redraw(void)
{
	char caption[40];

	VideoForm::redraw();

	sprintf(caption, "(%d) %s", m_curcell, m_captions[m_curcell]);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 13 * CHAR_H, "Ink", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 15 * CHAR_H, "Paper", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 17 * CHAR_H, "Bright", ZX_BLACK, ZX_WHITE);
        videoScreen->drawText(m_x + CHAR_SW, m_y + CHAR_SH + 19 * CHAR_H, "Flash", ZX_BLACK, ZX_WHITE);

	videoScreen->drawText(m_x + 6 * CHAR_W, m_y + CHAR_H + CHAR_SH / 2, caption, ZX_BLACK, ZX_WHITE);
	redrawBorder("Room graphics");
}



int Jsw64CellForm::doModal()
{
	return VideoForm::doModal();
}

#define SCOL(y) \
	m_ink   ->setColour (m_cells[y].pattern[0] & 7);       \
	m_paper ->setColour((m_cells[y].pattern[0] >> 3) & 7); \
	m_bright->setCheck  (m_cells[y].pattern[0] & 64); \
	m_flash ->setCheck  (m_cells[y].pattern[0] & 128); \
	m_list  ->setSelected(m_cells[y].behaviour);

#define GCOL(y) m_cells[y].pattern[0] = \
	 m_ink   ->getColour()           | \
	(m_paper ->getColour() << 3)     | \
	(m_bright->getCheck() ?  64 : 0) | \
	(m_flash ->getCheck() ? 128 : 0); \
	m_cells[y].behaviour = m_list->getSelected(); \
	setupCaptions();

void Jsw64CellForm::onChooseColour(VideoColourChooser *c)
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


void Jsw64CellForm::showChildren(int redraw)
{
        int n;
	addChild(m_ink);
	addChild(m_paper);
	addChild(m_bright);
	addChild(m_flash);
	addChild(m_edit);
	m_edit->setBitmap(m_cells[m_curcell].pattern + 1);
	m_edit->setAttribute(m_cells[m_curcell].pattern[0]);
	addChild(m_list);
	SCOL(m_curcell);
        addChild(m_ok);
        addChild(m_cancel);
        addChild(m_prev);
        addChild(m_next);
	VideoForm::showChildren(redraw);
}


void Jsw64CellForm::hideChildren(void)
{
        int n;
	VideoForm::hideChildren();
	GCOL(m_curcell);

	removeChild(m_ink);
	removeChild(m_paper);
	removeChild(m_bright);
	removeChild(m_flash);
	removeChild(m_edit);
	removeChild(m_list);

        removeChild(m_ok);
        removeChild(m_cancel);
        removeChild(m_next);
        removeChild(m_prev);

}

int Jsw64CellForm::onCheckboxSelect(VideoCheckbox *cb, int ticked)
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



int Jsw64CellForm::onButtonSelect(VideoButton *b)
{
	int move = 0;

	switch(b->getId())
	{
		case '<': move = -1; break;
		case '>': move = 1;  break;
		default: return b->getId();
	}
	GCOL(m_curcell);

	m_curcell += move;
	if (m_curcell >= m_count) m_curcell = 0;
	if (m_curcell < 0) m_curcell = m_count - 1;	

	m_edit->setBitmap(m_cells[m_curcell].pattern + 1);
	m_edit->setAttribute(m_cells[m_curcell].pattern[0]);
	SCOL(m_curcell);

	redraw();
	return ZXE_CONTINUE;
}

void Jsw64CellForm::setupCaptions()
{
	for (int n = 0; n < m_count; n++)
	{
		switch(m_cells[n].behaviour)
		{
			case CB_AIR:	m_captions[n] = "Air"; break;
			default:
			case CB_WATER:	m_captions[n] = "Water"; break;
			case CB_EARTH:	m_captions[n] = "Earth"; break;
			case CB_FIRE:	m_captions[n] = "Fire"; break;
			case CB_LRAMP:	m_captions[n] = "\\ Ramp"; break;
			case CB_RRAMP:	m_captions[n] = "/ Ramp"; break;
			case CB_LCONV:	m_captions[n] = "<< Conveyor"; break;
			case CB_RCONV:	m_captions[n] = ">> Conveyor"; break;
			case CB_CRUMBLY:m_captions[n] = "Crumbling"; break;
			case CB_TRAMP:	m_captions[n] = "Trampoline"; break;
			case CB_TRAP:	m_captions[n] = "Trap"; break;
		}
	}
}
