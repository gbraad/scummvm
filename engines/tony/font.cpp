/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Font.CPP.............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/mpal/stubs.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       Metodi di RMFont
\****************************************************************************/

RMFont::RMFont() {
	m_letter = NULL;
}

RMFont::~RMFont() {
	Unload();
}

/****************************************************************************\
*
* Function:     void RMFont::Load(byte *buf, int nChars, int dimx, int dimy);
*
* Description:  Carica un font da buffer
*
* Input:        byte *buf              Buffer contenente il font
*               int nChars              Numero di caratteri (max 256)
*               int dimx,dimy           Dimensione in pixel di un carattere
*
\****************************************************************************/

void DumpFontBMP(const char *filename, const byte *buf, int nChars, int charX, int charY, byte *pal) {
	error("DumpFontBMP not supported in ScummVM");
}


void RMFont::Load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	m_letter = new RMGfxSourceBuffer8RLEByte[nChars];

#if 0
	if (nChars == 112 && palResID == RES_F_PAL) {
		// Font parla
		DumpFontBMP("font_parla.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	}
	else if (nChars == 102 && palResID == RES_F_PAL) {
		// Font macc
		DumpFontBMP("font_macc.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 85 && palResID == RES_F_PAL) {
		// Font obj
		DumpFontBMP("font_obj.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 112 && palResID == RES_F_CPAL) {
		// Font credits
		DumpFontBMP("font_credits.bmp", buf, nChars, dimx, dimy, RMRes(palResID));		
	}
#endif

	// Carichiamoce 'sto font
	for (int i = 0; i < nChars; i++) {
		// Inizializza il buffer con le lettere
		m_letter[i].Init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		m_letter[i].LoadPaletteWA(palResID);
	}

	m_fontDimx = dimx;
	m_fontDimy = dimy;

	nLetters=nChars;
}

void RMFont::Load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.Size() < nChars * (dimy * dimx + 8))
		nChars = res.Size() / (dimy * dimx + 8);

	Load(res, nChars, dimx, dimy, palResID);
}

void RMFont::Unload(void) {
	if (m_letter != NULL) {
		delete[] m_letter;
		m_letter = NULL;
	}
}


RMGfxPrimitive *RMFont::MakeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;
	int nLett;

	// Converte da carattere a lettera
	nLett = ConvertToLetter(bChar);

	// Crea la primitiva per il font
	prim = new RMFontPrimitive(this);
	prim->m_nChar = nLett;

	// Si fa' dare la lunghezza della lettera in pixel
	nLength = LetterLength(bChar);

	return prim;
}

void RMFont::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
  RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

  // Richiama la Draw della lettera assegnata alla primitiva
  if (prim->m_nChar != -1)
	  m_letter[prim->m_nChar].Draw(bigBuf,prim);
}

void RMFont::Close(void) {
	Unload();
}

int RMFont::StringLen(RMString text) {
	int len, i;

	len = 0;
	for (i = 0; i < text.Length() - 1; i++)
		len += LetterLength(text[i], text[i + 1]);
	len += LetterLength(text[i]);

	return len;
}

int RMFont::StringLen(char bChar, char bNext) {
	return LetterLength(bChar, bNext);
}

/****************************************************************************\
*       Metodi di RMFontColor
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
 m_r = m_g = m_b = 255;
}

RMFontColor::~RMFontColor() {

}

void RMFontColor::SetBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	int i;
	byte pal[768*3];

	// Controlla se siamo gia' sul colore giusto
	if (m_r == r1 && m_g == g1 && m_b == b1)
		return;

	m_r = r1;
	m_g = g1;
	m_b = b1;

	// Costruisce la nuova palette per il font
	for (i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;	
		pal[i * 3 + 1] = g >> 16;	
		pal[i * 3 + 2] = b >> 16;	

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15*3 + 0] += 8;
	pal[15*3 + 1] += 8;
	pal[15*3 + 2] += 8;

	// La mette in tutte le lettere
	for (i = 0; i < nLetters; i++)
		m_letter[i].LoadPaletteWA(pal);
}


/***************************************************************************\
*       Metodi di RMFontParla
\****************************************************************************/

void RMFontParla::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  5;	// deutsch

	Load(RES_F_PARL, nchars, 20, 20);

	// Inizializziamo le tabelline del cazzo
	lDefault = 13;
	hDefault = 18;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 52;

	cTable[';'] = 62;
	cTable[','] = 63;
	cTable['.'] = 64;
	cTable[':'] = 65;
	cTable['-'] = 66;
	cTable['_'] = 67;
	cTable['+'] = 68;
	cTable['<'] = 69;
	cTable['>'] = 70;
	cTable['!'] = 71;
	//cTable['!'] = 72;  Esclamativo alla rovescia
	cTable['?'] = 73;
	//cTable['?'] = 74;  Interrogativo alla rovescia
	cTable['('] = 75;
	cTable[')'] = 76;
	cTable['\"'] = 77;
	cTable['^'] = 77;
	cTable['/'] = 78;
	cTable['�'] = 79;
	cTable['$'] = 80;
	cTable['%'] = 81;
	cTable['&'] = 82;
	cTable['='] = 83;
	cTable['�'] = 84;
	cTable['�'] = 85;
	cTable['�'] = 86;
	cTable['�'] = 87;
	cTable['�'] = 88;
	cTable['�'] = 89;
	cTable['�'] = 89;
	cTable['�'] = 90;
	cTable['�'] = 91;
	cTable['�'] = 92;
	cTable['�'] = 93;
	cTable['�'] = 94;
	cTable['�'] = 95;
	cTable['�'] = 96;
	cTable['�'] = 97;
	cTable['�'] = 98;
	cTable['�'] = 99;
	//cTable[' '] = 100;  e cerchietto
	//cTable[' '] = 101;  i cerchietto
	//cTable[' '] = 102;  o cerchietto
	//cTable[' '] = 103;  u cerchietto
	cTable['�'] = 104;
	cTable['�'] = 105;
	cTable['�'] = 106;
	cTable['�'] = 107;
	cTable['�'] = 108;
	cTable['�'] = 109;
	//cTable['�'] = 110;  integrale 
	cTable['\''] = 111; 

	// Un po' di lunghezze
	lTable[' '] = 9;
	lTable['\''] = 5;
	lTable['.'] = 5;
	lTable[','] = 5;
	lTable[':'] = 5;
	lTable[';'] = 5;
	lTable['!'] = 5;
	lTable['?'] = 10;
	lTable['\"'] = 5;
	lTable['^'] = 5;
	lTable['('] = 7;
	lTable[')'] = 7;

	lTable['4'] = 10;

	lTable['a'] = 14;
	lTable['b'] = 15;
	lTable['c'] = 12;
	lTable['e'] = 12;
	lTable['i'] = 6;
	lTable['�'] = 6;
	lTable['l'] = 5;
	lTable['m'] = 16;
	lTable['n'] = 12;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['s'] = 12;
	lTable['u'] = 12;

	lTable['E'] = 10;
	lTable['F'] = 11;


	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//ス謎戊３剔囀������
	
	cTable['�'] = 112;
	cTable['�'] = 113;
	cTable['�'] = 114;
	cTable['�'] = 115;
	cTable['�'] = 116;
	cTable['�'] = 117;
	cTable['�'] = 118;
	cTable['�'] = 119;
	cTable['�'] = 120;
	cTable['�'] = 121;
	cTable['�'] = 122;
	cTable['�'] = 123;
	cTable['�'] = 124;
	cTable['�'] = 125;
	cTable['�'] = 126;
	cTable['�'] = 127;
	cTable['�'] = 128;
	cTable['�'] = 129;

	lTable['�'] = 14;
	lTable['�'] = 16;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 14;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 14;
	lTable['�'] = 13;

