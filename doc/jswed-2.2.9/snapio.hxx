/************************************************************************

    JSWED 2.1.2 - Editor for Jet Set Willy and derivatives

    Copyright (C) 2000-2001,2004  John Elliott <jce@seasip.demon.co.uk>

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

#ifndef SNAPIO_H_INCLUDED
#define SNAPIO_H_INCLUDED 1

//
// This class models the Spectrum memory. It can load and save in SNA,
// TAP and +3DOS binary formats.
//
class TapeHeader
{
public:
	int read(FILE *fp);
	int readData(FILE *fp, jswByte *buf);
	inline int getType()	  { return m_data[0]; }
	inline int getSize()      { return m_data[11] + 256 * m_data[12]; }
	inline int getAddress()   { return m_data[13] + 256 * m_data[14]; }
	inline char *getName()    { return (char *)(m_data + 1); }
	void showMe(int y);
private:
	jswByte m_data[17];
};

class SpectrumMemory
{
public:
	SpectrumMemory();
	SpectrumMemory(char *path, FILE *fp);
	~SpectrumMemory();
	SpectrumMemory(SpectrumMemory *mem, int is128 = -1);

	int fromDisc(char *path, FILE *fp);
	int toDisc(char *path, FILE *fp);
	char m_type;
	
	inline int is128() { return m_is128; }

	void copyFrom(SpectrumMemory *mem);
	jswByte *memoryAt(int address, int bank = 0);
	jswByte peek(int address, int bank = 0);
	void poke(int address, int value, int bank = 0);
	void pokeSnaHeader(int address, int value);
	jswByte peekSnaHeader(int address);

	inline unsigned peekW(int address, int bank = 0)
	{
		return ((unsigned)peek(address+1,bank)) << 8 |
			peek(address, bank);
	}
	inline void pokeW(int address, int value, int bank = 0)
	{
		poke(address,   value & 0xFF);
		poke(address+1, value >> 8);
	}

	// loadHexPatch returns NULL if OK, error message if not.
	char *loadHexPatch(char *path);

	int saveZ80(FILE *fp);
protected:
	int loadPlus3(char *path, FILE *fp, jswByte *hdr);
	int loadTap(FILE *fp);
	int loadSna(FILE *fp);
	int loadSna128(FILE *fp);
	int loadZ80(FILE *fp);
	int loadZ80Bank(int z80Id, int zxAddr, int zxBank, FILE *fp, int z80base);
	int z80Seek(int bank,FILE *fp, int z80base);

	int saveSna(FILE *fp);
	int saveTap(FILE *fp);
	int savePlus3(char *path, FILE *fp);

	jswByte m_ram48 [49152];
	jswByte m_ram1  [16384], 
	        m_ram3  [16384], 
		m_ram4  [16384],
		m_ram6  [16384],
                m_ram7  [16384];

	int m_is128;
	jswByte m_snahdr[27];

private:
	void zeroise(void);
};

#endif // ndef SNAPIO_H_INCLUDED
