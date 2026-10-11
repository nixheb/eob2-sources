/*
 * Eye of the Beholder II - START.EXE
 * Shapes drawn in a window: module 1DA3 (Ghidra 2DA3).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y shape.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * Each function selects the window dim (Dims[], DS:1CFC) if it is not the
 * current one, then calls a drawing entry of the video driver with
 * coordinates relative to the window.
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern int            CurDim;            /* 6298 the window selected */
/* Uninitialised data: in bssefd0.c (DS:EFD0-EFD9) */
extern int ClipY2;  /* EFD0 */
extern int ClipY;   /* EFD2 */
extern int ClipX8;  /* EFD4 in bytes */
extern int ClipX;   /* EFD6 in pixels */
extern int ClipX82; /* EFD8 in bytes */
extern void (far     *Drv_65CA)(int page, byte far *shape, int x, int y, int a);   /* 65CA */
extern void (far     *Drv_656A)(int page, byte far *shape, int x, int y);   /* 656A */
extern void (far     *Drv_65BE)(int page, byte far *shape, int x, int y);   /* 65BE */
extern void (far     *Drv_655E)(int page, byte far *shape, int x, int y);   /* 655E */
extern void (far     *Drv_65BA)(int page, byte far *shape, int x, int y);   /* 65BA */
extern void (far     *Drv_65A6)(int page, byte far *shape, int x, int y, int a);   /* 65A6 */
extern void (far     *Drv_65AE)(int page, byte far *shape, int x, int y, int a);   /* 65AE */
extern void (far     *Drv_65A2)(int page, byte far *shape, int x, int y, int a);   /* 65A2 */
extern void (far     *Drv_65AA)(int page, byte far *shape, int x, int y, int a);   /* 65AA */
extern void (far     *Drv_65DA)(int x1, int x2, int y1, int y2);   /* 65DA the clipping rectangle */
void far set_dim(int dim);

/* ------------------------------------------------------------------ */
/* Module 2DA3, in address order                                       */
/* ------------------------------------------------------------------ */

/* 2DA3:0007 [asm] */
void far put_shape_x(int page, byte far *shape, int x, int y, register int dim, int a)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65CA(page, shape, x + ClipX, y + ClipY, a);
	}
}

/* 2DA3:004A [asm] */
void far put_shape(int page, byte far *shape, int x, int y, register int dim)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_656A(page, shape, x + ClipX, y + ClipY);
	}
}

/* 2DA3:008A [asm] */
void far put_shape_mirrored(int page, byte far *shape, int x, int y, register int dim)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65BE(page, shape, x + ClipX, y + ClipY);
	}
}

/* 2DA3:00CA [asm] */
void far put_shape_2(int page, byte far *shape, int x, int y, register int dim)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_655E(page, shape, x + ClipX, y + ClipY);
	}
}

/* 2DA3:010A [asm] */
void far put_shape_3(int page, byte far *shape, int x, int y, register int dim)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65BA(page, shape, x + ClipX, y + ClipY);
	}
}

/* 2DA3:014A [asm] */
void far put_shape_4(int page, byte far *shape, int x, int y, register int dim, int a)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65A6(page, shape, x + ClipX, y + ClipY, a);
	}
}

/* 2DA3:018D [asm] */
void far put_shape_5(int page, byte far *shape, int x, int y, register int dim, int a)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65AE(page, shape, x + ClipX, y + ClipY, a);
	}
}

/* 2DA3:01D0 [asm] */
void far put_shape_6(int page, byte far *shape, int x, int y, register int dim, int a)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65A2(page, shape, x + ClipX, y + ClipY, a);
	}
}

/* 2DA3:0213 [asm] */
void far put_shape_7(int page, byte far *shape, int x, int y, register int dim, int a)
{
	if (shape) {
		if (dim != CurDim)
			set_dim(dim);
		Drv_65AA(page, shape, x + ClipX, y + ClipY, a);
	}
}

/* 2DA3:0256 [asm] - select a window for the shapes */
void far set_dim(register int dim)
{
	CurDim = dim;
	ClipX8 = Dims[dim].x;
	ClipX = ClipX8 * 8;
	ClipX82 = ClipX8 + Dims[dim].w;
	ClipY = Dims[dim].y;
	ClipY2 = ClipY + Dims[dim].h;
	Drv_65DA(ClipX8, ClipX82, ClipY, ClipY2);
}
