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

/* SDL file dialog{ue} box */
/* Note that for any hypothetical Windows port, you'd need a  */
/* false "root directory" matching the My Computer window.    */ 

class FileList : public ScrollingList
{
public:
	FileList();
	virtual ~FileList();
	int doModal(char *buffer);

	void setDirName(char *name);
	char *getDirName(void);
protected:
	char m_dirname[PATH_MAX];

	void saneSlashes(char *s);
	bool isRootDir(char *s);

	virtual void drawTitle(int x, int y);
	virtual void drawItem(int x, int y, int index, int bg);
	virtual void drawFileIcon(int x, int y, char fileIcon);
//
// Given a file, determine its type. The type letters used by this library
// include:
//
// ? - could not determine type
// @ - does not exist
// B - block device
// C - character device
// D - directory
// E - program (executable)
// F - regular file
// L - symlink
// P - named pipe
// S - socket
// X - could not access file
//
// The SpectrumMemory class uses the types:
// N - .SNA snapshot
// O - +3DOS file
// T - .TAP tapefile
// Z - .Z80 snapshot
//                         
	virtual char getFileType(char *filename, size_t size);
public:
	int  fillBuffer(void);
	void getSelection(char *buffer, size_t len);
	void setSelection(char *buffer);
private:
	virtual int  onSelect(void);
};

// File utils
int isHeaderPlus3Dos(jswByte hdr[128]);
