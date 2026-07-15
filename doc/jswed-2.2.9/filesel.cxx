/************************************************************************

    JSWED 2.00 - Editor for Jet Set Willy and derivatives

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

#define FC_H 24

FileForm::FileForm() : VideoForm(0, 0, VIDEO_W, FC_H * CHAR_H)
{
	m_list = NULL;
	m_edit = NULL;
}

FileForm::~FileForm()
{
	if (m_list) delete m_list;
	if (m_edit) delete m_edit;
}

void FileForm::showChildren(int redraw)
{
        if (!m_list) 
	{
		m_list = new FileList;
		addChild(m_list);
	}
        if (!m_edit) 
	{
		m_edit = new VideoTextEdit(CHAR_W - 2,
                                   (FC_H - 2)*CHAR_H,
                                   VIDEO_W - 2 * CHAR_W + 4,
                                   CHAR_H  + 4);
        	addChild(m_edit);
	}
        setFocus(m_list);
	VideoForm::showChildren(redraw);
}

void FileForm::hideChildren(void)
{
	VideoForm::hideChildren();
}


int FileForm::doModal(char *buffer)
{
	int rv;
	m_list->fillBuffer();

	do
	{	
        	rv =  VideoForm::doModal();

		if (rv != ZXE_OK) break;
		if (getFocus() == m_edit)
		{
			strncpy(buffer, m_edit->getText(), PATH_MAX);
			buffer[PATH_MAX - 1] = 0;
		}
		else 
		{
			m_list->getSelection(buffer, PATH_MAX);
		}
                DIR *d = opendir(buffer);
                if (d)
                {
			rv = ZXE_CONTINUE;
			m_list->setDirName(buffer);
			closedir(d);
                }
	} while (rv == ZXE_CONTINUE);
        return rv;
}

int FileForm::onKeyDown(int keysym)
{
	char buffer[PATH_MAX];

	if (keysym == ZXK_TAB || keysym == ZXK_BACKTAB || keysym == ZXK_ENTER)
	{
		if (getFocus() == m_edit)
		{
			m_list->setSelection(m_edit->getText());	
		}
		else
		{
			m_list->getSelection(buffer, PATH_MAX);
			m_edit->setText(buffer);
		}
	}

	return VideoForm::onKeyDown(keysym);
}
