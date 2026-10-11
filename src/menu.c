/*
 * Eye of the Beholder II - START.EXE
 * The menus: a list of lines, some of them hidden by a mask, chosen with
 * the keyboard or the mouse: module 08A8 (Ghidra 28A8).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y menu.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A menu shows count lines taken from a table of strings: with a mask,
 * line n is the n-th string whose bit (n + shift) is set (all of them if
 * the mask is -1).
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Menu           Menus[];           /* 1E2C */
extern int            WinX, WinY;        /* F104 (characters), F106: the text window */
extern char           CharWidth;         /* 6288 */
extern char           LineHeight;        /* 6289 */
extern int            VideoMode;         /* 628A */
extern unsigned char  InventoryMode;          /* 6655 */
extern int            Var_6616;          /* 6616 */
/* Uninitialised data of the module (DS:EF4E-EF51, _BSS), in address order */
int MenuKey;     /* EF4E the key not used by menu_choose */
int MenuSpacing; /* EF50 the spacing of the last menu drawn */
extern int            ClickDown[2];      /* 661A */
extern int            ClickUp[2];        /* 6622 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
void flush_keys(void);                                           /* 3295:0516 */
int  key_waiting(void);                                          /* 3295:0507 */
int  read_key(void);                                             /* 3295:066D */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

/* ------------------------------------------------------------------ */
/* Module 28A8, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5D60) */
int MenuMouse = 1;                       /* 5D60 the mouse selects the line under it */

/* 28A8:0008 [asm] - the string of line n */
int far menu_item(register int n, unsigned long mask, int shift)
{
	int i;

	if (mask == 0xFFFFFFFFL)
		return n;
	for (i = 0; n; i++)
		if (mask & (1L << (i + shift)))
			n--;
	while (!(mask & (1L << (i + shift))))
		i++;
	return i;
}

/* 28A8:0079 [asm] - the chosen line blinks 3 times */
void far flash_text(char far *s, int x, int y, char colour, char hilite, char bg)
{
	register int i;

	for (i = 0; i < 3; i++) {
		hide_mouse();
		Drv_65F2(s, x, y, hilite, bg);
		wait_ticks(2);
		Drv_65F2(s, x, y, colour, bg);
		show_mouse();
		wait_ticks(2);
	}
}

/* 28A8:00E5 [asm] */
int far in_box(register int x, register int y, int x1, int y1, int x2, int y2)
{
	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return 1;
	else
		return 0;
}

/* 28A8:010F [asm] - the number of lines shown */
int far menu_count(int n, unsigned long mask, int shift)
{
	int i;
	register int count;

	if (mask == 0xFFFFFFFFL)
		return n;
	for (i = count = 0; i < n; i++)
		if (mask & (1 << (i + shift)))
			count++;
	return count;
}

/* 28A8:015D [asm] */
void far draw_menu(int menu, char far * far *items, unsigned long mask, int shift, int spacing)
{
	Menu far *m;
	int y0, x0, sel, count, y;
	register int i, item;

	m = &Menus[menu];
	y0 = WinY + m->y;
	x0 = (WinX + m->x) * 8;
	sel = menu_item(m->sel, mask, shift);
	count = m->count;
	hide_mouse();
	for (i = 0; i < count; i++) {
		item = menu_item(i, mask, shift);
		y = y0 + LineHeight * i + i * spacing;
		print_text(items[item], x0, y, VideoMode == 0 ? 1 : m->colour, 0);
		if (item == sel && (MenuMouse || InventoryMode))
			Drv_65F2(items[item], x0, y, m->hilite, 0);
	}
	MenuSpacing = spacing;
	show_mouse();
	flush_keys();
	Var_6616 = 0;
}

/* 28A8:027E [asm] - one turn of a menu: the arrows (0x60 up, 0x62 down,
 * 0x5B/0x65 first, 0x5D/0x67 last), Return (0x2B, 0x3D, 0x61) or a click
 * (0x41, 0x42) choose. Returns the string chosen, or -1 (the other keys
 * are left in MenuKey). */
int far menu_choose(int menu, char far * far *items, char far *unused, unsigned long mask, int shift)
{
	int last, key, y0, x0, x1, x2, y1, y2, my, y, half;
	char colour, hilite, old, cur, item;
	Menu far *m;
	register int lineH, chosen;

	unused++;                                     /* sic: never used */
	m = &Menus[menu];
	last = m->count - 1;
	cur = old = m->sel % (last + 1);
	chosen = -1;
	lineH = LineHeight + MenuSpacing;
	half = MenuSpacing >> 1;
	y0 = WinY + m->y;
	x0 = (WinX + m->x) * 8;
	colour = VideoMode == 0 ? 1 : m->colour;
	hilite = m->hilite;
	key = 0;
	MenuKey = 0;
	if (key_waiting())
		key = read_key() & 0x8FF;
	x1 = WinX * 8 + m->x * CharWidth;
	y1 = WinY + m->y - half;
	x2 = x1 + m->w * CharWidth - 1;
	y2 = y1 + m->count * lineH - 1;
	if (!InventoryMode) {
		my = ClickDown[1];
		if (in_box(ClickDown[0], my, x1, y1, x2, y2) && MenuMouse)
			cur = (my - y1) / lineH;
	}
	switch (key) {
	case 0x60:
		cur--;
		if (cur < 0)
			cur = last;
		break;
	case 0x62:
		cur++;
		if (cur > last)
			cur = 0;
		break;
	case 0x5B:
	case 0x65:
		cur = 0;
		break;
	case 0x5D:
	case 0x67:
		cur = last;
		break;
	case 0x41:
	case 0x42:
		if (in_box(ClickUp[0], ClickUp[1], x1, y1, x2, y2))
			cur = (ClickUp[1] - y1) / lineH;
		else {
			MenuKey = key;
			break;
		}
	case 0x2B:
	case 0x3D:
	case 0x61:
		chosen = cur;
		break;
	default:
		MenuKey = key;
	}
	if (cur != old) {
		hide_mouse();
		item = menu_item(old, mask, shift);
		y = y0 + old * lineH;
		Drv_65F2(items[item], x0, y, colour, 0);
		item = menu_item(cur, mask, shift);
		y = y0 + cur * lineH;
		Drv_65F2(items[item], x0, y, hilite, 0);
		show_mouse();
	}
	if (chosen != -1) {
		item = menu_item(chosen, mask, shift);
		hide_mouse();
		y = y0 + cur * lineH;
		flash_text(items[item], x0, y, colour, hilite, 0);
		show_mouse();
		chosen = item;
	}
	m->sel = cur;
	return chosen;
}
