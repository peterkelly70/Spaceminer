
#ifndef GAME_HXX_INCLUDED
#define GAME_HXX_INCLUDED 

/************************************************************************

    JSWED 2.2.5 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2005  John Elliott <jce@seasip.demon.co.uk>

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

#include "snapio.hxx"
#include "jnotebook.hxx"

class HexEditPage;

class Game : public KeyListener
{
public:
	virtual ~Game();
	virtual char *getID();	// 40-character ID string
	inline void setFilename(char *s) { strcpy(m_filename, s); }
	inline char *getFilename()       { return m_filename; }
	virtual int editMain();

	virtual void addPages(VideoNotebook *n) = 0;
	virtual void deletePages(void) = 0;
	inline SpectrumMemory *getMem() { return m_mem; }
	virtual int afterLoadFix(void);
protected:
	int onSave(void);
	int onTest(void);
	
	KeyListener *m_keyFilter;
public:
	// Overrides
	virtual int onKeyDown(int keysym);
	virtual int onKeyUp(int keysym);

	Game *m_upgrade;

protected:
	HexEditPage *m_hexPage;

	Game(SpectrumMemory *mem);
	SpectrumMemory *m_mem;
	char m_filename[PATH_MAX];
	char m_namebuf[41];
};

int saveFile(Game **gp);
int saveFile(char *filename, Game **gp);
int loadFile(Game **gp);
int loadFile(char *filename, Game **gp);

#endif // ndef GAME_HXX_INCLUDED
