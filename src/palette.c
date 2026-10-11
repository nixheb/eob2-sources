/*
 * Eye of the Beholder II - START.EXE
 * The palette: module 2111 (Ghidra 3111).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y palette.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A palette: 256 colours of 3 bytes (red, green, blue; 0-63 in VGA).
 * set_colour and fade_palette are called by nobody in START.EXE.
 */
#include "eob2.h"

extern int            VideoMode;         /* 628A */
extern volatile unsigned long TimerTicks; /* 667C */
extern void (far     *Drv_65E2)(char far *pal, int first, int last);   /* 65E2 sets colours */

void wait_retrace(int mode);                                      /* 3404:0019 (assembler) */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */

/* 3111:000D [asm] */
void far set_palette(char far *pal)
{
	Drv_65E2(pal, 0, 0xFF);
}

/* 3111:0026 [asm] - change colour i of pal (if any) and on the screen */
void far set_colour(char far *pal, register int i, char far *rgb)
{
	char far *p;

	if (pal) {
		p = pal;
		memcpy(p + i * 3, rgb, 3);
		Drv_65E2(pal, i, i);
	}
}

/* 3111:007A [asm] - move pal to target one step a colour at a time, in
 * about delay ticks (VGA only, else at once) */
void far fade_palette(char far *pal, char far *target, int delay)
{
	int changed, b;
	unsigned long end;
	int step, frac, d, t;
	register int i, a;

	if (target && pal) {
		if (VideoMode != 3)
			set_palette(target);
		else {
			step = delay << 8;
			for (i = 0; i < 0x300; i++) {
				a = target[i];
				b = pal[i];
				d = a - b;
				d = d < 0 ? -d : d;
				t = delay << 8;
				if (d)
					t = t / d;
				step = step < t ? step : t;
			}
			frac = 0;
			end = TimerTicks;
			do {
				changed = 0;
				frac += step;
				end += frac >> 8;
				frac &= 0xFF;
				for (i = 0; i < 0x300; i++) {
					a = target[i];
					b = pal[i];
					if (a != b) {
						if (b < a) {
							b++;
							changed = 1;
						}
						if (b > a) {
							b--;
							changed = 1;
						}
						pal[i] = b;
					}
				}
				if (changed) {
					wait_retrace(0);
					set_palette(pal);
					while (TimerTicks < end)
						;
				}
			} while (changed);
		}
	}
}
