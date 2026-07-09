/************************************************************************

    JSWED 2.2.9 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-1,2004-6  John Elliott <jce@seasip.demon.co.uk>

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


#define JSWED_VERSION "2.2.9"

// Standard
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
// Unix and Mingw32
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Windows only
#ifdef _WIN32
#include <windows.h>
#else
// Unix only
#include <pwd.h>
#include <sys/wait.h>
#endif

//
// On my particular Linux->mingw32 cross-compiler, va_start returns
// a __gnuc_va_list; but this is a void* whereas va_list is a char *.
// So in varargs situations, I have to use the "arglist" type and 
// cast where appropriate.
//
#if defined(_WIN32) && defined(__GNUC__)
#  define arglist __gnuc_va_list
#else
#  define arglist va_list
#endif


typedef unsigned char jswByte;
typedef char *pChar;
typedef unsigned char *pByte;

#include "video.hxx"
#include "game.hxx"
#include "snapio.hxx"
#include "jfilelist.hxx"
#include "jfilesel.hxx"

// The currently loaded gamefile
extern Game *gl_game;

// Screens
int aboutBox(void);
int configure(void);
int jumpRoomNumber(int *number);

// General utilities

char *jswLibDir(void);
FILE *openResource(char *fname, char *mode);
char *getResourceName(char *fname, char *mode);
FILE *openTemp(char *fname, char *ftype, char *mode);
//
// Execute a .Z80 file.
//
int executeZ80(char *fname, bool removeAfterwards = true);
const char *getSpectrumEmulator(void);
void setSpectrumEmulator(const char *program);
const char *getAutosaveName(void);
void setAutosaveName(const char *program);
int getAutosaveInterval(void);
void setAutosaveInterval(int interval);
const char *getDataDir(void);
void setDataDir(const char *dir);
//
// A brief alert
//
int alert(char *title, char *action, char *details = NULL, ...);

void jswDiewith(int e, char *s, ...);
#ifdef DEBUGGING
void jswLog(char *s, ...);
#else
extern inline void jswLog(char *s, ...) {}
#endif
