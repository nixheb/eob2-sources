/*
 * Eye of the Beholder II - START.EXE
 * The buttons of the interface and the lists of buttons of each screen:
 * module 0DFE (Ghidra 1DFE).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y gui.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The module is mostly data (6 820 bytes): 96 buttons (struct Button,
 * eob2.h), then the 8 lists. A screen sets its list: the first button
 * starts it, the others are added by gui_add_button().
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Button far    *GameGui;           /* EEE0 the buttons of the screen */
extern char           BookOpen;         /* E441 the spell book is open */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
Button far *gui_add_button(Button far *list, Button far *b);     /* 2D3D:0009 */
/* the callbacks: mainloop.c, spells.c, script.c, and an overlay (camp, the stub 3AD2:0020 of camp, 4F5F:1BBC: camp.c) */
int far portrait_select();
int far camp();
int far floor_click();
int far portrait_inventory();
int far portrait_hand();
int far portrait_swap();
int far inventory_slot();
int far eat_click();
int far inventory_toggle();
int far inventory_close();
int far move_forward();
int far move_back();
int far move_left();
int far move_right();
int far turn_left();
int far turn_right();
int far portrait_unselect();
int far throw_click();
int far click_front_wall();
int far inventory_prev();
int far inventory_next();
int far book_level_click();
int far book_click();
int far target_chosen();
int far close_spell_book();
int far book_turn_page();

/* ------------------------------------------------------------------ */
/* Module 1DFE, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:39B8...; each module's data is word aligned) */

/* The buttons (DS:39B8): next, index, key, key2, val0-2, unk0D, flags,
 * shape0-2, dim, x, y, w, h, colours, state, callback, page0, page1, arg.
 *   0     (empty)                     1-4    the portraits, characters 1-4 (select)
 *   5     camp                        6-9    the floor (4 places)
 *   10-13 the faces (inventory)       14-17  the hands
 *   18-21 the names (swap)            22-46  the 25 inventory slots, 47 eat
 *   48    inventory / statistics      49     the big face: close
 *   50-55 forward, back, left, right, turn left, turn right
 *   56    the view: unselect          57-58  the view: throw (left, right)
 *   59    the view: the wall ahead    60-61  previous / next character
 *   62-72 the spell book: levels 1-5, the 6 spell lines
 *   73-76 the target of a spell       77-78  the spell book: hidden keys
 *   79-92 the same for characters 5 and 6, added at the end (the places of
 *         the NPCs: the player creates 4), 85-86 slots 25-26 (the rings)
 *   93    a hidden key: next character  94  level 6  95 turn the page
 * The keys are Westwood key codes (0x70-0x75: the 6 portraits...). */
