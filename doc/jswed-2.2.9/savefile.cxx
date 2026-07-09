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


int saveFile(Game **gp)
{
        char filename[PATH_MAX];
	JswFileForm ff;
	int rv;

        videoScreen->cls();
        videoScreen->bottomBar("Save game file");

	filename[0] = 0;
        rv = ff.doModal(filename);
 
        if (rv != ZXE_OK) return rv;

        FILE *fp = fopen(filename, "rb");
        if (fp)
        {
		fclose(fp);
                VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);
                
                if ( (rv = vm.doModal()) != ZXE_OK) return rv;
		if (vm.getSelected()) return ZXE_CANCEL;
	}
	return saveFile(filename, gp);
}


int saveFile(char *filename, Game **gp)
{
	int rv;

	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		videoScreen->drawText(0, VIDEO_H - CHAR_H, 
			"F Failed to open file", ZX_BLACK, ZX_WHITE);	
		return videoWaitForKey(ZXE_CANCEL);
	}
	SpectrumMemory *mem = (*gp)->getMem();

	rv = mem->toDisc(filename, fp);
	fclose(fp);
	if (rv != ZXE_OK) return rv;

	videoScreen->drawText(0, VIDEO_H - CHAR_H, "0 OK",
                                ZX_BLACK, ZX_WHITE);
        return videoWaitForKey(ZXE_OK);
}
