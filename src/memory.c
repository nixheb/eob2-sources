/*
 * Eye of the Beholder II - START.EXE
 * The memory blocks of the Westwood library: module 1D0E (Ghidra 2D0E).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y memory.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A block from farmalloc() is given a one-byte header: the flags of the
 * allocation. With flag 0x20 the block starts on a paragraph (offset 0):
 * the byte before it holds the flags and how many bytes were skipped.
 * Flag 0x10 clears the block.
 */
#include "eob2.h"

typedef unsigned char byte;

/* Borland's dos.h */
#define MK_FP( seg,ofs )( (void _seg * )( seg ) +( void near * )( ofs ))
#define FP_SEG( fp )( (unsigned )( void _seg * )( void far * )( fp ))
#define FP_OFF( fp )( (unsigned )( fp ))

/* farheapwalk() (alloc.h) */
struct farheapinfo {
	void huge     *ptr;
	unsigned long  size;
	int            in_use;
};

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern int (far      *MemHandler)(void); /* 629A called when memory runs out */
extern int            MemBlocks;         /* 629E the blocks allocated */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void far *farmalloc(unsigned long n);
void farfree(void far *p);
void far *realloc(void far *p, unsigned n);
void far *memset(void far *p, int c, unsigned n);
int  farheapcheck(void);
unsigned long farcoreleft(void);
int  farheapwalk(struct farheapinfo far *hi);
int  free_cache(int a, char b);                                  /* 2DCE:0C71 */
void far *normalize(void far *p);                                /* 3374:0071 */
void set_text_mode(int mode);                                    /* 31A9:0033 */
void dos_puts(char far *s);                                      /* 3246:0006 */
int  get_key(void);                                              /* 3295:05D0 */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);

/* ------------------------------------------------------------------ */
/* Module 2D0E, in address order                                       */
/* ------------------------------------------------------------------ */

/* 2D0E:000B [asm] */
void far * far mem_alloc(unsigned long size, int flags)
{
	byte far *p;
	unsigned long left;
	byte far *q;

	left = size;
	size += (flags & 0x20) ? 16 : 1;
	do {
		while ((p = farmalloc(size)) == 0)
			if (!free_cache(0, 0xFF))
				break;
	} while (!p && MemHandler && MemHandler());
	if (!p)
		return 0;
	p = normalize(p);
	if (flags & 0x20) {
		register byte t;

		t = 0xF - (byte)FP_OFF(p);
		p[t] = t | flags;
		p = MK_FP(FP_SEG(p) + 1, 0);
	} else
		*p++ = flags;
	if (flags & 0x10) {
		for (q = p; left > 32000; left -= 32000, q = MK_FP(FP_SEG(q) + 2000, FP_OFF(q)))
			memset(q, 0, 32000);
		memset(q, 0, left);
	}
	MemBlocks++;
	return p;
}

/* 2D0E:013F [asm] */
void far mem_free(void far *p)
{
	byte far *h;
	byte f;

	if (p) {
		h = MK_FP(FP_SEG(p) - 1, FP_OFF(p) + 0xF);
		f = *h;
		if (f & 0x20)
			farfree(MK_FP(FP_SEG(h), 0xF - (f & 0xF)));
		else
			farfree(MK_FP(FP_SEG(p), FP_OFF(p) - 1));
	}
	MemBlocks--;
}

/* 2D0E:0194 [asm] */
void far * far mem_realloc(void far *p, register int size)
{
	byte far *h;
	byte skip, f;

	h = MK_FP(FP_SEG(p) - 1, FP_OFF(p) + 0xF);
	skip = *h & 0xF;
	f = *h & 0xF0;
	size += (f & 0x20) ? 16 : 1;
	if (f & 0x20) {
		h = realloc(MK_FP(FP_SEG(h), 0xF - skip), size);
		skip = 0x10 - (byte)FP_OFF(h);
		*(h + skip - 1) = (skip - 1) | f;
		h = MK_FP(FP_SEG(h) + 1, 0);
	} else {
		h = realloc(h, size);
		*h++ = f;
	}
	return h;
}

/* 2D0E:0249 [asm] - the largest free block */
long far mem_free_total(void)
{
	unsigned long best;
	struct farheapinfo hi;

	if (farheapcheck() == -1) {
		set_text_mode(9);
		dos_puts("Ram Free Error! Press any key to exit to DOS.\r\n");
		get_key();
		system_shutdown();
		exit(5);
	}
	best = farcoreleft();
	hi.ptr = 0;
	while (farheapwalk(&hi) == 2)
		if (!hi.in_use && hi.size > best)
			best = hi.size;
	best -= 16;
	if ((long)best > 0)
		return best;
	return 0;
}
