/*
 * Eye of the Beholder II - START.EXE
 * The font: module 2166 (Ghidra 3166).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y font.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* Uninitialised data: in bssf090.c (DS:F090-F09B) */
extern byte far *CurFont; /* F090 */
extern int CursorX;       /* F094 */
extern int CursorY;       /* F096 */
extern byte far *Font8;   /* F098 */
extern char           CharWidth;         /* 6288 */
extern char           LineHeight;        /* 6289 */
extern int            WinH, WinW;        /* F100, F102 (chars): the text window */
extern int            WinLines;          /* 6504 */
extern int            WinCols;           /* 6506 */

/* 3166:000A [asm] - returns the old font; a font: +102 the height,
 * +103 the width of the characters */
byte far * far set_font(byte far *font)
{
	byte far *old;

	old = CurFont;
	if (font) {
		CurFont = font;
		CharWidth = font[0x103];
		LineHeight = font[0x102];
		WinLines = WinH / LineHeight;
		WinCols = WinW * 8 / CharWidth;
	}
	return old;
}
