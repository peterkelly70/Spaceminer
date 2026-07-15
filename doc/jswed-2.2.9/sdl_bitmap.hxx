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


////////////////////////////////////////////////////////////////////////
// SdlVideoBitmap class
////////////////////////////////////////////////////////////////////////


class SdlVideoBitmap : public VideoBitmap
{
public:
	SdlVideoBitmap(int w, int h, Uint32 flags = 0);	
	SdlVideoBitmap(const char *filename);
	virtual ~SdlVideoBitmap();
	virtual void toScreen(int x, int y);
	virtual void toSurface(VideoSurface *s, int x, int y);
	virtual void fromScreen(int x, int y);
        virtual void toScreen(int x, int y, int xs, int ys, int ws, int hs);
	virtual void toSurface(VideoSurface *s, int x, int y, 
                                int xs, int ys, int ws, int hs);

	void fromSprite(const jswByte *sprite, int doubled = 0, 
			int fg = ZX_BLACK, int bg = ZX_BRWHITE, 
			int format = 0);
	inline void fromXbm   (const jswByte *xbm,    int doubled = 0,
			int fg = ZX_BLACK, int bg = ZX_BRWHITE)
	{
		fromSprite(xbm, doubled, fg, bg, 1);
	}
	inline void fromScreen(const jswByte *screen, int doubled = 0,
			int fg = ZX_BLACK, int bg = ZX_BRWHITE)
	{
		fromSprite(screen, doubled, fg, bg, 2);
	}
	virtual VideoSurface *getSurface();
	virtual void makeTransparent(void);
protected:
	SdlVideoSurface *m_surface;
};

VideoBitmap *newVideoBitmap(int w, int h);
VideoBitmap *newVideoBitmap(int x, int y, int w, int h);
VideoBitmap *newVideoBitmap(const char *filename);


