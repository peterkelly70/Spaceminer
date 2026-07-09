/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001, 2005-6  John Elliott <jce@seasip.demon.co.uk>

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

#ifdef _WIN32
# define S_ISLNK(x)  0
# define S_ISSOCK(x) 0
#endif

// 2.0.1: BeOS support
#ifdef __BEOS__
#  define S_ISSOCK(x) 0
#endif

#define FC_W 38
#define FC_H 20

FileList::FileList() : ScrollingList(CHAR_W, CHAR_H,
				FC_W * CHAR_W, FC_H * CHAR_H)
{
	const char *filename = getDataDir();
	if (filename && filename[0])
	{
		strcpy(m_dirname, filename);
	}
	else
	{
		if (getcwd(m_dirname, PATH_MAX) == NULL)
		{
			jswLog("JSWED file chooser: Could not get current dir!\n");
			strcpy(m_dirname, "/");
		}
	}
}

FileList::~FileList()
{
	emptyBuffer();
}

void FileList::drawTitle(int x, int y)
{
	char s[40];

        if ((int)strlen(m_dirname) < m_fcW - 6)
        {
                sprintf(s, "%-*.*s", m_fcW - 7, m_fcW - 7, m_dirname);
        }
        else
        {
                strcpy(s, "...");
                strcpy(s + 3, m_dirname + strlen(m_dirname) - (m_fcW - 10));
        }
        // Title text
        videoScreen->drawText(m_x, m_y, s, ZX_BRWHITE, ZX_BLACK);
}


void FileList::drawItem(int x, int y, int index, int bg)
{
	char s[40];

	if (index >= getItemCount())
	{
		sprintf(s, "@%-*.*s", m_fcW - 3, m_fcW - 3, "");
	}
	else
	{
		char *str = getItem(index);
		if ((int)strlen(str) <= m_fcW - 2)
		{
			sprintf(s, "%-*.*s", m_fcW - 2, m_fcW - 2, str);
		}
		else
		{
			sprintf(s, "%-*.*s", m_fcW - 8, m_fcW - 8, str);
			strcat(s, "...");
			strcat(s, str + strlen(str) - 3);
		}
	}
	videoScreen->drawText(x + CHAR_W * 2, y, 1 + s, ZX_BLACK, bg);
	drawFileIcon(x + CHAR_W - (CHAR_W / 4), y, s[0]);
}

void FileList::drawFileIcon(int x, int y, char fileIcon)
{
	return;
}

char FileList::getFileType(char *filename, size_t size)
{
	return 'F';
}



static int sortfunc(const void *p1, const void *p2)
{
	const pChar s1 = *(const pChar *)p1;
	const pChar s2 = *(const pChar *)p2;

	// Directories always come first.
	if (s1[0] == 'D' && s2[0] != 'D') return -1;  
	if (s1[0] != 'D' && s2[0] == 'D') return 1;

	return strcmp(s1 + 1, s2 + 1);	
}


int FileList::fillBuffer()
{
	char fname[PATH_MAX * 2];
	DIR *dir;
	struct dirent *dire;

	emptyBuffer();

	dir = opendir(m_dirname);
	if (!dir)
	{
		addString("D.");
		addString("D..");	
		return ZXE_CANCEL;
	}	
	setDataDir(m_dirname);
	while ((dire = readdir(dir)))
	{
		char buf[FILENAME_MAX + 2];
		char *s;
		struct stat st;

		s = m_dirname + strlen(m_dirname) - 1;

		/* Get rid of extra slashes */
		if (*s == '/') sprintf(fname, "%s%s", m_dirname, dire->d_name);
		else sprintf(fname, "%s/%s", m_dirname, dire->d_name);
		strcpy(buf + 1, dire->d_name);
		buf[0] = 'X';
		if (!stat(fname, &st))
		{
			if      (S_ISDIR (st.st_mode)) buf[0] = 'D';
			else if (S_ISLNK (st.st_mode)) buf[0] = 'L'; // BSD only
			else if (S_ISCHR (st.st_mode)) buf[0] = 'C';
			else if (S_ISBLK (st.st_mode)) buf[0] = 'B';
			else if (S_ISFIFO(st.st_mode)) buf[0] = 'P';
			else if (S_ISSOCK(st.st_mode)) buf[0] = 'S';
			else if (S_ISREG (st.st_mode)) 
				buf[0] = getFileType(fname, st.st_size);
			else	buf[0] = '?';
		}
		addString(buf);
	}
	closedir(dir);

	sortItems(sortfunc);		
	checkLimits();

	return ZXE_OK;
}


