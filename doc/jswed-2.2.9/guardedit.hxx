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

class JswGuardian;
class JswDirCheckbox;

class GuardianEditor: public VideoForm, 
		      TickListener, 
		      public ButtonSelectListener,
		      public EditChangeListener, 
		      ColourChooserListener, 
	 	      public CheckboxSelectListener
{
protected:
// Drawing
	VideoBitmap	*m_bmBg;
	VideoBitmap	*m_bmAnim;
        JswGuardian     *m_guard;
// Controls
	VideoButton 	*m_bnType, *m_bnOk, *m_bnCancel, *m_bnSprite, *m_bnPling;
	VideoSpinControl *m_spinY, *m_spinAnim, *m_spinRopeLen, 
			 *m_spinRopeSwing, *m_spinRopeX, *m_spinVstep,
			 *m_spinStartPos;
	VideoLabel       *m_labelY, *m_labelBright, *m_labelAnim, 
			 *m_labelRopeLen, *m_labelRopeSwing, *m_labelRopeX,
			 *m_labelDirection,
			 *m_labelStartPos;
	VideoColourChooser *m_chInk;
	VideoCheckbox      *m_cbBright;
	JswDirCheckbox     *m_cbDirection;
	BitmapEditor    *m_bmEdit;
	
// Data
	JswGame		*m_game;
	jswByte		*m_gdata;
// Animation
	TickListener    *m_ot;
	int 		m_haveTick, m_tick;
	int 		m_nx;	// Guardian X-coord
	int		m_roomNo;

protected:
        void preContextMenu(void);
        void postContextMenu(void);

	// The way vtables work means that these have to be called from the
	// constructor/destructor of the derived class, rather than the base 
	// class
	void setupControls(void);
	void deleteControls(void);

	virtual void removeVarChildren(void);
	virtual int setGuardianType(void) = 0;
      	int setSprite(void); 
	void drawFixed(void);
        void drawData(int tickOnly);
	virtual void drawTsData(void) = 0;
	virtual void changeType(void) = 0;
	virtual void subEditChange(VideoControl *v) = 0;
	virtual int  subCheckboxSelect(VideoCheckbox *c, int value) = 0;
	int onPling(void);
public: 
        GuardianEditor(int roomNo, JswGame *g, jswByte *guard, VideoBitmap *bg, int x);
	virtual ~GuardianEditor();
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
	virtual int doModal();
};


