/*
 * Eye of the Beholder II - START.EXE
 * The graphic pages: module 1F3E (Ghidra 2F3E).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y pages.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * PageSegs[] (DS:62A7) holds the segment of each page (0: the screen,
 * A000); a page is used whole by alloc_page, or by halves.
 */
#include "eob2.h"

#define MK_FP( seg,ofs )( (void _seg * )( seg ) +( void near * )( ofs ))

extern unsigned       PageSegs[];        /* 62A7 */

/* 2F3E:0009 [asm] - the memory of a page */
void far * far alloc_page(register int page)
{
	page &= 0xF;
	return MK_FP(PageSegs[page | 1], 0);
}

/* 2F3E:0025 [asm] */
void far * far page_memory(register int page)
{
	page &= 0xF;
	return MK_FP(PageSegs[page & ~1], 0);
}
