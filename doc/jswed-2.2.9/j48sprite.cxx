/************************************************************************

    JSWED 2.2.2 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "j48sprite.hxx"

Jsw48SpritePage::Jsw48SpritePage(JswGame *g) : SpritePage(g->getMem())
{
	m_list = new SpriteList(g->getMem(), 2, m_y + 20 * CHAR_H, VIDEO_W - 4,
				               CHAR_H + CHAR_SH + 40);
	g->fillSprites(-1, m_list);
	m_list->setTitle("Sprites in this game");
	addChild(m_list);
	addChild(m_editor);
	m_list->setListener(this);
	setFocus(m_list);
	m_reload = NULL;
}

Jsw48SpritePage::~Jsw48SpritePage()
{
}


void Jsw48SpritePage::reload(JswGame *g)
{	
	m_reload = g;
}

int Jsw48SpritePage::onReveal(int rect)
{
	int r = VideoNotePage::onReveal(rect);

	if (m_reload)
	{
		m_list->emptyBuffer();
		m_reload->fillSprites(-1, m_list);
		m_list->redraw();
		m_reload = 0;
	}
	return r;
}

