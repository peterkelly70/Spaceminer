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

class VideoColourChooser;

struct ColourChooserListener
{
	virtual void onChooseColour(VideoColourChooser *c) = 0;
};

class VideoColourChooser : public VideoControl
{
public:
	VideoColourChooser(int x, int y);
	virtual ~VideoColourChooser();
	virtual void redraw(void);

	void setColour(int i);
	int  getColour(void);

	inline void setListener(ColourChooserListener *l) { m_listener = l; }
	inline ColourChooserListener *getListener() { return m_listener; }

	// Overrides
	virtual int onKeyDown(int keysym);
	virtual void drawFocus(int focused);
	virtual int onButtonDown(int x, int y, int button);

protected:
	void redrawCtrl(void);

	int m_ink;
	ColourChooserListener *m_listener;
};


