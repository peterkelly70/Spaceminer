/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001  John Elliott <jce@seasip.demon.co.uk>

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

#include "jswed.hxx"
#include "j128guard.hxx"
#include "j128glist.hxx"
#include "guardedit.hxx"
#include "j128gedit.hxx"
#include "hexpage.hxx"

Game::Game(SpectrumMemory *mem)
{
	m_mem = mem;
	m_filename[0] = 0;
	m_hexPage = new HexEditPage(m_mem);
}

Game::~Game()
{
	delete m_hexPage;
	delete m_mem;
}

char *Game::getID()
{
	char *name, *pname;
	
	name  = m_filename;
	pname = strrchr(name, '/');
	if (pname) name = pname + 1;
#ifdef _WINDOWS
	pname = strrchr(name, '\\');
	if (pname) name = pname + 1;
	pname = strrchr(name, ':');
	if (pname) name = pname + 1;
#endif
	sprintf(m_namebuf, "%-40.40s", name);
	return m_namebuf;
}





int Game::editMain()
{
	m_keyFilter = setAppKeyFilter(this);
	JswNotebook p(0, 0, VIDEO_W, VIDEO_H);
	p.setTitle("Screens");
	addPages(&p);
	int rv = p.doModal();
	deletePages();
	setAppKeyFilter(m_keyFilter);
	return rv;
}

void Game::addPages(VideoNotebook *n)
{
	n->addPage(m_hexPage);
}


int Game::onKeyDown(int keysym)
{
	switch(keysym)
	{
		case ZXK_F5:	return onTest();
		case ZXK_F2:	return onSave();
	}

        if (m_keyFilter) return m_keyFilter->onKeyDown(keysym);
        return -1;
}


int Game::onKeyUp(int keysym)
{
	if (m_keyFilter) return m_keyFilter->onKeyUp(keysym);
	return -1;
}


int Game::onTest(void)
{
	char temp_name[PATH_MAX];
	FILE *fp = openTemp(temp_name, ".z80", "w+b");
	if (!fp) 
	{
		alert("Couldn't save temp file", "Cancel");
		return ZXE_CONTINUE;
	}
	m_mem->saveZ80(fp);
	fclose(fp);

	return executeZ80(temp_name);
}

int Game::onSave(void)
{
        int rv;

	if (!m_filename[0]) 
	{
		return alert("No filename, so can't save ", "Cancel");
	}
        FILE *fp = fopen(m_filename, "wb");
        if (!fp)
        {
		return alert("Failed to open file", "Cancel");
        }
        SpectrumMemory *mem = getMem();

        rv = mem->toDisc(m_filename, fp);
        fclose(fp);
        if (rv >= ZXE_QUIT) return rv;

	rv = alert("Game saved", "OK"); 
	if (rv >= ZXE_QUIT) return rv;
	return ZXE_CONTINUE;
}

int Game::afterLoadFix(void)
{
	return ZXE_CONTINUE;
}