#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['�'] = 130;
	cTable['�'] = 131;
	cTable['�'] = 132;
	cTable['�'] = 133;
	cTable['�'] = 134;
	cTable['�'] = 135;
	cTable['�'] = 136;
	cTable['�'] = 137;
	cTable['�'] = 138;
	cTable['�'] = 139;
	cTable['�'] = 140;
	cTable['�'] = 141;
	cTable['�'] = 142;
	cTable['�'] = 143;
	cTable['�'] = 144;
	cTable['�'] = 145;
	cTable['�'] = 146;
	cTable['�'] = 147;
	cTable['�'] = 148;
	cTable['�'] = 149;
	cTable['�'] = 150;
	cTable['�'] = 151;
	cTable['�'] = 152;
	cTable['�'] = 153;
	cTable['�'] = 154;
	cTable['�'] = 155;
	cTable['�'] = 156;
	cTable['�'] = 157;
	cTable['�'] = 158;
	cTable['�'] = 159;
	cTable['�'] = 160;
	cTable['�'] = 161;
	cTable['�'] = 162;

	cTable['�'] = 163;
	cTable['�'] = 164;
	cTable['�'] = 165;
	cTable['�'] = 166;
	cTable['�'] = 167;
	cTable['�'] = 168;
	cTable['�'] = 169;
	cTable['�'] = 170;
	cTable['�'] = 171;
	cTable['�'] = 172;
	cTable['�'] = 173;
	cTable['�'] = 174;
	cTable['�'] = 175;
	cTable['�'] = 176;
	cTable['�'] = 177;
	cTable['�'] = 178;
	cTable['�'] = 179;
	cTable['�'] = 180;
	cTable['�'] = 181;
	cTable['�'] = 182;
	cTable['�'] = 183;
	cTable['�'] = 184;
	cTable['�'] = 185;
	cTable['�'] = 186;
	cTable['�'] = 187;
	cTable['�'] = 188;
	cTable['�'] = 189;
	cTable['�'] = 190;
	cTable['�'] = 191;
	cTable['�'] = 192;
	cTable['�'] = 193;
	cTable['�'] = 194;
	cTable['�'] = 195;

	lTable['�'] = 13;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 11;
	lTable['�'] = 15;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 14;
	lTable['�'] = 14;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 18;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 17;
	lTable['�'] = 18;
	lTable['�'] = 16;
	lTable['�'] = 18;	
	lTable['�'] = 19;
	lTable['�'] = 11;
	lTable['�'] = 16;
	lTable['�'] = 14;

	lTable['�'] = 14;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 14;
	lTable['�'] =  8;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 16;
	lTable['�'] = 14;
	lTable['�'] = 16;
	lTable['�'] = 16;
	lTable['�'] =  9;
	lTable['�'] = 15;
	lTable['�'] = 14;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable['�'] = 196;
	cTable['�'] = 197;
	cTable['�'] = 198;
	cTable['�'] = 199;
	cTable['�'] = 200;
	cTable['�'] = 201;
	cTable['�'] = 202;
	cTable['�'] = 203;
	cTable['�'] = 204;
	cTable['�'] = 205;
	cTable['�'] = 206;
	cTable['�'] = 207;
	cTable['�'] = 208;
	cTable['�'] = 209;
	cTable['�'] = 210;

	cTable['�'] = 211;
	cTable['�'] = 212;
	cTable['�'] = 213;
	cTable['�'] = 214;
	cTable['�'] = 215;
	cTable['�'] = 216;
	cTable['�'] = 217;
	cTable['�'] = 218;
	cTable['�'] = 219;
	cTable['�'] = 220;
	cTable['�'] = 221;
	cTable['�'] = 222;
	cTable['�'] = 223;
	cTable['�'] = 224;
	cTable['�'] = 225;

	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 14;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 15;
	lTable['�'] = 19;
	lTable['�'] = 10;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 13;

	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 15;
	lTable['�'] = 7;
	lTable['�'] = 12;
	lTable['�'] = 17;
	lTable['�'] = 16;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 13;

#endif // FONT_CZECH

#ifdef FONT_FRENCH
	cTable['�'] = 226;
	cTable['�'] = 227;
	cTable['�'] = 228;
	cTable['�'] = 229;
	cTable['�'] = 230;
	cTable['�'] = 231;
	cTable['�'] = 232;
	cTable['�'] = 233;

	lTable['�'] = 15;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] =  9;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 11;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['�'] = 234;
	// 'SS' = 235
	cTable['�'] = 236;
	cTable['�'] = 237;
	cTable['�'] = 238;

	lTable['�'] = 15;

#endif // FONT_GERMAN
}


/***************************************************************************\
*       Metodi di RMFontMacc
\****************************************************************************/

