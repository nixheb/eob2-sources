/*
 * Eye of the Beholder II - START.EXE
 * A rectangle (a copy of draw_rect, 18D7:0571): module 1D9E (Ghidra 2D9E).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y frame.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern void (far     *Drv_6566)(int x1, int y1, int x2, int y2, int colour);   /* 6566 a line */

/* 2D9E:0000 [asm] */
void far draw_rect2(register int x1, register int y1, int x2, int y2, int colour)
{
	Drv_6566(x1, y1, x2, y1, colour);
	Drv_6566(x1, y2, x2, y2, colour);
	Drv_6566(x1, y1, x1, y2, colour);
	Drv_6566(x2, y1, x2, y2, colour);
}
