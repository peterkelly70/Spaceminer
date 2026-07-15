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
//
// Wrap a bitmap (used to save a segment of screen for reuse)
//
class VideoBitmap
{
protected:
	int m_width, m_height;
	inline VideoBitmap(int w, int h) { m_width = w; m_height = h; }
public:
	virtual ~VideoBitmap() {}
	inline int getWidth()  { return m_width;  };
	inline int getHeight() { return m_height; };
	virtual void toScreen(int x, int y)   = 0;
	virtual void toScreen(int x, int y, int xs, int ys, int ws, int hs) = 0;
	virtual void fromScreen(int x, int y) = 0;

	inline void toSurface(VideoBitmap *s, int x, int y)
	{ toSurface(s->getSurface(), x, y);	}
        virtual void toSurface(VideoSurface *s, int x, int y)   = 0;
      	inline void toSurface(VideoBitmap *s, int x, int y, int xs, int ys, int ws, int hs)
	{ toSurface(s->getSurface(), x, y, xs, ys, ws, hs); }
	virtual void toSurface(VideoSurface *s, int x, int y,                
                                int xs, int ys, int ws, int hs) = 0;

	virtual VideoSurface *getSurface() = 0;
	virtual void flash(jswByte attribute);
	virtual void makeTransparent(void) = 0;
};

VideoBitmap *newVideoBitmap(int w, int h);
VideoBitmap *newVideoBitmap(int x, int y, int w, int h);
VideoBitmap *newVideoBitmap(const char *filename);
VideoBitmap *bitmapFromSprite(int w, int h, const jswByte *sprite, 
		int doubled = 0, int fg = ZX_BLACK, int bg = ZX_BRWHITE);
VideoBitmap *bitmapFromXbm   (int w, int h, const jswByte *bits, 
		int doubled = 0, int fg = ZX_BLACK, int bg = ZX_BRWHITE);
VideoBitmap *bitmapFromScreen(int h, const jswByte *bits, int doubled = 0,
			      int fg = ZX_BLACK, int bg = ZX_BRWHITE);
