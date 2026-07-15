/************************************************************************

    JSWED 2.2.8 - Editor for Jet Set Willy and derivatives

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

#include "jswed.hxx"
#include "hexpage.hxx"

#define ROWS 27
#define BLANK 5	// Hex dump char for characters outside ASCII range

HexEditPage::HexEditPage(SpectrumMemory *m)
{
	m_mem = m;
	m_workMem = new SpectrumMemory(m);
	m_haveTimer = false;
	m_ot = NULL;
	m_base = 0x4000;
	m_bank = 0;
	setTitle("Hex    ");
	m_cx = m_cy = 0;
	m_ocx = m_ocy = -1;
	m_pattern = NULL;
	m_patLen = 0;
	m_ctlQ = false;

	int x, y, w, h;
	x = 0;
	if (m->is128())
		y = VIDEO_H - 16 * CHAR_H + CHAR_SH;
	else	y = VIDEO_H - 8 * CHAR_H + CHAR_SH;
	w = 8 * CHAR_W;
	h = CHAR_H;
	addHotspot(new Hotspot('a', x, y, w, h, 'A' - '@')); y += h;
	addHotspot(new Hotspot('f', x, y, w, h, 'F' - '@')); y += h;
	addHotspot(new Hotspot('w', x, y, w, h, 'W' - '@')); y += h;
	addHotspot(new Hotspot('z', x, y, w, h, 'Z' - '@')); y += h;
	if (m->is128())
	{
		addHotspot(new Hotspot('0', x, y, w, h, 127)); y+=h;
		addHotspot(new Hotspot('1', x, y, w, h, 128)); y+=h;
		addHotspot(new Hotspot('2', x, y, w, h, 129)); y+=h;
		addHotspot(new Hotspot('3', x, y, w, h, 130)); y+=h;
		addHotspot(new Hotspot('4', x, y, w, h, 131)); y+=h;
		addHotspot(new Hotspot('5', x, y, w, h, 132)); y+=h;
		addHotspot(new Hotspot('6', x, y, w, h, 133)); y+=h;
		addHotspot(new Hotspot('7', x, y, w, h, 134)); y+=h;
	}
}


HexEditPage::~HexEditPage()
{
	if (m_haveTimer) setTickListener(m_ot);
	if (m_pattern) delete m_pattern;
	delete m_workMem;
}


void HexEditPage::undo()
{
	m_workMem->copyFrom(m_mem);
	redraw(VPR_RIGHT);
}


int HexEditPage::preContextMenu(void)
{
	if (m_haveTimer)
	{
		setTickListener(m_ot);
		m_ticker = 0;
		m_haveTimer = 0;
	}
	return VideoNotePage::preContextMenu();
}

void HexEditPage::postContextMenu(void)
{
	if (!m_haveTimer)
	{
		m_ot = setTickListener(this);
		m_ticker = 0;
		m_haveTimer = 1;
	}
	return VideoNotePage::postContextMenu();
}


int HexEditPage::onReveal(int rect)
{
	if (!m_haveTimer)
	{
		m_ot = setTickListener(this);
		m_haveTimer = true;
		m_ticker = 0;
	}
	m_workMem->copyFrom(m_mem);
	return VideoNotePage::onReveal(rect);
}

int HexEditPage::onConceal(void)
{
	if (m_haveTimer)
	{
		setTickListener(m_ot);
		m_ticker = 0;
		m_haveTimer = 0;
	}
	return VideoNotePage::onConceal();
}


int HexEditPage::cursorAddress()
{
	int a, base, x;
	base = (m_base & 0xFFF8);

       	do
	{	
		a = base + (8 * m_cy) - 64;
		if (a < 0) ++m_cy;
		if (a > 0xFFF8) --m_cy;
	}
	while (a < 0 || a > 0xFFF8);

	if (m_cx < 16) x = (m_cx / 2);
	else	       x = m_cx - 16;
	return a + x;
}


void HexEditPage::drawCell(int x, int y)
{
	int addr;
	int base = (m_base & 0xFFF8);
	int ink;
	char buf[8];

	if (x < 0 || y < 0) 
	{
		//printf("x=%d y=%d\n", x, y);
		return;
	}
	addr = base + (8 * y) - 64;
	if (addr < 0 || addr > 0xFFF8) return;

	addr += x;

	ink = ZX_BLACK;
	int bank = 0;
	if (addr >= 0xC000) bank = m_bank;
	if (m_mem->peek(addr, bank) != m_workMem->peek(addr, bank))
	{
		ink = ZX_MAGENTA;
	}
	if (addr < 0x4000) ink = ZX_BLUE;
	sprintf(buf, "%02x", m_workMem->peek(addr, bank));
	videoScreen->drawText(m_x + ((2*x)+6)*CHAR_W, m_y + y * CHAR_H, 
			buf, ink, ZX_WHITE);
	sprintf(buf, "%c", m_workMem->peek(addr, bank));
	if (buf[0] < 32 || buf[0] & 0x80) buf[0] = BLANK;
	if (buf[0] == ZXCH_POUND) buf[0] = ZXK_POUND;
	if (buf[0] == ZXCH_COPY)  buf[0] = ZXK_COPY;
	videoScreen->drawText(m_x + m_w - ((8-x)*CHAR_W), m_y + y * CHAR_H, 
			buf[0], ink, ZX_WHITE);
}




void HexEditPage::bottomBar()
{
	char buf[40];
	sprintf(buf, "Hex editor [%04x]", cursorAddress());

	videoScreen->bottomBar(buf);
}

int HexEditPage::redraw(int whichRectangle)
{
	char buf[40];
	int x,y,w,h,n,m;

	if (whichRectangle == VPR_RIGHT)
	{
		int a, base;
		int ink;

		bottomBar();

		base = (m_base & 0xFFF8);
		for (y = 0; y < ROWS; y++)
		{
			a = base + (8*y) - 64;
			if (a < 0 || a > 0xFFF8) 
			{
				videoScreen->fillBox(m_x, m_y + CHAR_H * y, 
						m_w, CHAR_H, ZX_WHITE);
				continue;
			}
			sprintf(buf, "%04x: ", a);
			videoScreen->drawText(m_x, m_y + CHAR_H * y, 
					buf, ZX_BLUE, ZX_WHITE);
			videoScreen->fillBox(m_x + 22 * CHAR_W, 
					m_y + CHAR_H * y, 2*CHAR_W, CHAR_H,
					ZX_WHITE);

			for (x = 0; x < 8; x++)
			{
				drawCell(x, y);
			}
		}
		drawCursor();
		return 0;
	}
	else	// Left
	{
		int rv = VideoNotePage::redraw(whichRectangle);

		x = 0;
		w = 8 * CHAR_W;
		h = CHAR_H;
		if (m_mem->is128()) 
		{
			y = VIDEO_H - 16 * CHAR_H;
			videoScreen->fillBox(x, y, w, 13 * CHAR_H, ZX_BRWHITE);
			videoScreen->box(x, y, w, 13 * CHAR_H, ZX_BLACK);
		}
		else	
		{
			y = VIDEO_H -  8 * CHAR_H;	
			videoScreen->fillBox(x, y, w, 5 * CHAR_H, ZX_BRWHITE);
			videoScreen->box(x, y, w, 5 * CHAR_H, ZX_BLACK);
		}
		videoScreen->drawSmallText(x,y,"Control + abcde ",
				ZX_BRWHITE, ZX_BLACK);
		videoScreen->smallZxLogo(x + 10 * CHAR_SW, y);
		y += CHAR_SH;
		videoScreen->drawText(x+CHAR_SW,y,"Address", ZX_BLACK, ZX_BRWHITE);
		y += CHAR_H;
		videoScreen->drawText(x+CHAR_SW,y,"Find", ZX_BLACK, ZX_BRWHITE);
		y += CHAR_H;
		videoScreen->drawText(x+CHAR_SW,y,"Write", ZX_BLACK, ZX_BRWHITE);
		y += CHAR_H;
		videoScreen->drawText(x+CHAR_SW,y,"Z:Undo", ZX_BLACK, ZX_BRWHITE);
		y += CHAR_H;
		if (m_mem->is128()) for (n = 0; n < 8; n++)
		{
			int paper = ZX_BRWHITE;
			if (m_bank == n) paper = ZX_BRCYAN;
			sprintf(buf, "bank %d", n);
			videoScreen->drawText(x+CHAR_SW,y,buf, ZX_BLACK, paper);
			if (m_bank == n) videoScreen->fillReplace(x,y,w,CHAR_H, ZX_BRWHITE, paper);
			y += CHAR_H;
		}
		bottomBar();
		return rv;
	}
}

int HexEditPage::onHotspot(int id)
{
	int rv;

	if (id >= '0' && id <= '7')
	{
		m_bank = id - '0';
		if (m_base >= (0xBFF8 - (8 * ROWS))) redraw(VPR_RIGHT);
		redraw(VPR_LEFT);
	}
	else switch(id)
	{
		case 'a': preContextMenu();
			  rv = getAddress();
			  postContextMenu();
			  return rv;
		case 'f': preContextMenu();
			  rv = find();
			  postContextMenu();
			  return rv;
		case 'w': m_mem->copyFrom(m_workMem); redraw(VPR_RIGHT); break;
		case 'z': m_workMem->copyFrom(m_mem); redraw(VPR_RIGHT); break;
	}
	return ZXE_CONTINUE;
}


int HexEditPage::onButtonDown(int x, int y, int button)
{
	int rv;

        rv = HotspotManager::onButton(x, y, button);
        if (rv != -1) return rv;

	if (y >= 0 && y < ROWS * CHAR_H)
	{
		if (x >= (m_x + 6 * CHAR_W) && (x < (m_x + 22 * CHAR_W)))
		{
			m_cx = (x - m_x - 6 * CHAR_W) / CHAR_W;
		}
		if (x >= (m_x + m_w - 8 * CHAR_W) && (x < (m_x + m_w)))
		{
			m_cx = ((x - (m_x + m_w - 8 * CHAR_W)) / CHAR_W) + 16;
		}
		m_cy = y / CHAR_H;
		drawCursor();
	}
	return ZXE_CONTINUE;
}


int HexEditPage::onKeyDown(int keysym)
{
	int rv = HotspotManager::onKey(keysym);
	if (rv != -1) return rv;

	switch(keysym)
	{
		case 'G' - '@':
		case ZXK_F3:
			return findAgain();
		case ZXK_TAB:
		case ZXK_BACKTAB:
			if (m_cx >= 16) m_cx = (m_cx - 16) * 2;
			else		m_cx = (m_cx / 2) + 16;
			drawCursor();
			break;

		case ZXK_LEFT:
			if (m_ctlQ)
			{
				m_cx = 0;
				drawCursor();
				break;
			}	// Fall through
		case ZXK_LDEL:
			if (m_cx > 0) 
			{ 
				--m_cx; 
				drawCursor(); 
				break; 
			}
			else m_cx = 23;	// Fall through
		case ZXK_UP:
			if (m_ctlQ) 
			{
				m_cy = 0;
				drawCursor();
			}
			else if (m_cy > 0) 
			{ 
				--m_cy; 
				drawCursor(); 
			}
			else if (m_base > 8) 
			{ 
				m_base -= 8; 
				redraw(VPR_RIGHT); 
			}
			break;
		case ZXK_RIGHT:
			if (m_ctlQ)
			{
				m_cx = 23;
				drawCursor();
				break;
			}
			if (m_cx < 23) 
			{ 
				++m_cx; 
				drawCursor(); 
				break; 
			}
			else m_cx = 0;	// Fall through
		case ZXK_DOWN:
			if (m_ctlQ)
			{
				m_cy = ROWS - 1;
				drawCursor();	
			}
			else if (m_cy < (ROWS-1)) 
			{ 
				++m_cy; 
				drawCursor(); 
			}
			else if (m_base < 0xFFF8) 
			{ 
				m_base += 8; 
				redraw(VPR_RIGHT); 
			}
			break;
		case ZXK_PGDN:
			if (m_ctlQ) m_base = 0xFFF8;
			else	    m_base += (ROWS * 8);
			if (m_base > 0xFFF8) m_base = 0xFFF8;
			redraw(VPR_RIGHT);
			break;
		case ZXK_PGUP:
			if (m_ctlQ) m_base = 0;
			else        m_base -= (ROWS * 8);
			if (m_base < 0) m_base = 0;
			redraw(VPR_RIGHT);
			break;
	}
	if (m_cx < 16)
	{
		int digit = -1;

		if (keysym >= '0' && keysym <= '9') digit = keysym - '0';
		if (keysym >= 'A' && keysym <= 'F') digit = keysym - 'A' + 10;
		if (keysym >= 'a' && keysym <= 'f') digit = keysym - 'a' + 10;
	
		if (digit != -1)
		{
			int a = cursorAddress();
			int bank = 0;
			if (a >= 0xC000) bank = m_bank;
//			printf("cursorAddress=%d\n", a);
			jswByte v = m_workMem->peek(a, bank);
			if (m_cx & 1)
			{
				v = (v & 0xF0) | (digit & 0x0F);
			}
			else
			{
				v = (v & 0x0F) | (digit << 4);
			}
			m_workMem->poke(a, v, bank);
			drawCell(m_cx/2, m_cy);
			++m_cx;
			if (m_cx >= 16) 
			{
				m_cx = 0;
				onKeyDown(ZXK_DOWN);
			}
		}
	}
	if (keysym == ZXK_POUND) keysym = ZXCH_POUND;
	if (keysym == ZXK_COPY)  keysym = ZXCH_COPY;
	if (m_cx >= 16 && keysym >= ' ' && keysym <= 0x7F)
	{
		int a = cursorAddress();
		m_workMem->poke(a, keysym, m_bank);
		drawCell(m_cx - 16, m_cy);
		++m_cx;
		if (m_cx >= 24) 
		{
			m_cx = 16;
			onKeyDown(ZXK_DOWN);
		}
	}
	m_ctlQ = (keysym == ZXK_CTRLQ);

	return VideoNotePage::onKeyDown(keysym);
}


void HexEditPage::drawCursor()
{
	int a, y, x;
	char buf[40];
	int ink, paper;

//	printf("%d %d\n", m_ticker, m_ticker % 10);
	if ((m_ticker % 20) >= 10)  { ink = ZX_BRBLUE;  paper = ZX_BRWHITE; }
	else			    { ink = ZX_BRWHITE; paper = ZX_BRBLUE;  }

	if (m_cx != m_ocx || m_cy != m_ocy)
	{
		if (m_ocx < 16) drawCell(m_ocx / 2,  m_ocy);	
		else		drawCell(m_ocx - 16, m_ocy);	
		sprintf(buf, "Hex editor [%04x]", cursorAddress());
		videoScreen->drawText(0, VIDEO_H - 3 * CHAR_H, buf, 
				ZX_BRWHITE, ZX_BLACK);
	}

	a = cursorAddress() & 0xFFF8;	
	int bank = 0;
	if (a >= 0xC000) bank = m_bank;

	y = CHAR_H * m_cy;
	if (m_cx < 16) 
	{
		x = m_x + (6 + m_cx) * CHAR_W;
		sprintf(buf, "%02x", m_workMem->peek(a + (m_cx/2), bank));
		if (m_cx & 1)
		{
			videoScreen->fillReplace(x-CHAR_W, y, CHAR_H, CHAR_H,
					ZX_WHITE, ZX_BRWHITE);
			videoScreen->drawText(x, y, buf[1], ink, paper);
		}
		else
		{
			videoScreen->drawText(x, y, buf[0], ink, paper);
			videoScreen->fillReplace(x+CHAR_W, y, CHAR_H, CHAR_H,
					ZX_WHITE, ZX_BRWHITE);
		}
	}
	else
	{
		x = m_x + m_w - ((24 - m_cx) * CHAR_W);
		sprintf(buf, "%c", m_workMem->peek(a + (m_cx-16), bank));
		if (buf[0] < 32 || buf[0] & 0x80) buf[0] = BLANK;
		if (buf[0] == ZXCH_POUND) buf[0] = ZXK_POUND;
		if (buf[0] == ZXCH_COPY)  buf[0] = ZXK_COPY;
		videoScreen->drawText(x, y, buf[0], ink, paper);
	}	
	m_ocx = m_cx;
	m_ocy = m_cy;
}


void HexEditPage::onTick(void)
{
	++m_ticker;
	if ((m_ticker % 10) == 0) drawCursor();
}


class HexAddressForm : public VideoForm
{
protected:
	VideoTextEdit *m_edit;
	VideoLabel *m_label;
	VideoButton *m_ok, *m_cancel;
	int m_address;
public: 
	HexAddressForm(int addr) : VideoForm(10 * CHAR_W, 10 * CHAR_H,
			20 * CHAR_W, 6 * CHAR_H)
	{ 
		char buf[8];

		sprintf(buf, "%04x", addr);
		m_address = addr; 
		m_label = new VideoLabel(m_x + CHAR_W, m_y + 3 * CHAR_SH,
				"Address");
		m_edit = new VideoTextEdit(m_x + 9 * CHAR_W, m_y + 3 * CHAR_SH,
			       5 * CHAR_W + CHAR_SW, 4 + CHAR_H);
		m_edit->setLimit(4);
		m_edit->setText(""); // setText(buf);
		m_edit->setHexOnly();
		m_ok = new VideoButton(ZXE_OK, m_x + CHAR_W, m_y + 4 * CHAR_H,
			"  OK  ");
		m_cancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 *CHAR_W, 
				m_y + 4 * CHAR_H, "Cancel");
	}	
	
	~HexAddressForm()
	{
		delete m_label;
		delete m_edit;
		delete m_ok;
		delete m_cancel;
	}

	virtual int doModal()
	{
		int rv = VideoForm::doModal();
		if (rv != ZXE_OK) return rv;
		if (!sscanf(m_edit->getText(), "%x", &m_address))
			return ZXE_CANCEL;
		return ZXE_OK;
	}

	virtual void redraw()
	{
		VideoForm::redraw();
		redrawBorder("Go to");
	}

	inline int getAddress()
	{
		return m_address;
	}
protected:
	virtual void hideChildren()
	{
		removeChild(m_label);
		removeChild(m_edit);
		removeChild(m_ok);
		removeChild(m_cancel);
		VideoForm::hideChildren();
	}
	
	
	virtual void showChildren(int redraw)
	{
		addChild(m_edit);
		addChild(m_label);
		addChild(m_ok);
		addChild(m_cancel);
		VideoForm::showChildren(redraw);
	}
};


int HexEditPage::getAddress()
{
	HexAddressForm af(cursorAddress());

	int rv = af.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	int base = (af.getAddress() & 0xFFF8);
	if (base < 0) base = 0;

	m_base = base;
	base -= 64;
	if (base < 0) base = 0;
	m_cy = (af.getAddress() - base) / 8;
	m_cx = ((af.getAddress() - base) % 8) * 2;
	redraw(VPR_RIGHT);
	return ZXE_OK;
}


class HexFindForm : public VideoForm
{
protected:
	VideoTextEdit *m_edit;
	VideoLabel *m_label;
	VideoButton *m_ok, *m_cancel;
	jswByte *m_pattern;
public: 
	HexFindForm() : VideoForm(5 * CHAR_W, 10 * CHAR_H, 30 * CHAR_W, 
			6 * CHAR_H)
	{ 
		m_label = new VideoLabel(m_x + CHAR_W, m_y + 3 * CHAR_SH,
				"Find");
		m_edit = new VideoTextEdit(m_x + 6 * CHAR_W, m_y + 3 * CHAR_SH,
			       m_w - 7 * CHAR_W, 4 + CHAR_H);
		m_edit->setText("");
		m_edit->setHexOnly();
		m_ok = new VideoButton(ZXE_OK, m_x + CHAR_W, m_y + 4 * CHAR_H,
			"  OK  ");
		m_cancel = new VideoButton(ZXE_CANCEL, m_x + m_w - 7 *CHAR_W, 
				m_y + 4 * CHAR_H, "Cancel");
		m_pattern = NULL;
	}	
	
	~HexFindForm()
	{
		delete m_label;
		delete m_edit;
		delete m_ok;
		delete m_cancel;
		if (m_pattern) delete m_pattern;
	}

	virtual int doModal()
	{
		char buf[3];
		int n;

		int rv = VideoForm::doModal();
		if (rv != ZXE_OK) return rv;

		char *txt = m_edit->getText();
		int l = strlen(txt);
		if (l < 2) return ZXE_CANCEL;
		m_pattern = new jswByte[l];

		for (n = 0; n < l; n += 2)
		{
			int hexv = 0;

			sprintf(buf, "%-2.2s", txt + n);
//			printf("l=%d n=%d buf='%s'\n", l,n,buf); fflush(stdout);
			sscanf(buf, "%x", &hexv);
			m_pattern[n/2] = hexv;	
		}	
		return ZXE_OK;
	}

	virtual void redraw()
	{
		VideoForm::redraw();
		redrawBorder("Find Hex sequence");
	}

	inline jswByte *getPattern()
	{
		return m_pattern;
	}

	inline int getLength()
	{
		return strlen(m_edit->getText()) / 2;
	}

protected:
	virtual void hideChildren()
	{
		removeChild(m_label);
		removeChild(m_edit);
		removeChild(m_ok);
		removeChild(m_cancel);
		VideoForm::hideChildren();
	}
	
	
	virtual void showChildren(int redraw)
	{
		addChild(m_edit);
		addChild(m_label);
		addChild(m_ok);
		addChild(m_cancel);
		VideoForm::showChildren(redraw);
	}
};


int HexEditPage::find()
{
	HexFindForm ff;

	int rv = ff.doModal();

	if (rv >= ZXE_QUIT) return rv;
	if (rv != ZXE_OK) return ZXE_CONTINUE;

	int patLen = ff.getLength();
	jswByte *pattern = ff.getPattern();

	if (m_pattern) delete m_pattern;
	m_pattern = new jswByte[patLen];
	memcpy(m_pattern, pattern, patLen);
	m_patLen = patLen;
	return findAgain();
}

int HexEditPage::findAgain()
{
	if (!m_pattern || !m_patLen) return ZXE_CONTINUE;

	int n, bank, found;
	found = 0;
	for (n = cursorAddress()+1; n <= (0xFFFF - m_patLen); n++)
	{
		if (n < 0xC000) bank = 0;
		else		bank = m_bank;

		if (!memcmp(m_workMem->memoryAt(n, bank), m_pattern, m_patLen))
		{
			found = 1;
			break;
		}
	}
	if (!found) return alert("Pattern not found", "OK", "The specified "
			"search pattern was not found.");
	int base = (n & 0xFFF8);
	if (base < 0) base = 0;

	m_base = base;
	base -= 64;
	if (base < 0) base = 0;
	m_cy = (n - base) / 8;
	m_cx = ((n - base) % 8) * 2;
	redraw(VPR_RIGHT);
	return ZXE_CONTINUE;
}


