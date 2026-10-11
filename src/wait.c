/*
 * Eye of the Beholder II - START.EXE
 * Waiting: module 1D9B (Ghidra 2D9B).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y wait.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern int            WaitTimer;         /* F16C */

void wait_retrace(int mode);                                      /* 3404:0019 waits one tick */

/* 2D9B:0003 [asm] - wait n ticks */
void far wait_ticks(register int n)
{
	while (n--)
		wait_retrace(WaitTimer);
}

/* 2D9B:0021 [asm] */
void far wait_tick(void)
{
	wait_ticks(1);
}
