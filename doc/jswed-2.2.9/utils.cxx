/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "executor.hxx"
#ifdef _WIN32
#include <windows.h>
#endif

#define DOTFILE_DIR ".jswed"

// 2.0.1: BeOS support
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

/* Strange macro incantations to put quotes around a string */
#  define Q2(x)   Q1(x)
#  define Q1(x)    #x

char *
getResourceName(char *name, char *mode)
{
        FILE *fp;
        static char fname[PATH_MAX];
        char *home;

// 1. Look in the library dir.
        sprintf(fname, "%s/%s", jswLibDir(), name);
	fp = fopen(fname, mode);
        if (fp) { fclose(fp); return fname; }
// 2. Look in the user's home dir.

        home = getenv("HOME");
        if (!home) home = ".";
        sprintf(fname, "%s/%s", home, DOTFILE_DIR);
#ifdef _WIN32
	mkdir(fname);
#else
        mkdir(fname, 0777);
#endif
        sprintf(fname, "%s/%s/%s", home, DOTFILE_DIR, name);
        fp = fopen(fname, mode);
        if (fp) { fclose(fp); return fname; }

// 3. Look in the current directory
        return name;
}

// Wrapper for getResourceName()

FILE *
openResource(char *name, char *mode)
{
        char *s = getResourceName(name, mode);

        return fopen(s, mode);
}


char *
jswLibDir(void)
{
	char *s = getenv("JSWED_LIB");
	
	if (s) return s;
#ifdef _WIN32
        static char buf[PATH_MAX];
 
        GetModuleFileName(NULL, buf, PATH_MAX);
	s = strrchr(buf, '\\');
	if (!s) s = strrchr(buf, '/');
	if (s)
	{
		strcpy(s+1, "share");
		return buf;
	}
 
#endif  // _WIN32                                                                       
#ifdef DEBUGGING
	return "share";
#else
#  define JLD     Q2(JSW_LIB_DIR)
	return JLD ;
#endif	// DEBUGGING
}

#ifdef DEBUGGING
void
jswLog (char *s, ...)
{
        //arglist ap;
       	va_list ap; 
        va_start(ap, s);
#ifdef _WIN32
	{
		char b[1000];
		vsprintf(b, s, (va_list)ap);
		MessageBox(NULL, b, "JSWED", MB_OK);
	}
#else
        vprintf(s, (va_list)ap);
#endif
        va_end(ap);
}
#endif

void
jswDiewith(int e, char *s, ...)
{
	//arglist ap;
	va_list ap;
	
	va_start(ap, s);
	vfprintf(stderr, s, (va_list)ap);
	va_end(ap);
	exit(e);
}


FILE *openTemp(char *name, char *type, char *mode)
{
	FILE *fp = NULL;

#ifdef _WIN32
	do
	{
		char tname[PATH_MAX];
		GetTempPath(PATH_MAX, tname);
		if (!GetTempFileName(tname, "z80", 0, name)) return NULL;
		remove(name);

		char *s = strrchr(name, '.');
		if (s) strcpy(s, type);
		else   strcat(name, type);
                // The O_EXCL prevents the possibility of overwriting
                // existing files.
                int fd = open(name, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 
					S_IRUSR | S_IWUSR);
		fp = NULL;
 		if (fd >= 0) fp = fdopen(fd, mode); //
	} while (!fp); 
#else 
	char *tdir = getenv("TMPDIR");
	do
	{
#define JTMP    Q2(TMPDIR)	
		// I'm using tmpnam() here. By itself this is not 
		// sufficient to guarantee a unique temporary filename;
		// but mkstemp() doesn't allow me to specify a ".z80" 
		// ending, which I need.
		if (tdir) sprintf(name, "%s/jswedXXXXXXXX", tdir);
		else	  sprintf(name, "%s/jswedXXXXXXXX", JTMP);

		if (!tmpnam(name)) continue;
		strcat(name, type);

		// The O_EXCL | O_NOFOLLOW prevents symlink attacks on the 
		// possibly insecure temp filename I got. 
		int fd = open(name, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 
				S_IRUSR | S_IWUSR);
		if (fd < 0) continue;

		fp = fdopen(fd, mode);
		if (!fp) perror(name);
		return fp;
	} while (!fp);
#endif
	return fp;
}



