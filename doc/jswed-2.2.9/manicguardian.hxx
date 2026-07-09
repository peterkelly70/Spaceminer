#ifndef MANICGUARDIAN_HXX_INCLUDED
#define MANICGUARDIAN_HXX_INCLUDED

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

class ManicGuardian
{
protected:
        ManicGuardian(jswByte *room, jswByte *guard, jswByte *sprites = NULL);
public:
        virtual ~ManicGuardian();

        inline void draw(VideoBitmap *b) { draw(b->getSurface()); }
	virtual void draw(VideoSurface *s) = 0;
//        virtual void drawFrames(VideoSurface *s, int x, int y) = 0;
        virtual void drawThumb(VideoSurface *s, int cx, int cy) = 0;
        virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0) = 0; 

//	virtual int setPos(int cx, int cy) = 0;
	virtual void constructSprites(void);

// Accessor methods

	jswByte getInk   (void);   
	jswByte getPaper (void); 
	jswByte getBright(void); 
	virtual jswByte getFlash (void); 
	void setInk      (jswByte b);	
	void setPaper    (jswByte b);	
	void setBright   (jswByte b);	
	void setFlash    (jswByte b);	

        virtual jswByte getY(void)      = 0;
	virtual void    setY(jswByte b) = 0;
        virtual jswByte getX(void)      = 0;
	virtual void    setX(jswByte b) = 0;
        virtual signed char getSpeed(void) = 0; 
	virtual void        setSpeed(signed char c) = 0;

        jswByte getAnim(void);
	void    setAnim(jswByte b);
 //       virtual void setDirection(jswByte right) = 0;
  //      virtual jswByte getDirection(void) = 0;
	virtual void setBounds(jswByte b1, jswByte b2) = 0;
	virtual void move(void) = 0;

	inline void setTicker(int t)
	{
		m_ticker = t;
	}

protected:
	jswByte m_guard[7];
	jswByte m_orig[7];
	jswByte *m_room;
	jswByte *m_sprites;
	int m_x, m_y, m_frame, m_ticker;	
	signed char m_speed;

	inline VideoBitmap *getBmp()
	{
		if ((m_guard[0] & 0x80) && ((m_ticker % 40) >= 20)) 
			return m_bmpFlash;
		return m_bmp;
	}
	inline VideoBitmap *getBmpSmall()
	{
		if ((m_guard[0] & 0x80) && ((m_ticker % 40) >= 20)) 
			return m_bmpSmallFlash;
		return m_bmpSmall;
	}
private:
	VideoBitmap *m_bmp, *m_bmpFlash;
	VideoBitmap *m_bmpSmall, *m_bmpSmallFlash;
};


/* In Manic Miner, unlike in JSW, horizontal and vertical guardians are
 * quite different beasts */
class ManicHGuardian : public ManicGuardian
{
protected:
	bool	m_bidirectional;
	bool	m_halfspeed;
public:
        ManicHGuardian(bool bidirectional, jswByte *room, jswByte *guard);
        virtual ~ManicHGuardian();

        inline void draw(VideoBitmap *b) { draw(b->getSurface()); }
        virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0);
	virtual void draw(VideoSurface *s);
        virtual void drawThumb(VideoSurface *s, int cx, int cy);
        virtual jswByte getY(void); 
	virtual void    setY(jswByte b);
        virtual jswByte getX(void);
	virtual void    setX(jswByte b);
        virtual signed char getSpeed(void); 
	virtual void        setSpeed(signed char c);
	virtual void setBounds(jswByte b1, jswByte b2);
	virtual void    move(void);
	virtual jswByte getFlash (void); 
};


class ManicVGuardian : public ManicGuardian
{
public:
        ManicVGuardian(jswByte *room, jswByte *guard, jswByte *sprite = NULL);
        virtual ~ManicVGuardian();

        inline void draw(VideoBitmap *b) { draw(b->getSurface()); }
	virtual void draw(VideoSurface *s);
        virtual void drawThumb(VideoSurface *s, int cx, int cy);
        virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0);
        virtual jswByte getY(void); 
	virtual void    setY(jswByte b);
        virtual jswByte getX(void);
	virtual void    setX(jswByte b);
	virtual void    move(void);
        virtual signed char getSpeed(void); 
	virtual void        setSpeed(signed char c);
	virtual void setBounds(jswByte b1, jswByte b2);
};


class ManicEugene : public ManicVGuardian
{
	jswByte m_guard[7];
public:
        ManicEugene(jswByte *room, int bg);
	
	virtual void    draw(VideoSurface *s);
	jswByte * init(jswByte *room, int bg);
};

class ManicKong : public ManicVGuardian
{
	jswByte m_guard[7];
public:
        ManicKong(jswByte *room, int bg);
	
	virtual void    draw(VideoSurface *s);
	jswByte * init(jswByte *room, int bg);
};



class ManicSkylab : public ManicVGuardian
{
public:
        ManicSkylab(jswByte *room, jswByte *guard);
	virtual void    move(void);
	virtual void    draw(VideoSurface *s);
	virtual void    drawThumb(VideoSurface *s, int cx, int cy);
        virtual void drawBounds(VideoSurface *s, int ink = -1, int cx = 0, int cy = 0, int cyminor = 0);
};

#endif //ndef MANICGUARDIAN_HXX_INCLUDED
