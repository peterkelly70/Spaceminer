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

class JswGame;

class Jsw48Room: public Room
{
protected:
	jswByte *m_data;
	JswGame *m_game;
	jswByte m_guardianBuffer[65];
public:
	Jsw48Room(JswGame *g, jswByte *data, int roomNumber);

	virtual int getBackground();
        virtual int getRestartX();
        virtual int getRestartY();
        virtual void setRestartX(int x);
        virtual void setRestartY(int y);

	virtual int getTitle(char *buf);
	virtual void setTitle(const char *buf);
	virtual void drawOneOffs(VideoBitmap *bm, bool flashonly, bool draw = true);
	virtual void setCell(int x, int y, int type);
	virtual jswByte *getGuardianBuffer();
	virtual int getCellTypeCount();
	virtual int getRoomLength() const;
	virtual void getCells(Cell *c);
	virtual void setCells(Cell *c);
	virtual void clear();
	virtual void mirrorH();
	virtual void mirrorV();
	virtual void getRoomMemMap(char *buf);
	virtual int getCellAt(int x, int y);
	virtual void createCellPatterns();
protected:
	virtual jswByte *getCellPattern(int type);
	virtual int getConveyorDir();
	virtual int getConveyorY();
	virtual int getCellBehaviour(int index);
	virtual int getItemCoords(int *xs, int *ys, int *counts);
private:
	void drawConveyor(VideoBitmap *bm, bool flashonly, bool draw = true);
	void drawRamp(VideoBitmap *bm, bool flashonly, bool draw = true);

};

