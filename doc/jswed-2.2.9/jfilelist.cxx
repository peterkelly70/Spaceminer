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

/* SDL file dialog{ue} box */
/* Note that for any hypothetical Windows port, you'd need a  */
/* false "root directory" matching the My Computer window.    */ 

/* Design of file chooser: 
 *
 * The file chooser should ultimately be a form, containing a list and 
 * an edit field. Currently we just have the list.
 *
 * List geometry is: 38 chars wide, 20 lines high, at (CHAR_W, CHAR_H)
 *
 */


JswFileList::JswFileList() : FileList()
{
	m_types  = newVideoBitmap(getResourceName("fileicons.png", "rb"));
}

JswFileList::~JswFileList()
{
	if (m_types)  delete m_types;
}

void JswFileList::drawFileIcon(int x, int y, char fileIcon)
{
      m_types->toScreen(x, y, CHAR_W * (fileIcon - '?'), 0, CHAR_W, CHAR_H);

	return;
}

char JswFileList::getFileType(char *fname, size_t size)
{
	FILE *fp;
	char *s = new char[1 + strlen(fname)];
	unsigned char rec[128];	
	int rv;
	unsigned int n;

	// Lowercase version of the filename
	for (n = 0; n <= strlen(fname); n++) s[n] = tolower(fname[n]); 
	
	fp = fopen(fname, "rb");
	if (fp)			// Of course, it might not open it 
	{
		rv = fread(rec, 1, sizeof(rec), fp);
		if (rv < (int)sizeof(rec)) 	
			memset(rec + rv, 0x1A, sizeof(rec) - rv);
		fclose(fp);
	}
	else	memset(rec, 0x1A, sizeof(rec));

	// TAP
	if (rec[0] == 0x13 && rec[1] == 0x00 && rec[2] == 0x00) return 'T';

	// +3DOS
	if (isHeaderPlus3Dos(rec)) return 'O';

	// SNA
	if ((size == 49179 || size == 49280) && strstr(s, ".sna")) 
		return 'N';

	// Z80
	if (strstr(s, ".z80") || strstr(s, ".slt"))
		return 'Z';

	return 'F';
}


