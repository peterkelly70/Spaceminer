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
#include "manicgame.hxx"
#include "manicguardedit.hxx"
#include "manicguardian.hxx"
#include "jswdirbox.hxx"
#include "hexedit.hxx"
#include "vspritelist.hxx"

ManicGuardianEditor::ManicGuardianEditor(bool vertical, bool skylab, bool bidi,
		ManicGame *g, jswByte *room, jswByte *guard, VideoBitmap *bg)
	 : VideoForm(CHAR_W * 3, CHAR_H, CHAR_W * 34, CHAR_H * 28)
{
	m_bidi     = bidi;
	m_vertical = vertical;
	m_skylab   = skylab;
	m_bmBg   = bg;
	m_bmAnim = newVideoBitmap(512, 256); 
	m_game = g;
	m_room = room;
	m_gdata = guard;
	m_ot = NULL;
	m_haveTick = 0;

	setupControls();
	if (m_skylab)
		m_guard = new ManicSkylab(room, guard);
	else if (vertical)
		m_guard = new ManicVGuardian(room, guard);
	else	m_guard = new ManicHGuardian(bidi, room, guard);
}



void ManicGuardianEditor::setupControls(void)
{
	m_bnPling  = new VideoButton('!', m_x +      CHAR_W,
					  m_y + 24 * CHAR_H,"  !   ");
        m_bnOk = new VideoButton(ZXE_OK,  m_x +      CHAR_W, 
					  m_y + m_h - 2 * CHAR_H, "  OK  ");
        m_bnCancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 * CHAR_W, 
					m_y + m_h - 2 * CHAR_H, "Cancel");
	m_bnPling->setListener(this);	

	m_labelVstep = new VideoLabel(m_x + 20 * CHAR_W, m_y + 18 * CHAR_H,
					"Speed:");
	m_spinVstep  = new VideoSpinControl(m_x + 27 * CHAR_W-4, m_y + 18 * CHAR_H,
					7 * CHAR_W, CHAR_H + 4);
	m_spinVstep->setSigned(1);
	m_spinVstep->setListener(this);

	m_chInk = new VideoColourChooser(m_x + 8 * CHAR_W, m_y + 20 * CHAR_H);
	m_chInk->setListener(this);

	m_chPaper = new VideoColourChooser(m_x + 8 * CHAR_W, m_y + 22 * CHAR_H);
	m_chPaper->setListener(this);

	m_cbBright = new VideoCheckbox(m_x + 17 * CHAR_W, m_y + 20 * CHAR_H);
	m_cbBright->setListener(this);

	m_cbDirection    = new JswDirCheckbox(m_x + 6*CHAR_W, m_y + 20 * CHAR_W);
	if (m_vertical || m_skylab)
	{
		m_cbDirection->setVertical();
		m_labelDirection = new VideoLabel(m_x + CHAR_W, m_y + 20 * CHAR_W, "U/D?");
		m_labelAnim2 = new VideoLabel(m_x + 18 * CHAR_W + 2, m_y + 22 * CHAR_H,
					"Flash");
        	m_cbAnim2    = new VideoCheckbox(m_x + 17 * CHAR_W, m_y + 22 * CHAR_H);
	}
	else
	{
		m_labelDirection = new VideoLabel(m_x + CHAR_W, m_y + 20 * CHAR_W, "L/R?");
        	m_labelAnim2 = new VideoLabel(m_x + 27 * CHAR_W, m_y + 22 * CHAR_H, "Slow");
        	m_cbAnim2    = new VideoCheckbox(m_x + 32 * CHAR_W, m_y + 22 * CHAR_H);
	}
	m_cbDirection->setListener(this);
	m_labelBright = new VideoLabel(m_x + 18 * CHAR_W + 2, m_y + 20 * CHAR_H,
					"Bright");
	m_labelAnim = new VideoLabel(m_x + CHAR_W, m_y + 18 * CHAR_H, "Start frame"); 
	m_spinAnim  = new VideoSpinControl(m_x + 13 * CHAR_W, m_y + 18 * CHAR_H,
					   7 * CHAR_W, CHAR_H + 4);
	m_spinAnim->setListener(this);

	m_labelStartPos = new VideoLabel(m_x + 8 * CHAR_W, m_y + 22 * CHAR_H, "Start pos.");
	m_spinStartPos = new VideoSpinControl(m_x + 23 * CHAR_W, m_y + 22 * CHAR_H, 7*CHAR_W, CHAR_H+4);
	m_spinStartPos->setListener(this);


        m_labelBounds = new VideoLabel(m_x + 12 * CHAR_W, m_y + 24 * CHAR_H, "Bounds");
        m_spinBounds[0] = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_spinBounds[1] = new VideoSpinControl(m_x + 27 * CHAR_W-4, m_y + 24 * CHAR_H,
                                                7 * CHAR_W, CHAR_H + 4);
        m_spinBounds[0]->setListener(this);
        m_spinBounds[1]->setListener(this);
        m_cbAnim2->setListener(this);
}



