/*
 * Eye of the Beholder II - START.EXE
 * Pictures in Deluxe Paint's LBM format (IFF "ILBM" or "PBM "), or else CPS:
 * module 2095 (Ghidra 3095).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y lbm.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A page holds 320x200 pixels: one byte a pixel ("chunky", VGA), or in
 * EGA, 4 planes of 8000 bytes (40 bytes a line). An ILBM picture has its
 * planes line by line (each line: plane 0, plane 1...); a PBM one is
 * chunky.
 */
#include "eob2.h"

typedef unsigned char byte;

#define ID_FORM 0x4D524F46L              /* "FORM" */
#define ID_ILBM 0x4D424C49L              /* "ILBM" */
#define ID_PBM  0x204D4250L              /* "PBM " */
#define ID_BMHD 0x44484D42L              /* "BMHD" */
#define ID_CMAP 0x50414D43L              /* "CMAP" */
#define ID_BODY 0x59444F42L              /* "BODY" */

/* The header of the picture (big-endian words in the file) */
typedef struct BitMapHeader {
	unsigned      w, h;              /* -38 */
	int           x, y;
	char          planes;            /* -30 */
	unsigned char masking;           /* -2F 1: a mask plane follows the planes */
	unsigned char compression;       /* -2E 1: ByteRun1 */
	unsigned char pad;
	unsigned      transparent;
	unsigned char xAspect, yAspect;  /* -2A */
	unsigned      pageWidth, pageHeight;
} BitMapHeader;                          /* 20 bytes */

extern int            VideoMode;         /* 628A */
extern unsigned long  PageSizes[];       /* 62E7 */

void far *alloc_page(int page);                                  /* 2F3E:0009 */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
void mem_free(void far *p);                                      /* 2D0E:013F */
int  iff_open(char far *name);                                   /* 2F44:0008 */
void iff_close(int fd);                                          /* 2F44:008A */
long iff_read(int fd, long id, void far *buf, long max);         /* 2F44:01C9 */
unsigned long load_cps_file(char far *name, int page, int dest, byte far *pal);   /* 3069:0044 */
unsigned swap_word(unsigned v);                                  /* 33DF:0015 (assembler) */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */

/* 3095:000C [asm] - ILBM planes (page) to chunky pixels (dest) */
void far planes_to_chunky(int page, int dest, int planes)
{
	byte far *src;
	byte far *dst;
	int y, x;
	byte bits[8];
	byte pix;
	register int k, bit;

	src = alloc_page(page);
	dst = alloc_page(dest);
	memset(bits, 0, 8);
	for (y = 0; y < 200; y++) {
		for (x = 0; x < 40; x++) {
			for (k = 0; k < planes; k++)
				bits[k] = *(src + k * 40);
			src++;
			for (bit = 0; bit < 8; bit++) {
				pix = 0;
				for (k = planes - 1; k >= 0; k--) {
					pix <<= 1;
					if (bits[k] & 0x80)
						pix |= 1;
					bits[k] <<= 1;
				}
				*dst = pix;
				dst++;
			}
		}
		src += (planes - 1) * 40;
	}
}

/* 3095:00D1 [asm] - ILBM planes (page) to EGA planes (dest) */
void far planes_to_ega(int page, int dest, register int planes)
{
	int y;
	byte far *src;
	byte far *dst;
	register int k;

	src = alloc_page(page);
	dst = alloc_page(dest);
	for (y = 0; y < 200; y++) {
		for (k = 0; k < planes; k++) {
			mem_copy(src, dst + k * 8000, 40L);
			src += 40;
		}
		dst += 40;
	}
}

/* 3095:0149 [asm] - chunky pixels (page) to EGA planes (dest).
 * ORIGINAL BUG: the bit of pixel x is 1 << (x / 8) instead of
 * 0x80 >> (x & 7): a PBM picture comes out scrambled in EGA. */
void far chunky_to_ega(int page, int dest, int planes)
{
	int k;
	byte far *dst;
	byte far *src;
	byte pix, b;
	register int x, y;

	dst = alloc_page(dest);
	src = alloc_page(page);
	for (y = 0; y < 200; y++) {
		for (k = 0; k < planes; k++)
			memset(dst + y * 40 + k * 8000, 0, 40);
		for (x = 0; x < 320; x++) {
			pix = *(src + y * 320 + x);
			for (k = 0; k < planes; k++) {
				b = *(dst + y * 40 + k * 8000 + (x >> 3));
				if (pix & (1 << k))
					b |= 1 << (x / 8);   /* sic */
				*(dst + y * 40 + k * 8000 + (x >> 3)) = b;
			}
		}
	}
}

