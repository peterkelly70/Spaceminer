#ifndef JSWGUARDIAN_HXX_INCLUDED
#define JSWGUARDIAN_HXX_INCLUDED

/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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

class JswGame;

class JswGuardian
{
public:
        JswGuardian(jswByte *g, jswByte x, SpectrumMemory *mem, int bg);
        virtual ~JswGuardian();

        inline void draw(VideoBitmap *b) { draw(b->getSurface()); }
	virtual void draw(VideoSurface *s) = 0;
        virtual void drawFrames(VideoSurface *s, int x, int y) = 0;
        virtual void drawThumb(VideoSurface *s, int cx, int cy) = 0;
        virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0) = 0; 
	void drawRope(VideoSurface *s);
	virtual void move(void) = 0;

	virtual bool isArrow(void) = 0;
	virtual int setPos(int cx, int cy) = 0;
        virtual jswByte bumpFrame(void) = 0;
        virtual void setFrame(jswByte b) = 0;
	virtual void constructSprites(void) = 0;

// Accessor methods

	jswByte getSpritePage(void);
	void setSpritePage(jswByte page);
	jswByte getInk(void);
	virtual void setInk (jswByte b);	
        jswByte getY(void);
	jswByte getStep(void);
        jswByte getAnim(void);
	void    setAnim(jswByte b);
	virtual void setY(jswByte y) = 0; // setY() is very different
					  // in the methods it uses to 
					  // compensate for diagonal movement
					  // in JSW128 and Geoff Mode
	virtual void setStep(jswByte s)=0; // Ditto for setStep()
        virtual void setDirection(jswByte right) = 0;
        virtual jswByte getDirection(void) = 0;

// For the rope
	void setRopeLen(jswByte len);
	jswByte getRopeLen(void);
	void setRopeSwing(jswByte swing);
	jswByte getRopeSwing(void);


protected:
	jswByte m_guard[8];
	jswByte m_orig[8];
	int m_x, m_y, m_frame;	
	VideoBitmap *m_bmp, *m_bmpSmall;
	SpectrumMemory *m_mem;	
	int m_bgcol;

	void moveRope(void);
};

#endif //ndef JSWGUARDIAN_HXX_INCLUDED
