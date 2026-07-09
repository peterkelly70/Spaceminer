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

#define VPR_NONE  0
#define VPR_LEFT  1
#define VPR_RIGHT 2
#define VPR_BOTH  3


class VideoNotePage : public VideoForm
{
// Data
protected:
        VideoBitmap *m_bitmap, *m_lbitmap;  // Bitmaps that may be redrawn
	int m_lx, m_ly, m_lw, m_lh;
	int m_concealed;
	char *m_title;
public:
	VideoNotePage();
	virtual ~VideoNotePage();

	char *getTitle();
	void setTitle(char *);
	void setRectangle(int x, int y, int w, int h);
	void setLeftRectangle(int lx, int ly, int lw, int lh);

	virtual int onReveal(int whichRectangle = VPR_BOTH);
	virtual int onConceal();
	virtual int redraw(int whichRectangle);
	virtual int preContextMenu(void);
	virtual void postContextMenu(void);
};

typedef VideoNotePage *PVPAGE;

