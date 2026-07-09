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

#include "jswed.hxx"
#include "executor.hxx"
#include <SDL.h>
#include <SDL_thread.h>

Executor::Executor(char *filename, bool removeAfterwards)
{
	strcpy(m_filename, filename);
	m_removeAfterwards = removeAfterwards;
}

static int threadRun(void *p)
{
	Executor *e = (Executor *)p;

	return e->workThread();
}


int Executor::exec(void)
{
	const char *emu = getSpectrumEmulator();
	if (!emu[0]) return alert("Test run", "Cancel",
			"No Spectrum emulator has been configured.");

	SDL_CreateThread(threadRun, this);
	return ZXE_CONTINUE;
}

int Executor::workThread()
{
	// 2.0.2: Copy our data onto the stack. The "this" pointer can't
	//       be trusted to hang around until the launched program
	//       finishes (at least, it can't in win32; Linux doesn't seem to
	//       mind).
	char filename[PATH_MAX + 1];
	int  removeAfterwards = m_removeAfterwards;
	char *emu = (char *)getSpectrumEmulator();
#ifdef _WIN32	
	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	PROCESS_INFORMATION pinfo;	
	char cmdbuf[PATH_MAX * 2 + 4];

	strcpy(filename, m_filename);

// 2.0.2: Bug fix: Avoid buffer overflows by using strncat() rather
//        than sprintf()
	strncpy(cmdbuf, emu, PATH_MAX); cmdbuf[PATH_MAX] = 0;
	strcat(cmdbuf, " ");
	strncat(cmdbuf, m_filename, PATH_MAX);
	cmdbuf[PATH_MAX + 1 + PATH_MAX] = 0;

	if (CreateProcess(NULL, cmdbuf, NULL, NULL, 0, NORMAL_PRIORITY_CLASS,
			NULL, NULL, &sinfo, &pinfo))
	{
		WaitForSingleObject(pinfo.hProcess, INFINITE);
	}
	else
	{
		int n = GetLastError();
		LPBYTE buffer;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			      FORMAT_MESSAGE_FROM_SYSTEM, NULL, n, 0, 
				(LPTSTR)&buffer, 20, NULL);
		alert("Windows error", "Cancel", "%s", (char *)buffer);
//		MessageBox(NULL, (char *)buffer, "Error", MB_OK | MB_ICONSTOP);
		LocalFree(buffer);
	}
	
#else
	pid_t pid;

	strcpy(filename, m_filename);
//
// fork/exec to launch the new process.
//
	pid = fork();
	int status;

	if (pid == 0)
	{
		char buf[PATH_MAX];
		char *args[128];
		int n;
//
// Tokenise the command into words
//
		strcpy(buf, emu);
		emu = buf;
		for (n = 0; n < 126;)
		{
			args[n++] = emu;
			emu = strchr(emu, ' ');
			if (!emu) break;
			*emu = 0;
			++emu;
		}
		args[n++] = filename;
		args[n++]   = NULL;
		execv(args[0], args);
	}
	else waitpid(pid, &status, 0);
#endif
	if (removeAfterwards) remove(filename);
	return 0;
}


