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


class JswDirCheckbox : public VideoCheckbox
{
public:
	JswDirCheckbox(int x, int y);
	virtual ~JswDirCheckbox();

	void setVertical(int v = 1);
	inline int getVertical() { return m_vertical; }
protected:
	virtual void redrawCtrl(void);

	int m_vertical;
};

class JswDiagCheckbox : public VideoCheckbox
{
public:
        JswDiagCheckbox(int x, int y);
        virtual ~JswDiagCheckbox();

protected:
        virtual void redrawCtrl(void);

};

