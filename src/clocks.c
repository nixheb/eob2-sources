/*
 * Eye of the Beholder II - START.EXE
 * Start and stop the clocks: module 2187 (Ghidra 3187).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y clocks.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * Clock n (1, 2) runs while bit n - 1 of ClockEnable is set (loop.c):
 * clock 2 is GameTicks, stopped during the long actions.
 */
#include "eob2.h"

extern unsigned       ClockEnable;       /* 6690 */

/* 3187:0009 [asm] - returns whether the clock was running.
 * ORIGINAL BUG: it tests the bit 1 << n but sets the bit 1 << (n - 1)
 * (the value returned is never used) */
int far clock_start(register int n)
{
	int was;

	was = (ClockEnable & (1 << n)) ? 1 : 0;
	ClockEnable |= 1 << (n - 1);
	return was;
}

/* 3187:003D [asm] - the same bug */
int far clock_stop(register int n)
{
	int was;

	was = (ClockEnable & (1 << n)) ? 1 : 0;
	ClockEnable &= ~(1 << (n - 1));
	return was;
}
