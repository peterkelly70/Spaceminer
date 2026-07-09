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
#include "manicgame.hxx"
#include "manicpatchpage.hxx"
#include "filetypelist.hxx"
#include "patch.hxx"

PatchLine plSpecialsAnywhwereBB[] = {
		{0x86FC, 0x3A, 0xCD}, /* 1 */
		{0x86FD, 0x73, 0x4C},
		{0x86FE, 0x80, 0x93},

		{0x8770, 0x07, 0x73}, /* Eugene check */
		{0x8771, 0x84, 0x80},
		{0x8772, 0xFE, 0xE6},
		{0x8773, 0x04, 0x80},
		{0x8774, 0xCC, 0xC4},

		{0x8778, 0x07, 0x73}, /* Skylab check */
		{0x8779, 0x84, 0x80},
		{0x877A, 0xFE, 0xE6},
		{0x877B, 0x0D, 0x40},
		{0x877C, 0xCA, 0xC2},

		{0x8780, 0x07, 0x73}, /* Vertical guard check */
		{0x8781, 0x84, 0x80},
		{0x8782, 0xFE, 0xE6},
		{0x8783, 0x08, 0x20},
		{0x8784, 0xD4, 0xC4},

		{0x8788, 0x07, 0x73}, /* Kong check */
		{0x8789, 0x84, 0x80},
		{0x878A, 0xFE, 0xE6},
		{0x878B, 0x07, 0x10},
		{0x878C, 0xCC, 0xC4},

		{0x8790, 0x07, 0x73}, /* Solar Power check (previously Kong II) */
		{0x8791, 0x84, 0x80},
		{0x8792, 0xFE, 0xE6},
		{0x8793, 0x0B, 0x08},
		{0x8794, 0xCC, 0xC4},
		{0x8795, 0x35, 0x73},
		{0x8796, 0x91, 0x8D},
		{0x8797, 0x3A, 0x00}, /* Old Solar power check */
		{0x8798, 0x07, 0x00}, 
		{0x8799, 0x84, 0x00},
		{0x879A, 0xFE, 0x00},
		{0x879B, 0x12, 0x00},
		{0x879C, 0xCC, 0x00},
		{0x879D, 0x73, 0x00},
		{0x879E, 0x8D, 0x00},

		{0x884B, 0x3A, 0xCD}, /* Border colour... */
		{0x884C, 0x73, 0x4C},
		{0x884D, 0x80, 0x93},

		{0x8AF2, 0x3A, 0xCD}, /* Border colour... */
		{0x8AF3, 0x73, 0x4C},
		{0x8AF4, 0x80, 0x93},

		{0x8B6D, 0x3A, 0xCD}, /* Border colour... */
		{0x8B6E, 0x73, 0x4C},
		{0x8B6F, 0x80, 0x93},

		{0x8DD3, 0x07, 0x73}, /* Check for bidirectional guardians */
		{0x8DD4, 0x84, 0x80},
		{0x8DD5, 0xFE, 0xE6},
		{0x8DD6, 0x07, 0x30},
		{0x8DD7, 0x38, 0x28},
		{0x8DD9, 0xFE, 0xCB},
		{0x8DDA, 0x09, 0xFB},
		{0x8DDB, 0x28, 0x00},
		{0x8DDC, 0x06, 0x00},
		{0x8DDD, 0xFE, 0x00},
		{0x8DDE, 0x0F, 0x00},
		{0x8DDF, 0x28, 0x00},
		{0x8DE0, 0x02, 0x00},
		{0x8DE1, 0xCB, 0x00},
		{0x8DE2, 0xFB, 0x00},

		{0x934C, 0x09, 0x3A},
		{0x934D, 'D',  0x73},
		{0x934E, 'E',  0x80},
		{0x934F, 'C',  0xE6},
		{0x9350, 0x09, 0x07},
		{0x9351, 'E',  0xC9}
};

