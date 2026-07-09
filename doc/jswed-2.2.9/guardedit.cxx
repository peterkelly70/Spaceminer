/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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
#include "jswguardian.hxx"
#include "jswdirbox.hxx"
#include "hexedit.hxx"
#include "vspritelist.hxx"

GuardianEditor::GuardianEditor(int roomNo, JswGame *g,jswByte *guard,VideoBitmap *bg, int x)
	 : VideoForm(CHAR_W * 3, CHAR_H, CHAR_W * 34, CHAR_H * 28)
{
	m_roomNo = roomNo;
	m_nx     = x;
	m_bmBg   = bg;
	m_bmAnim = newVideoBitmap(512, 256); 
	m_game = g;
	m_gdata = guard;
	m_ot = NULL;
	m_haveTick = 0;
	m_guard = g->newGuardian(guard, m_nx, 0);
	m_tick = 0;
}



void GuardianEditor::setupControls(void)
{
	m_bnPling  = new VideoButton('!', m_x +      CHAR_W,
					  m_y + 24 * CHAR_H,"  !   ");
	m_bnType   = new VideoButton('T', m_x +      CHAR_W,
					  m_y + 18 * CHAR_H," Type ");
	m_bnSprite = new VideoButton('S', m_x +  8 * CHAR_W,
					  m_y + 18 * CHAR_H, " Sprite ");
        m_bnOk = new VideoButton(ZXE_OK,  m_x +      CHAR_W, 
					  m_y + m_h - 2 * CHAR_H, "  OK  ");
        m_bnCancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 * CHAR_W, 
					m_y + m_h - 2 * CHAR_H, "Cancel");
	m_bnPling->setListener(this);	
	m_bnType->setListener(this);
	m_bnSprite->setListener(this);

	m_labelY = new VideoLabel(m_x + 17 * CHAR_W, m_y + 18 * CHAR_H,
					"Y:");
	m_spinY = new VideoSpinControl(m_x + 19 * CHAR_W, 
				       m_y + 18 * CHAR_H, 
				       7 * CHAR_W, CHAR_H + 4);
	m_spinVstep  = new VideoSpinControl(m_x + 27 * CHAR_W-4, m_y + 18 * CHAR_H,
					7 * CHAR_W, CHAR_H + 4);
	m_spinVstep->setDelta(2);
	m_spinVstep->setSigned(1);
	m_spinVstep->setListener(this);

	m_spinY->setListener(this);
	m_spinY->setDelta(2);
	m_chInk = new VideoColourChooser(m_x + 8 * CHAR_W, m_y + 20 * CHAR_H);
	m_chInk->setListener(this);

	m_cbBright = new VideoCheckbox(m_x + 17 * CHAR_W, m_y + 20 * CHAR_H);
	m_cbBright->setListener(this);

	m_cbDirection    = new JswDirCheckbox(m_x + 6*CHAR_W, m_y + 20 * CHAR_W);
	m_cbDirection->setListener(this);
	m_labelDirection = new VideoLabel(m_x + CHAR_W, m_y + 20 * CHAR_W, "L/R?");

	m_labelBright = new VideoLabel(m_x + 18 * CHAR_W + 2, m_y + 20 * CHAR_H,
					"Bright");
	m_bmEdit = new BitmapEditor(m_x + 8 *CHAR_W, m_y + 18 * CHAR_H, 8, 1, 1);

	m_labelAnim = new VideoLabel(m_x + 8 * CHAR_W, m_y + 22 * CHAR_H, "Animation"); 
	m_spinAnim  = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 22 * CHAR_H,
					   7 * CHAR_W, CHAR_H + 4);
	m_spinAnim->setListener(this);

	m_labelRopeX   = new VideoLabel(m_x + 8 * CHAR_W, m_y + 20 * CHAR_H, "Rope pos.");
	m_spinRopeX    = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 20 * CHAR_H,
					   8 * CHAR_W, CHAR_H + 4);
	m_spinRopeX->setDelta(2);
	m_spinRopeX->setSigned(1);
	m_spinRopeX->setListener(this);

	m_labelStartPos = new VideoLabel(m_x + 8 * CHAR_W, m_y + 22 * CHAR_H, "Start pos.");
	m_spinStartPos = new VideoSpinControl(m_x + 23 * CHAR_W, m_y + 22 * CHAR_H, 7*CHAR_W, CHAR_H+4);
	m_spinStartPos->setListener(this);

        m_labelRopeLen = new VideoLabel(m_x + 8 * CHAR_W, m_y + 22 * CHAR_H, "Rope len.");
	m_spinRopeLen =  new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 22 * CHAR_H,
                                           7 * CHAR_W, CHAR_H + 4);
	m_spinRopeLen->setListener(this);

	m_labelRopeSwing = new VideoLabel(m_x + 8 * CHAR_W, m_y + 24 * CHAR_H, "Rope swing");
	m_spinRopeSwing = new VideoSpinControl(m_x + 19 * CHAR_W, m_y + 24 * CHAR_H, 
					7 * CHAR_W, CHAR_H + 4);
	m_spinRopeSwing->setListener(this);
}



