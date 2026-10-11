/*
 * Eye of the Beholder II - START.EXE
 * Westwood's GUI: the mouse and the keys on a list of buttons, the drawing
 * of a button: module 1F86 (Ghidra 2F86).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y guiproc.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * MouseKeys / MouseKeys2 (left / right button): 1 just pressed, 2 held,
 * 3 just released, 4 up. A button answers, by its flags:
 *   left  0x100 pressed, 0x200 held, 0x400 released, 0x800 up (hovered)
 *   right 0x1000, 0x2000, 0x4000, 0x8000 (LeftMasks / RightMasks below)
 *   1 does not toggle, 2 released when the mouse leaves, 4 lit (look2)
 *   under the mouse, 0x20 stops the list, 0x40 keeps the mouse ("grabbed")
 *   until released, 0x80 (with 0x40) lets it go when outside.
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern unsigned char  InventoryMode;     /* 6655 the keyboard moves the selection */
extern int            MouseButton;       /* F0FE the button of the last key: 1 left, 2 right */
extern int            MouseKeys;         /* 6385 */
extern int            MouseKeys2;        /* 6387 */
extern int            GuiX, GuiY;        /* 6379, 637B the mouse for the buttons */
extern int            ClickDown[2];      /* 661A */
extern int            ClickUp[2];        /* 6622 */
extern char           GuiDirty;          /* 6389 redraw the list */
extern int            Var_62A5;          /* 62A5 the page drawn on */
/* Uninitialised data: in bssf04c.c (DS:F04C-F051) */
extern int (far *ButtonHook)(Button far *b); /* F04C */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  key_ready(void);                                            /* 3295:049A */
int  get_ascii(void);                                            /* 3295:0621 */
int  key_waiting(void);                                          /* 3295:0507 */
int  read_key(void);                                             /* 3295:066D */
int  mouse_down(int button);                                     /* 3295:068B 0x41 left, 0x42 right */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
void draw_rect2(int x1, int y1, int x2, int y2, int colour);     /* 2D9E:0000 */
void mouse_hide_rect(int x1, int y1, int x2, int y2);            /* 338B:0206 */
void mouse_show_rect(void);                                      /* 338B:02CA */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_656E)(int page, int x, int y);         /* 656E copies from a page */
extern void (far     *Drv_6572)(int x1, int y1, int x2, int y2, int colour);   /* 6572 fills */
int far gui_handle(Button far *list, int key);
void far gui_draw_button(Button far *b);

/* ------------------------------------------------------------------ */
/* Module 2F86, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:6264...) */
Button far *LastList = 0;                /* 6264 the list drawn */
Button far *Grabbed = 0;                 /* 6268 the button that keeps the mouse */
int GuiActive = 1;                       /* 626C */
unsigned AnyFlags = 0;                   /* 626E what the list answers (flags & 0xAA04) */
unsigned LastFlags = 0xFFFF;             /* 6270 the flags of the button pressed */
unsigned LeftMasks[4] = { 0x100, 0x200, 0x400, 0x800 };                  /* 6272 */
unsigned RightMasks[4] = { 0x1000, 0x2000, 0x4000, 0x8000 };             /* 627A */

/* 2F86:000B [asm] - with an ASCII key (letters in upper case) */
int far gui_process_ascii(Button far *list)
{
	register unsigned key = 0;
	register int c;

	if (key_ready()) {
		key = get_ascii();
		key |= 0x8000;
		c = key & 0xFF;
		if (c >= 'a' && c <= 'z')
			key &= ~0x20;
	}
	return gui_handle(list, key);
}

/* 2F86:004E [asm] - with a key code (the letter keys without shift) */
int far gui_process(Button far *list)
{
	register unsigned key = 0;
	register int c;

	if (key_waiting()) {
		key = read_key();
		if (key & 0x100) {
			c = key & 0xFF;
			if ((c >= 0x11 && c <= 0x1A) || (c >= 0x1F && c <= 0x27) || (c >= 0x2E && c <= 0x34))
				key &= ~0x100;
		}
	}
	return gui_handle(list, key);
}

