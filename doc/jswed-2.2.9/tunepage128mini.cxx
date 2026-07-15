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

#include "jswed.hxx"
#include "jswgame.hxx"
#include "tunepage128mini.hxx"


TuneEdit128Page::TuneEdit128Page(JswGame *g)
{
	m_mem = g->getMem();
	setTitle("Music");
	m_dirty = false;
	for (int n = 0; n < 3; n++)
	{
		m_testButton[n] = new VideoButton('a'+n, m_x + 4 + 8 * CHAR_W,
				m_y + 4 * CHAR_H + 8 * n * CHAR_H, "Test");
		addChild(m_testButton[n]);
		m_testButton[n]->setListener(this);

		m_saveButton[n] = new VideoButton('d'+n, m_x + 4 + 14 * CHAR_W,
				m_y + 4 * CHAR_H + 8 * n * CHAR_H, "Export");
		addChild(m_saveButton[n]);
		m_saveButton[n]->setListener(this);

		m_loadButton[n] = new VideoButton('g'+n, m_x + 4 + 22 * CHAR_W,
				m_y + 4 * CHAR_H + 8 * n * CHAR_H, "Import");
		addChild(m_loadButton[n]);
		m_loadButton[n]->setListener(this);
	}
	m_symbols = NULL;
	m_symCount = 0;
	m_symMax = 0;
}

TuneEdit128Page::~TuneEdit128Page()
{
	for (int n = 0; n < 3; n++)
	{
		removeChild(m_testButton[n]);
		removeChild(m_saveButton[n]);
		removeChild(m_loadButton[n]);
		delete m_testButton[n];
		delete m_saveButton[n];
		delete m_loadButton[n];
	}
	if (m_symbols) delete m_symbols;
}


int TuneEdit128Page::tuneBase(int n)
{
	switch(n)
	{
		case 0: return 0x752A; break;
		case 1: return 0xF700; break;
		case 2: return 0xF902; break;
	}
	return 0;
}


int TuneEdit128Page::tuneLimit(int n)
{
	switch(n)
	{
		case 0: return 0x7FFF; break;
		case 1: return 0xF7FF; break;
		case 2: return 0xF9FF; break;
	}
	return 0;
}

void TuneEdit128Page::realign(SpectrumMemory *sm, SpectrumMemory *dm,
		int dest, int src, jswByte eoc)
{
	jswByte op;
	int na;

	do
	{
		op = sm->peek(src);
		dm->poke(dest, op);
		++src,++dest;
		switch(op)
		{
			case 0xFF: 
			case 0xFD:  break;
			case 0xFE:  na = sm->peekW(src);
				    dm->pokeW(dest,  (na - src + dest));
				    src += 2;
				    dest += 2;
				    realign(sm, dm, na - src + dest, na, 0xFD);
				    break;
			case 0xFC:  dm->pokeW(dest, sm->peekW(src));
				    src += 2;
				    dest += 2;
			default:    dm->poke(dest, sm->peek(src));
				    ++src, ++dest;
				    break;
		}
	}
	while (op != eoc);
}

void TuneEdit128Page::storeTune(SpectrumMemory *sm, SpectrumMemory *dm,
		int n, int addr)
{
	int base = tuneBase(n);
	int channel;

	for (channel = 0; channel < 3; channel++)
	{
		int achannel = sm->peekW(base + 2 * channel);

		dm->pokeW(addr + 2 * channel, achannel - base + addr);	
		realign(sm, dm, achannel - base + addr, achannel, 0xFF);	
	}

}



int TuneEdit128Page::redraw(int whichRectangle)
{
	int rv = VideoNotePage::redraw(whichRectangle);
	int n;

	if (whichRectangle & VPR_RIGHT)
	{
		videoScreen->bottomBar("Music save/load");

		videoScreen->drawText(m_x + 4 + 8 * CHAR_W, m_y, 
				"Title screen tune", ZX_BLACK, ZX_WHITE);
		videoScreen->drawText(m_x + 4 + 10 * CHAR_W, m_y + 8 * CHAR_H, 
				"In-game tune", ZX_BLACK, ZX_WHITE);
		videoScreen->drawText(m_x + 4 + 9 * CHAR_W, m_y + 16 * CHAR_H,
				"Cheat mode tune", ZX_BLACK, ZX_WHITE);

		/*
		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H,
			"Keyboard hints: Select a char. using cu"
			"rsor keys and ENTER; then press TAB to", 
			ZX_BLACK, ZX_WHITE);
		videoScreen->drawSmallText(0, VIDEO_H - 4 * CHAR_H + CHAR_SH,
			"switch to the editor. ^Z or ^U to undo", 
			ZX_BLACK, ZX_WHITE); */
	}
	return rv;
}