Button Buttons[96] = {
	/*  0 39B8 */ { 0, 1, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 0, 0, 0, 0, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, 0, 0, 0, 0 },
	/*  1 39F4 */ { 0, 1, 0x70, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 2, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 0 },
	/*  2 3A30 */ { 0, 2, 0x71, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 2, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 1 },
	/*  3 3A6C */ { 0, 3, 0x72, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 54, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 2 },
	/*  4 3AA8 */ { 0, 4, 0x73, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 54, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 3 },
	/*  5 3AE4 */ { 0, 5, 0x30, 0x6E, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 289, 177, 31, 21, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, camp, 0, 0, 0 },
	/*  6 3B20 */ { 0, 6, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 0, 102, 88, 18, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, floor_click, 0, 0, 0 },
	/*  7 3B5C */ { 0, 7, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 89, 102, 88, 18, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, floor_click, 0, 0, 1 },
	/*  8 3B98 */ { 0, 8, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 0, 72, 88, 29, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, floor_click, 0, 0, 2 },
	/*  9 3BD4 */ { 0, 9, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 89, 72, 88, 29, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, floor_click, 0, 0, 3 },
	/* 10 3C10 */ { 0, 10, 0x18, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 10, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 0 },
	/* 11 3C4C */ { 0, 11, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 10, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 1 },
	/* 12 3C88 */ { 0, 12, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 62, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 2 },
	/* 13 3CC4 */ { 0, 13, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 62, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 3 },
	/* 14 3D00 */ { 0, 14, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 216, 10, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 0 },
	/* 15 3D3C */ { 0, 15, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 288, 10, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 1 },
	/* 16 3D78 */ { 0, 16, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 216, 62, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 2 },
	/* 17 3DB4 */ { 0, 17, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 288, 62, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 3 },
	/* 18 3DF0 */ { 0, 18, 0x170, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 184, 2, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 0 },
	/* 19 3E2C */ { 0, 19, 0x171, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 256, 2, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 1 },
	/* 20 3E68 */ { 0, 20, 0x172, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 184, 54, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 2 },
	/* 21 3EA4 */ { 0, 21, 0x173, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 256, 54, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 3 },
	/* 22 3EE0 */ { 0, 39, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 230, 116, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 0 },
	/* 23 3F1C */ { 0, 43, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 278, 116, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 1 },
	/* 24 3F58 */ { 0, 22, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 40, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 2 },
	/* 25 3F94 */ { 0, 23, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 40, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 3 },
	/* 26 3FD0 */ { 0, 24, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 58, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 4 },
	/* 27 400C */ { 0, 25, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 58, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 5 },
	/* 28 4048 */ { 0, 26, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 76, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 6 },
	/* 29 4084 */ { 0, 27, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 76, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 7 },
	/* 30 40C0 */ { 0, 28, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 94, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 8 },
	/* 31 40FC */ { 0, 29, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 94, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 9 },
	/* 32 4138 */ { 0, 30, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 112, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 10 },
	/* 33 4174 */ { 0, 31, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 112, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 11 },
	/* 34 41B0 */ { 0, 32, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 130, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 12 },
	/* 35 41EC */ { 0, 33, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 130, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 13 },
	/* 36 4228 */ { 0, 34, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 148, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 14 },
	/* 37 4264 */ { 0, 35, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 199, 148, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 15 },
	/* 38 42A0 */ { 0, 36, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 225, 55, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 16 },
	/* 39 42DC */ { 0, 37, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 224, 76, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 17 },
	/* 40 4318 */ { 0, 38, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 225, 96, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 18 },
	/* 41 4354 */ { 0, 41, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 298, 55, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 19 },
	/* 42 4390 */ { 0, 42, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 287, 75, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 20 },
	/* 43 43CC */ { 0, 40, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 277, 137, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 21 },
	/* 44 4408 */ { 0, 44, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 300, 94, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 22 },
	/* 45 4444 */ { 0, 45, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 300, 112, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 23 },
	/* 46 4480 */ { 0, 46, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 300, 130, 16, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 24 },
	/* 47 44BC */ { 0, 48, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 236, 37, 31, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, eat_click, 0, 0, 25 },
	/* 48 44F8 */ { 0, 49, 0x1A, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 291, 149, 25, 17, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_toggle, 0, 0, 25 },
	/* 49 4534 */ { 0, 50, 0x6E, 0x18, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 181, 3, 32, 32, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_close, 0, 0, 25 },
	/* 50 4570 */ { 0, 51, 0x60, 0x160, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 24, 128, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, move_forward, 0, 0, 25 },
	/* 51 45AC */ { 0, 52, 0x62, 0x61, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 24, 144, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, move_back, 0, 0, 25 },
	/* 52 45E8 */ { 0, 53, 0x5C, 0x15C, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 3, 144, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, move_left, 0, 0, 25 },
	/* 53 4624 */ { 0, 54, 0x66, 0x166, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 45, 144, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, move_right, 0, 0, 25 },
	/* 54 4660 */ { 0, 55, 0x5B, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 3, 128, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, turn_left, 0, 0, 25 },
	/* 55 469C */ { 0, 56, 0x65, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 45, 128, 21, 16, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, turn_right, 0, 0, 25 },
	/* 56 46D8 */ { 0, 57, 0x6E, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 0, 136, 120, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_unselect, 0, 0, 0 },
	/* 57 4714 */ { 0, 58, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 0, 8, 88, 48, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, throw_click, 0, 0, 0 },
	/* 58 4750 */ { 0, 59, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 88, 8, 88, 48, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, throw_click, 0, 0, 1 },
	/* 59 478C */ { 0, 60, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 24, 8, 128, 96, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, click_front_wall, 0, 0, 1 },
	/* 60 47C8 */ { 0, 61, 0x70, 0x71, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 274, 35, 20, 15, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_prev, 0, 0, 1 },
	/* 61 4804 */ { 0, 62, 0x72, 0x73, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 297, 35, 20, 15, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_next, 0, 0, 1 },
	/* 62 4840 */ { 0, 63, 0x02, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 68, 121, 18, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 0 },
	/* 63 487C */ { 0, 64, 0x03, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 86, 121, 18, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 1 },
	/* 64 48B8 */ { 0, 65, 0x04, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 104, 121, 15, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 2 },
	/* 65 48F4 */ { 0, 66, 0x05, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 122, 121, 15, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 3 },
	/* 66 4930 */ { 0, 67, 0x06, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 140, 121, 15, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 4 },
	/* 67 496C */ { 0, 68, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 131, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 0 },
	/* 68 49A8 */ { 0, 69, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 137, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 1 },
	/* 69 49E4 */ { 0, 70, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 143, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 2 },
	/* 70 4A20 */ { 0, 71, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 149, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 3 },
	/* 71 4A5C */ { 0, 72, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 155, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 4 },
	/* 72 4A98 */ { 0, 73, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 75, 161, 97, 6, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 5 },
	/* 73 4AD4 */ { 0, 74, 0x70, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 2, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 0 },
	/* 74 4B10 */ { 0, 75, 0x71, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 2, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 1 },
	/* 75 4B4C */ { 0, 76, 0x72, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 54, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 2 },
	/* 76 4B88 */ { 0, 77, 0x73, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 54, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 3 },
	/* 77 4BC4 */ { 0, 82, 0x35, 0x36, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 320, 200, 0, 0, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 6 },
	/* 78 4C00 */ { 0, 83, 0x3D, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 320, 200, 0, 0, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_click, 0, 0, 7 },
	/* 79 4C3C */ { 0, 84, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 114, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 4 },
	/* 80 4C78 */ { 0, 85, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 114, 33, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_inventory, 0, 0, 5 },
	/* 81 4CB4 */ { 0, 86, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 216, 114, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 4 },
	/* 82 4CF0 */ { 0, 87, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 288, 114, 31, 33, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_hand, 0, 0, 5 },
	/* 83 4D2C */ { 0, 88, 0x174, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 184, 106, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 4 },
	/* 84 4D68 */ { 0, 89, 0x175, 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 0, 256, 106, 63, 8, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_swap, 0, 0, 5 },
	/* 85 4DA4 */ { 0, 90, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 227, 135, 10, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 25 },
	/* 86 4DE0 */ { 0, 91, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 239, 135, 10, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_slot, 0, 0, 26 },
	/* 87 4E1C */ { 0, 92, 0x74, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 106, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 4 },
	/* 88 4E58 */ { 0, 93, 0x75, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 106, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, 5 },
	/* 89 4E94 */ { 0, 94, 0x6E, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 68, 168, 78, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, close_spell_book, 0, 0, 0 },
	/* 90 4ED0 */ { 0, 95, 0x6E, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 68, 168, 78, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, target_chosen, 0, 0, -1 },
	/* 91 4F0C */ { 0, 96, 0x74, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 184, 106, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 4 },
	/* 92 4F48 */ { 0, 97, 0x75, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 256, 106, 63, 50, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, portrait_select, 0, 0, 5 },
	/* 93 4F84 */ { 0, 98, 0x74, 0x75, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 320, 200, 1, 1, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, inventory_next, 0, 0, 2 },
	/* 94 4FC0 */ { 0, 99, 0x07, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 158, 121, 15, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_level_click, 0, 0, 5 },
	/* 95 4FFC */ { 0, 101, 0, 0, 0, 0, 0, 0, 0x1100, 0, 0, 0, 0, 146, 168, 32, 10, { 12, 0, 15, 0, 15, 0, 8, 0 }, 0, book_turn_page, 0, 0, 0 }

};