/* 2F86:00A5 [asm] - one turn of a list of buttons: returns the index |
 * 0x8000 of the button acted on, else the key.
 * The keys 0xC6/0xC7 (key < 0) and 'A'/'B' stand for clicks (0x800: the
 * release). The four lines "GuiX = ClickUp[0]..." are written twice: the
 * compiler merged them and left a dead jmp at 014D, which betrays it. */
int far gui_handle(Button far *list, int key)
{
	int more, result, fire, grabbed, clicked, lit, lmask, rmask, st, code, m, held;
	int dim, wx, wy, ww, wh, x, y;
	unsigned char c;
	register unsigned state, flags;

	clicked = 0;
	MouseButton = 0;
	if (!InventoryMode) {
		GuiX = ClickDown[0];
		GuiY = ClickDown[1];
		if (mouse_down(0x41))
			MouseKeys = 2;
		else
			MouseKeys = 4;
		if (mouse_down(0x42))
			MouseKeys2 = 2;
		else
			MouseKeys2 = 4;
		c = key & 0xFF;
		if (key < 0) {
			if (c >= 0xC6 && c <= 0xC7) {
				if (key & 0x800)
					st = 3;
				else
					st = 1;
				if (c == 0xC6)
					MouseKeys = st;
				else
					MouseKeys2 = st;
				GuiX = ClickUp[0];
				GuiY = ClickUp[1];
				clicked = 1;
				key = 0;
			} else
				key &= 0xFF;
		} else if (c >= 0x41 && c <= 0x42) {
			if (key & 0x800)
				st = 3;
			else
				st = 1;
			if (c == 0x41)
				MouseKeys = st;
			else
				MouseKeys2 = st;
			GuiX = ClickUp[0];
			GuiY = ClickUp[1];
			clicked = 1;
			key = 0;
		}
	}
	result = 0;
	more = 1;
	if (!list)
		return key;
	if (list != LastList || GuiDirty) {
		LastList = list;
		AnyFlags = 0;
		do {
			gui_draw_button(list);
			AnyFlags |= list->flags & 0xAA04;
			if ((list->state & 0x20) && *(void far * far *)&list->page0 && ButtonHook)
				ButtonHook(list);
			if (!list->next)
				more = 0;
			if (more)
				list = list->next;
		} while (more);
		Grabbed = 0;
		GuiActive = 1;
		GuiDirty = 0;
		LastFlags = -1;
	}
	dim = 0;
	wx = Dims[dim].x * 8;
	ww = Dims[dim].w * 8;
	wy = Dims[dim].y;
	wh = Dims[dim].h;
	grabbed = 0;
	held = 0;
	if (Grabbed)
		list = Grabbed;
	do {
		if (list->flags & 8) {
			more = (list = list->next) != 0;
			continue;
		}
		fire = 0;
		lit = 0;
		code = list->index | 0x8000;
		state = list->state;
		flags = list->flags;
		if (state & 1)
			state |= 8;
		else
			state &= ~8;
		if (state & 4)
			state |= 0x10;
		else
			state &= ~0x10;
		if (key) {
			if (list->key == key) {
				MouseButton = 1;
				MouseKeys = 1;
				state ^= 1;
				result = code;
				fire = 1;
			} else if (list->key2 == key) {
				MouseButton = 2;
				MouseKeys2 = 1;
				state ^= 1;
				result = code;
				fire = 1;
			}
		} else if (!InventoryMode) {
			if (AnyFlags || clicked) {
				lmask = flags & 0xF00;
				rmask = flags & 0xF000;
				if (GuiActive) {
					if (list->dim != dim) {
						dim = list->dim;
						wx = Dims[dim].x * 8;
						ww = Dims[dim].w * 8;
						wy = Dims[dim].y;
						wh = Dims[dim].h;
					}
					x = wx;
					if (list->x < 0)
						x += ww;
					x += list->x;
					y = wy;
					if (list->y < 0)
						y += wh;
					y += list->y;
					if (GuiX >= x && x + list->w > GuiX && GuiY >= y && y + list->h > GuiY) {
						state |= 2;
						if (lmask)
							switch (MouseKeys) {
							case 4:
								if (!(flags & 4) && list->look2)
									state |= 4;
								else
									state &= ~4;
								if (flags & 0x800)
									fire = 1;
								else if ((flags & 2) && (state & 1))
									state &= ~1;
								break;
							case 2:
								if (flags != LastFlags)
									break;
								if ((flags & 4) && list->look2) {
									state |= 4;
									lit = 1;
								} else
									state &= ~4;
								if (!(flags & 0x200))
									break;
								fire = 1;
								if (flags & 1)
									break;
								state |= 1;
								result = code;
								break;
							case 1:
								held = 1;
								if ((flags & 4) && list->look2) {
									state |= 4;
									lit = 1;
								} else
									state &= ~4;
								if (flags & 0x100) {
									fire = 1;
									if (!(flags & 1)) {
										state ^= 1;
										result = code;
									}
								}
								if (flags & 0x40) {
									Grabbed = list;
									grabbed = 1;
								}
								LastFlags = flags;
								break;
							case 3:
								if (flags != LastFlags)
									break;
								if (flags & 0x400) {
									fire = 1;
									if (flags & 1) {
										state ^= 1;
										result = code;
									}
								}
								if ((flags & 2) && (state & 1))
									state &= ~1;
							}
						if (rmask && !fire && !lit)
							switch (MouseKeys2) {
							case 4:
								if (!(flags & 4) && list->look2)
									state |= 4;
								else
									state &= ~4;
								if (flags & 0x8000)
									fire = 1;
								else if ((flags & 2) && (state & 1))
									state &= ~1;
								break;
							case 2:
								if (flags != LastFlags)
									break;
								if ((flags & 4) && list->look2)
									state |= 4;
								else
									state &= ~4;
								if (!(flags & 0x2000))
									break;
								fire = 1;
								if (flags & 1)
									break;
								state |= 1;
								result = code;
								break;
							case 1:
								held = 1;
								if ((flags & 4) && list->look2)
									state |= 4;
								else
									state &= ~4;
								if (flags & 0x1000) {
									fire = 1;
									if (!(flags & 1)) {
										state ^= 1;
										result = code;
									}
								}
								if (flags & 0x40) {
									Grabbed = list;
									grabbed = 1;
								}
								LastFlags = flags;
								break;
							case 3:
								if (flags != LastFlags)
									break;
								if (flags & 0x4000) {
									fire = 1;
									if (flags & 1) {
										state ^= 1;
										result = code;
									}
								}
								if ((flags & 2) && (state & 1))
									state &= ~1;
							}
					} else {
						state &= ~6;
						if ((flags & 0x40) && Grabbed && !grabbed && !(flags & 0x80)) {
							if (lmask) {
								m = LeftMasks[MouseKeys - 1];
								if (m & flags)
									fire = 1;
							}
							if (rmask && !fire) {
								m = RightMasks[MouseKeys2 - 1];
								if (m & flags)
									fire = 1;
							}
							if (!fire) {
								Grabbed = 0;
								GuiActive = 1;
							}
						}
						if ((flags & 2) && (state & 1))
							state &= ~1;
					}
				}
			}
		}
		list->state = state;
		list->flags = flags;
		if (!(state & 8) != !(state & 1) || !(state & 0x10) != !(state & 4))
			gui_draw_button(list);
		if (fire && list->callback)
			more = !list->callback(list);
		if ((state & 2) && (flags & 0x20))
			more = 0;
		if (Grabbed && ((lmask && MouseKeys == 3) || (rmask && MouseKeys2 == 3))) {
			Grabbed = 0;
			GuiActive = 1;
			more = 0;
		}
		if (Grabbed && !grabbed)
			break;
		if (more && !(list = list->next))
			break;
	} while (more);
	if ((MouseKeys == 1 || MouseKeys2 == 1) && !held)
		LastFlags = -1;
	if (!result)
		result = key;
	return result;
}

