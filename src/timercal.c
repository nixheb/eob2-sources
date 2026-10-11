/*
 * Eye of the Beholder II - START.EXE
 * Measure the machine at start: module 1F7A (Ghidra 2F7A).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y timercal.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

#define MK_FP( seg,ofs )( (void _seg * )( seg ) +( void near * )( ofs ))

extern int            WaitTimer;         /* F16C passed to wait_retrace() (3404:0019) */

void ints_on(void);                                              /* 3269:0267 sti (assembler) */
void ints_off(void);                                             /* 3269:0269 cli */
int  vretrace(void);                                             /* 3404:000C port 3DA bit 3 */

/* 2F7A:0003 [asm] - counts the loops of 4 BIOS ticks (sic: the count is
 * never used), then whether the "vertical retrace" bit of the video card
 * is mostly set: then wait_retrace() waits for the other edge */
void far measure_machine(void)
{
	unsigned long far *bios;
	unsigned long t;
	int off, on, k;
	register int loops, j;

	bios = MK_FP(0, 0x46C);
	loops = WaitTimer = 0;
	ints_off();
	t = *bios;
	ints_on();
	while (*bios == t)
		;
	ints_off();
	t = *bios + 4;
	ints_on();
	while (*bios < t) {
		loops++;
		for (j = 200; j; j--)
			;
	}
	off = on = 0;
	for (k = 0; k < 10000; k++)
		if (vretrace())
			on++;
		else
			off++;
	if (on > off)
		WaitTimer = 1;
}