void RMFontMacc::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 102	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// francais
		+  5;	// deutsch


	Load(RES_F_MACC, nchars, 11, 16);

	// Default
	lDefault = 10;
	hDefault = 17;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A'+i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a'+i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0'+i] = i + 52;

	cTable['!'] = 62;
	//cTable['!'] = 63;			// ! rovescia
	cTable['\"'] = 64;		
	cTable['$'] = 65;		
	cTable['%'] = 66;		
	cTable['&'] = 67;		
	cTable['/'] = 68;		
	cTable['('] = 69;		
	cTable[')'] = 70;		
	cTable['='] = 71;		
	cTable['?'] = 72;		
	//cTable['?'] = 73;		   // ? rovescia
	cTable['*'] = 74;		
	cTable['+'] = 75;		
	cTable['�'] = 76;		
	cTable[';'] = 77;		
	cTable[','] = 78;		
	cTable['.'] = 79;
	cTable[':'] = 80;		
	cTable['-'] = 81;		
	cTable['<'] = 82;
	cTable[' > '] = 83;
	cTable['/'] = 84;
	cTable['�'] = 85;
	cTable['�'] = 86;
	cTable['�'] = 87;
	cTable['�'] = 88;
	cTable['�'] = 89;
	cTable['�'] = 90;
	//cTable[''] = 91;		  // e col pallino
	cTable['�'] = 92;		
	cTable['�'] = 93;		
	//cTable[''] = 94;			// i col pallino
	cTable['�'] = 95;		
	cTable['�'] = 96;		
	//cTable[''] = 97;		  // o col pallino
	cTable['�'] = 98;		
	cTable['�'] = 99;		
	//cTable[''] = 100;		  // u col pallino
	cTable['�'] = 101;		

	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//ス謎戊３剔囀������
	
	cTable['�'] = 102;
	cTable['�'] = 103;
	cTable['�'] = 104;
	cTable['�'] = 105;
	cTable['�'] = 106;
	cTable['�'] = 107;
	cTable['�'] = 108;
	cTable['�'] = 109;
	cTable['�'] = 110;
	cTable['�'] = 111;
	cTable['�'] = 112;
	cTable['�'] = 113;
	cTable['�'] = 114;
	cTable['�'] = 115;
	cTable['�'] = 116;
	cTable['�'] = 117;
	cTable['�'] = 118;
	cTable['�'] = 119;
	
	lTable['�'] = 14;
	lTable['�'] = 16;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 14;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 14;
	lTable['�'] = 13;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['�'] = 120;
	cTable['�'] = 121;
	cTable['�'] = 122;
	cTable['�'] = 123;
	cTable['�'] = 124;
	cTable['�'] = 125;
	cTable['�'] = 126;
	cTable['�'] = 127;
	cTable['�'] = 128;
	cTable['�'] = 129;
	cTable['�'] = 130;
	cTable['�'] = 131;
	cTable['�'] = 132;
	cTable['�'] = 133;
	cTable['�'] = 134;
	cTable['�'] = 135;
	cTable['�'] = 136;
	cTable['�'] = 137;
	cTable['�'] = 138;
	cTable['�'] = 139;
	cTable['�'] = 140;
	cTable['�'] = 141;
	cTable['�'] = 142;
	cTable['�'] = 143;
	cTable['�'] = 144;
	cTable['�'] = 145;
	cTable['�'] = 146;
	cTable['�'] = 147;
	cTable['�'] = 148;
	cTable['�'] = 149;
	cTable['�'] = 150;
	cTable['�'] = 151;
	cTable['�'] = 152;

	cTable['�'] = 153;
	cTable['�'] = 154;
	cTable['�'] = 155;
	cTable['�'] = 156;
	cTable['�'] = 157;
	cTable['�'] = 158;
	cTable['�'] = 159;
	cTable['�'] = 160;
	cTable['�'] = 161;
	cTable['�'] = 162;
	cTable['�'] = 163;
	cTable['�'] = 164;
	cTable['�'] = 165;
	cTable['�'] = 166;
	cTable['�'] = 167;
	cTable['�'] = 168;
	cTable['�'] = 169;
	cTable['�'] = 170;
	cTable['�'] = 171;
	cTable['�'] = 172;
	cTable['�'] = 173;
	cTable['�'] = 174;
	cTable['�'] = 175;
	cTable['�'] = 176;
	cTable['�'] = 177;
	cTable['�'] = 178;
	cTable['�'] = 179;
	cTable['�'] = 180;
	cTable['�'] = 181;
	cTable['�'] = 182;
	cTable['�'] = 183;
	cTable['�'] = 184;
	cTable['�'] = 185;

	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 8;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;

	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] =  9;
	lTable['�'] = 11;
	lTable['�'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	
	cTable['�'] = 186;
	cTable['�'] = 187;
	cTable['�'] = 188;
	cTable['�'] = 189;
	cTable['�'] = 190;
	cTable['�'] = 191;
	cTable['�'] = 192;
	cTable['�'] = 193;
	cTable['�'] = 194;
	cTable['�'] = 195;
	cTable['�'] = 196;
	cTable['�'] = 197;
	cTable['�'] = 198;
	cTable['�'] = 199;
	cTable['�'] = 200;

	cTable['�'] = 201;
	cTable['�'] = 202;
	cTable['�'] = 203;
	cTable['�'] = 204;
	cTable['�'] = 205;
	cTable['�'] = 206;
	cTable['�'] = 207;
	cTable['�'] = 208;
	cTable['�'] = 209;
	cTable['�'] = 210;
	cTable['�'] = 211;
	cTable['�'] = 212;
	cTable['�'] = 213;
	cTable['�'] = 214;
	cTable['�'] = 215;

	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;

	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 9;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable['�'] = 226;
	cTable['�'] = 227;
	cTable['�'] = 228;
	cTable['�'] = 229;
	cTable['�'] = 230;
	cTable['�'] = 231;
	cTable['�'] = 232;
	cTable['�'] = 233;

	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 8;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['�'] = 234;
	// 'SS' = 235
	cTable['�'] = 236;
	cTable['�'] = 237;
	cTable['�'] = 238;

	lTable['�'] = 11;
#endif // FONT_GERMAN
}

/***************************************************************************\
*       Metodi di RMFontCredits
\****************************************************************************/

void RMFontCredits::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  2;	// deutsch


	Load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	lDefault=10;
	hDefault=28;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;



	cTable['�'] = 52;
	cTable['�'] = 53;
//	cTable[''] = 54; // a ^
//	cTable[''] = 55; // a pallini
	cTable['�'] = 56;
	cTable['�'] = 57; 
//	cTable[''] = 58; // e ^
//	cTable[''] = 59; // e pallini
	cTable['�'] = 60;
	cTable['�'] = 61;
//	cTable[''] = 62; // i ^
//	cTable[''] = 63; // i pallini
	cTable['�'] = 64;
	cTable['�'] = 65;
//	cTable[''] = 66; // o ^
//	cTable[''] = 67; // o pallini
	cTable['�'] = 68;
	cTable['�'] = 69;
//	cTable[''] = 70; // u ^
//	cTable[''] = 71; // u pallini
//	cTable[''] = 72; // y pallini
	cTable['�'] = 73;
	cTable['�'] = 74; 
//	cTable[''] = 75; // o barrato
//	cTable[''] = 76; // ac
	cTable['�'] = 77;
//	cTable[''] = 78; // ? rovesciato
	cTable['?'] = 79; 
//	cTable[''] = 80; // ! rovesciato
	cTable['!'] = 81; 
//	cTable[''] = 82; // 1/2
//	cTable[''] = 83; // 1/4
	cTable['('] = 84;
	cTable[')'] = 85;
	cTable['�'] = 86;
	cTable['�'] = 87;
