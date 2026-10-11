/*
 * Eye of the Beholder II - START.EXE
 * Keep the mouse in a window: module 223F (Ghidra 323F).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y mousewin.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */

void mouse_hide_rect(int x1, int y1, int x2, int y2);            /* 338B:0206 */
void mouse_show_rect(void);                                      /* 338B:02CA */

/* 323F:0009 [asm] */
void far mouse_in_window(register int w)
{
	int y, width, h;
	register int x;

	x = Dims[w].x * 8;
	y = Dims[w].y;
	width = Dims[w].w * 8;
	h = Dims[w].h;
	mouse_hide_rect(x, y, x + width, y + h);
}

/* 323F:006C [asm] */
void far mouse_free(void)
{
	mouse_show_rect();
}
