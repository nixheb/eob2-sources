/*
 * Eye of the Beholder II - START.EXE
 * The game's toolbox: text input, waiting, dice, rectangles and frames,
 * shadowed text, pictures, files, the memory display: module 08D7
 * (Ghidra 18D7).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y lib.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* Borland's random() (stdlib.h, RAND_MAX = 0x7FFFU) */
int rand(void);
#define random(num) (int)(((long)rand() * (num)) / (0x7FFFU + 1))

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern volatile unsigned long TimerTicks; /* 667C */
/* Uninitialised data of the module (DS:EF1E-EF1F, _BSS), in address order */
char Blink; /* EF1E the cursor is shown */
extern char           Scratch[];         /* F16E */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            ClickUp[2];        /* 6622 where the mouse button went up (x, y) */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x in bytes) */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);  /* 2D92:000A */
int  key_pressed(void);                                          /* 3295:041E */
int  get_key(void);                                              /* 3295:05D0 */
void flush_keys(void);                                           /* 3295:0516 */
void put_shape(int page, byte far *shape, int x, int y, int dim);           /* 2DA3:004A */
void put_shape_mirrored(int page, byte far *shape, int x, int y, int dim);  /* 2DA3:008A */
void put_shape_2(int page, byte far *shape, int x, int y, int dim);         /* 2DA3:00CA */
void put_shape_3(int page, byte far *shape, int x, int y, int dim);         /* 2DA3:010A */
void load_cps_file(char far *name, int page, int dest, unsigned char far *pal);   /* 3069:0044 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
long mem_free_total(void);                                       /* 2D0E:0249 */
void print_number(int n, int x, int y, int fg, int bg);          /* 3060:000E */
void system_shutdown(void);                                      /* 315F:0000 */
int  open_file(char far *name, int mode);                        /* 2DDE:0007 */
void read_file(int fd, void far *buf, long n);                   /* 2E12:000E */
void close_file(int fd);                                         /* 2E0E:0006 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void play_sound(int sound);                                      /* 2C9F:00AA */
unsigned strlen(char far *s);
int  toupper(int c);
void far *memmove(void far *dst, void far *src, unsigned n);
int  sprintf(char far *buf, char far *fmt, ...);
char far *itoa(int value, char far *s, int radix);
char far *ltoa(long value, char far *s, int radix);
int  heapcheck(void);
unsigned long farcoreleft(void);
void exit(int status);
extern void (far     *Drv_6566)(int x1, int y1, int x2, int y2, int colour);   /* 6566 a line */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A fills */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
void far print_text(char far *s, int x, int y, int fg, int bg);
void far show_memory(int x, int y, int wait);

/* ------------------------------------------------------------------ */
/* Module 18D7, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:2FF0...; each module's data is word aligned) */
char CursorText[3] = " ";                                                /* 2FF0 the character under the cursor */

/* 18D7:0004 [asm] - edit a line of text at (x * 8, y), max characters,
 * with a blinking cursor; the arrows move, backspace and delete erase,
 * the letters are upper-cased. Returns the length, -1 on Escape. */
int far edit_line(char far *text, char x, char y, char max, char fg, char bg, char cursorBg)
{
	char pos, len;
	unsigned long t;
	register int i, key;

	hide_mouse();
	len = strlen(text);
	if (len > max) {
		len = max;
		text[max] = 0;
	}
	pos = len;
	if (pos >= max)
		pos--;
	copy_page(x - 1, y, 0, 0xBF, max + 2, 9, 0, 2);
	print_text(text, x * 8, y, fg, bg);
	Blink = 1;
	t = TimerTicks + 2;
	if (pos < len)
		CursorText[0] = text[pos];
	else
		CursorText[0] = ' ';
	Drv_65F2(CursorText, (x + pos) * 8, y, fg, cursorBg);
	do {
		while (!key_pressed()) {
			if (t <= TimerTicks) {
				if (Blink) {
					copy_page(pos + 1, 0xBF, x + pos, y, 1, 9, 2, 0);
					print_text(CursorText, (x + pos) * 8, y, fg, bg);
					Blink = 0;
				} else {
					Blink = 1;
					Drv_65F2(CursorText, (x + pos) * 8, y, fg, cursorBg);
				}
				t = TimerTicks + 2;
			}
		}
		key = get_key() & 0xFF;
		if (key == 0xAD)
			key = 0x7F;
		if (key == 0x7F)
			key = 0x7F;
		if (key != 8 && key != 0x1B && key != 0xD && key != 0x7F && key != 0xB5 &&
		    key != 0xB3 && key >= 0x20 && key <= 0x7E) {
			if (pos != 0 || key != ' ') {
				if (pos < len) {
					for (i = max - 1; i >= pos; i--)
						text[i + 1] = text[i];
					text[pos] = toupper(key);
					pos++;
					if (len == max)
						text[max] = 0;
				} else {
					if (pos == max) {
						pos--;
						len--;
					}
					text[pos] = toupper(key);
					pos++;
					text[pos] = 0;
				}
				if (++len > max)
					len = max;
				if (pos > max - 1)
					pos = max - 1;
			}
		} else if (key == 8) {
			if (pos < len) {
				if (pos > 0) {
					for (i = pos; i < max; i++)
						text[i - 1] = text[i];
					pos--;
					text[--len] = 0;
				}
			} else if (len > 0) {
				text[--len] = 0;
				pos--;
			}
		} else if (key == 0xB5) {
			if (pos > 0)
				pos--;
		} else if (key == 0xB3) {
			if (pos < len && pos < max - 1)
				pos++;
		} else if (key == 0x7F && text[pos]) {
			memmove(&text[pos], &text[pos + 1], len - pos);
			len--;
		}
		copy_page(0, 0xBF, x - 1, y, max + 2, 9, 2, 0);
		print_text(text, x * 8, y, fg, bg);
		if (pos < len)
			CursorText[0] = text[pos];
		else
			CursorText[0] = ' ';
		if (Blink)
			Drv_65F2(CursorText, (x + pos) * 8, y, fg, cursorBg);
		else
			print_text(CursorText, (x + pos) * 8, y, fg, bg);
	} while (key != 0xD && key != 0x1B);
	show_mouse();
	return key == 0x1B ? -1 : len;
}

