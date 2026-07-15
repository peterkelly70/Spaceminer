/************************************************************************

    JSWED 2.0.1 - Editor for Jet Set Willy and derivatives

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
#include "henrypatchpage.hxx"
#include "filetypelist.hxx"
#include "patch.hxx"

/* NEW in v2.0.1: In 2.0.0, this file was empty. Now it contains
 * a patch to fix the buggy victory routine. */

PatchLine plFixVictory[] = {
        { 0x9566, 0xC9, 0x21},
        { 0x9573, 0x00, 0x58},
        { 0x9574, 0x11, 0x06},
        { 0x9576, 0x58, 0x36},
        { 0x9577, 0x01, 0x06},
        { 0x9578, 0xFF, 0x2C},
        { 0x9579, 0x00, 0x10},
	{ 0x957A, 0xED, 0xFB},
	{ 0x957B, 0xB0, 0x00},
};

Patch patchFixVictory   = { NULL, NULL,  9, plFixVictory };

extern Patch patchSoftricksAttract;

HenryPatchPage::HenryPatchPage(JswGame *g) : JswPatchPage(g)
{
	int x = patchSoftricksAttract.label->getX();
	int y = patchSoftricksAttract.label->getY();
        y += CHAR_H + 4;
        patchFixVictory.label   = new VideoLabel(x, y, "Fix victory bug");
        patchFixVictory.checkbox=new VideoCheckbox(x+21*CHAR_W, y);
        addPatch(&patchFixVictory);

        updatePatches();


}

HenryPatchPage::~HenryPatchPage()
{	
}

int HenryPatchPage::onCheckboxSelect(VideoCheckbox *b, int value)
{
	return JswPatchPage::onCheckboxSelect(b, value);
}


