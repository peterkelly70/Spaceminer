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

//
// There are two classes here. Geoff2Game is based on the sample file 
// geoffmode.tap; while its base class, GeoffGame, is not currently used
// standalone. 
//
class GeoffGame : public Jsw48Game
{
public:
	GeoffGame(SpectrumMemory *mem);
	virtual char *getID();
	virtual ~GeoffGame();	

	virtual int getMariaRoom();
	virtual void setMariaRoom(int r);
	virtual int getToiletRoom();
	virtual void setToiletRoom(int r);
	virtual void getToiletPos(int *x, int *y);
	virtual void setToiletPos(int x, int y);	
};


class Geoff2Game : public GeoffGame
{
public:
	Geoff2Game(SpectrumMemory *mem);
	virtual char *getID();
	virtual jswByte *getTitleAttrs(void);
	virtual jswByte getWillySpritePage(void);
        virtual JswGuardian *newGuardian(jswByte *guard, int x, int bg);
        virtual GuardianEditor *newGuardianEditor(int room, jswByte *guard, VideoBitmap *b, int x);
	virtual ~Geoff2Game();	
};


