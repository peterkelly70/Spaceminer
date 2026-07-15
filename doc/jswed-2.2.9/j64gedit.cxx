/************************************************************************

    JSWED 2.1.7 - Editor for Jet Set Willy and derivatives

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
#include "guardedit.hxx"
#include "j128gedit.hxx"
#include "j64gedit.hxx"
#include "j128guard.hxx"
#include "jswdirbox.hxx"
#include "hexedit.hxx"
#include "vspritelist.hxx"
#include "room.hxx"
#include "j64room.hxx"


Jsw64GuardEditor::Jsw64GuardEditor(int room, JswGame *g,jswByte *guard,VideoBitmap *bg) 
     : Jsw128GuardEditor(room, g, guard, bg, guard[2])
{
	m_triggerTarget = guard + 8;
	delete m_labelAnim;
	m_labelAnim = new VideoLabel(m_x + CHAR_W, m_y + 22 * CHAR_H, "Animation");
	delete m_bmEdit;
	m_bmEdit = new BitmapEditor(m_x + 17 * CHAR_W, m_y + 18 * CHAR_H, 8, 2, 1);
	m_spinFrame = new VideoSpinControl(m_x + 11 * CHAR_W - 4, 
			m_y + 22 * CHAR_H,
			7 * CHAR_W, CHAR_H + 4);
	m_frame = (guard[2] >> 5) & 7;

	m_btnTarget = new VideoButton('E', m_x + 8 * CHAR_W, m_y + 24 * CHAR_H,
			"Effect");
	m_btnSource = new VideoButton('S', m_x + 16 * CHAR_W, m_y + 24 * CHAR_H,
			"Source");

	m_btnTarget->setListener(this);
	m_btnSource->setListener(this);
}



Jsw64GuardEditor::~Jsw64GuardEditor()
{
	m_spinFrame->bind(NULL);
	delete m_spinFrame;
	delete m_btnTarget;
	delete m_btnSource;
}


/*
jswByte *Jsw64GuardEditor::maybeEugene(jswByte *guard)
{
	if (guard[0] == 0x88)
	{
		m_eguard = guard;
		memcpy(m_eugene, guard + 8, 4);
		m_eugene[0] = guard[3];
		memcpy(m_eugene + 4, guard + 4, 4);
		m_iseugene = true;
		return m_eugene;
	}
	else 
	{ 
		m_iseugene = false;
		return guard;
	}
}
*/


void Jsw64GuardEditor::removeVarChildren()
{
	removeChild(m_btnSource);
	removeChild(m_btnTarget);
	removeChild(m_spinFrame);
	Jsw128GuardEditor::removeVarChildren();
}

int Jsw64GuardEditor::doModal()
{
	int rv = Jsw128GuardEditor::doModal();

	// Retrieve the 'arrow' bitmap.
	if ((m_gdata[0] & 0x0F) == 4)	
	{
		m_gdata[6] = m_arrow[0];
		m_gdata[3] = m_arrow[1];
	}
/*	if (m_iseugene)
	{
		m_eguard[0] = 0x88;
		m_eguard[3] = m_eugene[0];
		memcpy(m_eguard + 4, m_eugene + 4, 4);
	} */
	return rv;
}


void Jsw64GuardEditor::changeType(void)
{
	int t = m_gdata[0] & 0x0F;
	if (t == 8) t = m_gdata[0];

	if (t == 4)
	{
		removeVarChildren();
		m_arrow[0] = m_gdata[6];
		m_arrow[1] = m_gdata[3];
		addChild(m_labelDirection);
		addChild(m_cbDirection);
		addChild(m_bmEdit);
		m_bmEdit->setBitmap(m_arrow);
		addChild(m_chInk);
		addChild(m_labelStartPos);
		addChild(m_spinStartPos);
		m_spinStartPos->bind(m_gdata + 4);
		m_cbDirection->setVertical(0);
		redraw();
		return;
	}

	Jsw128GuardEditor::changeType();
	switch(t)
	{
		case 5: case 6: case 13: case 14:
		case 1: case 9: case 2:  case 10: 
		case 7: case 15:
		case 0x08: case 0x18: case 0x28: case 0x38:
		case 0x58: case 0x68: case 0x98: 
			m_frame = (m_gdata[2] >> 5) & 7;
			addChild(m_spinFrame);
			m_spinFrame->bind(&m_frame);
			m_spinFrame->setLimits(0, 7);
			m_spinFrame->setListener(this);
			break;
		case 0x88:	// Trigger
		case 0xD8:	// Weak trigger
			addChild(m_btnSource);
			addChild(m_btnTarget);
			break;
		case 0xA8:	// Opening wall
			addChild(m_btnSource);
			break;
	}
	redraw();
}