Button far *MainButtons[38] = {   /* 5038 */
	&Buttons[59], &Buttons[1], &Buttons[2], &Buttons[3], &Buttons[4], &Buttons[91], &Buttons[92], &Buttons[5],
	&Buttons[6], &Buttons[7], &Buttons[8], &Buttons[9], &Buttons[10], &Buttons[11], &Buttons[12], &Buttons[13],
	&Buttons[79], &Buttons[80], &Buttons[14], &Buttons[15], &Buttons[16], &Buttons[17], &Buttons[81], &Buttons[82],
	&Buttons[18], &Buttons[19], &Buttons[20], &Buttons[21], &Buttons[83], &Buttons[84], &Buttons[50], &Buttons[51],
	&Buttons[52], &Buttons[53], &Buttons[54], &Buttons[55], &Buttons[57], &Buttons[58]
};
Button far *MainBookButtons[54] = {   /* 50D0 */
	&Buttons[59], &Buttons[62], &Buttons[63], &Buttons[64], &Buttons[65], &Buttons[66], &Buttons[94], &Buttons[95],
	&Buttons[67], &Buttons[68], &Buttons[69], &Buttons[70], &Buttons[71], &Buttons[72], &Buttons[77], &Buttons[78],
	&Buttons[89], &Buttons[1], &Buttons[2], &Buttons[3], &Buttons[4], &Buttons[91], &Buttons[92], &Buttons[5],
	&Buttons[6], &Buttons[7], &Buttons[8], &Buttons[9], &Buttons[10], &Buttons[11], &Buttons[12], &Buttons[13],
	&Buttons[79], &Buttons[80], &Buttons[14], &Buttons[15], &Buttons[16], &Buttons[17], &Buttons[81], &Buttons[82],
	&Buttons[18], &Buttons[19], &Buttons[20], &Buttons[21], &Buttons[83], &Buttons[84], &Buttons[50], &Buttons[51],
	&Buttons[52], &Buttons[53], &Buttons[54], &Buttons[55], &Buttons[57], &Buttons[58]
};
Button far *InventoryButtons[47] = {   /* 51A8 */
	&Buttons[59], &Buttons[22], &Buttons[23], &Buttons[24], &Buttons[25], &Buttons[26], &Buttons[27], &Buttons[28],
	&Buttons[29], &Buttons[30], &Buttons[31], &Buttons[32], &Buttons[33], &Buttons[34], &Buttons[35], &Buttons[36],
	&Buttons[37], &Buttons[38], &Buttons[39], &Buttons[40], &Buttons[41], &Buttons[42], &Buttons[43], &Buttons[44],
	&Buttons[45], &Buttons[46], &Buttons[85], &Buttons[86], &Buttons[47], &Buttons[48], &Buttons[49], &Buttons[61],
	&Buttons[60], &Buttons[93], &Buttons[5], &Buttons[6], &Buttons[7], &Buttons[8], &Buttons[9], &Buttons[50],
	&Buttons[51], &Buttons[52], &Buttons[53], &Buttons[54], &Buttons[55], &Buttons[57], &Buttons[58]
};
Button far *StatsButtons[19] = {   /* 5264 */
	&Buttons[59], &Buttons[48], &Buttons[49], &Buttons[61], &Buttons[60], &Buttons[93], &Buttons[5], &Buttons[6],
	&Buttons[7], &Buttons[8], &Buttons[9], &Buttons[50], &Buttons[51], &Buttons[52], &Buttons[53], &Buttons[54],
	&Buttons[55], &Buttons[57], &Buttons[58]
};
Button far *InventoryBookButtons[60] = {   /* 52B0 */
	&Buttons[59], &Buttons[62], &Buttons[63], &Buttons[64], &Buttons[65], &Buttons[66], &Buttons[94], &Buttons[67],
	&Buttons[68], &Buttons[69], &Buttons[70], &Buttons[71], &Buttons[72], &Buttons[89], &Buttons[22], &Buttons[23],
	&Buttons[24], &Buttons[25], &Buttons[26], &Buttons[27], &Buttons[28], &Buttons[29], &Buttons[30], &Buttons[31],
	&Buttons[32], &Buttons[33], &Buttons[34], &Buttons[35], &Buttons[36], &Buttons[37], &Buttons[38], &Buttons[39],
	&Buttons[40], &Buttons[41], &Buttons[42], &Buttons[43], &Buttons[44], &Buttons[45], &Buttons[46], &Buttons[85],
	&Buttons[86], &Buttons[47], &Buttons[48], &Buttons[49], &Buttons[61], &Buttons[60], &Buttons[93], &Buttons[5],
	&Buttons[6], &Buttons[7], &Buttons[8], &Buttons[9], &Buttons[50], &Buttons[51], &Buttons[52], &Buttons[53],
	&Buttons[54], &Buttons[55], &Buttons[57], &Buttons[58]
};
Button far *StatsBookButtons[33] = {   /* 53A0 */
	&Buttons[59], &Buttons[62], &Buttons[63], &Buttons[64], &Buttons[65], &Buttons[66], &Buttons[94], &Buttons[67],
	&Buttons[68], &Buttons[69], &Buttons[70], &Buttons[71], &Buttons[72], &Buttons[89], &Buttons[47], &Buttons[48],
	&Buttons[49], &Buttons[61], &Buttons[60], &Buttons[93], &Buttons[5], &Buttons[6], &Buttons[7], &Buttons[8],
	&Buttons[9], &Buttons[50], &Buttons[51], &Buttons[52], &Buttons[53], &Buttons[54], &Buttons[55], &Buttons[57],
	&Buttons[58]
};
Button far *SwapButtons[7] = {   /* 5424 */
	&Buttons[18], &Buttons[19], &Buttons[20], &Buttons[21], &Buttons[83], &Buttons[84], &Buttons[56]
};
Button far *TargetButtons[7] = {   /* 5440 */
	&Buttons[73], &Buttons[74], &Buttons[75], &Buttons[76], &Buttons[87], &Buttons[88], &Buttons[90]
};


