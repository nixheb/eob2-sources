/*
 * Eye of the Beholder II - START.EXE
 * Files read whole, CPS pictures: module 2069 (Ghidra 3069).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y picture.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A CPS file: a word (the size), then an 8-byte header: +0 the
 * compression (0 none, 3 or 4 Westwood's LCW), +2 the size once
 * unpacked (long), +6 the size of the palette that follows.
 */
#include "eob2.h"

typedef unsigned char byte;

#define MK_FP( seg,ofs )( (void _seg * )( seg ) +( void near * )( ofs ))
#define FP_SEG( fp )( (unsigned )( void _seg * )( void far * )( fp ))

extern unsigned       PageSegs[];        /* 62A7 */
extern unsigned long  PageSizes[];       /* 62E7 */

int  open_file(char far *name, int mode);                        /* 2DDE:0007 */
long read_file(int fd, void far *buf, long n);                   /* 2E12:000E */
void close_file(int fd);                                         /* 2E0E:0006 */
void seek_file(int fd, long pos, int from);                      /* 2DCE:0750 */
void far *alloc_page(int page);                                  /* 2F3E:0009 */
void far *normalize(void far *p);                                /* 3374:0071 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
unsigned unpack_3(unsigned srcseg, unsigned dstseg);             /* 33EC:0104 */
unsigned unpack_lcw(void far *src, void far *dst, int max);      /* 33EC:0004 */
long far unpack_picture(byte far *src, void far *dst);

/* ------------------------------------------------------------------ */
/* Module 3069, in address order                                       */
/* ------------------------------------------------------------------ */

/* 3069:0000 [asm] - returns the size read */
long far load_file_into(char far *name, void far *buf, long n)
{
	register int fd;

	fd = open_file(name, 1);
	n = read_file(fd, buf, n);
	close_file(fd);
	return n;
}

/* 3069:0044 [asm] - load a CPS file: packed at the end of page, unpacked
 * into page dest; its palette into pal (if any) */
void far load_cps_file(char far *name, int page, int dest, byte far *pal)
{
	unsigned size;
	byte far *hdr;
	byte far *q;
	register int fd;
	register unsigned palSize;

	hdr = alloc_page(page);
	fd = open_file(name, 1);
	read_file(fd, &size, 2L);
	read_file(fd, hdr, 8L);
	size -= 8;
	palSize = ((unsigned far *)hdr)[3];
	if (pal && palSize)
		read_file(fd, pal, palSize);
	else
		seek_file(fd, palSize, 1);
	((unsigned far *)hdr)[3] = 0;
	size -= palSize;
	q = MK_FP(PageSegs[page], 0);
	q += (unsigned)PageSizes[page] - (size + 8);
	q = normalize(q);
	q = MK_FP(FP_SEG(q), 0);
	mem_copy(hdr, q, 8L);
	read_file(fd, q + 8, size);
	close_file(fd);
	unpack_picture(q, alloc_page(dest));
}

/* 3069:0186 [asm] - returns the size unpacked (sic: nothing for an
 * unknown compression) */
long far unpack_picture(byte far *src, void far *dst)
{
	long n;
	register int skip;

	switch (*src) {
	case 0:
		n = *(long far *)(src + 2);
		src += 6;
		skip = *(int far *)src;
		src += 2;
		src += skip;
		mem_copy(src, dst, n);
		break;
	case 3:
		n = unpack_3(FP_SEG(src), FP_SEG(dst));
		break;
	case 4:
		src += 6;
		skip = *(int far *)src;
		src += 2;
		src += skip;
		n = unpack_lcw(src, dst, -1);
		break;
	}
	return n;
}

/* 3069:0230 [asm] - empty */
void far fn_3069_0230(int a, unsigned b)
{
}