/* 18D7:04D2 [asm] - wait so many ticks (1/18 s) */
void far delay_ticks(int ticks)
{
	unsigned long end;

	end = TimerTicks + ticks;
	while (TimerTicks < end)
		;
}

/* 18D7:0505 [asm] - count dice of sides faces, plus add */
int far roll_dice(char count, char sides, char add)
{
	int total;
	register int r, i;

	total = 0;
	if (sides > 0 && count > 0)
		for (i = 0; i < count; i++) {
			r = random(sides) + 1;
			if (r > sides)
				r -= sides;
			total += r;
		}
	return total + add;
}

/* 18D7:0571 [asm] - a rectangle */
void far draw_rect(register int x1, register int y1, int x2, int y2, int colour)
{
	Drv_6566(x1, y1, x2, y1, colour);
	Drv_6566(x1, y2, x2, y2, colour);
	Drv_6566(x1, y1, x1, y2, colour);
	Drv_6566(x2, y1, x2, y2, colour);
}

/* 18D7:05C8 [asm] - load NAME.CPS into a page; mode -1: nothing more,
 * 0: the palette, and the page copied to the screen */
void far load_picture(char far *name, byte far *pal, int a, register int page, register int mode)
{
	sprintf(Scratch, "%s.%s", name, "CPS");
	load_cps_file(Scratch, a, page, 0L);
	if (mode != -1) {
		if (mode)
			Drv_654A(page, mode, pal);
		else {
			Drv_654A(page, 2, pal);
			copy_page(0, 0, 0, 0, 0x28, 200, 2, 0);
		}
	}
	pal++;                                        /* sic: never used */
}

/* 18D7:0657 [asm] */
void far load_cps(char far *name, int a, int page)
{
	load_picture(name, 0L, a, page, -1);
	Var_62A5 = 2;
}

/* 18D7:067F [asm] - is (x, y) in the rectangle */
int far in_rect(register int x, register int y, int x1, int y1, int x2, int y2)
{
	if (x < x1 || x > x2)
		return 0;
	else if (y < y1 || y > y2)
		return 0;
	else
		return 1;
}

/* 18D7:06AD [asm] - a shape at (x, y) of the window dim */
void far draw_shape(int page, byte far *shape, register int x, register int y, int dim)
{
	x -= Dims[dim].x * 8;
	y -= Dims[dim].y;
	put_shape(page, shape, x, y, dim);
}

/* 18D7:06EE [asm] */
void far draw_shape_mirrored(int page, byte far *shape, register int x, register int y, int dim)
{
	x -= Dims[dim].x * 8;
	y -= Dims[dim].y;
	put_shape_mirrored(page, shape, x, y, dim);
}

/* 18D7:072F [asm] */
void far draw_shape_2(int page, byte far *shape, register int x, register int y, int dim)
{
	x -= Dims[dim].x * 8;
	y -= Dims[dim].y;
	put_shape_2(page, shape, x, y, dim);
}

/* 18D7:0770 [asm] */
void far draw_shape_3(int page, byte far *shape, register int x, register int y, int dim)
{
	x -= Dims[dim].x * 8;
	y -= Dims[dim].y;
	put_shape_3(page, shape, x, y, dim);
}