int TuneEdit128Page::onReveal(int rect)
{
	int r = VideoNotePage::onReveal(rect);

// If changes have been made, don't reload the original data
	if (!m_dirty)
	{
		memcpy(m_mem->memoryAt(0x752A),
		       m_mem->memoryAt(0xF52A,7),
		       0x0AD6);

		// XXX Show tune info	
		// XXX redraw(VPR_RIGHT);
	}
	return r;
}



/*
void TuneEdit128Page::undo(void)
{
	if (!m_font) return;

	jswByte buf[8];
	jswByte *bm = m_editor->getBitmap();
	
	memcpy(buf, bm, 8);
	memcpy(bm, m_undoBuf, 8);
	memcpy(m_undoBuf, buf, 8);
	m_editor->setBitmap(bm);	// Make it redraw
}
*/

int TuneEdit128Page::preContextMenu(void)
{
	return VideoNotePage::preContextMenu();
	//return m_editor->doContextMenu();
}

void TuneEdit128Page::channelRange(SpectrumMemory *m, int src, 
		int *first, int *last, jswByte eoc)
{
	jswByte op;
	int na;

	do
	{
		if (*first < 0 || src < *first) *first = src;
		op = m->peek(src);
		++src;
		switch(op)
		{
			case 0xFF: 
			case 0xFD:  break;
			case 0xFE:  na = m->peekW(src);
				    src += 2;
				    channelRange(m, na, first, last, 0xFD);
				    break;
			case 0xFC:  src += 3;
				    break;
			default:    src++;
				    break;
		}
		if (*last < 0  || (src-1) > *last)  *last = src - 1;
	}
	while (op != eoc);
}


void TuneEdit128Page::tuneRange(SpectrumMemory *m, int n, int *first, int *last)
{
	int ch;
	int base = tuneBase(n);
	int chan;

	*first = base;
	*last = base + 5;

	for (ch = 0; ch < 3; ch++)
	{
		chan = m->peekW(base + 2*ch);
		channelRange(m, chan, first, last, 0xFF);
	}
}


void TuneEdit128Page::analyseChannel(int src, jswByte eoc, int *labels)
{
	jswByte op;
	int na, lbl;

	do
	{
		op = m_mem->peek(src);
		++src;
		switch(op)
		{
			case 0xFF: 
			case 0xFD:  break;
			case 0xFE:  na = m_mem->peekW(src);
				    for (lbl = 0; labels[lbl]; lbl++)
				    {
					    if (labels[lbl] == na) break;
				    }
				    if (labels[lbl] == 0) labels[lbl] = na;
				    src += 2;
				    analyseChannel(na, 0xFD, labels);
				    break;
			case 0xFC:  src += 3;
				    break;
			default:    src++;
				    break;
		}
	}
	while (op != eoc);
}