//	cTable[''] = 88; // AE
	cTable[':'] = 89;
	cTable['%'] = 90;
	cTable['&'] = 91;
	cTable['/'] = 92;
	cTable['+'] = 93;
	cTable[';'] = 94;
	cTable[','] = 95;
	cTable['^'] = 96;
	cTable['='] = 97;
	cTable['_'] = 98; 
	cTable['*'] = 99; 
	cTable['.'] = 100;  

	for (i = 0; i < 10; i++)
		cTable['0'+i] = i+101;
	cTable['\''] = 111;  

	lTable[' '] = 11;
	lTable['�'] = lTable['A'] = 19;
	lTable['B'] = 15;
	lTable['C'] = 14;
	lTable['D'] = 13;
	lTable['E'] = 14;
	lTable['F'] = 13;
	lTable['G'] = 16;
	lTable['H'] = 15;
	lTable['I'] = 5;
	lTable['J'] = 8;
	lTable['K'] = 15;
	lTable['L'] = 13;
	lTable['M'] = 17;
	lTable['N'] = 15;
	lTable['�'] = lTable['O'] = 14;
	lTable['P'] = 12;
	lTable['Q'] = 14;
	lTable['R'] = 14;
	lTable['S'] = 15;
	lTable['T'] = 11;
	lTable['�'] = lTable['U'] = 12;
	lTable['V'] = 12;
	lTable['W'] = 16;
	lTable['X'] = 12;
	lTable['Y'] = 13;
	lTable['Z'] = 14;

	lTable['a'] = 11;
	lTable['b'] = 9;
	lTable['c'] = 9;
	lTable['d'] = 10;
	lTable['e'] = 9;
	lTable['f'] = 8;
	lTable['g'] = 9;
	lTable['h'] = 10;
	lTable['i'] = 5;
	lTable['j'] = 6;
	lTable['k'] = 12;
	lTable['l'] = 6;
	lTable['m'] = 14;
	lTable['n'] = 10;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['q'] = 9;
	lTable['r'] = 9;
	lTable['s'] = 9;
	lTable['t'] = 6;
	lTable['u'] = 9;
	lTable['v'] = 10;
	lTable['w'] = 14;
	lTable['x'] = 9;
	lTable['y'] = 10;
	lTable['z'] = 9;

	lTable['0'] = 12;
	lTable['1'] = 8;
	lTable['2'] = 10;
	lTable['3'] = 11;
	lTable['4'] = 12;
	lTable['5'] = 11;
	lTable['6'] = 12;
	lTable['7'] = 10;
	lTable['8'] = 11;
	lTable['9'] = 10;

	lTable['/'] = 10;
	lTable['^'] = 9;
	lTable[','] = 5;
	lTable['.'] = 5;
	lTable[';'] = 5;
	lTable[':'] = 5;
	lTable['\''] = 5;



	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//ス謎戊３剔囀������
	
	cTable['�'] = 112;
	cTable['�'] = 113;
	cTable['�'] = 114;
	cTable['�'] = 115;
	cTable['�'] = 116;
	cTable['�'] = 117;
	cTable['�'] = 118;
	cTable['�'] = 119;
	cTable['�'] = 120;
	cTable['�'] = 121;
	cTable['�'] = 122;
	cTable['�'] = 123;
	cTable['�'] = 124;
	cTable['�'] = 125;
	cTable['�'] = 126;
	cTable['�'] = 127;
	cTable['�'] = 128;
	cTable['�'] = 129;
	
	lTable['�'] = 20;
	lTable['�'] = 12;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 14;
	lTable['�'] = 11;
	lTable['�'] = 16;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 11;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 10;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['�'] = 130;
	cTable['�'] = 131;
	cTable['�'] = 132;
	cTable['�'] = 133;
	cTable['�'] = 134;
	cTable['�'] = 135;
	cTable['�'] = 136;
	cTable['�'] = 137;
	cTable['�'] = 138;
	cTable['�'] = 139;
	cTable['�'] = 140;
	cTable['�'] = 141;
	cTable['�'] = 142;
	cTable['�'] = 143;
	cTable['�'] = 144;
	cTable['�'] = 145;
	cTable['�'] = 146;
	cTable['�'] = 147;
	cTable['�'] = 148;
	cTable['�'] = 149;
	cTable['�'] = 150;
	cTable['�'] = 151;
	cTable['�'] = 152;
	cTable['�'] = 153;
	cTable['�'] = 154;
	cTable['�'] = 155;
	cTable['�'] = 156;
	cTable['�'] = 157;
	cTable['�'] = 158;
	cTable['�'] = 159;
	cTable['�'] = 160;
	cTable['�'] = 161;
	cTable['�'] = 162;

	cTable['�'] = 163;
	cTable['�'] = 164;
	cTable['�'] = 165;
	cTable['�'] = 166;
	cTable['�'] = 167;
	cTable['�'] = 168;
	cTable['�'] = 169;
	cTable['�'] = 170;
	cTable['�'] = 171;
	cTable['�'] = 172;
	cTable['�'] = 173;
	cTable['�'] = 174;
	cTable['�'] = 175;
	cTable['�'] = 176;
	cTable['�'] = 177;
	cTable['�'] = 178;
	cTable['�'] = 179;
	cTable['�'] = 180;
	cTable['�'] = 181;
	cTable['�'] = 182;
	cTable['�'] = 183;
	cTable['�'] = 184;
	cTable['�'] = 185;
	cTable['�'] = 186;
	cTable['�'] = 187;
	cTable['�'] = 188;
	cTable['�'] = 189;
	cTable['�'] = 190;
	cTable['�'] = 191;
	cTable['�'] = 192;
	cTable['�'] = 193;
	cTable['�'] = 194;
	cTable['�'] = 195;

	lTable['�'] = 20;
	lTable['�'] = 16;
	lTable['�'] = 16;
	lTable['�'] = 14;
	lTable['�'] = 22;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 20;
	lTable['�'] = 12;
	lTable['�'] = 16;
	lTable['�'] = 16;
	lTable['�'] = 16;
	lTable['�'] = 22;
	lTable['�'] = 18;
	lTable['�'] = 16;
	lTable['�'] = 15;
	lTable['�'] = 14;
	lTable['�'] = 13;
	lTable['�'] = 15;
	lTable['�'] = 12;
	lTable['�'] = 14;
	lTable['�'] = 15;
	lTable['�'] = 13;
	lTable['�'] = 16;
	lTable['�'] = 14;
	lTable['�'] = 23;
	lTable['�'] = 23;
	lTable['�'] = 10;
	lTable['�'] = 12;	
	lTable['�'] = 16;
	lTable['�'] = 12;
	lTable['�'] = 20;
	lTable['�'] = 15;

	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 16;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 13;
	lTable['�'] = 12;
	lTable['�'] = 13;
	lTable['�'] = 14;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 10;
	lTable['�'] = 12;
	lTable['�'] = 16;
	lTable['�'] = 11;
	lTable['�'] = 13;
	lTable['�'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable['�'] = 196;
	cTable['�'] = 197;
	cTable['�'] = 198;
	cTable['�'] = 199;
	cTable['�'] = 200;
	cTable['�'] = 201;
	cTable['�'] = 202;
	cTable['�'] = 203;
	cTable['�'] = 204;
	cTable['�'] = 205;
	cTable['�'] = 206;
	cTable['�'] = 207;
	cTable['�'] = 208;
	cTable['�'] = 209;
	cTable['�'] = 210;

	cTable['�'] = 211;
	cTable['�'] = 212;
	cTable['�'] = 213;
	cTable['�'] = 214;
	cTable['�'] = 215;
	cTable['�'] = 216;
	cTable['�'] = 217;
	cTable['�'] = 218;
	cTable['�'] = 219;
	cTable['�'] = 220;
	cTable['�'] = 221;
	cTable['�'] = 222;
	cTable['�'] = 223;
	cTable['�'] = 224;
	cTable['�'] = 225;

	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 15;
	lTable['�'] = 14;
	lTable['�'] = 20;
	lTable['�'] = 7;
	lTable['�'] = 15;
	lTable['�'] = 20;
	lTable['�'] = 19;
	lTable['�'] = 16;
	lTable['�'] = 15;
	lTable['�'] = 13;
	lTable['�'] = 13;

	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 12;
	lTable['�'] = 6;
	lTable['�'] = 10;
	lTable['�'] = 15;
	lTable['�'] = 12;
	lTable['�'] = 11;
	lTable['�'] = 11;
	lTable['�'] = 10;
	lTable['�'] = 10;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable['�'] = 226;
	cTable['�'] = 227;
	cTable['�'] = 228;
	cTable['�'] = 229;
	cTable['�'] = 230;
	cTable['�'] = 231;
	cTable['�'] = 232;
	cTable['�'] = 233;

	lTable['�'] = 12;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 6;
	lTable['�'] = 10;
	lTable['�'] = 10;
	lTable['�'] = 11;
	lTable['�'] = 11;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['�'] = 234;
	// 'SS' = 235

	// old chars overrides
	cTable['�'] = cTable['�'] = 55;
	cTable['�'] = cTable['�'] = 67;
	cTable['�'] = cTable['�'] = 71;

	lTable['�'] = 11;

#endif // FONT_GERMAN
}



/***************************************************************************\
*       Metodi di RMFontObj
\****************************************************************************/

#define TOUPPER(a)	((a) >='a'&&(a)<='z'?(a)+'A'-'a':(a))
#define TOLOWER(a)	((a) >='A'&&(a)<='Z'?(a)+'a'-'A':(a))

void RMFontObj::SetBothCase(int nChar, int nNext, signed char spiazz) {
	l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}


