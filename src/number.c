/*
 * Eye of the Beholder II - START.EXE
 * Print a number: module 2060 (Ghidra 3060).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y number.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

char far *itoa(int value, char far *s, int radix);
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

/* 3060:000E [asm] */
void far print_number(int n, int x, int y, int fg, int bg)
{
	char buf[30];

	itoa(n, buf, 10);
	Drv_65F2(buf, x, y, fg, bg);
}