int TuneEdit128Page::disChannel(int start, int finish, FILE *fp, int *labels)
{
	jswByte op;
	int src, na, lbl;

	for (src = start; src < finish; )
	{
		for (lbl = 0; labels[lbl]; lbl++)
		{
			if (labels[lbl] == src)
			{
				fprintf(fp, "G%04x:", src);
			}
		}
		op = m_mem->peek(src);
		++src;
		fprintf(fp, "\t");	
		switch(op)
		{
			case 0xFF:  fprintf(fp, "defb\t0FFh\t;End of channel\n;\n");
				    break;
			case 0xFE:  na = m_mem->peekW(src);
				    fprintf(fp, "defb\t0FEh\t;Call subroutine\n");
				    fprintf(fp,"\tdefw\tG%04x\n", na);
				    src += 2;
				    break;
			case 0xFD:  fprintf(fp, "defb\t0FDh\t;End of subroutine\n;\n");
				    break;
			case 0xFC:  fprintf(fp, "defb\t0FCh\t;Loop\n");
				    fprintf(fp, "\tdefb\t%03xh,%03xh\t;Repeats\n",
					m_mem->peek(src), m_mem->peek(src+1));
				    fprintf(fp, "\tdefb\t%03xh\t;Jump offset\n",
					m_mem->peek(src+2));
				    src += 3;
				    break;
			case 0xFB:  fprintf(fp, "defb\t0FBh,%03xh\t;"
					   "Set volume\n", m_mem->peek(src));
				    src++;
				    break;
			case 0xFA:  fprintf(fp, "defb\t0FAh,%03xh\t;"
				   "Set waveform duration\n", m_mem->peek(src));
				    src++;
				    break;
			case 0xF9:  fprintf(fp, "defb\t0F9h,%03xh\t;"
				   "Set waveform\n", m_mem->peek(src));
				    src++;
				    break;
			case 0x00:  fprintf(fp, "defb\t00h,%03xh\t;"
				   "Rest\n", m_mem->peek(src));
				    src++;
				    break;
			default:    fprintf(fp, "defb\t%03xh,%03xh\n",
				     op, m_mem->peek(src));
				    src++;
				    break;
		}
	}
	return 0;
}




int TuneEdit128Page::disassemble(int n, FILE *fp)
{
	int first, last, len, base, ptr, ch;
	int *labels;

	tuneRange(m_mem, n, &first, &last);
	len = last + 1 - first;
	labels = (int *)malloc(len * sizeof(int));

	if (!labels)
	{
		alert("Out of memory", "Cancel");
		return -1;
	}
	base = tuneBase(n);
	memset(labels, 0, len * sizeof(int));

	for (ch = 0; ch < 3; ch++)
	{
		labels[ch] = m_mem->peekW(base + 2*ch);
	}
	for (ch = 0; ch < 3; ch++)
	{
		analyseChannel(labels[ch], 0xFF, labels);	
	}
	fprintf(fp, "\torg\t%05xh\n\n", first);
	ptr = first;
	if (ptr < base) disChannel(ptr, base, fp, labels);
	for (ch = 0; ch < 3; ch++)
	{
		fprintf(fp, "\tdefw\tG%04x\n", labels[ch]);
	}
	ptr = base + 6;
	if (ptr < last) disChannel(ptr, last + 1, fp, labels);

	fprintf(fp, "\n\tend\n");
	free(labels);
	return 0;
}



int TuneEdit128Page::saveTXT(int n)
{
	int rv, m;
        char filename[PATH_MAX];
        JswFileForm ff;

	filename[0] = 0;
	rv = ff.doModal(filename);
	if (rv != ZXE_OK) return rv;
	FILE *fp = fopen(filename, "rb");
        if (fp)
        {
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

                if ( (rv = vm.doModal()) != ZXE_OK) return ZXE_CONTINUE;
                if (vm.getSelected()) return ZXE_CONTINUE;
	}

	fp = fopen(filename, "w");
	if (fp)
	{
		if (disassemble(n, fp) || fclose(fp) == EOF)
		{
			return alert("Write failed", "Cancel",
			             "Could not write to file:\n%s", filename);

		}
	}
	else
	{
		return alert("Write failed", "Cancel",
                        "Could not open file:\n%s", filename);
	}	
	return ZXE_CONTINUE;
}




