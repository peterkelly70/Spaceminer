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


class Jsw64GuardEditor: public Jsw128GuardEditor
{
public: 
        Jsw64GuardEditor(int room, JswGame *g, jswByte *guard, VideoBitmap *bg);
	virtual ~Jsw64GuardEditor();
	virtual int doModal();
protected:
	VideoSpinControl *m_spinFrame;
	VideoButton	 *m_btnTarget, *m_btnSource;
	jswByte m_frame;
	jswByte m_arrow[2];
//	jswByte m_eugene[8];
//	jswByte *m_eguard;
//	bool m_iseugene;
	jswByte *m_triggerTarget;

//	jswByte *maybeEugene(jswByte *guard);
	virtual void removeVarChildren(void);
	virtual void changeType(void);
	virtual void subEditChange(VideoControl *c);
	virtual int onButtonSelect(VideoButton *b);
	virtual void redraw();

	int onTriggerSource();
	int onTriggerTarget();
};


