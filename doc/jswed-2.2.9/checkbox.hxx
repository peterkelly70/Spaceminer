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

class VideoCheckbox;

struct CheckboxSelectListener
{
        virtual int onCheckboxSelect(VideoCheckbox *b, int value) = 0;
};


class VideoCheckbox : public VideoControl
{
public:
	VideoCheckbox(int x, int y);
	virtual ~VideoCheckbox();
	virtual void redraw(void);

	void setStates(int t);
	int  getStates(void);
	void setCheck(int i);
	int  getCheck(void);

        inline void setListener(CheckboxSelectListener *l) { m_listener = l; }
	inline CheckboxSelectListener *getListener() { return m_listener; }
	int updateCheck(int val);
	int toggleCheck(void);

	// Overrides
	virtual int onKeyDown(int keysym);
	virtual void drawFocus(int focused);
	virtual int onButtonDown(int x, int y, int button);

protected:
	virtual void redrawCtrl(void);

	int m_tick;
	int m_nstates;
	CheckboxSelectListener *m_listener;
};