void GuardianEditor::deleteControls()
{
	delete m_labelStartPos;
	m_spinStartPos->bind(NULL);
	delete m_spinStartPos;
	delete m_labelDirection;
	m_spinVstep->bind(NULL);
	delete m_spinVstep;
	delete m_labelRopeX;
	m_spinRopeX->bind(NULL);
	delete m_spinRopeX;
	m_spinRopeSwing->bind(NULL);
	delete m_spinRopeSwing;
	delete m_labelRopeSwing;
	m_spinRopeLen->bind(NULL);
	delete m_spinRopeLen;
	delete m_labelRopeLen;
	delete m_bmAnim;
	delete m_bnType;
	delete m_bnPling;
	delete m_bnSprite;
	delete m_labelY;
	delete m_spinY;
	delete m_labelAnim;
	m_spinAnim->bind(NULL);
	delete m_spinAnim;
	delete m_chInk;
	delete m_cbBright;
	delete m_labelBright;
	delete m_bmEdit;
	delete m_cbDirection;
	delete m_bnOk;
	delete m_bnCancel;
}

GuardianEditor::~GuardianEditor()
{
	// nb: Delete guard *after* deleting controls; otherwise the
	// control destructors may cause callbacks that try to access
	// the guard.
	if (m_guard) delete m_guard;
}


void GuardianEditor::drawFixed(void)
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

void GuardianEditor::drawData(int tickonly)
{
	char hexBytes[80];

	if (m_bmBg) m_bmBg->toSurface(m_bmAnim->getSurface(), 0, 0);
	else	    m_bmAnim->getSurface()->fillBox(0, 0, 512, 256, ZX_BLACK);

	m_guard->draw(m_bmAnim->getSurface());
	m_guard->drawBounds(m_bmAnim->getSurface());

	m_bmAnim->toScreen(m_x + CHAR_W, m_y + CHAR_H);	

	strcpy(hexBytes, "As Hex:");
	for (int n = 0; n < 8; n++) 
	{
		sprintf(hexBytes + strlen(hexBytes), " %02x", m_gdata[n]);
	}
	strcat(hexBytes, ". Click \"!\" for manual override");
	
	videoScreen->drawSmallText(m_x +      CHAR_W,
                                    m_y + 17 * CHAR_H + (CHAR_SH / 2),
				    hexBytes, ZX_BLACK, ZX_WHITE);
	if (!tickonly)
	{	
       		m_chInk->setColour(m_gdata[1] & 7);
	        m_cbBright->setCheck((m_gdata[1] & 8) ? 1 : 0);

		drawTsData();
	}
        if (m_gdata[7] > 20) m_spinRopeSwing->setDelta(2);
        else                 m_spinRopeSwing->setDelta(4);
}

void GuardianEditor::redraw(void)
{
	// Don't call base class. 
	//        videoScreen->fillBox(m_x, m_y, m_w, m_h, ZX_WHITE);

	drawFixed();
	drawData(0);
}



void GuardianEditor::showChildren(int redraw)
{
	postContextMenu();
	addChild(m_bnType);
	addChild(m_cbDirection);
	addChild(m_labelDirection);
	addChild(m_bnPling);
	addChild(m_bnOk);
	addChild(m_bnCancel);
	changeType();
	VideoForm::showChildren(redraw);
}