//
// Execute a .Z80 file
//
int executeZ80(char *filename, bool removeAfterwards)
{
	Executor ex(filename, removeAfterwards);	

	return ex.exec();
}


static int oldalert(char *title, char *option)
{
	TickListener *tl = setTickListener(NULL);
	VideoMenu v(title, option, NULL);
	int r = v.doModal();
	setTickListener(tl);
	if (r >= ZXE_QUIT) return r;
	return ZXE_CONTINUE;
} 

static int splitLine(char *src, char *dest)
{
	char buf[33];
	char *p;

	if (strlen(src) < 32) 
	{
		strcpy(dest, src);
		while (p = strchr(dest, '\n'))
		{
			*p = ' ';
		}
		return strlen(dest);
	}
	sprintf(buf, "%-32.32s", src);
	p = strchr(buf, '\n');
	if (p)
	{
		*p = 0;
		strcpy(dest, buf);
		return strlen(buf) + 1;
	}
	p = strrchr(buf, ' ');
	if (p)
	{
		*p = 0;
		strcpy(dest, buf);
		return strlen(buf) + 1;
	}
	strcpy(dest, buf);
	return strlen(buf);
}

typedef VideoLabel *PLABEL;

class AlertForm : public VideoForm
{
	VideoLabel **m_lbl;
	VideoButton *m_ok;
	int m_lines;
	char *m_title;
public:
	AlertForm(int lines, char *title, char *buf, char *action) 
		: VideoForm(3 * CHAR_W, (14-(lines/2))*CHAR_H,
			34 * CHAR_W, (lines + 4) * CHAR_H)
	{
		char ln[33];
		int n;
		char *p = buf;

		m_title = title;
		m_lines = lines;
		m_lbl = new PLABEL[lines];
		for (n = 0; n < lines; n++)
		{
			p += splitLine(p, ln);
			m_lbl[n] = new VideoLabel
				(m_x + CHAR_W, m_y + (n+1)*CHAR_H, ln);
		}
		m_ok = new VideoButton(ZXE_OK, 
				m_x + (17-(strlen(action)/2))*CHAR_W,
				       m_y + m_h - 2 * CHAR_H,
				       action);
	}

	~AlertForm()
	{
		delete m_ok;
		for (int n = 0; n < m_lines; n++) delete m_lbl[n];
		delete m_lbl;
	}

	virtual void showChildren(int redraw)
	{
		int n;

		addChild(m_ok);
		for (n = 0; n < m_lines; n++) addChild(m_lbl[n]);
		VideoForm::showChildren(redraw);	
	}

	virtual void hideChildren()
	{
		VideoForm::hideChildren();	
		int n;

		for (n = 0; n < m_lines; n++) removeChild(m_lbl[n]);
		removeChild(m_ok);
	}

	virtual void redraw()
	{
		VideoForm::redraw();
		redrawBorder(m_title);
	}
};

int alert(char *title, char *option, char *details, ...)
{
	va_list ap;
	char buffer[2000];
	char tmp[33];
	int lines;
	char *p, *q;

	if (!details) return oldalert(title, option);
	va_start(ap, details);
#ifdef HAVE_VSNPRINTF
	vsnprintf(buffer, sizeof(buffer), details, ap);
#else
	vsprintf(buffer, details, ap);
#endif
	for (lines=0, p = buffer; p[0]; lines++)
	{
		p += splitLine(p, tmp);
	}
	TickListener *tl = setTickListener(NULL);
	AlertForm af(lines, title, buffer, option);
	int r = af.doModal();
	setTickListener(tl);
	if (r >= ZXE_QUIT) return r;
	return ZXE_CONTINUE;
} 



#ifndef _WIN32
char *getHomeDir(void)
{
	static char buf[PATH_MAX];

	struct passwd *pwd = getpwuid(getuid());
	if (pwd) 
	{
		strcpy(buf, pwd->pw_dir);
		return buf;
	}
	char *hdir = getenv("HOME");
	if (hdir)
	{
		strcpy(buf, hdir);
		return buf;	
	}
	return "/";	// What else can you do?
}
#endif



