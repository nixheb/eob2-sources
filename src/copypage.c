/*
 * Eye of the Beholder II - START.EXE
 * Copy a rectangle between pages, clipped: module 1D92 (Ghidra 2D92).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y copypage.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

/* the video driver: copy a rectangle (x in bytes) */
extern void (far     *Drv_6556)(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 6556 */

/* 2D92:000A [asm] - (x, y) of page from to (x2, y2) of page to; x and w
 * in bytes (8 pixels), the screen is 40 x 200 */
void far copy_page(register int x, int y, register int x2, int y2, int w, int h, int from, int to)
{
	if (w > 40)
		w = 40;
	if (h > 200)
		h = 200;
	if (x < 0) {
		x2 -= x;
		w += x;
		x = 0;
	}
	if (x < 40 && x2 < 40) {
		if (x2 < 0) {
			x -= x2;
			w += x2;
			x2 = 0;
		}
		if (y < 0) {
			y2 -= y;
			h += y;
			y = 0;
		}
		if (y2 < 0) {
			y -= y2;
			h += y2;
			y2 = 0;
		}
		Drv_6556(x, y, x2, y2, w, h, from, to);
	}
}