PatchLine plSpecialsAnywhwereSP[] = {
		{0x8702, 0x3A, 0xCD}, /* 1 */
		{0x8703, 0x73, 0x57},
		{0x8704, 0x80, 0x93},

		{0x8776, 0x07, 0x73}, /* Eugene check */
		{0x8777, 0x84, 0x80},
		{0x8778, 0xFE, 0xE6},
		{0x8779, 0x04, 0x80},
		{0x877A, 0xCC, 0xC4},

		{0x877E, 0x07, 0x73}, /* Skylab check */
		{0x877F, 0x84, 0x80},
		{0x8780, 0xFE, 0xE6},
		{0x8781, 0x0D, 0x40},
		{0x8782, 0xCA, 0xC2},

		{0x8786, 0x07, 0x73}, /* Vertical guard check */
		{0x8787, 0x84, 0x80},
		{0x8788, 0xFE, 0xE6},
		{0x8789, 0x08, 0x20},
		{0x878A, 0xD4, 0xC4},

		{0x878E, 0x07, 0x73}, /* Kong check */
		{0x878F, 0x84, 0x80},
		{0x8790, 0xFE, 0xE6},
		{0x8791, 0x07, 0x10},
		{0x8792, 0xCC, 0xC4},

		{0x8796, 0x07, 0x73}, /* Solar Power check (previously Kong II) */
		{0x8797, 0x84, 0x80},
		{0x8798, 0xFE, 0xE6},
		{0x8799, 0x0B, 0x08},
		{0x879A, 0xCC, 0xC4},
		{0x879B, 0x40, 0x7E},
		{0x879C, 0x91, 0x8D},
		{0x879D, 0x3A, 0x00}, /* Old Solar power check */
		{0x879E, 0x07, 0x00}, 
		{0x879F, 0x84, 0x00},
		{0x87A0, 0xFE, 0x00},
		{0x87A1, 0x12, 0x00},
		{0x87A2, 0xCC, 0x00},
		{0x87A3, 0x7E, 0x00},
		{0x87A4, 0x8D, 0x00},

		{0x8851, 0x3A, 0xCD}, /* Border colour... */
		{0x8852, 0x73, 0x57},
		{0x8853, 0x80, 0x93},

		{0x8AF8, 0x3A, 0xCD}, /* Border colour... */
		{0x8AF9, 0x73, 0x57},
		{0x8AFA, 0x80, 0x93},

		{0x8B73, 0x3A, 0xCD}, /* Border colour... */
		{0x8B74, 0x73, 0x57},
		{0x8B75, 0x80, 0x93},

		{0x8DDE, 0x07, 0x73}, /* Check for bidirectional guardians */
		{0x8DDF, 0x84, 0x80},
		{0x8DE0, 0xFE, 0xE6},
		{0x8DE1, 0x07, 0x30},
		{0x8DE2, 0x38, 0x28},
		{0x8DE4, 0xFE, 0xCB},
		{0x8DE5, 0x09, 0xFB},
		{0x8DE6, 0x28, 0x00},
		{0x8DE7, 0x06, 0x00},
		{0x8DE8, 0xFE, 0x00},
		{0x8DE9, 0x0F, 0x00},
		{0x8DEA, 0x28, 0x00},
		{0x8DEB, 0x02, 0x00},
		{0x8DEC, 0xCB, 0x00},
		{0x8DED, 0xFB, 0x00},

		{0x9357, 0xFF, 0x3A},
		{0x9358, 0x00, 0x73},
		{0x9359, 0xFF, 0x80},
		{0x935A, 0x00, 0xE6},
		{0x935B, 0xFF, 0x07},
		{0x935C, 0x00, 0xC9}
};

Patch patchSpecialsAnywhereBB = { NULL, NULL,  
			sizeof(plSpecialsAnywhwereBB) / sizeof(plSpecialsAnywhwereBB[0]),
			plSpecialsAnywhwereBB };

Patch patchSpecialsAnywhereSP = { NULL, NULL,  
			sizeof(plSpecialsAnywhwereSP) / sizeof(plSpecialsAnywhwereSP[0]),
			plSpecialsAnywhwereSP };

ManicPatchPage::ManicPatchPage(ManicGame *g)
{
	int x, y;
	m_game = g;

	x = 9 * CHAR_W;
	y = CHAR_H;

	m_fflabel = new VideoLabel(x, y, "File format:");

	m_fflist = new FileTypeList((jswByte *)&m_game->getMem()->m_type, 
				x + 13 * CHAR_W, y, 8 * CHAR_W, 6*CHAR_H);
	m_fflist->setTitle("");
	m_fflist->requireClick(1);

	setTitle("Game");
	addChild(m_fflabel);
	addChild(m_fflist);
	setFocus(m_fflist);

	y += 6 * CHAR_H + 2;

	m_patchData = new Patch[20];
	m_patchCount = 0;
	m_patchMax = 20;

	/* Add the patches */
	if (m_game->xltAddress(0x9000) == 0x9000)
	{
		patchSpecialsAnywhereBB.label    = new VideoLabel(x, y, "Specials by room");
		patchSpecialsAnywhereBB.checkbox = new VideoCheckbox(x + 21 * CHAR_W, y);
		addPatch(&patchSpecialsAnywhereBB);
	}
	else
	{
		patchSpecialsAnywhereSP.label    = new VideoLabel(x, y, "Specials by room");
		patchSpecialsAnywhereSP.checkbox = new VideoCheckbox(x + 21 * CHAR_W, y);
		addPatch(&patchSpecialsAnywhereSP);
	}	
	y += CHAR_H + 4;
	updatePatches();

}