int TuneEdit128Page::saveTUN(int n)
{
	int first, last, len;
	int rv, m;
        char filename[PATH_MAX];
        JswFileForm ff;
	jswByte tunHeader[128];

	tuneRange(m_mem, n, &first, &last);
	filename[0] = 0;
	rv = ff.doModal(filename);
	if (rv != ZXE_OK) return rv;
	FILE *fp = fopen(filename, "rb");
        if (fp)
        {
		fclose(fp);
		VideoMenu vm("Overwrite existing file?", "Yes", "No", NULL);

                if ( (rv = vm.doModal()) != ZXE_OK) return ZXE_CONTINUE;
                if (vm.getSelected()) return ZXE_CONTINUE;
	}
	len = last + 1 - first;
	memset(tunHeader, 0, sizeof(tunHeader));
	memcpy(tunHeader, "PLUS3DOS\032\001\000", 11);
	tunHeader[11] = ((len + 128)      ) & 0xFF;
	tunHeader[12] = ((len + 128) >>  8) & 0xFF;
	tunHeader[13] = ((len + 128) >> 16) & 0xFF;
	tunHeader[14] = ((len + 128) >> 24) & 0xFF;
	tunHeader[15] = 3;
	tunHeader[16] = len & 0xFF;
	tunHeader[17] = len >> 8;
	tunHeader[18] = first & 0xFF;
	tunHeader[19] = first >> 8;
	for (m = 0; m < 127; m++) tunHeader[127] += tunHeader[m];
	fp = fopen(filename, "wb");
	if (fp)
	{
		if (fwrite(tunHeader, 1, sizeof(tunHeader), fp) < sizeof(tunHeader) ||
		    fwrite(m_mem->memoryAt(first), 1, len, fp) < len ||
		    fclose(fp) == EOF)
		{
			return alert("Write failed", "Cancel",
			             "Could not write to file:\n%s", filename);

		}
	}
	else
	{
		return alert("Write failed", "Cancel",
                        "Could not open file:\n%s", filename);
	}	
	return ZXE_CONTINUE;
}

int TuneEdit128Page::import3dos(int tune, char *filename, jswByte *header, FILE *fp)
{
	SpectrumMemory newMem(m_mem);
	jswByte *buf;
	int ch;
	int first, last, limit;
	int loadadd = header[18] + 256 * header[19];
	int len =     header[16] + 256 * header[17];

	if (loadadd == 0) buf = newMem.memoryAt(0x4000);
	else		  buf = newMem.memoryAt(loadadd);
			
	if (fread(buf, 1, len, fp) < len ||
	    fclose(fp) == EOF)
	{
		return alert("Read failed", "Cancel",
                        "Could read file:\n%s", filename);
	}
/* If the load address was not given in the header, assume that the tune 
 * starts with the channel pointers, and work out from them where it should
 * load. */
	if (loadadd == 0)
	{
		if (buf[1] == 0xF7)
		{
			loadadd = 0xF700;
			memcpy(newMem.memoryAt(loadadd), buf, len);
		}
		else if (buf[1] == 0xF9)
		{
			loadadd = 0xF902;
			memcpy(newMem.memoryAt(loadadd), buf, len);
		}
		else
		{
			loadadd = 0x752A;
			memcpy(newMem.memoryAt(loadadd), buf, len);
		}
	}
	/* Now work out how much space the tune occupies */
	int base = tuneBase(tune);
	limit = tuneLimit(tune);
	first = loadadd;
	last  = loadadd + 5;

	for (ch = 0; ch < 3; ch++)
	{
		int chan = newMem.peekW(loadadd + 2*ch);
		channelRange(&newMem, chan, &first, &last, 0xFF);
	}
	if ((last - first) > (limit - base))
	{
		return alert("Load failed", "Cancel",
			"This tune is too big to load into the chosen slot.");
	}
/* Copy the tune data in */

	for (ch = 0; ch < 3; ch++)
	{
		int achannel = newMem.peekW(base + 2 * ch);

		m_mem->pokeW(base + 2 * ch, achannel - loadadd + base);	
		realign(&newMem, m_mem, achannel - loadadd + base, 
				achannel, 0xFF);
	}
	if (!tune) memcpy(m_mem->memoryAt(0xF52A, 7), m_mem->memoryAt(0x752A), 
			0xAD6);
	return ZXE_CONTINUE;
}

void TuneEdit128Page::zapSymbols()
{
	if (m_symbols) delete m_symbols;
	m_symbols = NULL;
	m_symCount = 0;
	m_symMax = 0;
}


