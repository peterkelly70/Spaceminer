/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

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
#include "j64gtableform.hxx"
#include "room.hxx"

Jsw64GuardianTableForm::Jsw64GuardianTableForm(Room *rm) : 
		VideoForm( 3 * CHAR_W, 10 * CHAR_H, 34 * CHAR_W, 12 * CHAR_H)
{
	m_room = rm;
	m_bmp = newVideoBitmap(512, 8 * CHAR_H);

	m_ok = new VideoButton(ZXE_OK, 30 * CHAR_W, 20 * CHAR_H, 
		     "  OK  ");
	m_cancel = new VideoButton(ZXE_CANCEL, 22 * CHAR_W, 20 * CHAR_H, 
                     "Cancel");
	m_room->getRoomMemMap(m_buf);
	m_label = new VideoLabel(m_x + CHAR_W, m_y + CHAR_H + CHAR_SH, 
			"Guardian table size");
	m_spinSize = new VideoSpinControl(m_x + 22 * CHAR_W, 
			m_y + CHAR_H + CHAR_SH,
			7 * CHAR_W, CHAR_H + 4);
	m_spinSize->setLimits(0, 32);
	m_spinSize->setValue(rm->getMaxGuards());
}



Jsw64GuardianTableForm::~Jsw64GuardianTableForm()
{
	delete m_bmp;
	delete m_spinSize;
	delete m_label;
	delete m_ok;
	delete m_cancel;
}

void Jsw64GuardianTableForm::redraw(void)
{
	VideoForm::redraw();
	videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);			
	videoScreen->drawText(m_x, m_y, "Guardian table           "
                              "         ", ZX_BRWHITE, ZX_BLACK);
	videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);


	int y = m_y + 3 * CHAR_H + CHAR_SH;

	videoScreen->fillBox(m_x + CHAR_W, y, CHAR_W, CHAR_H, ZX_GREEN);
	videoScreen->drawText(m_x + 2 * CHAR_W, y, "Free", ZX_BLACK, ZX_WHITE);
	videoScreen->fillBox(m_x + 11 * CHAR_W, y, CHAR_W, CHAR_H,
			ZX_RED);
	videoScreen->drawText(m_x +12 * CHAR_W, y, "Guards", ZX_BLACK, ZX_WHITE);
	videoScreen->fillBox(m_x + 21 * CHAR_W, y, CHAR_W, CHAR_H,
			ZX_BLUE);
	videoScreen->drawText(m_x +22 * CHAR_W, y, "Other", ZX_BLACK, ZX_WHITE);
	redrawMap();
}

void Jsw64GuardianTableForm::redrawMap(void)
{
	int x,y, n, ink;

	x = 0;
	y = 0;
	for (n = 0; m_buf[n]; n++)
	{
		ink = ZX_BLUE;
		switch(m_buf[n])
		{
			case RMM_FREE:		ink = ZX_GREEN; break;
			case RMM_GUARDIAN:	ink = ZX_RED;   break;
			case RMM_SPRITE:	ink = ZX_CYAN;  break;
		}
		m_bmp->getSurface()->fillBox(x, y, 2, CHAR_H, ink);
		x += 2;
		if ((n % 256) == 255)
		{
			x -= 512;
			y += CHAR_H;
		}
	}
	m_bmp->toScreen(m_x + CHAR_W, m_y + 5 * CHAR_H, 0, 0, 512, y);
	videoScreen->box(m_x + CHAR_W - 1, m_y + 5 * CHAR_H - 1, 
			514, y + 2, ZX_BLACK);
}



int Jsw64GuardianTableForm::doModal()
{
	int r = VideoForm::doModal();
	int first, last, prevmax, newmax;
	jswByte *copy, *prev;

	if (r == ZXE_OK)
	{
		char *s = strchr(m_buf, RMM_GUARDIAN);
		if (!s) return r;
		first = s - m_buf;
		for (last = first; m_buf[last] == RMM_GUARDIAN; ++last);
		newmax = (last - first - 1) / 8;
		copy = new jswByte[8 * newmax + 1];
		prev = m_room->getGuardianBuffer();
		prevmax = m_room->getMaxGuards();
		if (prevmax > newmax) prevmax = newmax;
		memcpy(copy, prev, prevmax * 8 + 1);
		copy[prevmax * 8] = 0xFF;
		m_room->setGuardianTable(first + 0x8000);
		memcpy(m_room->getGuardianBuffer(), copy, prevmax * 8 + 1);
		delete copy;
	}
	return r;
}


void Jsw64GuardianTableForm::showChildren(int redraw)
{
	// Order does matter - it dictates tabbing order 
	addChild(m_spinSize);
	addChild(m_label);
	addChild(m_ok);
        addChild(m_cancel);
	m_spinSize->setListener(this);
	VideoForm::showChildren(redraw);
}

void Jsw64GuardianTableForm::hideChildren(void)
{
	VideoForm::hideChildren();
	removeChild(m_label);
	removeChild(m_spinSize);
        removeChild(m_ok);
        removeChild(m_cancel);
	m_spinSize->setListener(NULL);
}


void Jsw64GuardianTableForm::onEditChange(VideoControl *c)
{
	char match[1025];
	int count, n, offset;
	char *ptr = NULL;

	if (c == m_spinSize)
	{
		for (n = 0; m_buf[n]; n++) 
		{
			if (m_buf[n] == RMM_GUARDIAN) m_buf[n] = RMM_FREE;
		}
		count = m_spinSize->getValue();
		for (n = 0; n < (8*count+1); n++) match[n] = RMM_FREE;
		match[n] = 0;
		ptr = strstr(m_buf, match);
		while (!ptr)
		{
			count--;
			match[8*count+1] = 0;
			ptr = strstr(m_buf, match);
			m_spinSize->setListener(NULL);
			m_spinSize->setValue(count);	
			m_spinSize->setListener(this);
		}
		offset = ptr - m_buf;
		for (n = 0; m_buf[n + offset] == RMM_FREE; n++);
		offset += (n - (8*count + 1));
		for (n = 0; n < (8*count+1); n++)
		       m_buf[n + offset] = RMM_GUARDIAN;
		redrawMap();
	}
}