void GuardianEditor::hideChildren(void)
{
	removeChild(m_bnType);
	removeChild(m_bnPling);
	removeChild(m_cbDirection);
	removeChild(m_labelDirection);
	removeChild(m_bnOk);
	removeChild(m_bnCancel);
	removeVarChildren();
	preContextMenu();
	VideoForm::hideChildren();
}



void GuardianEditor::preContextMenu()
{
	if (m_haveTick)
	{
		setTickListener(m_ot);
		m_haveTick = 0;
	}
}


void GuardianEditor::postContextMenu()
{
        if (!m_haveTick)
        {
                m_ot = setTickListener(this);
                m_haveTick = 1;
        }
}





void GuardianEditor::onTick(void)
{
	++m_tick;
	if ((m_tick % 5) == 0 && m_guard != NULL)
	{
		m_guard->move();
		drawData(1);
	}
}

int GuardianEditor::setSprite(void)
{
	preContextMenu();

        VSpriteList list(m_game->getMem());
        m_game->fillSprites(m_roomNo, &list);
        list.setTitle("Choose sprite");

        list.setSelected(m_gdata[5] * 256);

        int rv = list.doModal();

        if (rv >= ZXE_QUIT) return rv;
        if (rv != ZXE_OK)   
	{
		postContextMenu();
		return ZXE_CONTINUE;
	}
	m_gdata[5] = list.getSelected() / 256;

	postContextMenu();
	m_guard->setSpritePage(m_gdata[5]);
	drawData(0);
	return ZXE_CONTINUE;
}




void GuardianEditor::removeVarChildren(void)
{
	removeChild(m_spinStartPos);
	removeChild(m_labelStartPos);
	removeChild(m_spinVstep);
	removeChild(m_bnSprite);
	removeChild(m_spinY);
	removeChild(m_labelY);
	removeChild(m_spinRopeX);
	removeChild(m_labelRopeX);
	removeChild(m_spinRopeLen);
	removeChild(m_labelRopeLen);
	removeChild(m_spinRopeSwing);
	removeChild(m_labelRopeSwing);	
	removeChild(m_spinAnim);
	removeChild(m_labelAnim);
	removeChild(m_chInk);
	removeChild(m_cbBright);
	removeChild(m_labelBright);
	removeChild(m_bmEdit);
}



int GuardianEditor::onButtonSelect(VideoButton *b)
{
	switch(b->getId())
	{
		case 'T': return setGuardianType();
		case 'S': return setSprite();
		case '!': return onPling();

	}
	return b->getId();
}



void GuardianEditor::onChooseColour(VideoColourChooser *c)
{
	if (c == m_chInk)
	{
		m_gdata[1] &= ~7;
		m_gdata[1] |= c->getColour();
		m_guard->setInk(m_gdata[1] & 0x0F);
	}
}


int  GuardianEditor::onCheckboxSelect(VideoCheckbox *c, int value)
{
	if (c == m_cbBright)
	{
		m_gdata[1] &= ~8;
		m_gdata[1] |= (value ? 8 : 0);
		m_guard->setInk(m_gdata[1] & 0x0F);
		return ZXE_OK;	
	}
	return subCheckboxSelect(c, value);
}


void GuardianEditor::addChild(VideoControl *c)
{
	VideoForm::addChild(c);
	c->setVisible(1, 0);
}

void GuardianEditor::removeChild(VideoControl *c)
{
	c->setVisible(0, 0);	
	VideoForm::removeChild(c);
}


int GuardianEditor::onPling(void)
{
	HexEditor hex(m_gdata);

	preContextMenu();
	int rv = hex.doModal();
	postContextMenu();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

        delete m_guard;
        m_guard = m_game->newGuardian(m_gdata, m_nx, 0);          
        changeType();
        drawData(0);                                                            
	return ZXE_CONTINUE;
}

void GuardianEditor::onEditChange(VideoControl *v)
{
	subEditChange(v);
}


int GuardianEditor::doModal()
{
	postContextMenu();
	int r = VideoForm::doModal();
	preContextMenu();
	return r;
}