/* 18D7:07B1 [asm] - a box, light top and right, dark left and bottom */
void far draw_box(register int x1, register int y1, int x2, int y2, int light, int dark)
{
	Drv_6566(x1, y1, x2, y1, light);
	Drv_6566(x1, y1, x1, y2, dark);
	Drv_6566(x1, y2, x2, y2, dark);
	Drv_6566(x2, y1, x2, y2, light);
}

/* 18D7:0808 [asm] */
void far swap_bytes(char far *a, char far *b)
{
	char t;

	t = *a;
	*a = *b;
	*b = t;
}

/* 18D7:082F [asm] - swap two blocks through tmp */
void far swap_mem(void far *a, void far *b, void far *tmp, register int n)
{
	mem_copy(a, tmp, n);
	mem_copy(b, a, n);
	mem_copy(tmp, b, n);
}

/* 18D7:0884 [asm] - the free memory (a debugging aid) */
void far show_memory(int x, int y, int wait)
{
	int page;

	page = Var_62A5;
	Var_62A5 = 0;
	if (heapcheck() >= 0)
		sprintf(Scratch, "%lu:%lu", mem_free_total(), farcoreleft());
	else
		sprintf(Scratch, "HEAP CORRUPT!! (free memory) = %lu\n", mem_free_total());
	Drv_65F2(Scratch, x, y, 15, 12);
	if (wait)
		get_key();
	Var_62A5 = page;
}

/* 18D7:0908 [asm] - a text with a shadow */
void far print_text(char far *s, register int x, register int y, int fg, int bg)
{
	Drv_65F2(s, x - 1, y, 12, bg);
	Drv_65F2(s, x, y + 1, 12, 0);
	Drv_65F2(s, x - 1, y + 1, 12, 0);
	Drv_65F2(s, x, y, fg, 0);
}

/* 18D7:097A [asm] - a number with a shadow */
void far print_number_shadow(register int n, register int x, int y, int fg, int bg)
{
	print_number(n, x - 1, y, 12, bg);
	print_number(n, x, y + 1, 12, 0);
	print_number(n, x - 1, y + 1, 12, 0);
	print_number(n, x, y, fg, 0);
}

/* 18D7:09E2 [asm] - half the width of a number, in pixels */
int far long_width(unsigned long n)
{
	char buf[20];

	ltoa(n, buf, 10);
	return strlen(buf) * 3;
}

/* 18D7:0A15 [asm] */
int far number_width(int n)
{
	char buf[20];

	itoa(n, buf, 10);
	return strlen(buf) * 3;
}

/* 18D7:0A45 [asm] - a 3D frame, filled unless fill is -1 */
void far draw_frame3d(register int x1, register int y1, int x2, int y2, int light, int dark, int fill)
{
	if (fill != -1)
		Drv_657A(x1 + 1, y1 + 1, x2 - 1, y2 - 1, fill);
	Drv_6566(x1 + 1, y1, x2, y1, light);
	Drv_6566(x2, y1, x2, y2 - 1, light);
	Drv_6566(x1, y1, x1, y2, dark);
	Drv_6566(x1, y2, x2, y2, dark);
}

/* 18D7:0AC3 [asm] */
void far quit(int code)
{
	system_shutdown();
	exit(code);
}

/* 18D7:0AD7 [asm] - was the mouse released in the rectangle */
int far mouse_in_rect(int x1, int y1, int x2, int y2)
{
	return in_rect(ClickUp[0], ClickUp[1], x1, y1, x2, y2);
}

/* 18D7:0AF7 [asm] - read a whole file */
void far load_file(char far *name, void far *buf, long n)
{
	register int fd;

	fd = open_file(name, 1);
	read_file(fd, buf, n);
	close_file(fd);
}

/* 18D7:0B2F [asm] - keep v in [min, max]: clamp, or wrap around */
int far clamp(register int v, register int min, register int max, register int wrap)
{
	if (v < min) {
		if (wrap)
			v = max;
		else
			v = min;
	}
	if (v > max) {
		if (!wrap)
			v = max;
		else
			v = min;
	}
	return v;
}

/* 18D7:0B62 [asm] - a debugging message with the free memory */
void far debug_text(char far *s, register int x, register int y)
{
	Drv_65F2(s, x, y, 15, 12);
	show_memory(x, y + 8, 1);
}

/* 18D7:0B9A [asm] - wait some seconds, then a key */
void far wait_key(int seconds)
{
	unsigned long end;

	end = TimerTicks + seconds * 18;
	while (end > TimerTicks)
		;
	flush_keys();
	while (!key_pressed())
		;
	flush_keys();
}

/* 18D7:0BE6 [asm] - a message, with the "beep" */
void far message(char far *text)
{
	gui_printf(text);
	play_sound(0x4F);
}