void ManicGuardianEditor::deleteControls()
{
        delete m_spinBounds[0];
        delete m_spinBounds[1];
        delete m_labelBounds;
        delete m_labelAnim2;
        delete m_cbAnim2;
	delete m_labelStartPos;
	delete m_spinStartPos;
	delete m_labelDirection;
	delete m_spinVstep;
	delete m_bmAnim;
	delete m_bnPling;
	delete m_labelVstep;
	delete m_labelAnim;
	delete m_spinAnim;
	delete m_chInk;
	delete m_chPaper;
	delete m_cbBright;
	delete m_labelBright;
}

ManicGuardianEditor::~ManicGuardianEditor()
{
	// nb: Delete guard *after* deleting controls; otherwise the
	// control destructors may cause callbacks that try to access
	// the guard.
	deleteControls();
	if (m_guard) delete m_guard;
}


void ManicGuardianEditor::drawFixed(void)
{
        videoScreen->fillBox(m_x, m_y, m_w, CHAR_H, ZX_WHITE);
        videoScreen->fillBox(m_x, m_y, CHAR_W, m_h, ZX_WHITE);
        videoScreen->fillBox(m_x + m_w - CHAR_W, m_y, CHAR_W, m_h, ZX_WHITE);
        videoScreen->fillBox(m_x, m_y + 17 * CHAR_H, m_w, m_h - 17 * CHAR_H, ZX_WHITE); 
        videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
        for (int n = 0; n < m_ccount; n++)
        {
                m_controls[n]->redraw();
                m_controls[n]->drawFocus(n == m_focus);
        }
}

void ManicGuardianEditor::drawData(int tickonly)
{
	char hexBytes[80];

	if (m_bmBg) m_bmBg->toSurface(m_bmAnim->getSurface(), 0, 0);
	else	    m_bmAnim->getSurface()->fillBox(0, 0, 512, 256, ZX_BLACK);

	m_guard->draw(m_bmAnim->getSurface());
	m_guard->drawBounds(m_bmAnim->getSurface());

	m_bmAnim->toScreen(m_x + CHAR_W, m_y + CHAR_H);	

	strcpy(hexBytes, "As Hex:");
	for (int n = 0; n < 7; n++) 
	{
		sprintf(hexBytes + strlen(hexBytes), " %02x", m_gdata[n]);
	}
	strcat(hexBytes, ". Click \"!\" for manual override");
	
	videoScreen->drawSmallText(m_x +      CHAR_W,
                                    m_y + 17 * CHAR_H + (CHAR_SH / 2),
				    hexBytes, ZX_BLACK, ZX_WHITE);
	if (!tickonly)
	{	
       		m_chInk->setColour(m_gdata[0] & 7);
       		m_chPaper->setColour((m_gdata[0] >> 3) & 7);
	        m_cbBright->setCheck((m_gdata[0] & 0x40) ? 1 : 0);
	        m_cbAnim2->setCheck((m_gdata[0] & 0x80) ? 1 : 0);

		drawTsData();
	}
}

void ManicGuardianEditor::redraw(void)
{
	// Don't call base class. 
	//        videoScreen->fillBox(m_x, m_y, m_w, m_h, ZX_WHITE);

	drawFixed();
	drawData(0);
}



void ManicGuardianEditor::showChildren(int redraw)
{
	postContextMenu();
	if (m_vertical || m_skylab)
	{
		addChild(m_spinVstep);
		addChild(m_labelVstep);
		m_spinVstep->bind(m_gdata + 4);
		addChild(m_spinAnim);
		addChild(m_labelAnim);
		m_spinAnim->bind(m_gdata + 1);
		m_spinAnim->setLimits(0, 3);
	}
	else
	{
		addChild(m_spinAnim);
		addChild(m_labelAnim);
		m_spinAnim->bind(m_gdata + 4);
		m_spinAnim->setLimits(0, 7);
	}
	addChild(m_cbDirection);
	addChild(m_labelDirection);
	addChild(m_chInk);
	addChild(m_chPaper);
	addChild(m_cbBright);
	addChild(m_labelBright);
	addChild(m_labelAnim2);
	addChild(m_cbAnim2);
	addChild(m_labelBounds);
	addChild(m_spinBounds[0]);
	addChild(m_spinBounds[1]);
	addChild(m_bnPling);
	addChild(m_bnOk);
	addChild(m_bnCancel);

	m_spinBounds[0]->setDelta(1);
	m_spinBounds[1]->setDelta(1);
	if (m_vertical || m_skylab)
	{
		m_spinBounds[0]->setLimits(0, 112);
		m_spinBounds[1]->setLimits(0, 112);
		m_spinBounds[0]->bind(m_gdata + 5);
		m_spinBounds[1]->bind(m_gdata + 6);
	}
	else
	{
		m_ll = m_gdata[5] & 0x1F;
		m_rl = m_gdata[6] & 0x1F;
		m_spinBounds[0]->bind(&m_ll);
		m_spinBounds[1]->bind(&m_rl);
		m_spinBounds[0]->setLimits(0, 31);
		m_spinBounds[1]->setLimits(0, 31);
	}
	VideoForm::showChildren(redraw);
}

