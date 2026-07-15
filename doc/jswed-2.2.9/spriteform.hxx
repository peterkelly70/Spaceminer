/************************************************************************

    JSWED 2.2.2 - Editor for Jet Set Willy and derivatives

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

class SpriteList;

class SpriteForm: public VideoForm, ButtonSelectListener, ListSelectListener
{
protected:
	VideoButton   *m_ok, *m_cancel, *m_first, *m_last, *m_prev;
	VideoBitmap   *m_bmp0, *m_bmp1;
	SpriteList    *m_list;
	ScrollingList *m_type;
	SpectrumMemory *m_mem;
	int m_mode;
	int m_png;
	int m_addr0, m_addr1;
	jswByte *m_iobuf;
	int m_nsprites;

public:
	SpriteForm (int mode, SpectrumMemory *m, SpriteList *list);
	virtual ~SpriteForm();
	virtual int doModal();
        virtual void redraw(void);
        virtual int onButtonSelect(VideoButton *b);
	virtual void onSelect(ScrollingList *sender, int nSel);
protected:
	virtual void hideChildren(void);
	virtual void showChildren(int redraw = 1);
	void makeBitmaps();

	int exportPNG();
	int exportBin();
	int importImage(const char *filaname);
	int importBin(FILE *fp);
};


