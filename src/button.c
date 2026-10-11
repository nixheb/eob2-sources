/*
 * Eye of the Beholder II - START.EXE
 * The buttons of Westwood's GUI: lists, states: module 1D3D (Ghidra 2D3D).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y button.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * Button flags (+0E): 8 disabled; 1 / 0x100 / 0x1000 and 0x400 / 0x4000
 * say which mouse button "holds" when the button is pressed or released.
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern int            MouseKeys;         /* 6385 the left button: 1, 3, 0x81, 0x83 */
extern int            MouseKeys2;        /* 6387 the right button */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void gui_draw_button(Button far *b);                             /* 2F86:08D5 */

/* the end of gui_press, gui_release, gui_set (a macro?) */
#define BUTTON_DONE(b, call)                                           \
	gui_draw_button(b);                                                \
	if (!(MouseKeys & 0x80) && !(MouseKeys2 & 0x80)) {                 \
		if (b->flags & 1) {                                            \
			if (b->flags & 0x400) {                                    \
				if (MouseKeys != 3)                                    \
					MouseKeys = 0x83;                                  \
			} else if ((b->flags & 0x4000) && MouseKeys2 != 3)         \
				MouseKeys2 = 0x83;                                     \
		} else if (b->flags & 0x100) {                                 \
			if (MouseKeys != 1)                                        \
				MouseKeys = 0x81;                                      \
		} else if ((b->flags & 0x1000) && MouseKeys2 != 1)             \
			MouseKeys2 = 0x81;                                         \
	}                                                                  \
	if (call && b->callback)                                           \
		b->callback(b)

/* ------------------------------------------------------------------ */
/* Module 2D3D, in address order                                       */
/* ------------------------------------------------------------------ */

/* 2D3D:0009 [asm] - add a button at the end of a list */
Button far * far gui_add_button(Button far *list, Button far *b)
{
	Button far *head;

	if (list) {
		head = list;
		if (b) {
			while (list->next)
				list = list->next;
			list->next = b;
			b->next = 0;
		}
	} else
		head = b;
	return head;
}

/* 2D3D:007C [asm] - toggle a button */
Button far * far gui_press(Button far *b, int call)
{
	if (b && !(b->flags & 8)) {
		if (b->state & 1)
			b->state |= 8;
		else
			b->state &= ~8;
		b->state ^= 1;
		BUTTON_DONE(b, call);
	}
	return b;
}

/* 2D3D:016E [asm] */
Button far * far gui_disable(Button far *b)
{
	if (b && !(b->flags & 8)) {
		b->flags |= 8;
		gui_draw_button(b);
	}
	return b;
}

/* 2D3D:01A0 [asm] */
Button far * far gui_enable(Button far *b)
{
	if (b && (b->flags & 8)) {
		b->flags &= ~8;
		gui_draw_button(b);
	}
	return b;
}

/* 2D3D:01D2 [asm] - the button of a list with this index */
Button far * far gui_find(Button far *list, register unsigned index)
{
	for (; list; list = list->next)
		if (list->index == index)
			break;
	return list;
}

/* 2D3D:0205 [asm] */
Button far * far gui_release(Button far *b, int call)
{
	if (b && !(b->flags & 8)) {
		if (b->state & 1)
			b->state |= 8;
		else
			b->state &= ~8;
		b->state &= ~1;
		b->state &= ~2;
		b->state |= 4;
		if (b->state >> 0)                        /* sic: always true; ">> 0": a macro? (no other form compiles the same) */
			b->state |= 0x10;
		else
			b->state &= ~0x10;
		b->state &= ~4;
		BUTTON_DONE(b, call);
	}
	return b;
}

/* 2D3D:0325 [asm] */
Button far * far gui_set(Button far *b, int call)
{
	if (b && !(b->flags & 8)) {
		if (b->state & 1)
			b->state |= 8;
		else
			b->state &= ~8;
		b->state |= 1;
		BUTTON_DONE(b, call);
	}
	return b;
}

/* 2D3D:0417 [asm] - insert a button in a list sorted by index */
Button far * far gui_insert(Button far *list, Button far *b)
{
	Button far *head, far *prev;

	if (!list)
		return b;
	if (!b)
		return list;
	prev = head = list;
	if (b->index <= head->index) {
		b->next = list;
		return b;
	}
	while (b->index > list->index && list->next) {
		prev = list;
		list = list->next;
	}
	if (b->index > list->index)
		list = gui_add_button(head, b);
	else {
		prev->next = b;
		b->next = list;
	}
	return head;
}

/* 2D3D:0504 [asm] */
int far gui_enabled(Button far *b)
{
	return b ? !(b->flags & 8) : 0;
}

/* 2D3D:0524 [asm] */
int far gui_pressed(Button far *b)
{
	return b ? b->state & 1 : 0;
}

/* 2D3D:053F [asm] */
int far gui_state2(Button far *b)
{
	return b ? b->state & 2 : 0;
}
