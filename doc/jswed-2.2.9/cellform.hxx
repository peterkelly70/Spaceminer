/************************************************************************

    JSWED 2.2.6 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004,2005  John Elliott <jce@seasip.demon.co.uk>

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

class CellForm: public VideoForm, 
		ButtonSelectListener,
		CheckboxSelectListener,
		ColourChooserListener
{
protected:
	VideoColourChooser  *m_ink, *m_paper;
	VideoCheckbox       *m_bright, *m_flash;
	VideoButton         *m_ok, *m_cancel, *m_prev, *m_next;
	BitmapEditor        *m_edit;
	jswByte 	    *m_cells;
	int		     m_curcell;
	int		     m_count;
	char	           **m_captions;

public:
	CellForm(jswByte *cells, int count, ...);
	virtual ~CellForm();
	virtual int doModal();
        virtual void redraw(void);
	virtual int onButtonSelect(VideoButton *b);
	virtual void onChooseColour(VideoColourChooser *c);
	virtual int onCheckboxSelect(VideoCheckbox *b, int value);

protected:
	virtual void showChildren(int redraw = 1);
	virtual void hideChildren(void);
};