void RMFontObj::Init(void) {
	int i;

	//bernie: numero di caratteri nel font (solo maiuscolo)
	int nchars =
		  85	// base
		+  9	// polish
		+ 33	// russian
		+ 15	// czech
		+  0	// francais (no uppercase chars)
		+  1;	// deutsch


	Load(RES_F_OBJ, nchars, 25, 30);

	// Inizializziamo le tabelline del cazzo
	lDefault = 26;
	hDefault = 30;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++) {
		cTable['A' + i] = i+0;
		cTable['a' + i] = i+0;
	}

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 26;

	cTable[','] = 36;
	cTable[';'] = 37;
	cTable['.'] = 38;
	cTable[':'] = 39;
	cTable['-'] = 40;
	cTable['+'] = 41;
	cTable['!'] = 42;
	// cTable['!'] = 43; Esclamativo alla rovescia
	cTable['?'] = 44;
	//cTable['?'] = 45;  Interrogativo alla rovescia
	cTable['/'] = 46;
	cTable['('] = 47;
	cTable[')'] = 48;
	cTable['='] = 49;
	cTable['\''] = 50;
	cTable['\"'] = 51;
	cTable['�'] = 52;
	cTable['$'] = 53;
	cTable['%'] = 54;
	cTable['&'] = 55;
	cTable['^'] = 56;
	cTable['*'] = 57;
	cTable['<'] = 58;
	cTable[' > '] = 59;
	cTable['�'] = 60;
	cTable['�'] = 61;
	cTable['�'] = 62;
	cTable['�'] = 63;
	//cTable['�'] = 64;   integrale
	cTable['�'] = 65;
	cTable['�'] = 66;
	cTable['�'] = 67;
	cTable['�'] = 68;
	cTable['�'] = 69;
	cTable['�'] = cTable['�'] = 70;
	cTable['�'] = 71;
	cTable['�'] = 72;
	cTable['�'] = 73;
	//cTable[' '] = 74;   e cerchietto
	cTable['�'] = 75;
	cTable['�'] = 76;
	//cTable[' '] = 77;	  i cerchietto
	cTable['�'] = 78;
	cTable['�'] = cTable['�'] = 79;
	//cTable[' '] = 80;		o cerchietto
	cTable['�'] = 81;
	cTable['�'] = cTable['�'] = 82;
	//cTable[' '] = 83;		u cerchietto
	//cTable[' '] = 84;   y dieresi

	/* Un po' di lunghezze */
	lTable[' '] = 11;
	lTable['.'] = 8;
	lTable['-'] = 12;
	lTable['\''] = 8;
	lTable['0'] = 20;
	lTable['1'] = 20;
	lTable['2'] = 15;
	lTable['3'] = 20;
	lTable['4'] = 20;
	lTable['5'] = 20;
	lTable['6'] = 20;
	lTable['7'] = 20;
	lTable['8'] = 20;
	lTable['9'] = 20;

	
	lTable['a'] = lTable['A'] = lTable['�'] = lTable['�'] = 17;
	lTable['b'] = lTable['B'] = 17;
	lTable['c'] = lTable['C'] = 19;
	lTable['d'] = lTable['D'] = 17;
	lTable['e'] = lTable['E'] = 15;
	lTable['f'] = lTable['F'] = 17;
	lTable['g'] = lTable['G'] = 19;
	lTable['i'] = lTable['I'] = 16;
	lTable['h'] = lTable['H'] = 17;
	lTable['k'] = lTable['K'] = 17;
	lTable['l'] = lTable['L'] = 14;
	lTable['m'] = lTable['M'] = 19;
	lTable['n'] = lTable['N'] = 17;
	lTable['o'] = lTable['O'] = lTable['�'] = lTable['�'] = 19;
	lTable['p'] = lTable['P'] = 17;
	lTable['q'] = lTable['Q'] = 19;
	lTable['r'] = lTable['R'] = 14;
	lTable['s'] = lTable['S'] = 13;
	lTable['t'] = lTable['T'] = 15;
	lTable['u'] = lTable['U'] = lTable['�'] = lTable['�'] = 15;
	lTable['v'] = lTable['V'] = 13;
	lTable['x'] = lTable['X'] = 15;
	lTable['y'] = lTable['Y'] = 13;
	lTable['w'] = lTable['W'] = 19;
	lTable['z'] = lTable['Z'] = 20;
	lTable['�'] = 17;

	/* Casi particolari */
	SetBothCase('C','C',2);
	SetBothCase('A','T',-2);
	SetBothCase('R','S',2);
	SetBothCase('H','I',-2);
	SetBothCase('T','S',2);
	SetBothCase('O','R',2);
	SetBothCase('O','L',2);
	SetBothCase('O','G',2);
	SetBothCase('Z','A',-1);
	SetBothCase('R','R',1);
	SetBothCase('R','U',3);


	// Polish characters
	//ス謎戊３剔囀������
	//AaCcEeLlNnOoSsZzZz
	cTable['�'] = cTable['�'] = 85;
	lTable['�'] = lTable['�'] = 20;

	cTable['�'] = cTable['�'] = 86;
	lTable['�'] = lTable['�'] = 22;
	
	cTable['�'] = cTable['�'] = 87;
	lTable['�'] = lTable['�'] = 17;
	
	cTable['�'] = cTable['�'] = 88;
	lTable['�'] = lTable['�'] = 19;

	cTable['�'] = cTable['�'] = 89;
	lTable['�'] = lTable['�'] = 17;

	cTable['�'] = cTable['�'] = 90;
	lTable['�'] = lTable['�'] = 22;

	cTable['�'] = cTable['�'] = 91;
	lTable['�'] = lTable['�'] = 15;

	cTable['�'] = cTable['�'] = 92;
	lTable['�'] = lTable['�'] = 21;

	cTable['�'] = cTable['�'] = 93;
	lTable['�'] = lTable['�'] = 21;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['�'] = cTable['�'] = 85;
	lTable['�'] = lTable['�'] = 20;

	cTable['�'] = cTable['�'] =94;
	cTable['�'] = cTable['�'] =95;
	cTable['�'] = cTable['�'] =96;
	cTable['�'] = cTable['�'] =97;
	cTable['�'] = cTable['�'] =98;
	cTable['�'] = cTable['�'] =99;
	cTable['�'] = cTable['�'] =100;
	cTable['�'] = cTable['�'] =101;
	cTable['�'] = cTable['�'] =102;
	cTable['�'] = cTable['�'] =103;
	cTable['�'] = cTable['�'] =104;
	cTable['�'] = cTable['�'] =105;
	cTable['�'] = cTable['�'] =106;
	cTable['�'] = cTable['�'] =107;
	cTable['�'] = cTable['�'] =108;
	cTable['�'] = cTable['�'] =109;
	cTable['�'] = cTable['�'] =110;
	cTable['�'] = cTable['�'] =111;
	cTable['�'] = cTable['�'] =112;
	cTable['�'] = cTable['�'] =113;
	cTable['�'] = cTable['�'] =114;
	cTable['�'] = cTable['�'] =115;
	cTable['�'] = cTable['�'] =116;
	cTable['�'] = cTable['�'] =117;
	cTable['�'] = cTable['�'] =118;
	cTable['�'] = cTable['�'] =119;
	cTable['�'] = cTable['�'] =120;
	cTable['�'] = cTable['�'] =121;
	cTable['�'] = cTable['�'] =122;
	cTable['�'] = cTable['�'] =123;
	cTable['�'] = cTable['�'] =124;
	cTable['�'] = cTable['�'] =125;
	cTable['�'] = cTable['�'] =126;


	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 17;
	lTable['�'] = lTable['�'] = 16;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 20;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 16;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 15;
	lTable['�'] = lTable['�'] = 22;
	lTable['�'] = lTable['�'] = 15;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 22;
	lTable['�'] = lTable['�'] = 19;
	lTable['�'] = lTable['�'] = 16;
	lTable['�'] = lTable['�'] = 21;
	lTable['�'] = lTable['�'] = 20;
	lTable['�'] = lTable['�'] = 16;
	lTable['�'] = lTable['�'] = 16;
	lTable['�'] = lTable['�'] = 19;
	lTable['�'] = lTable['�'] = 22;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 19;	
	lTable['�'] = lTable['�'] = 19;
	lTable['�'] = lTable['�'] = 15;
	lTable['�'] = lTable['�'] = 18;
	lTable['�'] = lTable['�'] = 18;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	// rep. ceca characters	

	cTable['�'] = cTable['�'] = 127;
	cTable['�'] = cTable['�'] = 128;
	cTable['�'] = cTable['�'] = 129;
	cTable['�'] = cTable['�'] = 130;
	cTable['�'] = cTable['�'] = 131;
	cTable['�'] = cTable['�'] = 132;
	cTable['�'] = cTable['�'] = 133;
	cTable['�'] = cTable['�'] = 134;
	cTable['�'] = cTable['�'] = 135;
	cTable['�'] = cTable['�'] = 136;
	cTable['�'] = cTable['�'] = 137;
	cTable['�'] = cTable['�'] = 138;
	cTable['�'] = cTable['�'] = 139;
	cTable['�'] = cTable['�'] = 140;
	cTable['�'] = cTable['�'] = 141;

	lTable['�'] = lTable['�'] =17;
	lTable['�'] = lTable['�'] =15;
	lTable['�'] = lTable['�'] =22;
	lTable['�'] = lTable['�'] =18;
	lTable['�'] = lTable['�'] =21;
	lTable['�'] = lTable['�'] =16;
	lTable['�'] = lTable['�'] =18;
	lTable['�'] = lTable['�'] =19;
	lTable['�'] = lTable['�'] =17;
	lTable['�'] = lTable['�'] =23;
	lTable['�'] = lTable['�'] =24;
	lTable['�'] = lTable['�'] =17;
	lTable['�'] = lTable['�'] =22;
	lTable['�'] = lTable['�'] =16;
	lTable['�'] = lTable['�'] =16;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	// traduci le lettere accentate in lettere normali

	cTable['�'] = cTable['�'] = cTable['�'] = 0; // a
	lTable['�'] = lTable['�'] = lTable['�'] = 17;

	cTable['�'] = cTable['�'] = 4; // e
	lTable['�'] = lTable['�'] = 15;

	cTable['�'] = cTable['�'] = cTable['�'] = 8; // i
	lTable['�'] = lTable['�'] = lTable['�'] = 16;

	cTable['�'] = cTable['�'] = cTable['�'] = cTable['�'] = 14; // o
	lTable['�'] = lTable['�'] = lTable['�'] = lTable['�'] = 19;

	cTable['�'] = cTable['�'] = 20; // u
	lTable['�'] = lTable['�'] = 15;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['�'] = 142;
	// SS = 143

	lTable['�'] = 24;
