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

class VideoSpinControl : public VideoControl, public EditChangeListener
{
public:
	VideoSpinControl(int x, int y, int w, int h);
	virtual ~VideoSpinControl();
	virtual void redraw(void);

        inline EditChangeListener *getListener(void) { return m_listener; }
        inline void setListener(EditChangeListener *l) { m_listener = l; }      

	void setDelta(int d);
	inline int getDelta(void)   { return m_delta; }

	inline void setSigned(int s) { m_signed = s; }
	inline int getSigned(void) { return m_signed; }

	void setValue(int v);
	int  getValue(void);

	void setLimits(int ll, int ul);
	void bind(jswByte *b);
	virtual void updateBind(void);

	// Overrides
	virtual int onKeyDown(int keysym);
	virtual void drawFocus(int focused);
        virtual int onButtonUp  (int x, int y, int button);
        virtual int onButtonDown(int x, int y, int button);
	virtual void onEditChange(VideoControl *v);
	virtual void setVisible(int visible = 1, int redraw = 1);
protected:
	EditChangeListener *m_listener;
	VideoTextEdit m_te;
	void onSpin(int up);
	jswByte *m_bind;
	int m_delta;
	int m_signed;
	int m_ll, m_ul;
};


