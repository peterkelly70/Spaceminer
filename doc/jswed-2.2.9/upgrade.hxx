/************************************************************************

    JSWED 2.1.0 - Editor for Jet Set Willy and derivatives

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

extern jswByte jsw_bank_0[];
extern jswByte title[];

// Upgrades 48->128
extern unsigned short patch_1[], patch_2[], patch_3[], patch_4[], patch_5[], 
                patch_6[], patch_7[], patch_8[], patch_9[], patch_10[],
		patch_11[], patch_12[], patch_13[], patch_14[], patch_15[],
		patch_mmap[], patch_dguards[];

// HL3 ... HL8 upgrades
extern unsigned short patch_hl8[];

extern jswByte patch_84d2[], patch_90ca[], 
	       patch_9133[];

// HL9 upgrades: Extra tune, new BASIC loader, etcetera.
extern jswByte patch_tune[], patch_tune2[], patch_tune3[];
extern unsigned short patch_hl9[];
