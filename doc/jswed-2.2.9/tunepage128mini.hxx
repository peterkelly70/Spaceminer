/************************************************************************

    JSWED 2.2.7 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2005  John Elliott <jce@seasip.demon.co.uk>

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

#define SYMBOLSIZE 16

struct TuneSymbol
{
	int value;
	char name[SYMBOLSIZE];
};

class TuneEdit128Page : public VideoNotePage, ButtonSelectListener
{
// Data
protected:
	SpectrumMemory *m_mem;	
	TuneSymbol *m_symbols;
	int m_symCount, m_symMax;

	bool m_dirty;
	VideoButton *m_testButton[3];
	VideoButton *m_saveButton[3];
	VideoButton *m_loadButton[3];

	void undo(void);
	int onTest(int n);
	int onSave(int n);
	int onLoad(int n);
	int tuneBase(int n);
	int tuneLimit(int n);
	void channelRange(SpectrumMemory *m, int src, int *first, int *last,
			jswByte eoc);
	void tuneRange(SpectrumMemory *m, int n, int *first, int *last);
	void storeTune(SpectrumMemory *sm, SpectrumMemory *dm, int n, int addr);
	void realign  (SpectrumMemory *sm, SpectrumMemory *dm, 
			int dest, int src, jswByte eoc);

	int disassemble(int n, FILE *fp);
	void analyseChannel(int src, jswByte eoc, int *labels);
	int disChannel(int start, int finish, FILE *fp, int *labels);
	int saveTUN(int n);
	int saveTXT(int n);
	int import3dos(int tune, char *filename, jswByte *header, FILE *fp);
	int importAsm(int tune, char *filename, FILE *fp);
	void zapSymbols();
	void addSymbol(char *s, int value);
	int getNum(char *txt, int *result, int pass);
public:
	TuneEdit128Page(JswGame *g);
	virtual ~TuneEdit128Page();

	virtual int preContextMenu(void);

	virtual int redraw(int whichRectangle);
	virtual int onReveal(int rect);

	virtual int onButtonSelect(VideoButton *b);
};


