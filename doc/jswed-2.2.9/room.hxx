/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-6  John Elliott <jce@seasip.demon.co.uk>

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

class VideoBitmap;

struct Portal
{
	unsigned x;
	unsigned y;
	jswByte attribute;
	jswByte *sprite;

	Portal();
	bool operator == (Portal &q);
	bool operator != (Portal &q);
	void operator =  (Portal &v);
};


struct Cell
{
	int behaviour;
	jswByte pattern[9];
};


#define CB_AIR     0
#define CB_WATER   1
#define CB_EARTH   2
#define CB_FIRE    3
#define CB_LRAMP   4
#define CB_RRAMP   5
#define CB_LCONV   6
#define CB_RCONV   7
#define CB_CRUMBLY 8
#define CB_TRAMP   9
#define CB_TRAP   10

#define CT_ITEM  0x201
#define CT_SOLAR 0x202
#define CT_CONV  0x203

class Room
{
protected:
	Room(int ncelltypes, int roomNumber);
	virtual jswByte *getCellPattern(int celltype) = 0;
	virtual int getConveyorDir() = 0;
	virtual int getConveyorY() = 0;
	/* Get item counts and positions. */
	virtual int getItemCoords(int *px, int *py, int *count) = 0;
	
	/* Draw a single item */
	virtual void drawItem(VideoBitmap *bm, int x, int y, int colour);
public:
	~Room();
	virtual int getTitle(char *buf) = 0;
	virtual void setTitle(const char *buf) = 0;
	/* Get the background colour to use for guardians */
	virtual int getBackground() = 0;

	virtual int getCellTypeCount() = 0;
	virtual void createCellPatterns();
	virtual int getCellBehaviour(int index) = 0;
	virtual void setCellBehaviour(int index, int cb);

	/* Draw the single cell at (x,y). */
	void drawCell(VideoBitmap *bm, int x, int y, bool flashonly, bool animated);
	/* Draw an example of a cell */
	void drawCell(int x, int y, int type);

	/* Draw things like the JSW48 conveyor and ramp */
	virtual void drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw = true);
	/* Draw all items in the room. */
	void drawItems(VideoBitmap *bm, int colour);

	/* Swap the colours in anything that flashes */
	void flashCells();

	/* Animate the bitmap of the conveyor */
	void animateConveyor();	

	/* Set the cell type at position x,y */
	virtual void setCell(int x, int y, int type) = 0;
	/* Get the cell type at position x,y */
	virtual int getCellAt(int x, int y) = 0;

	/* Get/set the 'safe restart' coordinates */
	virtual int getRestartX();
	virtual int getRestartY();
	virtual void setRestartX(int x);
	virtual void setRestartY(int y);

	/* Populate a portal structure */
	virtual bool getPortal(Portal &p);

	/* Get the buffer containing guardian details */
	virtual jswByte *getGuardianBuffer();

	virtual int getMaxGuards();

	virtual int getRoomLength() const = 0;

	virtual jswByte getSolarAttr();
	virtual void getSolarPos(int *x, int *y);
	virtual void setSolarPos(int x, int y);

	virtual void getCells(Cell *c) = 0;
	virtual void setCells(Cell *c) = 0;

	virtual void clear() = 0;
	virtual void mirrorH() = 0;
	virtual void mirrorV() = 0;

#define RMM_FREE     ' '
#define RMM_BITMAP   'b'
#define RMM_CELL     'c'
#define RMM_GUARDIAN 'g'
#define RMM_SPRITE   's'
#define RMM_TITLE    't'
#define RMM_INUSE    'x'
	virtual void getRoomMemMap(char *buf) = 0;
	virtual void setGuardianTable(int offset);
	virtual void getSprites(int *buf, int granularity);

	virtual void setPortalPos(int x, int y);
private:
	int m_ncells;	
	jswByte *m_zxCellBitmaps;
	Portal m_portal;
	jswByte m_portalSprite[32];
	jswByte m_solarFlag;
	VideoBitmap  *m_portalBitmap;
	VideoBitmap  *m_solarBitmap;
protected:
	VideoBitmap **m_cellBitmaps;
	jswByte	m_itemPattern[9];
	int m_roomNumber;
	inline int cellCount() { return m_ncells; }
};

