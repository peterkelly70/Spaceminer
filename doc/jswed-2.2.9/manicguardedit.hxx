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

class ManicGuardian;
class JswDirCheckbox;

class ManicGuardianEditor: public VideoForm, 
		      TickListener, 
		      ButtonSelectListener,
		      public EditChangeListener, 
		      ColourChooserListener, 
	 	      public CheckboxSelectListener
{
protected:
	bool		m_vertical, m_skylab, m_bidi;
// Drawing
	VideoBitmap	*m_bmBg;
	VideoBitmap	*m_bmAnim;
        ManicGuardian   *m_guard;
// Controls
	VideoButton 	 *m_bnOk, *m_bnCancel, *m_bnPling;
	VideoSpinControl *m_spinAnim, *m_spinVstep,
			 *m_spinStartPos;
	VideoSpinControl *m_spinBounds[2];
	VideoLabel       *m_labelVstep, *m_labelBright, *m_labelAnim, 
			 *m_labelDirection, *m_labelAnim2,
			 *m_labelStartPos, *m_labelBounds;
	VideoColourChooser *m_chInk;
	VideoColourChooser *m_chPaper;
	VideoCheckbox      *m_cbBright, *m_cbAnim2;
	JswDirCheckbox     *m_cbDirection;
	
// Data
	jswByte		*m_room;
	ManicGame	*m_game;
	jswByte		*m_gdata;
	jswByte		m_ll, m_rl;
// Animation
	TickListener    *m_ot;
	int 		m_haveTick, m_tick;
protected:
        void preContextMenu(void);
        void postContextMenu(void);

	// The way vtables work means that these have to be called from the
	// constructor/destructor of the derived class, rather than the base 
	// class
	void setupControls(void);
	void deleteControls(void);

	virtual void removeVarChildren(void);
      	int setSprite(void); 
	void drawFixed(void);
        void drawData(int tickOnly);
	virtual void drawTsData(void);
	int onPling(void);
public: 
        ManicGuardianEditor(bool vertical, bool skylab, bool bidi,
		ManicGame *g, jswByte *room, jswByte *guard, VideoBitmap *bg);
	virtual ~ManicGuardianEditor();
// Overrides
        void addChild(VideoControl *c);
        void removeChild(VideoControl *c);
	virtual void redraw(void);
	virtual void hideChildren(void);
	virtual void showChildren(int redraw = 1);
// Events
        virtual void onTick(void);
	virtual int onButtonSelect(VideoButton *b);
	virtual void onChooseColour(VideoColourChooser *c);
	virtual int  onCheckboxSelect(VideoCheckbox *c, int value);
	virtual void onEditChange(VideoControl *v); 
};


