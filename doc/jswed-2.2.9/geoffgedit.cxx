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
#include "guardedit.hxx"
#include "jswdirbox.hxx"
#include "geoffgedit.hxx"
#include "geoffguard.hxx"
#include "hexedit.hxx"
#include "vspritelist.hxx"

GeoffGuardEditor::GeoffGuardEditor(int room, JswGame *g,jswByte *guard,VideoBitmap *bg, int x) 
	 : GuardianEditor(room, g, guard, bg, x)
{
	setupControls();

        m_labelWrap = new VideoLabel(m_x + 27 * CHAR_W, m_y + 22 * CHAR_H, "Wrap");
        m_cbWrap    = new VideoCheckbox(m_x + 32 * CHAR_W, m_y + 22 * CHAR_H);

	m_labelDiagonal = new VideoLabel(m_x + CHAR_W, m_y + 22 * CHAR_H, "H/D?");
        m_cbDiagonal  = new JswDiagCheckbox(m_x + 6*CHAR_W, m_y + 22 * CHAR_H);
        m_cbDiagonal->setListener(this);

        m_labelTick = new VideoLabel(m_x + 8 * CHAR_W, m_y + 24 * CHAR_H, "Bounds");
        m_spinTick = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_spinTickLimit = new VideoSpinControl(m_x + 27 * CHAR_W-4, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_spinTick->setListener(this);
        m_spinTickLimit->setListener(this);
	m_cbWrap->setListener(this);
	m_cbDiagonal->setListener(this);
}

GeoffGuardEditor::~GeoffGuardEditor()
{
	delete m_spinTick;
	delete m_spinTickLimit;
	delete m_labelTick;
	delete m_labelWrap;
	delete m_labelDiagonal;
	delete m_cbDiagonal;
	delete m_cbWrap;
	deleteControls();
}


void GeoffGuardEditor::subEditChange(VideoControl *c)
{	
	if (c == m_spinTick)
	{
		((GeoffGuard *)m_guard)->setTick(m_gdata[6]);
	}
	if (c == m_spinTickLimit)
	{
		((GeoffGuard *)m_guard)->setTickLimit(m_gdata[7]);
	}
        int type = m_gdata[0] & 0x03;

        switch(type)
        {
		case 1: case 2: 
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

int  GeoffGuardEditor::subCheckboxSelect(VideoCheckbox *c, int value)
{
        int type = m_gdata[0] & 3;

	if (c == m_cbDiagonal && (type == 1))
	{
		m_gdata[0] &= ~0x18;
		m_gdata[0] |= ((value & 3) << 3);
		((GeoffGuard *)m_guard)->setDiagonal(value & 3);
		return ZXE_OK;
	}
	if (c == m_cbWrap && (type == 1 || type == 2))
	{
		m_gdata[0] &= ~0x4;
		m_gdata[0] |= (value ? 0x4 : 0);
		((GeoffGuard *)m_guard)->setWrap(value);
		return ZXE_OK;
	}
        if (c == m_cbDirection)
        {
                m_gdata[1] &= ~0x10;
                if (value) m_gdata[0] |= 0x10;
		((GeoffGuard *)m_guard)->setDirection(value);
        }
	return ZXE_OK;
}

void GeoffGuardEditor::changeType(void)
{
	int t = m_gdata[0] & 3;
	
	removeVarChildren();
	switch(t)
	{
		// Horizontals and diagonals
		case 1: case 2:
			addChild(m_cbWrap);
			addChild(m_labelWrap);
			addChild(m_cbDiagonal);
			addChild(m_labelDiagonal);
			addChild(m_spinVstep);
                        m_spinVstep->bind(m_gdata + 4);
			if (t == 1) m_spinVstep->setDelta(1);
			else	    m_spinVstep->setDelta(2);
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
			addChild(m_labelTick);
			addChild(m_spinTick);
			addChild(m_spinTickLimit);
			m_spinTick->bind(m_gdata + 6);
			m_spinTickLimit->bind(m_gdata + 7);
			break;
		// Rope
		case 3: addChild(m_spinRopeX);
			addChild(m_labelRopeX);
			addChild(m_spinRopeLen);
			addChild(m_labelRopeLen);
			addChild(m_spinRopeSwing);
			addChild(m_labelRopeSwing);
			m_cbDirection->setVertical(0);
			break;
		// Arrow
		case 0: addChild(m_bmEdit);
			m_bmEdit->setBitmap(m_gdata + 6);
			addChild(m_spinStartPos);
			addChild(m_labelStartPos);
			m_spinStartPos->bind(m_gdata + 4);	
			m_cbDirection->setVertical(0);
			break;
	}
	redraw();
}

void GeoffGuardEditor::removeVarChildren(void)
{
	removeChild(m_labelDiagonal);
	removeChild(m_cbDiagonal);
        removeChild(m_labelTick);
        removeChild(m_spinTick);
        removeChild(m_spinTickLimit);
        removeChild(m_cbWrap);
        removeChild(m_labelWrap);
	GuardianEditor::removeVarChildren();
}

void GeoffGuardEditor::drawTsData(void)
{
        int rx = m_gdata[1] & 0x7F;
        if (m_gdata[1] & 0x80) rx = -rx;

        m_cbDirection->setCheck(m_gdata[1] & 0x10);
	switch(m_gdata[0] & 3)
	{
	        case 1: case 2: 
	        m_cbWrap->setCheck((m_gdata[0] & 4) ? 1 : 0);
	        m_spinAnim->setValue(m_gdata[1] >> 5);
	        m_cbDiagonal->setCheck((m_gdata[0] >> 3) & 3);
		break;

	        case 3:
	        m_spinRopeX->setValue(rx);
	        m_spinRopeLen->setValue(m_gdata[4]);
	        m_spinRopeSwing->setValue(m_gdata[7]);
	        break;
	}
}


int GeoffGuardEditor::setGuardianType(void)
{
	static jswByte stdRope[] = {0x03, 0x22, 0x00, 0x00, 
				    0x20, 0x00, 0x83, 0x36 };
	static jswByte stdArrow[]= {0x04, 0x06, 0x00, 0x00,
				    0xd0, 0x00, 0x41, 0x00 };
	static jswByte stdHoriz[]= {0x01, 0xE6, 0x09, 0x80,
				    0x01, 0xB5, 0x08, 0x10 };
        static jswByte stdVert[] = {0x02, 0xE6, 0x09, 0x40,
                                    0x06, 0xB5, 0x08, 0x10 };

	static int types[] = { 0, 1, 2, 3, 4 };
	VideoMenu vm48("Guardian type", "Blank", "Horiz/Diag", "Vertical", 
			"Rope", "Arrow", NULL);

	int rv, sel;

	preContextMenu();

	rv  = vm48.doModal();
	sel = vm48.getSelected();

	postContextMenu();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;
	
	int ot = m_gdata[0] & 0x07;
	int nt = types[sel];
	switch(nt)
	{
		case 1:		
		case 2: 
		case 5:
		case 6: if (ot == 1 || ot == 2 ||
                            ot == 5 || ot == 6)
			{
				m_gdata[0] &= 0xF8;
				m_gdata[0] |= nt;
			}
			else
			{
				if (nt == 1 || nt == 5) 
					memcpy(m_gdata, stdHoriz, 8);
				else	memcpy(m_gdata, stdVert,  8);
			}
			break;	
		case 3:	if (ot != 3) memcpy(m_gdata, stdRope, 8);
			break;
		case 4: if (ot != 4) memcpy(m_gdata, stdArrow, 8);
			break;
		default:
			m_gdata[0] &= 0xF8; m_gdata[0] |= nt;	
	}
	delete m_guard;
        m_guard = m_game->newGuardian(m_gdata, m_nx, 0);
	changeType();
	drawData(0);
	return ZXE_CONTINUE;
}