ManicPatchPage::~ManicPatchPage()
{	
	int n;
	delete m_fflabel;
	delete m_fflist;

	for (n = 0; n < m_patchCount; n++)	
	{
		delete m_patchData[n].label;
		delete m_patchData[n].checkbox;
	}	
	delete m_patchData;
}


void ManicPatchPage::updatePatches(void)
{
	int check;
	int n, m, value;
	SpectrumMemory *mem = m_game->getMem();
	
	for (n = 0; n < m_patchCount; n++)
	{
		check = -1;
		for (m = 0; m < m_patchData[n].lineCount; m++)
                {

                	value = mem->peek(m_patchData[n].lines[m].address);

			if (value == m_patchData[n].lines[m].oldVal &&
			    (check == -1 || check == 0)) check = 0;
			else if (value == m_patchData[n].lines[m].newVal && 
                            (check == -1 || check == 1)) check = 1;
			else check = 2;
// for debugging	
// 			printf("%04x: %02x %02x %02x  %d\n", 
//				m_patchData[n].lines[m].address, 
//				m_patchData[n].lines[m].oldVal, 
//				m_patchData[n].lines[m].newVal, 
//				value, check);
		}
		if (check == -1) check = 0;
		if (check == 2) m_patchData[n].checkbox->setStates(3);
		else		m_patchData[n].checkbox->setStates(2);
		m_patchData[n].checkbox->setCheck(check);
		m_patchData[n].checkbox->setListener(this);	
	}
}


void ManicPatchPage::addPatch(Patch *p)
{
	if (m_patchCount == m_patchMax)
	{
		Patch *q = new Patch[2 * m_patchMax];
		memcpy(q, m_patchData, m_patchMax * sizeof(Patch));
		delete m_patchData;
		m_patchData = q;
		m_patchMax *= 2;
	}
	memcpy(&m_patchData[m_patchCount], p, sizeof(Patch));
	++m_patchCount;
	addChild(p->label);
	addChild(p->checkbox);
}


int ManicPatchPage::redraw(int whichRectangle)
{

	int rv = VideoNotePage::redraw(whichRectangle);

	videoScreen->bottomBar("Game properties");
	return rv;
}



int ManicPatchPage::onCheckboxSelect(VideoCheckbox *b, int value)
{
	int n, m;
	jswByte from, to;
	int addr;
	SpectrumMemory *mem = m_game->getMem();

	if (value == 2) return ZXE_CANCEL;
	if (b->getCheck() == 2) return ZXE_CANCEL;	

	for (n = 0; n < m_patchCount; n++)
	{
		if (m_patchData[n].checkbox == b)
		{
			for (m = 0; m < m_patchData[n].lineCount; m++)
			{
				addr = m_patchData[n].lines[m].address;
				if (value)
				{ 
					from = m_patchData[n].lines[m].oldVal;
					to   = m_patchData[n].lines[m].newVal;
				}
				else
				{
                                        to   = m_patchData[n].lines[m].oldVal;
                                        from = m_patchData[n].lines[m].newVal;
				}
				if (mem->peek(addr) == from) mem->poke(addr,to);
			}
			updatePatches();
			return ZXE_OK;
		}
	}
	return ZXE_CANCEL;
}

/*
int ManicPatchPage::preContextMenu(void)
{
	VideoMenu vm("Game options", "Cancel", "Clear rooms", 
			"Clear guardians", "Clear sprites", "Exit", NULL);
	int rv = vm.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;
	switch(vm.getSelected())
	{
		case 0: return ZXE_CONTINUE;
		case 1: return m_game->clearRooms();
		case 2: return m_game->clearGuardians();
		case 3: return m_game->clearSprites();
		case 4: return ZXE_OK;
	}
	return ZXE_CONTINUE;
}
*/