#endif // FONT_GERMAN
}


/****************************************************************************\
*       Metodi di RMText
\****************************************************************************/

RMFontColor *RMText::m_fonts[4] = { NULL, NULL, NULL, NULL };
OSystem::MutexRef RMText::m_cs;
RMGfxClearTask RMText::m_clear;

RMText::RMText() {
	// Colore di default: bianco
	m_r = m_g = m_b = 255;

	// Lunghezza di default
	maxLineLength = 350;

	m_bTrasp0 = true;
	aHorType = HCENTER;
	aVerType = VTOP;
	SetPriority(150);
}

RMText::~RMText() {
	
}

void RMText::SetMaxLineLength(int max) {
	maxLineLength = max;
}

bool RMText::RemoveThis() {
 // Qui possiamo fare i controlli sul numero di frame, sul tempo trascorso
 // etc.
	return true;
}


void RMText::WriteText(RMString text, int nFont, int *time) {
	// Inizializza i font (una volta sola)	
	if (m_fonts[0] == NULL) {
		m_fonts[0] = new RMFontParla; m_fonts[0]->Init();
		m_fonts[1] = new RMFontObj;   m_fonts[1]->Init();
		m_fonts[2] = new RMFontMacc;  m_fonts[2]->Init();
		m_fonts[3] = new RMFontCredits;  m_fonts[3]->Init();
	}

	g_system->lockMutex(m_cs);
	WriteText(text,m_fonts[nFont],time);
	g_system->unlockMutex(m_cs);
}


void RMText::WriteText(RMString text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;
	char *p, *old_p;
	int i, j, x, y;
	int len;
	int numchar;
	int width, height;
	char *string;
	int numlines;

	// Setta il colore di base
	font->SetBaseColor(m_r, m_g, m_b);

	// Si autodistrugge il buffer prima di iniziare
	Destroy();

	// Se la stringa � vuota, non fare nulla	
	if (text == NULL || text[0] == '\0')
		return;
	
	// Divide la frase in linee. In questo ciclo, X contiene la lunghezza massima raggiunta da una linea
	// e I il numero delle linee
	string=p = text;
	i = j = x = 0;
	while (*p != '\0') {
		j += font->StringLen(*p);
		if (j > (((aHorType == HLEFTPAR) && (i > 0)) ? maxLineLength - 25 : maxLineLength)) {
			j -= font->StringLen(*p, p[1]);
			if (j > x) x = j;

			// Torna indietro al primo spazio utile
			//
			// BERNIE: nella versione originale le frasi contenenti
			// parole che superano la larghezza di una riga causavano
			// l'interruzione dell'intera frase.
			// Questo workaround e' parziale: la parola troppo lunga
			// viene spezzata bruscamente e si perde una lettera.
			// Oltre allo spazio e' ammesso il wrap sul carattere '-'.
			//
			old_p = p;
			while (*p != ' ' && *p != '-' && p > string) p--;

			if (p == string)
				p = old_p;

			// Controlla se sono tutti spazi fino alla fine
			while (*p == ' ' && *p != '\0') p++;
			if (*p == '\0')
				break;
			p--;
			i++;
			*p = '\0';
			j = 0;
		}
		p++;
	}

	if (j > x) x = j;
 
	i++;
	numlines = i;

	// X=Lunghezza della linea piu' lunga. Controlla se puo' essere puttata a X1
	//x+=font->StringLen(-1)+1;          // Meglio esagerare per sicurezza
	x += 8;

	// Posizione di partenza per la surface: X1,Y
	width = x;
	height = (numlines - 1) * font->LetterHeight() + font->m_fontDimy;

	// Crea la surface
	Create(width, height);
	//AddPrim(new RMGfxPrimitive(&m_clear));
	Common::fill(m_buf, m_buf + width * height * 2, 0);

	p = string;

	y = 0;
	numchar = 0;
	for (; i > 0; i--) {
		// Misura la lunghezza della linea
		x = 0;
		j = font->StringLen(RMString(p));

		switch (aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == numlines)
				x=0;
			else
				x = 25;
			break;

		case HCENTER:
			x = width / 2 - j / 2;
			break;

		case HRIGHT:
			x = width - j - 1;
			break;
		}

		while (*p != '\0') {
			if (*p == ' ') {
				x += font->StringLen(*p);
				p++;
				continue;
			}

			prim = font->MakeLetterPrimitive(*p, len);
			prim->Dst().x1 = x;
			prim->Dst().y1 = y;
			AddPrim(prim);

			numchar++;

			x += font->StringLen(*p, p[1]);
			p++;
		}
		p++;
		y += font->LetterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - nCfgTextSpeed) * 14;
}