void Jsw64GuardEditor::subEditChange(VideoControl *c)
{
	int type = m_gdata[0] & 0x0F;
	if (type == 8) type = m_gdata[0];

	if (c == m_spinFrame)
	{
		switch(type)
		{
			case 1:  case  9: case 2: case 10: 
			case 5:  case  6: case 7: 
			case 13: case 14: case 15:
			case 0x08: case 0x18: case 0x28: 
			case 0x38: case 0x58: case 0x68:
			case 0x98:
			m_gdata[2] &= 0x1F;
			m_gdata[2] |= (m_frame << 5);
			m_guard->setFrame(m_frame);
			return;
		}
	}
	Jsw128GuardEditor::subEditChange(c);
}


int Jsw64GuardEditor::onTriggerSource()
{
	int n, mg, valid;
	jswByte *buf, *roomData;
	char **options;
	int  *offsets;
	int  guardBase;
	Room *rm;

	roomData = m_game->getRoom(m_roomNo);
	rm       = m_game->getRoomClass(m_roomNo);
	buf      = ((Jsw64Room *)rm)->getGuardianBuffer();
	mg       = rm->getMaxGuards();
	options  = new (char *)[mg + 1];
	offsets  = new int[mg + 1];

	guardBase = roomData[0xDB] *256 + roomData[0xDA];

	options[0] = "Items taken in room";
	offsets[0] = 0x8553;
	valid = 1;
	for (n = 0; n < mg; n++)
	{
		if (buf[n*8] == 0xFF) break;
		switch(buf[n*8])
		{
			case 0x18:
			case 0x28:
			case 0x38: offsets[valid] = guardBase + n * 8 + 4; 
				   options[valid] = "Eugene";
				   ++valid;
				   break; 
			case 0x98: offsets[valid] = guardBase + n * 8 + 4; 
				   options[valid] = "Switch";
				   ++valid;
				   break; 
			case 0xA8: offsets[valid] = guardBase + n * 8 + 4; 
				   options[valid] = "Opening wall";
				   ++valid;
				   break; 
		}
	}
	preContextMenu();
	VideoMenu vm(valid, "Triggered by", options);
	int rv = vm.doModal();
	postContextMenu();

	if (rv == ZXE_OK)
	{
		valid = vm.getSelected();
		m_gdata[1] = (offsets[valid]) & 0xFF;
		m_gdata[2] = (offsets[valid]) >> 8;
		redraw();
		rv = ZXE_CONTINUE;
	}
	if (rv == ZXE_CANCEL)
	{
		rv = ZXE_CONTINUE;
	}

	delete options;
	delete offsets;
	return rv;
}


int Jsw64GuardEditor::onTriggerTarget()
{
	jswByte compositeGuard[8];
	int rv;

	if (m_triggerTarget[0] == 0xFF)
	{
		return ZXE_CONTINUE;
	}
	memcpy(compositeGuard, m_triggerTarget, 8);
	memcpy(compositeGuard + 4, m_gdata + 4, 4);
	compositeGuard[0] = m_gdata[3];
	Jsw64GuardEditor ge2(m_roomNo, m_game, compositeGuard, m_bmBg);

	preContextMenu();
	rv = ge2.doModal();	
	postContextMenu();
	if (rv == ZXE_OK)
	{
		memcpy(m_gdata + 4, compositeGuard + 4, 4);
		m_gdata[3] = compositeGuard[0];
		rv = ZXE_CONTINUE;
	}
	if (rv == ZXE_CANCEL) rv = ZXE_CONTINUE;
	return rv;
}

int Jsw64GuardEditor::onButtonSelect(VideoButton *b)
{
	if (b == m_btnSource)
	{
		return onTriggerSource();
	}
	if (b == m_btnTarget)
	{
		return onTriggerTarget();
	}
	return Jsw128GuardEditor::onButtonSelect(b);
}

void Jsw64GuardEditor::redraw()
{
	Jsw128GuardEditor::redraw();
	if (m_gdata[0] == 0x88 && m_triggerTarget[0] == 0xFF)
	{
		videoScreen->drawText(m_x + CHAR_W,
				     m_y + 20 * CHAR_H,
			"This trigger is not followed by",
			ZX_BRRED, ZX_WHITE);
		videoScreen->drawText(m_x + CHAR_W,
				     m_y + 21 * CHAR_H,
			"a guardian!",
			ZX_BRRED, ZX_WHITE);
	}
}

