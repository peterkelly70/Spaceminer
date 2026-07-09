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
#include "guardedit.hxx"
#include "j128gedit.hxx"
#include "j128guard.hxx"
#include "jswdirbox.hxx"
#include "hexedit.hxx"
#include "vspritelist.hxx"

Jsw128GuardEditor::Jsw128GuardEditor(int room, JswGame *g,jswByte *guard,VideoBitmap *bg, int x) 
	 : GuardianEditor(room, g, guard, bg, x)
{
	m_spinAnim = NULL;
	m_spinVstep = NULL;
	m_spinY = NULL;
	m_spinBounds[0] = m_spinBounds[1] = NULL;
	setupControls();

	m_labelStopped = new VideoLabel(m_x + 8 * CHAR_W, m_y + 18 * CHAR_H, "Stopped");
	m_cbStopped   = new VideoCheckbox(m_x + 19 * CHAR_W, m_y + 18 * CHAR_H);
        m_labelAnim2 = new VideoLabel(m_x + 27 * CHAR_W, m_y + 22 * CHAR_H, "Fast");
        m_cbAnim2    = new VideoCheckbox(m_x + 32 * CHAR_W, m_y + 22 * CHAR_H);

        m_labelSlow = new VideoLabel(m_x + 27 * CHAR_W, m_y + 20 * CHAR_H, "Slow");
        m_cbSlow    = new VideoCheckbox(m_x + 32 * CHAR_W, m_y + 20 * CHAR_H);

        m_labelBounds = new VideoLabel(m_x + 8 * CHAR_W, m_y + 24 * CHAR_H, "Bounds");
        m_spinBounds[0] = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_spinBounds[1] = new VideoSpinControl(m_x + 27 * CHAR_W-4, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_labelH = new VideoLabel(m_x + 10 * CHAR_W, m_y + 18 * CHAR_H, "Height");

	m_labelSW[0] = new VideoLabel(m_x + 19 * CHAR_W, m_y + 20 * CHAR_H,
					"On/Off?");
	m_labelSW[1] = new VideoLabel(m_x + 19 * CHAR_W, m_y + 22 * CHAR_H,
					"Stays off?");
	m_labelSW[2] = new VideoLabel(m_x + 19 * CHAR_W, m_y + 24 * CHAR_H,
					"Stays on?");
	m_cbSW[0]    = new VideoCheckbox(m_x + 30 * CHAR_W, m_y + 20 * CHAR_H);
	m_cbSW[1]    = new VideoCheckbox(m_x + 30 * CHAR_W, m_y + 22 * CHAR_H);
	m_cbSW[2]    = new VideoCheckbox(m_x + 30 * CHAR_W, m_y + 24 * CHAR_H);

	m_cbSW[0]->setListener(this);
	m_cbSW[1]->setListener(this);
	m_cbSW[2]->setListener(this);
        m_spinBounds[0]->setListener(this);
        m_spinBounds[1]->setListener(this);	
	m_cbAnim2->setListener(this);
	m_cbStopped->setListener(this);
	m_cbSlow->setListener(this);
}


Jsw128GuardEditor::~Jsw128GuardEditor()
{
	delete m_cbSW[0];
	delete m_cbSW[1];
	delete m_cbSW[2];
	delete m_labelSW[0];
	delete m_labelSW[1];
	delete m_labelSW[2];
	m_spinBounds[0]->bind(NULL);
	m_spinBounds[1]->bind(NULL);
	delete m_spinBounds[0];
	delete m_spinBounds[1];
	delete m_labelBounds;
	delete m_labelAnim2;
	delete m_cbAnim2;
	delete m_labelSlow;
	delete m_cbSlow;
	delete m_labelStopped;
	delete m_cbStopped;
	delete m_labelH;
	deleteControls();
}


void Jsw128GuardEditor::subEditChange(VideoControl *c)
{	
        int type = m_gdata[0] & 0x0F;

	if (type == 8) type = m_gdata[0];

	switch(type)
	{
		case 1: case 5:  case 6: 
		case 9: case 13: case 14:
		if (c == m_spinBounds[0] || c == m_spinBounds[1])
		{
			m_gdata[6] = (m_gdata[6] & 0xE0) | (m_bound1 & 0x1F);
			m_gdata[7] = (m_gdata[7] & 0xE0) | (m_bound2 & 0x1F);
			((Jsw128Guard *)m_guard)->setBounds(m_gdata[6] & 0x1F, m_gdata[7] & 0x1F);
		}
		break;
		case 2: case 10:
		case 7: case 15:
		// Skylabs and Eugenes
		case 0x08: case 0x18: case 0x28: case 0x38: case 0x58: 
		case 0x68:
		if (c == m_spinBounds[0] || c == m_spinBounds[1])
		{
			((Jsw128Guard *)m_guard)->setBounds(m_gdata[6], m_gdata[7]);
		}
		break;
	}

        switch(type)
        {
		case 1: case 2:  case  5: case  6: case 7:
		case 9: case 10: case 13: case 14: case 15:
		// Skylabs and Eugenes
		case 0x08: case 0x18: case 0x28: case 0x38: 
		case 0x58: case 0x68:
                if (c == m_spinVstep)
                {
                        m_guard->setStep(m_gdata[4]);
                }
                if (c == m_spinY)
                {
                        m_guard->setY(m_gdata[3]);
                }
                if (c == m_spinAnim)
                {
                        m_guard->setAnim(m_spinAnim->getValue());
                        m_gdata[1] &= 0x1F;
                        m_gdata[1] |= ((m_spinAnim->getValue() & 0x07) << 5);
                }
                break;

		case 0xA8:
                if (c == m_spinY)
                {
                        m_guard->setStep(m_gdata[3]);
                }
		break;

                case 3:
                if (c == m_spinRopeLen)
                {
                        m_guard->setRopeLen(m_spinRopeLen->getValue());
                        m_gdata[4] = m_spinRopeLen->getValue();
                }
                if (c == m_spinRopeSwing)
                {
                        m_guard->setRopeSwing(m_spinRopeSwing->getValue());
                        m_gdata[7] = m_spinRopeSwing->getValue();
                }
                if (c == m_spinRopeX)
                {
                        int rx = m_spinRopeX->getValue();
                        if (rx < 0) m_gdata[1] = 0x80 | (-rx);
                        else        m_gdata[1] = rx;
                }
                break;
        }
}

int  Jsw128GuardEditor::subCheckboxSelect(VideoCheckbox *c, int value)
{
        int type = m_gdata[0] & 0x0F;

	if (c == m_cbAnim2) switch(type)
	{
		case 2: case 10: case 7: case 15:
			m_gdata[0] &= ~0x10;
			m_gdata[0] |= (value ? 0x10 : 0);
			((Jsw128Guard *)m_guard)->setFastAnim(value);
			return ZXE_OK;

		case 1: case  9: case  5: 
		case 6: case 13: case 14:
			m_gdata[7] &= ~0x80;
			m_gdata[7] |= (value ? 0x80 : 0);
			((Jsw128Guard *)m_guard)->setFastAnim(value);
			return ZXE_OK;
	}
	if (c == m_cbSlow)
	{
		m_gdata[7] &= ~0x20;
		m_gdata[7] |= (value ? 0x20 : 0);
		((Jsw128Guard *)m_guard)->setSlow(m_gdata[7] & 0x20);
	}
	if (c == m_cbStopped)
	{
		m_gdata[0] &= ~0x40;
		m_gdata[0] |= (value ? 0x40 : 0);
		((Jsw128Guard *)m_guard)->setStopped(m_gdata[0] & 0x40);
	}
        if (c == m_cbDirection)
        {
                m_gdata[0] &= 0x7F;
                if (value) m_gdata[0] |= 0x80;
        }
	if (c == m_cbSW[0])
	{
		m_gdata[4] = value ? 0 : 1;
	}
	if (c == m_cbSW[1])
	{
		m_gdata[6] = value ? 1 : 0;
	}
	if (c == m_cbSW[2])
	{
		m_gdata[7] = value ? 0 : 1;
	}
	return ZXE_OK;
}

void Jsw128GuardEditor::changeType(void)
{
	int t = m_gdata[0] & 0x0F;
	if (t == 8) t = m_gdata[0];
	
	removeVarChildren();
	switch(t)
	{
		// Horizontals and diagonals
		case 5: case 6: 
		case 13: case 14: 
			addChild(m_spinVstep);
                        m_spinVstep->bind(m_gdata + 4);
		case 1: case 9:
			addChild(m_labelDirection);
			addChild(m_cbDirection);
			if (m_game->supportsExtGuards())
			{
				addChild(m_labelAnim2);
				addChild(m_cbAnim2);
				addChild(m_labelSlow);
				addChild(m_cbSlow);
			}
			addChild(m_bnSprite);
			addChild(m_labelY);
			addChild(m_spinY);
			m_spinY->bind(m_gdata + 3);
			addChild(m_chInk);
			addChild(m_cbBright);
			addChild(m_labelBright);
			addChild(m_labelAnim);
			addChild(m_spinAnim);
			m_cbDirection->setVertical(0);
			addChild(m_labelBounds);
			addChild(m_spinBounds[0]);
			addChild(m_spinBounds[1]);
			m_bound1 = m_gdata[6] & 0x1F;
			m_bound2 = m_gdata[7] & 0x1F;
			m_spinBounds[0]->bind(&m_bound1);
			m_spinBounds[1]->bind(&m_bound2);
			m_spinBounds[0]->setDelta(1);
			m_spinBounds[1]->setDelta(1);
			m_spinBounds[0]->setLimits(0, 31);
			m_spinBounds[1]->setLimits(0, 31);
			break;
		// Verticals
		case 2: case 10: 
			addChild(m_labelAnim2);
			addChild(m_cbAnim2);
		case 7: case 15:
			addChild(m_labelDirection);
			addChild(m_cbDirection);
			addChild(m_bnSprite);
			addChild(m_labelY);
			addChild(m_spinY);
			m_spinY->bind(m_gdata + 3);
			addChild(m_spinVstep);
			m_spinVstep->bind(m_gdata + 4);
			addChild(m_chInk);
			addChild(m_cbBright);
			addChild(m_labelBright);
                        addChild(m_labelAnim);
                        addChild(m_spinAnim); 
			m_cbDirection->setVertical(1);
			addChild(m_labelBounds);
                        addChild(m_spinBounds[0]);
                        addChild(m_spinBounds[1]);
                        m_spinBounds[0]->bind(m_gdata + 6);
                        m_spinBounds[1]->bind(m_gdata + 7);
                        m_spinBounds[0]->setDelta(2);
                        m_spinBounds[1]->setDelta(2);

			break;
		// Skylabs, Eugenes
		case 0x08: case 0x18: case 0x28: case 0x38:
		case 0x58: case 0x68:
			addChild(m_bnSprite);
			addChild(m_labelY);
			addChild(m_spinY);
			addChild(m_labelAnim);
			m_spinY->bind(m_gdata + 3);
			addChild(m_spinVstep);
			m_spinVstep->bind(m_gdata + 4);
			addChild(m_chInk);
			addChild(m_cbBright);
			addChild(m_labelBright);
			m_cbDirection->setVertical(1);
			addChild(m_labelBounds);
                        addChild(m_spinBounds[0]);
                        addChild(m_spinBounds[1]);
                        m_spinBounds[0]->bind(m_gdata + 6);
                        m_spinBounds[1]->bind(m_gdata + 7);
                        m_spinBounds[0]->setDelta(2);
                        m_spinBounds[1]->setDelta(2);
			break;
		// Rope
		case 3: if (m_game->supportsExtGuards())
			{
				addChild(m_labelStopped);
				addChild(m_cbStopped);
			}
			addChild(m_labelDirection);
			addChild(m_cbDirection);
			addChild(m_spinRopeX);
			addChild(m_labelRopeX);
			addChild(m_spinRopeLen);
			addChild(m_labelRopeLen);
			addChild(m_spinRopeSwing);
			addChild(m_labelRopeSwing);
			m_cbDirection->setVertical(0);
			break;
		// Arrow
		case 4: addChild(m_labelDirection);
			addChild(m_cbDirection);
			addChild(m_bmEdit);
			m_bmEdit->setBitmap(m_gdata + 6);
			addChild(m_spinStartPos);
			addChild(m_labelStartPos);
			m_spinStartPos->bind(m_gdata + 4);	
			m_cbDirection->setVertical(0);
			break;
		// Switch
		case 0x98:
			addChild(m_chInk);
			addChild(m_cbBright);
			addChild(m_labelSW[0]);
			addChild(m_labelSW[1]);
			addChild(m_labelSW[2]);
			addChild(m_cbSW[0]);
			addChild(m_cbSW[1]);
			addChild(m_cbSW[2]);
			addChild(m_bnSprite);
			addChild(m_labelAnim);
			break;
		// Opening wall
		case 0xA8:
			addChild(m_labelH);
			addChild(m_spinY);
			m_spinY->bind(m_gdata + 3);
			break;
	}
	redraw();
}

void Jsw128GuardEditor::removeVarChildren(void)
{
        removeChild(m_cbSW[0]);
        removeChild(m_cbSW[1]);
        removeChild(m_cbSW[2]);
        removeChild(m_labelSW[0]);
        removeChild(m_labelSW[1]);
        removeChild(m_labelSW[2]);
        removeChild(m_labelH);
        removeChild(m_labelBounds);
        removeChild(m_spinBounds[0]);
        removeChild(m_spinBounds[1]);
        removeChild(m_cbAnim2);
        removeChild(m_labelAnim2);
        removeChild(m_cbSlow);
        removeChild(m_labelSlow);
	removeChild(m_cbStopped);
	removeChild(m_labelStopped);
	removeChild(m_cbDirection);
	removeChild(m_labelDirection);
	GuardianEditor::removeVarChildren();
}

void Jsw128GuardEditor::drawTsData(void)
{
	int type = m_gdata[0] & 0x0F;

        int rx = m_gdata[1] & 0x7F;
        if (m_gdata[1] & 0x80) rx = -rx;

	if (type == 8) type = m_gdata[0];
        m_cbDirection->setCheck(m_gdata[0] & 0x80);
	switch(type)
	{
	        case 2: case 10:
		case 7: case 15:
	        m_cbAnim2->setCheck((m_gdata[0] & 0x10) ? 1 : 0);
	        m_spinAnim->setValue(m_gdata[1] >> 5);
		break;

	        case 1: case 9:
	        case 5: case 6: case 13: case 14:
	        m_spinAnim->setValue(m_gdata[1] >> 5);
	        m_cbAnim2->setCheck((m_gdata[7] & 0x80) ? 1 : 0);
	        m_cbSlow->setCheck((m_gdata[7] & 0x20) ? 1 : 0);
	        break;

	        case 3:
		m_cbStopped->setCheck((m_gdata[0] & 0x40) ? 1 : 0);
	        m_spinRopeX->setValue(rx);
	        m_spinRopeLen->setValue(m_gdata[4]);
	        m_spinRopeSwing->setValue(m_gdata[7]);
	        break;

		case 0x98:
		m_cbSW[0]->setCheck(!m_gdata[4]);
		m_cbSW[1]->setCheck(m_gdata[6]);
		m_cbSW[2]->setCheck(!m_gdata[7]);
		break;
	}
}


int Jsw128GuardEditor::setGuardianType(void)
{
	int rv, nt;
	int ot;

	static jswByte stdRope[] = {0x03, 0x22, 0x00, 0x00, 
				    0x20, 0x00, 0x83, 0x36 };
	static jswByte stdArrow[]= {0x04, 0x06, 0x00, 0x00,
				    0xd0, 0x00, 0x41, 0x00 };
	static jswByte stdSkylab[] = { 0x08, 0x84, 0x0f, 0x10,
					0x04, 0x9D, 0x10, 0x60 };
	static jswByte stdTrigger[] = { 0x88, 0x53, 0x85, 0x28,
					0x02, 0xA2, 0xB0, 0xB0 };
	static jswByte stdSwitch[] = { 0x98, 0x06, 0x06, 0x40,
				       0x01, 0x9D, 0x00, 0x00 };
	static jswByte stdVWall[] = { 0xA8, 0x53, 0x85, 0x10,
				      0x10, 0x00, 0x10, 0xC0 };
/*
	static int types[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 13, 14 };
	VideoMenu vm48("Guardian type", "Blank", "Horizontal", "Vertical", 
			"Rope", "Arrow", NULL);
	VideoMenu vm128("Guardian type", 
			"Blank", 			// 0
			"Horizontal", 			// 1
			"Vertical",   			// 2
			"Rope",       			// 3
			"Arrow",      			// 4
			"Diagonal NW/SE", 		// 5
			"Diagonal NE/SW", 		// 6 
			"Vertical multicolour", 	// 7
			"Horizontal multicolour",	// 8
			"NW/SE multicolour", 		// 9
			"NE/SW multicolour");		// 10

	int rv, sel;

	preContextMenu();
	if (m_game->supportsExtGuards()) 
	{
		rv  = vm128.doModal();
		sel = vm128.getSelected();
	}
	else
	{
		rv  = vm48.doModal();
		sel = vm48.getSelected();
	}
	postContextMenu();
	*/
       	ot = m_gdata[0] & 0x0F;
	if (ot == 8) ot = m_gdata[0];
	preContextMenu();
	rv = m_game->chooseGuardianType(&nt);
	postContextMenu();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;
	
	switch(nt)
	{
		case 1:		
		case 5:
		case 6:
		case 9:
		case 13:
		case 14: if (ot == 1 || ot == 5  || ot == 6 || 
			    ot == 9 || ot == 13 || ot == 14)
			{
				m_gdata[0] &= 0xF0;
				m_gdata[0] |= nt;
			}
			else
			{
				m_gdata[0] = nt;
				m_gdata[6] = 19;
				m_gdata[7] = 29;
				m_gdata[4] = 0xFC;
			}
			break;	
		case 2:
		case 7:	if (ot == 2 || ot == 10 || ot == 7 || ot == 15)
			{
                                m_gdata[0] &= 0xF0;
                                m_gdata[0] |= nt;
			}
			else
			{
				m_gdata[0] = nt;
				m_gdata[6] = 0x80;
				m_gdata[7] = 0xE0;
				m_gdata[4] = 0x04;
			}
			break;
		case 3:	if (ot != 3) memcpy(m_gdata, stdRope, 8);
			break;
		case 4: if (ot != 4) memcpy(m_gdata, stdArrow, 8);
			break;
		case 0x08:	// Skylab
			if (ot != 8) memcpy(m_gdata, stdSkylab, 8);
			break;
		case 0x18:	// Angry Eugene 
		case 0x28:	// Angry Eugene, cycling JSW-style
		case 0x38:	// Angry Eugene, cycling MM-style
		case 0x58:	// Lift
		case 0x68:	// Scenery
			if (ot != 8 && ot != 0x18 && ot != 0x28 &&
			    ot != 0x58 && ot != 0x68) 
			{
				memcpy(m_gdata, stdSkylab, 8);
				m_gdata[0] = nt;
			}
			else	m_gdata[0] = nt;
			break;
		case 0x88:
		case 0xD8:
			if (ot != 0x88 && ot != 0xD8) 
				memcpy(m_gdata, stdTrigger, 8);
			else	m_gdata[0] = nt;
			break;
		case 0x98:
			if (ot != 0x98) memcpy(m_gdata, stdSwitch, 8);
			break;
		case 0xA8:
			if (ot != 0xA8) memcpy(m_gdata, stdVWall, 8);
			break;
		default:
			if ((nt & 0x0F) == 8)
			{
				m_gdata[0] = nt;
			}
			else
			{
				m_gdata[0] &= 0xF0; m_gdata[0] |= nt;	
			}
			break;
	}
	delete m_guard;
        m_guard = m_game->newGuardian(m_gdata, m_nx, 0);
	changeType();
	drawData(0);
	return ZXE_CONTINUE;
}

