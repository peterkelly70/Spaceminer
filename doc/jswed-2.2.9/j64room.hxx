/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004-5  John Elliott <jce@seasip.demon.co.uk>

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

class Jsw64Game;

class Jsw64Room: public Room
{
private:
	jswByte *m_data;
	Jsw64Game *m_game;
	jswByte *getBits();
	int getCellSize();
	jswByte m_queryPattern[9];
public:
	Jsw64Room(Jsw64Game *g, jswByte *data, int roomNumber);

	virtual int getBackground();
	virtual int getTitle(char *buf);
	virtual void setTitle(const char *buf);
	virtual void setCell(int x, int y, int type);
	virtual jswByte *getGuardianBuffer();
	virtual bool getPortal(Portal &p);
	virtual int getMaxGuards();
	virtual int getCellBehaviour(int index);
	virtual void setCellBehaviour(int index, int cb);
	virtual void drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw);
	virtual int getCellTypeCount();
	virtual int getRoomLength() const;
        virtual jswByte getSolarAttr();
        virtual void getSolarPos(int *x, int *y);
        virtual void setSolarPos(int x, int y);
	virtual void getCells(Cell *c);
	virtual void setCells(Cell *c);
	virtual void clear();
	virtual void mirrorH();
	virtual void mirrorV();
	virtual void getRoomMemMap(char *buf);
	virtual void setGuardianTable(int offset);
	virtual void getSprites(int *list, int granularity);
	virtual void setPortalPos(int x, int y);
	virtual int getCellAt(int x, int y);
	virtual void createCellPatterns();
protected:
	virtual jswByte *getCellPattern(int cell);
	virtual int getConveyorDir();
	virtual int getConveyorY();
	virtual int getItemCoords(int *px, int *py, int *counts);
	jswByte *addressOf(unsigned offset);
};

