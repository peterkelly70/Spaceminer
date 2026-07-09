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


class Jsw128GuardEditor: public GuardianEditor
{
protected:
	VideoSpinControl *m_spinBounds[2];
	VideoLabel       *m_labelBounds, *m_labelAnim2, *m_labelStopped,
			 *m_labelSlow, *m_labelH, *m_labelSW[3];
	VideoCheckbox    *m_cbAnim2, *m_cbStopped, *m_cbSlow, *m_cbSW[3];
	jswByte 	 m_bound1, m_bound2;
public: 
        Jsw128GuardEditor(int room, JswGame *g, jswByte *guard, VideoBitmap *bg, int x);
	virtual ~Jsw128GuardEditor();
	virtual void changeType(void);
	virtual void subEditChange(VideoControl *c);
	virtual int  subCheckboxSelect(VideoCheckbox *c, int value);
	virtual void removeVarChildren(void);
	virtual void drawTsData(void);
	virtual int Jsw128GuardEditor::setGuardianType(void);
};


