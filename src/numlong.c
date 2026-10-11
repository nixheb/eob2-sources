/*
 * Eye of the Beholder II - START.EXE
 * Print a long number: module 2107 (Ghidra 3107).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y numlong.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

char far *ltoa(long value, char far *s, int radix);
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

/* 3107:0008 [asm] */
void far print_long(unsigned long n, int x, int y, int fg, int bg)
{
	char buf[14];

	ltoa(n, buf, 10);
	Drv_65F2(buf, x, y, fg, bg);
}
