/*
 * Eye of the Beholder II - START.EXE
 * The page drawn on: module 210B (Ghidra 310B).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y setpage.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern int            Var_62A5;          /* 62A5 the page drawn on */

/* 310B:0001 [asm] - returns the old one */
int far set_page(int page)
{
	int old;

	old = Var_62A5;
	Var_62A5 = page;
	return old;
}