/* 3095:0264 [asm] - load a picture into page dest (page: work space), its
 * palette into pal (if any); mode 0: EGA planes, 1: chunky (VGA).
 * Returns the number of planes; 0 if it fails.
 * ORIGINAL BUG: the returns 0 after iff_open leave the file open. */
int far load_lbm(char far *name, int page, int dest, byte far *pal, int mode)
{
	int fd;
	long id;
	int pbm;
	int i;
	byte far *body;
	byte far *dst;
	char c;
	int n;
	byte far *buf;
	byte t;
	byte far *p;
	byte far *q;
	BitMapHeader bmhd;
	register unsigned left;
	register int k;

	fd = open_file(name, 1);
	if (fd == -1)
		return 0;
	read_file(fd, &id, 4L);
	close_file(fd);
	if (id != ID_FORM)
		return load_cps_file(name, page, dest,
			VideoMode == 6 || VideoMode == 3 || VideoMode == 5 ? pal : 0) / 8000;
	fd = iff_open(name);
	if (fd == -1)
		return 0;
	read_file(fd, &id, 4L);
	if (id == ID_ILBM)
		pbm = 0;
	else if (id == ID_PBM)
		pbm = 1;
	else
		return 0;
	if (iff_read(fd, ID_BMHD, &bmhd, 20L)) {
		bmhd.w = swap_word(bmhd.w);
		bmhd.h = swap_word(bmhd.h);
		bmhd.x = swap_word(bmhd.x);
		bmhd.y = swap_word(bmhd.y);
		bmhd.xAspect = swap_word(bmhd.xAspect);
		bmhd.yAspect = swap_word(bmhd.yAspect);
		bmhd.pageWidth = swap_word(bmhd.pageWidth);
		bmhd.pageHeight = swap_word(bmhd.pageHeight);
		if (bmhd.masking > 2)
			return 0;
		if (bmhd.compression > 1)
			return 0;
	} else
		return 0;
	if (pal) {
		n = (1 << bmhd.planes) * 3;
		buf = mem_alloc((long)n, 0x10);
		p = buf;
		q = pal;
		n = iff_read(fd, ID_CMAP, buf, (long)n);
		if (n && VideoMode == 3)
			switch (mode) {
			case 1:
				for (i = 0; i < n; i++) {
					*q = *p >> 2;
					p++;
					q++;
				}
				break;
			case 0:
				for (i = 0; i < n; i += 3) {
					*q = *p >> 4;
					p++;
					q++;
					t = *p & 0xF0;
					p++;
					t += *p >> 4;
					p++;
					*q = t;
					q++;
				}
			}
		mem_free(buf);
	}
	dst = alloc_page(page);
	body = alloc_page(dest);
	if (iff_read(fd, ID_BODY, body, PageSizes[dest])) {
		for (i = 0; i < bmhd.h; i++) {
			left = (bmhd.w >> 3) * bmhd.planes;
			if (bmhd.masking == 1)
				left = left + (bmhd.w >> 3);
			if (bmhd.compression == 1)
				while (left) {
					c = *body;
					body++;
					if (c == -128)
						continue;
					if (c >= 0) {
						k = c + 1;
						if (bmhd.masking != 1 || (bmhd.masking == 1 && (bmhd.w >> 3) < left)) {
							memcpy(dst, body, k);
							dst += k;
						}
						left -= k;
						body += k;
					} else {
						k = -c + 1;
						c = *body;
						body++;
						if (bmhd.masking != 1 || (bmhd.masking == 1 && (bmhd.w >> 3) < left)) {
							memset(dst, c, k);
							dst += k;
						}
						left -= k;
					}
				}
			else {
				memcpy(dst, body, left);
				dst += left;
				body += left;
			}
		}
		switch (mode) {
		case 1:
			if (!pbm)
				planes_to_chunky(page, dest, bmhd.planes);
			else
				mem_copy(alloc_page(page), alloc_page(dest), 64000L);
			break;
		case 0:
			if (!pbm)
				planes_to_ega(page, dest, bmhd.planes);
			else
				chunky_to_ega(page, dest, bmhd.planes);
			break;
		}
	}
	iff_close(fd);
	return bmhd.planes;
}