void RMText::ClipOnScreen(RMGfxPrimitive *prim) {
	// Cerca di non farlo uscire dallo schermo
	if (prim->Dst().x1 < 5) prim->Dst().x1 = 5;
	if (prim->Dst().y1 < 5) prim->Dst().y1 = 5;
	if (prim->Dst().x1+m_dimx > 635) prim->Dst().x1 = 635 - m_dimx;
	if (prim->Dst().y1+m_dimy > 475) prim->Dst().y1 = 475 - m_dimy;
}

void RMText::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Allinea orizzontalmente
	if (aHorType == HCENTER)
		prim->Dst().TopLeft() -= RMPoint(m_dimx / 2, 0);
	else if (aHorType == HRIGHT)
		prim->Dst().TopLeft() -= RMPoint(m_dimx, 0);


	// Alinea verticalemente
	switch (aVerType) {
	case VTOP:
		break;

	case VCENTER:
		prim->Dst().y1-=m_dimy/2;
		break;

	case VBOTTOM:
		prim->Dst().y1-=m_dimy;
		break;
	}

	ClipOnScreen(prim);

	RMGfxWoodyBuffer::Draw(bigBuf,prim);
}

/****************************************************************************\
*       Metodi di RMTextDialog
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	m_startTime = 0;
	dst = RMPoint(0,0);

	m_bSkipStatus = true;
	m_bShowed = true;
	m_bForceTime = false;
	m_bForceNoTime = false;
	m_bAlwaysDisplay = false;
	m_bNoTab = false;
	hCustomSkip = INVALID_HANDLE_VALUE;
	hCustomSkip2 = INVALID_HANDLE_VALUE;
	m_input = NULL;

	// Crea l'evento di fine displaying
	hEndDisplay = CreateEvent(NULL, false, false, NULL);
}

RMTextDialog::~RMTextDialog() {
	CloseHandle(hEndDisplay);
}

void RMTextDialog::Show(void) {
	m_bShowed = true;
}

void RMTextDialog::Hide(void) {
	m_bShowed = false;
}

void RMTextDialog::WriteText(RMString text, int font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}

void RMTextDialog::WriteText(RMString text, RMFontColor *font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}


void RMTextDialog::SetSkipStatus(bool bEnabled) {
	m_bSkipStatus=bEnabled;
}

void RMTextDialog::ForceTime(void) {
	m_bForceTime = true;
}

void RMTextDialog::ForceNoTime(void) {
	m_bForceNoTime = true;
}

void RMTextDialog::SetNoTab(void) {
	m_bNoTab = true;
}

void RMTextDialog::SetForcedTime(uint32 dwTime) {
	m_time = dwTime;	
}

void RMTextDialog::SetAlwaysDisplay(void) {
	m_bAlwaysDisplay = true;
}

bool RMTextDialog::RemoveThis(void) {
	// Frase NON di background
	if (m_bSkipStatus) {
		if (!(bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE))
			if (bCfgTimerizedText) {
				if (!m_bForceNoTime)
					if (_vm->GetTime() > (uint32)m_time + m_startTime)
						return true;
			}

		if (!m_bNoTab)
			if ((GetAsyncKeyState(Common::KEYCODE_TAB) & 0x8001) == 0x8001)
				return true;

		if (!m_bNoTab)
			if (m_input)
				if (m_input->MouseLeftClicked() || m_input->MouseRightClicked())
					return true;
	}
	// Frase di background
	else {
		if (!(bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE))
			if (!m_bForceNoTime)
				if (_vm->GetTime() > (uint32)m_time + m_startTime)
					return true;
	}

	// Se il tempo � forzato
	if (m_bForceTime)
		if (_vm->GetTime() > (uint32)m_time + m_startTime)
			return true;

	if (hCustomSkip != INVALID_HANDLE_VALUE)
		if (WaitForSingleObject(hCustomSkip, 0) == WAIT_OBJECT_0)
			return true;

	if (bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE)
		if (WaitForSingleObject(hCustomSkip2,0) == WAIT_OBJECT_0)
			return true;

	return false;
}

void RMTextDialog::Unregister(void) {
	RMGfxTask::Unregister();
	assert(m_nInList == 0);
	SetEvent(hEndDisplay);
}

void RMTextDialog::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_startTime == 0)
		m_startTime = _vm->GetTime();
	
	if (m_bShowed) {
		if (bCfgSottotitoli || m_bAlwaysDisplay) {
			prim->Dst().TopLeft() = dst;
			RMText::Draw(bigBuf, prim);
		}
	}
}

void RMTextDialog::SetCustomSkipHandle(HANDLE hCustom) {
	hCustomSkip = hCustom;
}

void RMTextDialog::SetCustomSkipHandle2(HANDLE hCustom) {
	hCustomSkip2 = hCustom;
}

void RMTextDialog::WaitForEndDisplay(void) {
	WaitForSingleObject(hEndDisplay, INFINITE);
}

void RMTextDialog::SetInput(RMInput *input) {
	m_input = input;
}

/****************************************************************************\
*       Metodi di RMTextDialogScrolling
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	curLoc = loc;
	startScroll = loc->ScrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	RMPoint curDst;

	curDst = dst;

	if (curLoc != NULL)
		dst -= curLoc->ScrollPosition() - startScroll;

	RMTextDialog::Draw(bigBuf, prim);

	dst = curDst;
}

void RMTextDialogScrolling::ClipOnScreen(RMGfxPrimitive *prim) {
	// Non dobbiamo fare nulla!
}


/****************************************************************************\
*       Metodi di RMTextItemName
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	m_item = NULL;
	SetPriority(220);
}

RMTextItemName::~RMTextItemName() {

}

void RMTextItemName::DoFrame(RMGfxTargetBuffer& bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	RMString itemName;
	RMItem *lastItem = m_item;

	// Si aggiunge alla lista se c'e' bisogno
	if (!m_nInList)
		bigBuf.AddPrim(new RMGfxPrimitive(this));
	
	// Aggiorna le coordinate di scrolling
	m_curscroll = loc.ScrollPosition();

	// Controlla se siamo sopra l'inventario
	if (inv.ItemInFocus(m_mpos))
		m_item = inv.WhichItemIsIn(m_mpos);
	else
		m_item = loc.WhichItemIsIn(m_mpos);
	
	itemName = "";

	// Si fa dare il nuovo nome
	if (m_item != NULL)
		m_item->GetName(itemName);

	// Se lo scrive
	WriteText(itemName, 1);

	// Se e' diverso dal precedente, e' il caso di aggiornare anche il puntatore con la WhichPointer
	if (lastItem != m_item) {
		if (m_item == NULL)
			ptr.SetSpecialPointer(RMPointer::PTR_NONE);
		else {
			HANDLE hThread = mpalQueryDoAction(20, m_item->MpalCode(), 0);		
			if (hThread == INVALID_HANDLE_VALUE)
				ptr.SetSpecialPointer(RMPointer::PTR_NONE);
			else
				WaitForSingleObject(hThread,INFINITE);
		}
	}
}


void RMTextItemName::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Se non c'e' testo, e' inutile continuare
	if (m_buf == NULL)
		return;

	// Setta come coordinate destinazione quelle del mouse
	prim->Dst().TopLeft() = m_mpos-RMPoint(0, 30);

	RMText::Draw(bigBuf,prim);
}

RMPoint RMTextItemName::GetHotspot() { 
	if (m_item == NULL) 
		return m_mpos + m_curscroll; 
	else 
		return m_item->Hotspot();  
}

RMItem *RMTextItemName::GetSelectedItem() { 
	return m_item; 
}

bool RMTextItemName::IsItemSelected() { 
	return m_item != NULL; 
}

bool RMTextItemName::IsNormalItemSelected() { 
	return m_item != NULL && m_itemName.Length() > 0; 
}


/****************************************************************************\
*       Metodi di RMDialogChoice
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);
	
	DlgText.Init(dlg1, dlg1.Width(), dlg1.Height());
	DlgTextLine.Init(dlg2, dlg2.Width(), dlg2.Height());

	DlgText.LoadPaletteWA(dlgpal);
	DlgTextLine.LoadPaletteWA(dlgpal);
	
	hUnreg=CreateEvent(NULL, false, false, NULL);
	bRemoveFromOT = false;
}

RMDialogChoice::~RMDialogChoice() {
	CloseHandle(hUnreg);
}

void RMDialogChoice::Unregister(void) {
	RMGfxWoodyBuffer::Unregister();
	assert(!m_nInList);
	PulseEvent(hUnreg);

	bRemoveFromOT = false;
}

void RMDialogChoice::Init(void)
{
	m_numChoices = 0;
	m_drawedStrings = NULL;
	m_ptDrawStrings = NULL;
	m_curSelection = -1;

	Create(640, 477);
	SetPriority(140);
}


void RMDialogChoice::Close(void) {
	if (m_drawedStrings != NULL) {
		delete[] m_drawedStrings;
		m_drawedStrings = NULL;
	}

	if (m_ptDrawStrings != NULL) {
		delete[] m_ptDrawStrings;
		m_ptDrawStrings = NULL;
	}

	Destroy();
}

void RMDialogChoice::SetNumChoices(int num) {
	int i;

	m_numChoices = num;
	m_curAdded = 0;
	
	// Alloca lo spazio per le stringhe disegnate
	m_drawedStrings = new RMText[num];
	m_ptDrawStrings = new RMPoint[num];

	// Le inizializza
	for (i = 0; i < m_numChoices; i++) {
		m_drawedStrings[i].SetColor(0, 255, 0);
		m_drawedStrings[i].SetAlignType(RMText::HLEFTPAR, RMText::VTOP);
		m_drawedStrings[i].SetMaxLineLength(600);
		m_drawedStrings[i].SetPriority(10);
	}
}

void RMDialogChoice::AddChoice(RMString string) {
	// Si disegna la stringa
	assert(m_curAdded < m_numChoices);
	m_drawedStrings[m_curAdded++].WriteText(string,0);	
}

void RMDialogChoice::Prepare(void) {
	int i;
	RMPoint ptPos;

	AddPrim(new RMGfxPrimitive(&DlgText,RMPoint(0,0)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83+83)));

	ptPos.Set(20,90);

	for (i = 0; i < m_numChoices; i++) {
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[i], ptPos));
		m_ptDrawStrings[i] = ptPos;
		ptPos.Offset(0,m_drawedStrings[i].Dimy() + 15);
	}

	DrawOT();
	ClearOT();

	m_ptDrawPos.Set(0,480-ptPos.y);
}

void RMDialogChoice::SetSelected(int pos) {
	//uint16 * buf = (uint16 *)m_buf;
	RMGfxBox box;
	RMRect rc;

	if (pos == m_curSelection)
		return;

	box.SetPriority(5);

	if (m_curSelection != -1) {
		box.SetColor(0xCC, 0xCC, 0xFF);
		rc.TopLeft()=RMPoint(18, m_ptDrawStrings[m_curSelection].y); 
		rc.BottomRight() = rc.TopLeft() + RMPoint(597, m_drawedStrings[m_curSelection].Dimy());
		AddPrim(new RMGfxPrimitive(&box, rc));

		AddPrim(new RMGfxPrimitive(&m_drawedStrings[m_curSelection], m_ptDrawStrings[m_curSelection]));
		DrawOT();
		ClearOT();
	}

	if (pos != -1) {
		box.SetColor(100, 100, 100);
		rc.TopLeft()=RMPoint(18, m_ptDrawStrings[pos].y); 
		rc.BottomRight() = rc.TopLeft()+RMPoint(597, m_drawedStrings[pos].Dimy());
		AddPrim(new RMGfxPrimitive(&box, rc));
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[pos], m_ptDrawStrings[pos]));
	}

	DrawOT();
	ClearOT();

	m_curSelection = pos;
}

void RMDialogChoice::Show(RMGfxTargetBuffer *bigBuf) {
	Prepare();
	m_bShow = false;

	if (!m_nInList && bigBuf != NULL)
		bigBuf->AddPrim(new RMGfxPrimitive(this));

	if (0) {
		m_bShow = true;
	} else {
		RMPoint destpt;
		int deltay;
		int starttime = _vm->GetTime();
		int elaps;

		deltay=480 - m_ptDrawPos.y;
		destpt = m_ptDrawPos;
		m_ptDrawPos.Set(0, 480);

  	if (!m_nInList && bigBuf != NULL)
	  	bigBuf->AddPrim(new RMGfxPrimitive(this));
		m_bShow = true;

		elaps = 0;
		while (elaps < 700) {
			MainWaitFrame();
			MainFreeze();
			elaps = _vm->GetTime() - starttime;
			m_ptDrawPos.y = 480 - ((deltay * 100) / 700 * elaps) / 100;
			MainUnfreeze();
		}

		m_ptDrawPos.y = destpt.y;
	}
}

void RMDialogChoice::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_bShow == false)
		return;

	prim->SetDst(m_ptDrawPos);
	RMGfxSourceBuffer16::Draw(bigBuf, prim);
}


void RMDialogChoice::Hide(void) {
	if (1) {
		int deltay;
		int starttime = _vm->GetTime();
		int elaps;

		deltay=480 - m_ptDrawPos.y;
		elaps = 0;
		while (elaps < 700) {
			MainWaitFrame();
			MainFreeze();
			elaps=_vm->GetTime()-starttime;
			m_ptDrawPos.y=480-((deltay*100)/700*(700-elaps))/100;
			MainUnfreeze();
		}
	}

	m_bShow = false;
	bRemoveFromOT = true;
	WaitForSingleObject(hUnreg, INFINITE);
}


bool RMDialogChoice::RemoveThis(void) {
	return bRemoveFromOT;
}

void RMDialogChoice::DoFrame(RMPoint ptMousePos) {
	int i;
	
	if (ptMousePos.y > m_ptDrawPos.y) {		
		for (i = 0; i < m_numChoices; i++) {
			if ((ptMousePos.y >= m_ptDrawPos.y+m_ptDrawStrings[i].y) && (ptMousePos.y < m_ptDrawPos.y+m_ptDrawStrings[i].y+m_drawedStrings[i].Dimy())) {
				SetSelected(i);
				break;
			}
		}

		if (i == m_numChoices)
			SetSelected(-1);
	}
}

int RMDialogChoice::GetSelection(void) {
	return m_curSelection;
}

} // End of namespace Tony
