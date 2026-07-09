#ifndef JSW128GUARD_HXX_INCLUDED
#define JSW128GUARD_HXX_INCLUDED

/************************************************************************

    JSWED 2.1.6 - Editor for Jet Set Willy and derivatives

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

#include "jswguardian.hxx"

class JswGame;
class Room;

class Jsw128Guard : public JswGuardian
{
protected:
	void hshift();
	void vshift();
	void cycle();
	void eucycle();
	virtual void constructSprites(void);
	int getType();
	void drawOpeningWall(VideoSurface *s);
public:
	Jsw128Guard(jswByte *g, jswByte x, SpectrumMemory *mem, int bg);
	virtual ~Jsw128Guard();

// Accessors
// 	
	virtual bool isArrow(void);
//
	virtual void setDirection(jswByte right);	// 0=left !0=right
	virtual jswByte getDirection(void);
	void setBounds(jswByte b1, jswByte b2);
	void getBounds(jswByte *b1, jswByte *b2);
	jswByte getAnim(void);
	void setAnim(jswByte anim);
	void setStopped(jswByte stop);
	void setSlow(jswByte slow);
	virtual void setY(jswByte y);
	virtual void setStep(jswByte s);
//
// For the vertical guardian
//
	void setFastAnim(jswByte b);
	jswByte getFastAnim(void);
// 
// For the arrow
//
//	jswByte getArrowBitmap(void);
//	void setArrowBitmap(jswByte b);
 	
	virtual void draw(VideoSurface *s);
	virtual void drawFrames(VideoSurface *s, int x, int y);
	virtual void drawThumb(VideoSurface *s, int cx, int cy);
	virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0);
	virtual void move(void);
	virtual int setPos(int cx, int cy);
	virtual jswByte bumpFrame(void);
	virtual void  setFrame(jswByte b);
};

#endif //ndef JSW128GUARD_HXX_INCLUDED