void TuneEdit128Page::addSymbol(char *name, int value)
{
	while (*name == ' ' || *name == '\t') ++name;

	if (!m_symbols)
	{
		m_symCount = 0;
		m_symMax   = 8;
		m_symbols  = new TuneSymbol[8];
	}	
	if (m_symCount >= m_symMax)
	{
		TuneSymbol *s = new TuneSymbol[2 * m_symMax];
		memcpy(s, m_symbols, m_symMax * sizeof(TuneSymbol));
		delete m_symbols;
		m_symbols = s;
		m_symMax *= 2;	
	}
	m_symbols[m_symCount].value = value;
	strncpy(m_symbols[m_symCount].name, name, SYMBOLSIZE - 1);
	m_symbols[m_symCount].name[SYMBOLSIZE - 1] = 0;
	char *s = strchr(m_symbols[m_symCount].name, ' ');
	if (s) *s = 0;
	s = strchr(m_symbols[m_symCount].name, '\t');
	if (s) *s = 0;
	s = strchr(m_symbols[m_symCount].name, ':');
	if (s) *s = 0;
	++m_symCount;
}


int TuneEdit128Page::getNum(char *txt, int *result, int pass)
{
	char buf[SYMBOLSIZE];
	char *s;
	int res;
	int n;

	while (*txt == ' ' || *txt == '\t') ++txt;

	if (isdigit(*txt))	/* Real number */
	{
		if (strchr(txt, 'h')) res = sscanf(txt, "%xh", result);
		else		      res = sscanf(txt, "%d", result);
		return res;
	}
	if (pass == -1) return 0;
	if (pass == 0) return 1;

	strncpy(buf, txt, SYMBOLSIZE - 1);
	buf[SYMBOLSIZE - 1] = 0;
	s = strchr(buf, ' '); if (s) *s = 0;
	s = strchr(buf, '\t'); if (s) *s = 0;
	for (n = 0; n < m_symCount; n++)
	{
		if (!strcmp(buf, m_symbols[n].name)) 
		{
			*result = m_symbols[n].value;
			return 1;
		}
	}	
	return 0;
}


int TuneEdit128Page::importAsm(int tune, char *filename, FILE *fp)
{
	int loadadd = 0;
	int locptr  = 0;
	char *semi;
	int n, pass, hexv, line;
	char linebuf[PATH_MAX];
	SpectrumMemory newMem(m_mem);

	zapSymbols();
	for (pass = 0; pass < 2; pass++)
	{
		line = 0;
		/* This is a miniature 2-pass assembler which only understands  
		 * ORG, DEFB, DEFW and labels */
		while (fgets(linebuf, sizeof(linebuf), fp))
		{
			++line;
			/* Reduce input to lowercase */
			for (n = strlen(linebuf)-1; n >= 0; n--) 
				linebuf[n] = tolower(linebuf[n]);
			semi = strchr(linebuf, ';');
			if (semi) *semi = 0;
			semi = strchr(linebuf, '\n');
			if (semi) *semi = 0;
			semi = strchr(linebuf, ':');
		/* Deal with labels */
			if (semi)
			{
				*semi = 0;
				if (pass == 0) addSymbol(linebuf, locptr);
				memmove(linebuf, semi + 1, 1 + strlen(semi+1));
			}
			semi = strstr(linebuf, "org");
			if (semi)
			{
				semi += 3;
				while (*semi == '\t' || *semi == ' ') ++semi;
				if (getNum(semi, &hexv, -1))
				{
					locptr = loadadd = hexv;
				}		
				else
				{
					fclose(fp);
					return alert("Load failed", "Cancel",
						   "Syntax error on line %d: "
						   "ORG not followed by number",
							line);
				}
			}
			semi = strstr(linebuf, "defb");
			if (semi)
			{
				char *token;
				semi += 4;
				for (token = strtok(semi, ","); token;
						token = strtok(NULL, ","))
				{
					if (getNum(token, &hexv, pass))
					{
					    if (pass == 1) 
						newMem.poke(locptr, hexv);
					    ++locptr;
					}
					else return alert("Load failed", 
						   "Cancel",
						   "Syntax error on line %d: "
						   "DEFB is incorrect", line);
					
				}
			}
			semi = strstr(linebuf, "defw");
			if (semi)
			{
				char *token;
				semi += 4;
				for (token = strtok(semi, ","); token;
						token = strtok(NULL, ","))
				{
					if (getNum(token, &hexv, pass))
					{
					    if (pass == 1) 
						newMem.pokeW(locptr, hexv);
					    locptr += 2;
					}
					else return alert("Load failed", 
						   "Cancel",
						   "Syntax error on line %d: "
						   "DEFB is incorrect", line);
					
				}
			}
		} /* END WHILE */
		rewind (fp);
	}	/* END FOR */

	fclose(fp);
	/* Now work out how much space the tune occupies */
	int limit = tuneLimit(tune);
	int base = tuneBase(tune);
	int first = loadadd;
	int last  = loadadd + 5;
	int ch;

	for (ch = 0; ch < 3; ch++)
	{
		int chan = newMem.peekW(loadadd + 2*ch);
		channelRange(&newMem, chan, &first, &last, 0xFF);
	}
	if ((last - first) > (limit - base))
	{
		return alert("Load failed", "Cancel",
			"This tune is too big to load into the chosen slot.");
	}
/* Copy the tune data in */

	for (ch = 0; ch < 3; ch++)
	{
		int achannel = newMem.peekW(base + 2 * ch);

		m_mem->pokeW(base + 2 * ch, achannel - loadadd + base);	
		realign(&newMem, m_mem, achannel - loadadd + base, 
				achannel, 0xFF);
	}
	if (!tune) memcpy(m_mem->memoryAt(0xF52A, 7), m_mem->memoryAt(0x752A), 
			0xAD6);
	return ZXE_CONTINUE;
}