void ManicGuardianEditor::hideChildren(void)
{
	removeChild(m_bnPling);
	removeChild(m_cbDirection);
	removeChild(m_labelDirection);
	removeChild(m_chInk);
	removeChild(m_chPaper);
	removeChild(m_cbBright);
	removeChild(m_labelBright);
	removeChild(m_labelAnim);
	removeChild(m_spinAnim);
	removeChild(m_labelVstep);
	removeChild(m_spinVstep);
	removeChild(m_labelBounds);
	removeChild(m_spinBounds[0]);
	removeChild(m_spinBounds[1]);
	removeChild(m_bnOk);
	removeChild(m_bnCancel);
	removeVarChildren();
	preContextMenu();
	VideoForm::hideChildren();
}



void ManicGuardianEditor::preContextMenu()
{
	if (m_haveTick)
	{
		setTickListener(m_ot);
		m_haveTick = 0;
	}
}


void ManicGuardianEditor::postContextMenu()
{
        if (!m_haveTick)
        {
                m_ot = setTickListener(this);
                m_haveTick = 1;
        }
}





void ManicGuardianEditor::onTick(void)
{
	++m_tick;
	if ((m_tick % 5) == 0)
	{
		m_guard->move();
		drawData(1);
	}
}



void ManicGuardianEditor::removeVarChildren(void)
{
}



int ManicGuardianEditor::onButtonSelect(VideoButton *b)
{
	switch(b->getId())
	{
		case '!': return onPling();
	}
	return b->getId();
}



void ManicGuardianEditor::onChooseColour(VideoColourChooser *c)
{
	if (c == m_chInk)
	{
		m_gdata[0] &= ~7;
		m_gdata[0] |= c->getColour();
		m_guard->setInk(m_gdata[0] & 7);
	}
	if (c == m_chPaper)
	{
		m_gdata[0] &= ~0x38;
		m_gdata[0] |= (c->getColour() << 3);
		m_guard->setPaper((m_gdata[0] & 0x38) >> 3);
	}
}


int  ManicGuardianEditor::onCheckboxSelect(VideoCheckbox *c, int value)
{
	if (c == m_cbAnim2)
	{
		m_gdata[0] &= ~0x80;
		m_gdata[0] |= (value ? 0x80 : 0);
		m_guard->setFlash(m_gdata[0] & 0x80);
		return ZXE_OK;	
	}
	if (c == m_cbBright)
	{
		m_gdata[0] &= ~0x40;
		m_gdata[0] |= (value ? 0x40 : 0);
		m_guard->setBright(m_gdata[0] & 0x40);
		return ZXE_OK;	
	}
	if (c == m_cbDirection)
	{
		if (m_vertical || m_skylab)
		{
			m_gdata[4] = -m_gdata[4];
			m_guard->setSpeed(m_gdata[4]);
			m_spinVstep->setValue(m_gdata[4]);
		}
		else
		{
			m_gdata[4] = (value ? 0 : 7); 
			m_spinAnim->setValue(m_gdata[4]);
		}
		return ZXE_OK;
	}
}


void ManicGuardianEditor::addChild(VideoControl *c)
{
	VideoForm::addChild(c);
	c->setVisible(1, 0);
}

void ManicGuardianEditor::removeChild(VideoControl *c)
{
	c->setVisible(0, 0);	
	VideoForm::removeChild(c);
}


int ManicGuardianEditor::onPling(void)
{
	HexEditor hex(m_gdata, 7);

	preContextMenu();
	int rv = hex.doModal();
	postContextMenu();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

        delete m_guard;
	if (m_skylab)		m_guard = new ManicSkylab(m_room, m_gdata);
	else if (m_vertical)	m_guard = new ManicVGuardian(m_room, m_gdata);
	else			m_guard = new ManicHGuardian(m_bidi, m_room, m_gdata);
        drawData(0);                                                            
	return ZXE_CONTINUE;
}


void ManicGuardianEditor::onEditChange(VideoControl *v)
{
        if (v == m_spinBounds[0] || v == m_spinBounds[1])
        {
		if (m_vertical || m_skylab)
		{
                	(m_guard)->setBounds(m_gdata[5], m_gdata[6]);
		}
		else
		{
			m_gdata[5] = (m_gdata[5] & 0xE0) | m_ll;
			m_gdata[6] = (m_gdata[6] & 0xE0) | m_rl;
                	(m_guard)->setBounds(m_gdata[5], m_gdata[6]);
		}
        }
	if (v == m_spinVstep)
	{
		m_guard->setSpeed(m_gdata[4]);
		m_cbDirection->setCheck(!(m_gdata[4] & 0x80));
	}
	if (v == m_spinAnim)
	{
		// m_guard->setAnim(m_gdata[4]);
		m_cbDirection->setCheck(m_gdata[4] < 4);

	}
}



void ManicGuardianEditor::drawTsData()
{
	if (m_vertical || m_skylab)
	{
		m_cbDirection->setCheck(!(m_gdata[4] & 0x80));
	}
	else
	{
		m_cbDirection->setCheck(m_gdata[4] < 4);
	}
}
