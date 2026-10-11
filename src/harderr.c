/*
 * Eye of the Beholder II - START.EXE
 * The DOS critical error handler (INT 24h): module 1F42 (Ghidra 2F42).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y harderr.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern int            DiskError;         /* F0FC the last critical error */

void harderr(int (far *handler)());
void hardretn(int retn);

/* 2F42:0001 [asm] - keep the error and go on (no "Abort, Retry, Fail?") */
int far critical_error(int errval)
{
	DiskError = errval;
	hardretn(0);
}

/* 2F42:0016 [asm] */
void far install_critical_error(void)
{
	harderr(critical_error);
}