int TuneEdit128Page::onLoad(int tune)
{
	char filename[PATH_MAX];
	JswFileForm ff;
	int rv, n;
	FILE *fp;
	jswByte magic[128], sum;

	filename[0] = 0;
	rv = ff.doModal(filename);
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK)   return ZXE_CONTINUE;

        fp = fopen(filename, "rb");
        if (!fp)
        {
		return alert("Could not open file", "Cancel",
	                     "Failed to open file:\n%s", filename);
	}
        // Work out what we have here
	memset(magic, 0, sizeof(magic));
	fread(magic, 1, sizeof(magic), fp);
	for (n = 0, sum = 0; n < 127; n++) sum += magic[n];
	if (!memcmp(magic, "PLUS3DOS\032", 9) && sum == magic[127])
	{
		return import3dos(tune, filename, magic, fp);
	}
	fclose(fp);
	fp = fopen(filename, "r");
        if (!fp)
        {
		return alert("Could not open file", "Cancel",
	                     "Failed to open file:\n%s", filename);
	}
	return importAsm(tune, filename, fp);
}



int TuneEdit128Page::onSave(int tune)
{
	VideoMenu vm("Export format ", "+3DOS", "Assembly", "Cancel", NULL);

	int rv = vm.doModal();
	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	switch(vm.getSelected())
	{
		case 0: return saveTUN(tune);
		case 1: return saveTXT(tune);
	}
	return ZXE_CONTINUE;
}

int TuneEdit128Page::onTest(int tune)
{
	int r;
        char temp_name[PATH_MAX];
	SpectrumMemory *mt = new SpectrumMemory(m_mem);

	storeTune(m_mem, mt, tune, 0x752A);
	memcpy(mt->memoryAt(0xF52A, 7), mt->memoryAt(0x752A), 0xAD6);

	FILE *fp = openTemp(temp_name, ".z80", "w+b");
        if (!fp)
        {
                r = alert("Test tune", "Cancel",
                      "Couldn't save temporary file:\n%s", temp_name);
		if (r >= ZXE_QUIT) return r;
                return ZXE_CONTINUE;
	}
	mt->saveZ80(fp);
	fclose(fp);
	delete mt;
	return executeZ80(temp_name);
}



int TuneEdit128Page::onButtonSelect(VideoButton *b)
{
	switch(b->getId())
	{
		case 'a': 
		case 'b':
		case 'c': return onTest(b->getId() - 'a');

		case 'd': 
		case 'e':
		case 'f': return onSave(b->getId() - 'd');

		case 'g': 
		case 'h':
		case 'i': return onLoad(b->getId() - 'g');

	}
	return ZXE_CONTINUE;
}
