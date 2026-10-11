/*
 * Eye of the Beholder II - START.EXE
 * A random number in a range: module 2064 (Ghidra 3064).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y rnd.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

/* Borland's random() (stdlib.h, RAND_MAX = 0x7FFFU) */
int rand(void);
#define random(num) (int)(((long)rand() * (num)) / (0x7FFFU + 1))

/* 3064:0004 [asm] - a .. b, in any order */
unsigned far random_range(register unsigned a, register unsigned b)
{
	unsigned t;

	if (a > b) {
		t = a;
		a = b;
		b = t;
	}
	while ((t = random(b - a + 1) + a) > b)
		;
	return t;
}