/* 2F86:08D5 [asm] - draw a button in its state */
void far gui_draw_button(Button far *b)
{
	int t0, lit, pressed, t2, t1, dim, bx, by, waslit, waspressed, c, w, h, x2, y2, c2, c3;
	byte far *shape1;
	byte far *shape0;
	byte far *shape2;
	register int x, y;

	t0 = b->look0;
	t2 = b->look2;
	t1 = b->look1;
	if (!t0 && !t2 && !t1)
		return;
	if ((b->flags & 0x18) == 0x18)
		return;
	lit = b->state & 4;
	pressed = b->state & 1;
	waslit = b->state & 0x10;
	waspressed = b->state & 8;
	dim = b->dim;
	x = 0;
	if (b->x < 0)
		x += Dims[dim].w * 8;
	x += b->x;
	bx = x;
	x += Dims[dim].x * 8;
	y = 0;
	if (b->y < 0)
		y += Dims[dim].h;
	y += b->y;
	by = y;
	y += Dims[dim].y;
	w = b->w;
	h = b->h;
	shape0 = b->shape0;
	shape1 = b->shape1;
	shape2 = b->shape2;
	c2 = b->col[2];
	c3 = b->col[3];
	x2 = x + w - 1;
	y2 = y + h - 1;
	mouse_hide_rect(x, y, x + w, y + h);
	if (pressed) {
		if (t1 == 1) {
			if (t0 == 1)
				put_shape(Var_62A5, shape1, bx, by, dim);
			else if (t0 == 2) {
				if (!lit)
					Drv_65F2((char far *)shape1, x, y, c2, c3);
			} else if (t0 == 3)
				Drv_656E(b->page1, x >> 3, y);
			else if (t0 == 4) {
				if (shape1)
					((int (far *)())shape1)(b);
			}
		} else if (t1 == 2) {
			if (!lit)
				draw_rect2(x, y, x2, y2, c2);
		} else if (t1 == 3) {
			if (!lit)
				Drv_6572(x, y, x2, y2, c2);
		} else if (t1 == 4) {
			if (shape1)
				((int (far *)())shape1)(b);
		}
	}
	if (lit) {
		if (t2 == 1) {
			if (t0 == 1)
				put_shape(Var_62A5, shape2, bx, by, dim);
			else if (t0 == 2) {
				if (pressed)
					Drv_65F2((char far *)shape2, x, y, b->col[6], b->col[7]);
				else
					Drv_65F2((char far *)shape2, x, y, b->col[4], b->col[5]);
			} else if (t0 == 3)
				Drv_656E(b->page1, x >> 3, y);
			else if (t0 == 4) {
				if (shape2)
					((int (far *)())shape2)(b);
			}
		} else if (t2 == 2) {
			c = pressed ? b->col[6] : b->col[4];
			draw_rect2(x, y, x2, y2, c);
		} else if (t2 == 3) {
			c = pressed ? b->col[6] : b->col[4];
			Drv_6572(x, y, x2, y2, c);
		} else if (t2 == 4) {
			if (shape2)
				((int (far *)())shape2)(b);
		}
	}
	if (!lit && !pressed) {
		if (t0 == 1)
			put_shape(Var_62A5, shape0, bx, by, dim);
		else if (t0 == 2)
			Drv_65F2((char far *)shape0, x, y, b->col[0], b->col[1]);
		else if (t0 == 3)
			Drv_656E(b->page0, x >> 3, y);
		else if (t0 == 4) {
			if (shape0)
				((int (far *)())shape0)(b);
		} else if (t0 == 5)
			draw_rect2(x, y, x2, y2, b->col[0]);
		else if (t0 == 0) {
			if (t1 == 2 || t2 == 2)
				draw_rect2(x, y, x2, y2, b->col[0]);
			else if (t1 == 3 && waspressed)
				Drv_6572(x, y, x2, y2, c2);
			else if (t2 == 3 && waslit)
				Drv_6572(x, y, x2, y2, b->col[4]);
		}
	}
	mouse_show_rect();
}

/* 2F86:0D9C [asm] - called for the buttons 0x20 that have pages */
void far gui_set_hook(int unused, int (far *hook)(Button far *b))
{
	ButtonHook = hook;
}
