
/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2005  John Elliott <jce@seasip.demon.co.uk>

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

class ConfigureForm : public VideoForm
{
protected:
	VideoTextEdit *m_edit[3];
	VideoButton   *m_ok, *m_cancel;
public:
	int m_interval;


	ConfigureForm(char *emupath, char *asvpath) : 
		VideoForm(3 * CHAR_W, 8 * CHAR_H, 34 * CHAR_W,  14 * CHAR_H) 
	{

		m_edit[0] = new VideoTextEdit(3*CHAR_W + CHAR_SW,
				  m_y + 3*CHAR_H, 33*CHAR_W + 4, CHAR_H+4);
		m_edit[1] = new VideoTextEdit(3*CHAR_W + CHAR_SW,
				  m_y + 6*CHAR_H, 33*CHAR_W + 4, CHAR_H+4);
		m_edit[2] = new VideoTextEdit(3*CHAR_W + CHAR_SW,
				  m_y + 9*CHAR_H, 14*CHAR_W + 4, CHAR_H+4);
		m_edit[0]->setLimit(PATH_MAX);
		m_edit[0]->bind(emupath);
		m_edit[1]->setLimit(PATH_MAX);
		m_edit[1]->bind(asvpath);
		m_edit[2]->setNumeric();

	        m_ok = new VideoButton(ZXE_OK, 30 * CHAR_W, m_y + 11 * CHAR_H,
                     "  OK  ");
	        m_cancel = new VideoButton(ZXE_CANCEL, 22 * CHAR_W, m_y + 11 * CHAR_H,
                     "Cancel");

	}

	virtual ~ConfigureForm()
	{
		delete m_edit[0];
		delete m_edit[1];
		delete m_edit[2];
		delete m_ok;
		delete m_cancel;	
	}
        virtual void redraw(void)
	{
		VideoForm::redraw();
		videoScreen->box(m_x, m_y, m_w, m_h, ZX_BLACK);
	        videoScreen->drawText(m_x, m_y, "Configure JSWED          "
                              "         ", ZX_BRWHITE, ZX_BLACK);
	        videoScreen->zxLogo(m_x + m_w - 6 * CHAR_W, m_y);
		videoScreen->drawSmallText(m_x + CHAR_SW, m_y + 2 * CHAR_SH + 4,
		"Enter the full path to your Spectrum emulator - eg:", ZX_BLACK, ZX_WHITE);
		videoScreen->drawSmallText(m_x + CHAR_SW, m_y + 3 * CHAR_SH + 4,
#ifdef _WIN32
		"C:/WSPECEM/BINARIES/WSPECEM.EXE",
#else
		"/usr/local/bin/xzx -scale 2",
#endif
		ZX_BLACK, ZX_WHITE); 
		videoScreen->drawSmallText(m_x + CHAR_SW, m_y + 10 * CHAR_SH + 4, "Enter the path for a .Z80 snapshot which will be autosaved:", ZX_BLACK, ZX_WHITE);
		videoScreen->drawSmallText(m_x + CHAR_SW, m_y + 16 * CHAR_SH + 4, "Autosave interval (seconds, 0 to disable autosave):", ZX_BLACK, ZX_WHITE);
		
	}
protected:
	virtual void hideChildren(void)
	{
		m_interval = atoi(m_edit[2]->getText());
		VideoForm::hideChildren();	
		removeChild(m_edit[0]);
		removeChild(m_edit[1]);
		removeChild(m_edit[2]);
		removeChild(m_ok);
		removeChild(m_cancel);
	}
	virtual void showChildren(int redraw = 1)
	{
		char ibuf[30];
		sprintf(ibuf, "%d", m_interval);
		m_edit[2]->setText(ibuf);
	        addChild(m_edit[0]);
	        addChild(m_edit[1]);
	        addChild(m_edit[2]);
		addChild(m_ok);
		addChild(m_cancel);
		VideoForm::showChildren(redraw);
	}
};


int configure(void)
{
	char emubuf[PATH_MAX + 1];
	char asvbuf[PATH_MAX + 1];

	strcpy(emubuf, getSpectrumEmulator());
	strcpy(asvbuf, getAutosaveName());
	ConfigureForm cfg(emubuf, asvbuf);
	cfg.m_interval = getAutosaveInterval();

	int rv = cfg.doModal();

	if (rv == ZXE_OK) 
	{
		setSpectrumEmulator(emubuf);		
		setAutosaveName(asvbuf);
		setAutosaveInterval(cfg.m_interval);
	}
	return rv;	
}


