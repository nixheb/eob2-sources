/*
 * Eye of the Beholder II - START.EXE
 * Define a window: module 210C (Ghidra 310C).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y window.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern int            GuiStyle;          /* 636F the window of the text */
extern int            CurDim;            /* 6298 the window of the shapes */

int  select_window(int w);                                       /* 31B0:0133 */
void set_dim(int dim);                                           /* 2DA3:0256 */

/* 310C:0009 [asm] */
void far clip_window(register int w, int x, int y, int width, int h)
{
	Dims[w].x = x;
	Dims[w].y = y;
	Dims[w].w = width;
	Dims[w].h = h;
	if (GuiStyle == w)
		select_window(w);
	if (w == CurDim)
		set_dim(w);
}
