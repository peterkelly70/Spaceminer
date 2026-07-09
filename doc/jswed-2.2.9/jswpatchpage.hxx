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

struct Patch;

class JswPatchPage : public VideoNotePage, CheckboxSelectListener
{
protected:
	ScrollingList *m_fflist;	
	VideoLabel    *m_fflabel;
	JswGame       *m_game;
	Patch 	      *m_patchData;
	int 	      m_patchCount, m_patchMax;
public:
	JswPatchPage(JswGame *g);
	virtual ~JswPatchPage();

	void addPatch(Patch *p);
	void removePatch(int id);
	void updatePatches(void);

        virtual int redraw(int whichRectangle);
        virtual int onCheckboxSelect(VideoCheckbox *b, int value);

	virtual int preContextMenu(void);
};