const char *getSetting(const char *setting)
{
	static char buf[PATH_MAX + 20];
#ifdef _WIN32
	HKEY hKey;
	DWORD disp;

	long r = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\jce@seasip\\JSWED2",
			0, "REG_SZ", REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS, NULL, &hKey, &disp);
	if (r != ERROR_SUCCESS)	return "";
	disp = PATH_MAX;
	r = RegQueryValueEx(hKey, setting, NULL, NULL, 
				(BYTE *)buf, &disp);
	if (r != ERROR_SUCCESS) 
	{
		RegCloseKey(hKey);
		return "";
	}
	RegCloseKey(hKey);
	buf[disp] = 0;
#else
	FILE *fp;
	char *homedir = getHomeDir();

	sprintf(buf, "%s/.jswedrc", homedir);
	fp = fopen(buf, "r");
	if (!fp) { buf[0] = 0; return buf; }	// 2.0.1: Don't return path to .jswedrc!
	while (fgets(buf, sizeof(buf), fp))
	{
		if (!strncmp(buf, setting, strlen(setting)))
		{
			char *t = strrchr(buf, '\n');
			if (t) *t = 0;
			char *s = strchr(buf, '=');
			if (!s) continue;
			return s + 1;
		}
	}
	buf[0] = 0;
#endif
	return buf;
}


void setSetting(const char *variable, const char *value)
{
#ifdef _WIN32
	HKEY hKey;
	DWORD disp;

	long r = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\jce@seasip\\JSWED2",
			0, "REG_SZ", REG_OPTION_NON_VOLATILE, 
			KEY_ALL_ACCESS, NULL, &hKey, &disp);
	if (r != ERROR_SUCCESS)	return;
	RegSetValueEx(hKey, variable, 0, REG_SZ, (BYTE *)value,
			1 + strlen(value));
	RegCloseKey(hKey);
#else
        char buf[PATH_MAX];
        char buf2[PATH_MAX];
	FILE *fp, *fp2;
	char *homedir = getHomeDir();
	int found;

	sprintf(buf, "%s/.jswedrc.new", homedir);
	fp2 = fopen(buf, "w");
	sprintf(buf, "%s/.jswedrc", homedir);
	fp = fopen(buf, "r");
	if (!fp)
	{
		fp = fopen(buf, "w");
		if (!fp) return;
		fprintf(fp, "%s=%s\n", variable, value);
		fclose(fp);
		return;
	}
	found = 0;
	while (fgets(buf, sizeof(buf), fp))
	{
		if (!strncmp(buf, variable, strlen(variable)))
		{
			fprintf(fp2, "%s=%s\n", variable, value);
			found = 1;
		}
		else fprintf(fp2, "%s", buf);
	}
	if (!found)
	{
		fprintf(fp2, "%s=%s\n", variable, value);
	}
	fclose(fp);
	fclose(fp2);
	sprintf(buf,  "%s/.jswedrc.new", homedir);
	sprintf(buf2, "%s/.jswedrc", homedir);
	if (!remove(buf2)) rename(buf, buf2);
#endif
}


const char *getSpectrumEmulator(void)
{
	return getSetting("SpectrumEmulator");
}

void setSpectrumEmulator(const char *program)
{
	setSetting("SpectrumEmulator", program);
}
	

static char cachedAutosaveName[PATH_MAX];
static int  cachedNameValid = 0;
static int  cachedAutosaveInterval = 0;
static int  cachedIntervalValid = 0;

// This will want to be read 50 times a second or so. To avoid hitting the
// Registry or the disk that often, keep a copy of the name in memory.
const char *getAutosaveName(void)
{
	if (!cachedNameValid)
	{
		strcpy(cachedAutosaveName, getSetting("AutosaveName"));
		cachedNameValid = 1;
	}
	return cachedAutosaveName;
}

void setAutosaveName(const char *program)
{
	strcpy(cachedAutosaveName, program);
	cachedNameValid = 1;
	setSetting("AutosaveName", program);
}

int getAutosaveInterval(void)
{
	if (!cachedIntervalValid)
	{
		const char *s = getSetting("AutosaveInterval");
		cachedAutosaveInterval = atoi(s);
		cachedIntervalValid = 1;
	}
	return cachedAutosaveInterval;
}

void setAutosaveInterval(int n)
{
	char buf [30];
	sprintf(buf, "%d", n);

	cachedAutosaveInterval = n;
	cachedIntervalValid = 1;
	setSetting("AutosaveInterval", buf);
}



const char *getDataDir()
{
	return getSetting("DataDir");
}

void setDataDir(const char *dir)
{
	setSetting("DataDir", dir);
}
