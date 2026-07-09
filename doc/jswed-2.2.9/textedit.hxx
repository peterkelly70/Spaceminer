/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-5  John Elliott <jce@seasip.demon.co.uk>

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

struct EditChangeListener
{
	virtual void onEditChange(VideoControl *v) = 0;
};


class VideoTextEdit : public VideoControl, TickListener
{
public:
	VideoTextEdit(int x, int y, int w, int h);
	virtual ~VideoTextEdit();
	virtual void redraw(void);

	inline EditChangeListener *getListener(void) { return m_listener; }
	inline void setListener(EditChangeListener *l) { m_listener = l; }	

	inline void setNumeric(bool v = true) { m_numeric = v; }
	inline bool getNumeric()              { return m_numeric; }

	inline void setHexOnly(bool v = true) { m_hexOnly = v; }
	inline bool getHexOnly()              { return m_hexOnly; }

	void setText(char *s);
	char *getText(void);

	/* Limit length of input. Note that if you have limited to 4 
         * characters, you still have to draw the control with room for
	 * 5, so there's room for the cursor */
	int getLimit(void);
	void setLimit(int limit);

	/* Bind the control to some external string buffer. bind() will
	 * make it use a 0-terminated string, while bindFixed() makes
	 * it use a fixed-length buffer with no 0-termination. Other
         * methods may be added. */
	void bind(char *xbuf);
	void bindFixed(char *xbuf);

	// Overrides
	virtual int onKeyDown(int keysym);
	virtual void onTick(void);
	virtual void drawFocus(int focused);

protected:
	void drawCursor(int draw = 1);
	void redrawText(void);
	void insertChar(char c);
	virtual void updateBind(void);

	EditChangeListener *m_listener;
	char *m_bind;
	int m_bindFixed;
	int m_limit;
	char *m_buf;
	unsigned int m_buflen;
	int m_tcount;
	int m_curpos;
	TickListener *m_ot;
	int m_haveTimer;
	bool m_hexOnly;
	bool m_numeric;
	bool m_ctrlQ;
};