int FileList::doModal(char *buffer)
{
	fillBuffer();	

	int rv =  ScrollingList::doModal();	
 
	getSelection(buffer, PATH_MAX);
	return rv;
}

void FileList::getSelection(char *buffer, size_t blen)
{
	char *item = getItem(m_selected);
	int  len   = strlen(item) + strlen(m_dirname) + 1;
	char *buf  = new char[len];

	// Avoid doubled slashes in filenames
	if (m_dirname[strlen(m_dirname) - 1] == '/') 
		sprintf(buf, "%s%s", m_dirname, item + 1);
	else	sprintf(buf, "%s/%s", m_dirname, item + 1); 

	strncpy(buffer, buf, blen);
	buffer[blen-1] = 0;
	delete [] buf;
}

int FileList::onSelect(void)
{
	char *s , *r;

	s= getItem(m_selected);

	if (s[0] == 'D') 
	{
		if (!strcmp(s + 1, ".")) return ZXE_CONTINUE;

		if (!strcmp(s + 1, ".."))
		{
			saneSlashes(m_dirname);
			r = strrchr(m_dirname, '/');
#ifdef _WIN32
			if (!r) r = strrchr(m_dirname, ':');
#endif
			if (r)
			{
				*r = 0;
//
// Special treatment for root directories: They keep their trailing "/",
// because it's also a leading "/".
//
#ifdef _WIN32
				if (strlen(m_dirname) == 2 &&
				    m_dirname[1] == ':') strcat(m_dirname, "/");
#else
				if (m_dirname[0] == 0) strcpy(m_dirname, "/");
#endif
				fillBuffer();
				draw(1);
				return ZXE_CONTINUE;
			}
			else return ZXE_CONTINUE;
		}
		if (!isRootDir(m_dirname)) strcat(m_dirname, "/");
		strcat(m_dirname, s + 1);
		fillBuffer();
		draw(1);
		return ZXE_CONTINUE;	
	}
	// XXX What about pipes, sockets etc. ?
	return ZXE_OK;
}


void FileList::setSelection(char *filename)
{
	char dirname[PATH_MAX];

	strncpy(dirname, filename, PATH_MAX);
	dirname[PATH_MAX - 1] = 0;
	saneSlashes(dirname);

	char *r = strrchr(dirname, '/');
#ifdef _WIN32
	if (!r) r = strrchr(dirname, ':');
#endif
	if (r) // don't chop off first "/"
	{
		*r = 0;
		if (dirname[0] == 0) strcpy(dirname, "/");
		strcpy(m_dirname, dirname);
		fillBuffer();
		draw(1);
	}
}

void FileList::setDirName(char *dirname)
{
	strncpy(m_dirname, dirname, PATH_MAX);
	saneSlashes(m_dirname);
	m_dirname[PATH_MAX - 1] = 0;

// Remove trailing slash from non-root directory
	if (m_dirname[0] == 0) strcpy(m_dirname, "/");
#ifdef _WIN32
	else if (m_dirname[1] == ':' && m_dirname[2] == 0)  strcat(m_dirname, "/");
#endif
	else if (!isRootDir(m_dirname))
	{
		char *s = m_dirname + strlen(m_dirname) - 1;
		if (*s == '/') *s = 0;
	}
	fillBuffer();
	draw(1);
}

char *FileList::getDirName(void)
{
	return m_dirname;
}



int isHeaderPlus3Dos(unsigned char hdr[128])
{
	int n,csum;

        for (n = csum = 0; n < 127; n++) csum += hdr[n];

        return ((csum & 0xFF) == hdr[n] && !memcmp(hdr, "PLUS3DOS\032", 9));
}


void FileList::saneSlashes(char *s)
{
#ifdef _WIN32
	int n;

	for (n = strlen(s) - 1; n >= 0; n--)
	{
		if (s[n] == '\\') s[n] = '/';
	}
#endif
}


bool FileList::isRootDir(char *s)
{
#ifdef _WIN32
	saneSlashes(s);

	if (!strcmp(s + 1, ":/"))  return true;
#else
	if (!strcmp(s, "/")) return true;
#endif
	return false;
}

