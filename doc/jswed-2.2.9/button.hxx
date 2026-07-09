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

class VideoButton;

struct ButtonSelectListener
{
	virtual int onButtonSelect(VideoButton *b) = 0;
};

class VideoButton : public VideoControl 
{
public:
	VideoButton(int id, int x, int y, int w, int h);
	VideoButton(int id, int x, int y, char *s);
	virtual ~VideoButton();
// Overrides
	virtual void redraw(void);
        virtual int onKeyDown(int keysym);
	virtual int onButtonDown(int x, int y, int button);

	inline int getId(void)   { return m_id; }
	inline void setId(int i) { m_id = i;    } 
	inline void setListener(ButtonSelectListener *b) { m_listener = b; }
	inline ButtonSelectListener *getListener(void)   { return m_listener; }
	void setText(char *s);
	char *getText(void);

protected:
	void redrawText(void);

	int m_id;
	char *m_buf;
	unsigned int m_buflen;
	ButtonSelectListener *m_listener;
};