/* 1DFE:000E [asm] - a new list of buttons */
void far gui_set_list(Button far *b)
{
	GameGui = b;
	GameGui->next = 0;
}

/* 1DFE:002F [asm] - the main screen */
void far gui_main(void)
{
	register int i;

	if (BookOpen) {
		gui_set_list(MainBookButtons[0]);
		for (i = 1; i < 54; i++)
			GameGui = gui_add_button(GameGui, MainBookButtons[i]);
	} else {
		gui_set_list(MainButtons[0]);
		for (i = 1; i < 38; i++)
			GameGui = gui_add_button(GameGui, MainButtons[i]);
	}
}

/* 1DFE:00BD [asm] - the inventory page */
void far gui_inventory(void)
{
	register int i;

	if (BookOpen) {
		gui_set_list(InventoryBookButtons[0]);
		for (i = 1; i < 60; i++)
			GameGui = gui_add_button(GameGui, InventoryBookButtons[i]);
	} else {
		gui_set_list(InventoryButtons[0]);
		for (i = 1; i < 47; i++)
			GameGui = gui_add_button(GameGui, InventoryButtons[i]);
	}
}

/* 1DFE:014B [asm] - the statistics page */
void far gui_stats(void)
{
	register int i;

	if (BookOpen) {
		gui_set_list(StatsBookButtons[0]);
		for (i = 1; i < 33; i++)
			GameGui = gui_add_button(GameGui, StatsBookButtons[i]);
	} else {
		gui_set_list(StatsButtons[0]);
		for (i = 1; i < 19; i++)
			GameGui = gui_add_button(GameGui, StatsButtons[i]);
	}
}

/* 1DFE:01D9 [asm] - swapping two characters: only the portraits */
void far gui_swap(void)
{
	register int i;

	gui_set_list(SwapButtons[0]);
	for (i = 1; i < 7; i++)
		GameGui = gui_add_button(GameGui, SwapButtons[i]);
}

/* 1DFE:021F [asm] - choosing the target of a spell */
void far gui_target(void)
{
	register int i;

	gui_set_list(TargetButtons[0]);
	for (i = 1; i < 7; i++)
		GameGui = gui_add_button(GameGui, TargetButtons[i]);
}
