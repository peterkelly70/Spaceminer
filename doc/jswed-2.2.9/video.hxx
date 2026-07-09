/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

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

extern unsigned char videoFont[];

class VideoSurface
{
protected:
	VideoSurface();
public:
	virtual ~VideoSurface();
	void cls(void);
	virtual void box(int x, int y, int w, int h, int colour) = 0;
	virtual void dottedBox(int x, int y, int w, int h, int colour) = 0;
	virtual void fillBox(int x, int y, int w, int h, int colour) = 0;
	virtual void shadedBox(int x, int y, int w, int h, int c1, int c2) = 0;
	virtual void fillReplace(int x, int y, int w, int h, int c1, int c2)= 0;
	virtual void stripe(int x, int y, int w, int h, int colour1, int colour2) = 0;
	virtual void rotateL(int x, int y, int w, int h, int count = 1) = 0;
	virtual void rotateR(int x, int y, int w, int h, int count = 1) = 0;
	virtual void plot(int x, int y, int colour) = 0;
        virtual unsigned int point(int x, int y) = 0;
	virtual unsigned int pointZx(int x, int y) = 0;
	void zxLogo(int x, int y);
	void smallZxLogo(int x, int y);
	virtual void drawSmallText(int x1, int y1, const char *str, int fg, int bg)=0;
	virtual void drawText(int x1, int y1, const char *str, int fg, int bg) = 0;
	void drawText(int x1, int y1, char c, int fg, int bg);
	void drawSmallText(int x1, int y1, char c, int fg, int bg);
	void bottomBar(char *s);
        int  savePng(int ix, int iy, int iw, int ih,
                       const char *filename, int scale);
};

extern VideoSurface *videoScreen;

int videoInit(char *iconFile, char *caption, char *iconCaption);
void videoDeInit(void);

int videoMenu(char *title, int count, ...);
int videoWaitForKey(int defcode);

#define gl_rbutton 3 	// Right mouse button (at least on Linux) is no. 3

// Colours 

#define ZX_BLACK   0
#define ZX_BLUE    1
#define ZX_RED     2
#define ZX_MAGENTA 3
#define ZX_GREEN   4
#define ZX_CYAN    5
#define ZX_YELLOW  6
#define ZX_WHITE   7
#define ZX_BRIGHT  8

#define ZX_BRBLUE    (ZX_BLUE   | ZX_BRIGHT)
#define ZX_BRRED     (ZX_RED    | ZX_BRIGHT)
#define ZX_BRMAGENTA (ZX_MAGENTA| ZX_BRIGHT)
#define ZX_BRGREEN   (ZX_GREEN  | ZX_BRIGHT) 
#define ZX_BRCYAN    (ZX_CYAN   | ZX_BRIGHT) 
#define ZX_BRYELLOW  (ZX_YELLOW | ZX_BRIGHT)
#define ZX_BRWHITE   (ZX_WHITE  | ZX_BRIGHT)

// Non-Spectrum colours
#define ZX_ORANGE   16
#define ZX_BRORANGE 17
#define ZX_GREY     18
#define ZX_BRGREY   19
#define ZX_TRANS    20	// Transparent

// Classes
#include "event.hxx"
#include "bitmap.hxx"

// "Control" classes
#include "control.hxx"
#include "menu.hxx"
#include "list.hxx"
#include "button.hxx"
#include "checkbox.hxx"
#include "label.hxx"
#include "textedit.hxx"
#include "spinctrl.hxx"
#include "clrchooser.hxx"
#include "filelist.hxx"
#include "form.hxx"
#include "filesel.hxx"
#include "notepage.hxx"
#include "notebook.hxx"
#include "bitmapedit.hxx"

// Keys (WordStar mappings)
#define ZXK_CTRLA ('A'-'@')
#define ZXK_EDIT  ('B'-'@')
#define ZXK_PGDN  ('C'-'@')
#define ZXK_RIGHT ('D'-'@')
#define ZXK_UP    ('E'-'@')
#define ZXK_RDEL  ('G'-'@')
#define ZXK_LDEL  ('H'-'@')
#define ZXK_TAB	  ('I'-'@')
#define ZXK_CTRLJ ('J'-'@')
#define ZXK_CTRLL ('L'-'@')
#define ZXK_ENTER ('M'-'@')
#define ZXK_CTRLP ('P'-'@')
#define ZXK_CTRLQ ('Q'-'@')
#define ZXK_PGUP  ('R'-'@')
#define ZXK_LEFT  ('S'-'@')
#define ZXK_DOWN  ('X'-'@')
#define ZXK_ZAP   ('Y'-'@')
#define ZXK_BREAK ('['-'@')
#define ZXK_POUND 	163	// Pound sign
#define ZXK_COPY  	164	// Copyright  
#define ZXK_ALT	    0x1000
#define ZXK_CONTROL 0x2000	// [JCE] Was 0x2000-2001, and I don't see
				// how that can work as a mask. -2001
#define ZXK_ALTUP   (ZXK_UP   | ZXK_ALT)
#define ZXK_ALTDOWN (ZXK_DOWN | ZXK_ALT) 
#define ZXK_BACKTAB (ZXK_TAB  | ZXK_ALT) // Would clash with ALT-TAB but
				       // for the fact that we never trap
				       // ALT-TAB.
// Spectrum non-standard ASCII characters
#define ZXCH_COPY 127
#define ZXCH_POUND 96

#define ZXK_F1 256
#define ZXK_F2 257
#define ZXK_F3 258
#define ZXK_F4 259
#define ZXK_F5 260
#define ZXK_F6 261
#define ZXK_F7 262
#define ZXK_F8 264
#define ZXK_F9 265
#define ZXK_F10 266
#define ZXK_SHIFTSPACE 0x120  // Space + 256

// Screen geometry 
#define VIDEO_W 640
#define VIDEO_H 480
//#define VIDEO_W 800
//#define VIDEO_H 600
// Character sizes
#define CHAR_W   16
#define CHAR_H   16
#define CHAR_SW   8
#define CHAR_SH   8
