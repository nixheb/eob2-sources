/*
 * Eye of the Beholder II - START.EXE
 * A rectangle copied from page to page pixel by pixel, in a random order
 * (the dissolve of the portal and of the dialogue pictures); overlay module
 * (Ghidra 56DD:1880 = 5865:0000), right after the dialogues.
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -Y copypix.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */

typedef unsigned char byte;

extern byte far      *ShrinkBuffer;      /* CBC4 a work buffer */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int (far      *Drv_657E)(int x, int y);   /* 657E reads a pixel */
extern void (far     *Drv_65B2)(int x, int y, int colour);      /* 65B2 a point */
int  random_range(int min, int max);                             /* 3064:0004 */

/* 56DD:1880 The w x h rectangle at (x, y) of page from to (x2, y2) of page
 * to: the columns and the rows are shuffled, then each pixel is copied,
 * row by row, the row index turning with the column */
void far copy_pixels(int x, int y, int x2, int y2, unsigned w, unsigned h, int from, int to)
{
	int far *xs, far *ys;
	int t;
	unsigned n;
	int j, k;
	int sx, sy, dx, dy;
	int c, page;
	register unsigned i, row;

	xs = (int far *)ShrinkBuffer;
	ys = xs + 0x140;
	for (i = 0; i < w; i++)
		xs[i] = i;
	for (i = 0; i < h; i++)
		ys[i] = i;
	for (i = 0; i < w; i++) {
		j = random_range(0, w - 1);
		k = i;
		t = xs[j];
		xs[j] = xs[k];
		xs[k] = t;
	}
	for (i = 0; i < h; i++) {
		j = random_range(0, h - 1);
		k = i;
		t = ys[j];
		ys[j] = ys[k];
		ys[k] = t;
	}
	page = Var_62A5;
	for (n = 0; n < h; n++) {
		row = n;
		for (i = 0; i < w; i++) {
			sx = x + xs[i];
			sy = y + ys[row];
			dx = x2 + xs[i];
			dy = y2 + ys[row];
			row++;
			if (row >= h)
				row = 0;
			Var_62A5 = from;
			c = Drv_657E(sx, sy);
			Var_62A5 = to;
			Drv_65B2(dx, dy, c);
		}
	}
	Var_62A5 = page;
}
