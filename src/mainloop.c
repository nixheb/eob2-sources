/*
 * Eye of the Beholder II - START.EXE
 * The main loop and the player's actions: moving, turning, the buttons of
 * the inventory, the hands, the floor: module 0BB3 (Ghidra 1BB3).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y mainloop.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The click functions get the GUI button that was clicked: +0x38 its
 * parameter, +4 its identifier (returned).
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern Item           Items[600];        /* 71CA */
extern Character      Party[6];          /* E608 */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */
extern char           InvCharacter;      /* 9317 */
extern char           InvSlot;           /* E5A4 */
extern int            MouseButton;       /* F0FE */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern byte far      *LoadBuffer;          /* D0AC */
extern char           BookOpen;         /* E441 the spell book is open */
extern char           GuiState[];        /* E5CA */
extern char           SelectedChar;      /* EE20 the portrait clicked first (to swap), -1 */
extern char           SceneDirty;        /* D0AA */
extern int            GuiY;            /* 637B */
extern int            MouseKeys;         /* 6385 */
extern signed char    FrontPos[4][4];    /* 2342 */
extern char           Var_DF2C;          /* DF2C the party has moved */
extern char           Var_DF24;          /* DF24 */
extern char           Front;             /* 9444 */
extern char           WallTypeFlags[256];/* 2268 */
extern char           Var_CB76;          /* CB76 */
extern int            Var_DF4C;          /* DF4C */
extern char           Scratch[];         /* F16E */
extern byte           InventoryMode;          /* 6655 */
extern int            Var_E442;          /* E442 the first item picked up with the keyboard */
extern void far      *GameGui;           /* EEE0 */
extern unsigned long  GameTicks;         /* 6684 */
extern unsigned long  TimerTicks;        /* 667C */
extern unsigned long  Var_EEF2;          /* EEF2 */
/* Uninitialised data: in bssef2a.c (DS:EF2A-EF2F) */
extern unsigned long NextAmbient; /* EF2A the next ambient sound */
extern int PrevPartyBlock;        /* EF2E */
extern int            Var_6616;          /* 6616 */
extern char           Var_9316;          /* 9316 0: a new game */
extern int            HandItem;          /* 6DD6 the item in hand of a saved game */
extern int            CurrentLevel;      /* 6DCE */
extern int            CurrentSubLevel;   /* 6DD0 */
extern byte far      *GameFont;          /* 9342 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void message(char far *text);                                    /* 18D7:0BE6 */
int  sprintf(char far *buf, char far *fmt, ...);
void play_sound(int sound);                                      /* 2C9F:00AA */
void flush_keys(void);                                         /* 3295:0516 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void fn_1CBE_0B5B(int a);                                        /* 1CBE:0B5B */
void gui_main(void);                                         /* 1DFE:002F */
void gui_inventory(void);                                         /* 1DFE:00BD */
void gui_swap(void);                                         /* 1DFE:01D9 */
void gui_stats(void);                                         /* 1DFE:014B */
void inventory_click(int slot);                                  /* 1CBE:0CB7 */
void redraw_party(void);                                         /* 1E24:0686 */
void copy_page(int a, int b, int c, int d, int w, int h, int from, int to);  /* 2D92:000A */
int  close_spell_book(void far *gui);                            /* 20E2:14A7 */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
void set_timer(int n, int on);                                   /* 27B7:066D */
void swap_mem(void far *a, void far *b, void far *buf, unsigned n);   /* 18D7:082F swaps */
void select_char(int ch);                                        /* 1E24:0040 */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void fn_1E24_0086(int slot);                                     /* 1E24:0086 */
void fn_1E24_0075(int slot, int colour);                         /* 1E24:0075 */
void hand_click(int ch, int hand);                               /* 1CBE:0DA7 */
void use_item(int ch, int hand);                                 /* 1CBE:1140 */
int  get_hand_item(void);                                        /* 1E24:0352 */
void take_item(int item);                                    /* 18AE:0197 */
int  launch_item(int from, int item, int block, int pos, int dir, int type);   /* 1CBE:086A */
int  get_walkable_block(int block, char dir);                    /* 1E24:0091 */
void teleport_party(int block);                                  /* 24EC:0076 */
int  unlink_item(unsigned far *list, int pos, int item);         /* 1CBE:02BF */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
void run_block_script(int block, unsigned event);                /* 24EC:2BE9 */
void draw_food(int ch);                                       /* 2901:0504 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
void wall_click(int block, int dir);                             /* 24EC:0A81 */
void recompute_ac(int ch);                                       /* 1F55:0CF7 */
void draw_inventory_slot(int slot, int frame, int a);            /* 1CBE:09A6 */
int  item_fits_slot(int item, int ch, int slot);                 /* 1CBE:0B90 */
void place_item(int block, int item, int pos);                   /* 1CBE:0474 */
int  gui_process(void far *gui);                                 /* 2F86:004E */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  check_party_alive(int ask);                                 /* 1F55:09FE */
void run_timers(void);                                           /* 27B7:05A6 */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void play_sound_at(int sound, int block);                        /* 2A30:0014 */
void play_sound_here(int sound);                                 /* 2A30:0000 */
void load_level(int level, int sub);                             /* 3AE3:007F (overlay) */
void view_nothing(void);                                         /* 3ABA:005C (overlay) */
void draw_play_screen(int a);                                        /* 3AE3:0061 (overlay) */
void set_font(byte far *font);                                   /* 3166:000A */
int  select_window(int w);                                       /* 31B0:0133 */
void far level_start(void);
void far new_game_start(void);
extern void (far     *Drv_6542)(int x, int y, int w, int h, byte far *buf); /* 6542 */

/* ------------------------------------------------------------------ */
/* Module 1BB3, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:3502...) */
void (far *InvScreens[2])(void) = { gui_stats, gui_inventory };       /* 3502 */
char InvScreenModes[2] = { 2, 1 };                                       /* 350A */
int HandY[6] = { 24, 24, 80, 80, 136, 136 };                                                 /* 350C */

/* 1BB3:000F [asm] */
void far cant_go(void)
{
	gui_printf("You can't go that way.\r");
	play_sound(29);
	flush_keys();
}

/* 1BB3:0031 [asm] */
int far button_id(byte far *button)
{
	return *(int far *)(button + 4);
}

/* 1BB3:003D [asm] - a portrait: the inventory of the character */
int far portrait_inventory(byte far *button)
{
	if (MouseButton != 0 || char_is_active(*(int far *)(button + 0x38), 1)) {
		hide_mouse();
		Var_EE3A = 1;
		if (MouseButton == 0)
			InvCharacter = button[0x38];
		fn_1CBE_0B5B(1);
		gui_inventory();
		show_mouse();
	}
	return *(int far *)(button + 4);
}

/* 1BB3:0097 [asm] - the next present character, in direction dir */
int far next_present(register int ch, register int dir)
{
	do {
		ch += dir;
		if (ch < 0)
			ch = 5;
		if (ch > 5)
			ch = 0;
	} while (!char_is_active(ch, 1));
	return ch;
}

/* 1BB3:00C9 [asm] - the inventory of the previous character */
int far inventory_prev(byte far *button)
{
	if (MouseButton == 1)
		InvCharacter = 0;
	else if (MouseButton == 2)
		InvCharacter = 1;
	else
		InvCharacter = next_present(InvCharacter, -1);
	fn_1CBE_0B5B(0);
	return *(int far *)(button + 4);
}

/* 1BB3:010E [asm] - the inventory of the next character */
int far inventory_next(byte far *button)
{
	int base, old;

	old = InvCharacter;
	if (*(int far *)(button + 0x38) == 2)
		base = 2;
	else
		base = 0;
	if (MouseButton == 1)
		InvCharacter = base + 2;
	else if (MouseButton == 2)
		InvCharacter = base + 3;
	else
		InvCharacter = next_present(InvCharacter, 1);
	if (!char_is_active(InvCharacter, 1)) {
		InvCharacter = old;
		return 1;
	}
	fn_1CBE_0B5B(0);
	return *(int far *)(button + 4);
}

/* 1BB3:0195 [asm] - the inventory / statistics switch */
int far inventory_toggle(byte far *button)
{
	register int k;

	k = Var_EE3A == 2;
	InvScreens[k]();
	Var_EE3A = InvScreenModes[k];
	fn_1CBE_0B5B(0);
	return *(int far *)(button + 4);
}

/* 1BB3:01CE [asm] - an inventory slot */
int far inventory_slot(byte far *button)
{
	hide_mouse();
	inventory_click(*(int far *)(button + 0x38));
	show_mouse();
	return *(int far *)(button + 4);
}

/* 1BB3:01F2 [asm] - back to the party */
int far inventory_close(byte far *button)
{
	hide_mouse();
	Var_62A5 = 2;
	Drv_6542(0, 0, 0x12, 0xA8, LoadBuffer);
	if (InvSlot > 1)
		InvSlot = 0;
	Var_EE3A = 0;
	redraw_party();
	Var_62A5 = 0;
	copy_page(0, 0, 0x16, 0, 0x12, 0xA8, 2, 0);
	gui_main();
	show_mouse();
	return *(int far *)(button + 4);
}

/* 1BB3:026C [asm] - a portrait: the first click selects it, the second
 * swaps the two characters */
int far portrait_swap(byte far *button)
{
	register int second, first;

	if (char_is_active(*(int far *)(button + 0x38), 1)) {
		if (BookOpen)
			close_spell_book(GuiState);
		if (SelectedChar == -1) {
			SelectedChar = button[0x38];
			gui_swap();
			draw_portrait(SelectedChar);
			set_timer(0, 1);
		} else {
			first = SelectedChar;
			second = *(int far *)(button + 0x38);
			SelectedChar = -1;
			swap_mem(&Party[first], &Party[second], LoadBuffer, 0x159);
			set_timer(0, 0);
			draw_portrait(first);
			select_char(second);
			draw_portrait(second);
			gui_main();
			refresh_char_timers();
		}
	}
	return *(int far *)(button + 4);
}

/* 1BB3:0349 [asm] */
int far portrait_unselect(byte far *button)
{
	int ch;

	set_timer(0, 0);
	ch = SelectedChar;
	SelectedChar = -1;
	draw_portrait(ch);
	gui_main();
	return *(int far *)(button + 4);
}

/* 1BB3:0380 [asm] - the selection frame goes to the next/previous slot */
void far inventory_slot_move(int next)
{
	fn_1E24_0086(InvSlot);
	if (next)
		InvSlot++;
	else
		InvSlot--;
	if (InvSlot < 0)
		InvSlot = 2;
	if (InvSlot > 2)
		InvSlot = 0;
	fn_1E24_0075(InvSlot, 9);
}

/* 1BB3:03CA [asm] - a hand of a portrait: the left button attacks, the
 * right one uses the item */
int far portrait_hand(byte far *button)
{
	char ch, hand;

	ch = button[0x38];
	if (!char_is_active(ch, 1))
		return 1;
	hide_mouse();
	if (HandY[ch] <= GuiY)
		hand = 1;
	else
		hand = 0;
	if ((MouseKeys & 0x7F) == 1)
		hand_click(ch, hand);
	else
		use_item(ch, hand);
	show_mouse();
	return 1;
}

/* 1BB3:044B [asm] - throw the item in hand */
int far throw_click(byte far *button)
{
	register int item;

	item = get_hand_item();
	if (item && launch_item(InvCharacter, item, PartyBlock, FrontPos[0][*(int far *)(button + 0x38) + PartyDirection * 4],
	                        PartyDirection, Items[item].type)) {
		take_item(0);
		SceneDirty = 1;
	}
	return *(int far *)(button + 4);
}

/* 1BB3:04B0 [asm] */
int far move_forward(byte far *button)
{
	register int block;

	block = get_walkable_block(PartyBlock, PartyDirection);
	if (block != -1) {
		teleport_party(block);
		Var_DF2C = 1;
	} else
		cant_go();
	return *(int far *)(button + 4);
}

/* 1BB3:04E8 [asm] */
int far move_back(byte far *button)
{
	int dir;
	register int block;

	dir = (PartyDirection + 2) & 3;
	block = get_walkable_block(PartyBlock, dir);
	if (block != -1) {
		teleport_party(block);
		Var_DF2C = 1;
	} else
		cant_go();
	return *(int far *)(button + 4);
}

/* 1BB3:052F [asm] */
int far turn_left(byte far *button)
{
	PartyDirection--;
	PartyDirection &= 3;
	Var_DF24 = -1;
	SceneDirty = Var_DF2C = 1;
	return *(int far *)(button + 4);
}

/* 1BB3:0552 [asm] */
int far turn_right(byte far *button)
{
	PartyDirection++;
	PartyDirection &= 3;
	Var_DF24 = -1;
	SceneDirty = Var_DF2C = 1;
	return *(int far *)(button + 4);
}

/* 1BB3:0575 [asm] */
int far move_left(byte far *button)
{
	int dir;
	register int block;

	dir = PartyDirection - 1;
	dir &= 3;
	block = get_walkable_block(PartyBlock, dir);
	if (block != -1) {
		Var_DF2C = 1;
		teleport_party(block);
	} else
		cant_go();
	return *(int far *)(button + 4);
}

/* 1BB3:05BD [asm] */
int far move_right(byte far *button)
{
	int dir;
	register int block;

	dir = PartyDirection + 1;
	dir &= 3;
	block = get_walkable_block(PartyBlock, dir);
	if (block != -1) {
		Var_DF2C = 1;
		teleport_party(block);
	} else
		cant_go();
	return *(int far *)(button + 4);
}

/* 1BB3:0605 [asm] - the floor (2 places on the party's block, 2 ahead):
 * pick up the item there or drop the one in hand (events 8 and 4) */
int far floor_click(byte far *button)
{
	signed char step[4] = { -32, 1, 32, -1 };
	signed char places[4][4] = { 0, 1, 2, 3, 1, 3, 0, 2, 3, 2, 1, 0, 2, 0, 3, 1 };
	int item;
	register int block, pos;

	pos = *(int far *)(button + 0x38);
	if (pos < 2)
		block = PartyBlock;
	else {
		block = (PartyBlock + step[PartyDirection]) & 0x3FF;
		if (!(WallTypeFlags[LevelMap[block].walls[Front]] & 0x0B))
			return 1;
	}
	pos = places[PartyDirection][pos];
	if (get_hand_item() == 0) {
		item = unlink_item(&LevelMap[block].items, pos, -1);
		if (item) {
			take_item(item);
			run_block_script(block, 8);
			SceneDirty = 1;
		}
	} else {
		put_item(&LevelMap[block].items, block, get_hand_item(), pos);
		take_item(0);
		run_block_script(block, 4);
		SceneDirty = 1;
	}
	return 1;
}

/* 1BB3:071E [asm] */
void far fn_1BB3_071E(void)
{
	Var_CB76++;
	if (Var_CB76 > 2)
		Var_CB76 = 1;
	if (Var_CB76 == 2)
		fn_1E24_0075(InvSlot, 9);
	else
		fn_1E24_0086(InvSlot);
}

/* 1BB3:0759 [asm] */
int far portrait_select(byte far *button)
{
	if (!char_is_active(*(int far *)(button + 0x38), 1))
		return 1;
	hide_mouse();
	fn_1E24_0086(InvSlot);
	select_char(*(int far *)(button + 0x38));
	fn_1E24_0075(InvSlot, 9);
	show_mouse();
	return 0;
}

/* 1BB3:07B1 [asm] - eat the food in hand */
void far eat_food(register int ch)
{
	Character far *c;
	register int item;

	c = &Party[ch];
	if (!char_is_active(ch, 5)) {
		sprintf(Scratch, "%s isn't capable of eating food!\r", c->name);
		gui_printf(Scratch);
	} else {
		item = get_hand_item();
		if (Items[item].type == 0x1F) {
			if (Items[item].value == -1)
				message("That food is rotten!  You don't want to eat that!\r");
			else {
				if (c->food + Items[item].value > 100)
					c->food = 100;
				else
					c->food += Items[item].value;
				Items[item].block = -1;
				take_item(0);
				draw_food(ch);
				play_sound(9);
			}
		} else if (item)
			gui_printf("You may only eat food!\r");
	}
}

/* 1BB3:08B6 [asm] */
int far eat_click(byte far *button)
{
	eat_food(InvCharacter);
	return *(int far *)(button + 4);
}

/* 1BB3:08CD [asm] - a click on the wall ahead */
void far wall_ahead_click(void)
{
	int block;

	block = block_ahead(PartyBlock, PartyDirection);
	Var_DF4C = -1;
	wall_click(block, PartyDirection);
	recompute_ac(InvCharacter);
}

/* 1BB3:090A [asm] - the arrow keys on the party: up/down change the row,
 * left/right the character of the row */
void far key_party(int key)
{
	register int old;

	old = InvCharacter;
	switch (key) {
	case 0:
		if (InvSlot) {
			/* (only this form keeps BCC from sharing the code with case 2,
			 * as in the original: a macro ?) */
			do InvSlot = 0; while (0);
			break;
		}
		do {
			InvCharacter -= 2;
			if (InvCharacter < 0)
				InvCharacter += 6;
		} while (!char_is_active(InvCharacter, 1));
		InvSlot = 1;
		break;
	case 1:
	case 3:
		if (char_is_active(InvCharacter, 1))
			InvCharacter ^= 1;
		break;
	case 2:
		if (!InvSlot) {
			InvSlot = 1;
			break;
		}
		do {
			InvCharacter += 2;
			if (InvCharacter > 5)
				InvCharacter -= 6;
		} while (!char_is_active(InvCharacter, 1));
		InvSlot = 0;
	}
	draw_portrait(old);
	if (InvCharacter != old)
		draw_portrait(InvCharacter);
}

/* the inventory slot reached by the arrow keys: [slot][direction]      352C */
char SlotNav[27][4] = {
	18, 1, 25, 11,
	21, 23, 21, 0,
	14, 3, 4, 3,
	15, 2, 5, 2,
	2, 5, 6, 19,
	3, 16, 7, 4,
	4, 7, 8, 20,
	5, 17, 9, 6,
	6, 9, 10, 22,
	7, 18, 11, 8,
	8, 11, 12, 23,
	9, 0, 13, 10,
	10, 13, 14, 24,
	11, 25, 15, 12,
	12, 15, 2, 15,
	13, 14, 3, 14,
	25, 19, 17, 5,
	16, 20, 18, 7,
	17, 22, 0, 9,
	24, 4, 20, 16,
	19, 6, 22, 17,
	1, 24, 1, 26,
	20, 8, 23, 18,
	22, 10, 24, 1,
	23, 12, 19, 21,
	0, 26, 16, 13,
	0, 21, 16, 25
};

/* 1BB3:09E5 [asm] - the arrow keys in the inventory */
void far key_inventory(int dir)
{
	int old;

	old = InvSlot;
	InvSlot = SlotNav[InvSlot][dir];
	draw_inventory_slot(old, 1, 0);
	draw_inventory_slot(InvSlot, 1, 0);
}

/* 1BB3:0A2F [asm] */
void far key_arrow(register int dir)
{
	if (Var_CB76 != 5)
		switch (Var_EE3A) {
		case 0:
			key_party(dir);
			break;
		case 1:
			key_inventory(dir);
		}
	return;
}

/* the place on the floor where a character drops an item              3598 */
signed char DropPlace[4][2] = {
	0, 1,
	1, 3,
	3, 2,
	2, 0
};
signed char DropSide[6] = {
	0, 1, 0, 1, 0, 1
};                                      /* 35A0 */
/* the 4 places the keyboard picks up from (the 2 ahead on the next block)  35A6 */
int PickAhead[4] = {
	0, 0, 1, 1
};

/* 1BB3:0A5F [asm] - drop (or throw) the item in hand / the selected arrow */
void far key_drop(int throw)
{
	int ok;
	register int item, pos;

	if (Var_EE3A != 2 && Var_CB76 != 5) {
		if (InvSlot != 16) {
			item = get_hand_item();
			if (Items[item].flags & 0x20)
				return;                       /* cursed */
		} else
			item = unlink_item(&Party[InvCharacter].inventory[16], 0, -1);
		if (!item)
			return;
		ok = 1;
		pos = DropSide[InvCharacter];
		pos = DropPlace[PartyDirection][pos];
		if (!throw) {
			put_item(&LevelMap[PartyBlock].items, PartyBlock, item, pos);
			run_block_script(PartyBlock, 4);
		} else
			ok = launch_item(InvCharacter, item, PartyBlock, pos, PartyDirection, Items[item].type);
		if (ok) {
			if (!InventoryMode || InvSlot != 16)
				take_item(0);
			else
				draw_portrait(InvCharacter);
			SceneDirty = 1;
			Var_DF24 = -1;
		} else if (InvSlot == 16)
			put_item(&Party[InvCharacter].inventory[16], -2, item, 0);
	}
	recompute_ac(InvCharacter);
}

/* 1BB3:0BA9 [asm] - pick up with the keyboard: the 4 places in turn */
void far key_pickup(void)
{
	int got, pos, tries, hand;
	register int block, item;

	block = PartyBlock;
	if (LevelMap[block].items || LevelMap[block_ahead(block, PartyDirection)].items) {
		tries = 0;
		got = 0;
		do {
			if (Var_DF24 == -1)
				Var_E442 = -1;
			if (Var_E442 == -1) {
				Var_DF24 = (Var_DF24 + 1) & 3;
				tries++;
			}
			if (PickAhead[Var_DF24])
				block = block_ahead(PartyBlock, PartyDirection);
			else
				block = PartyBlock;
			pos = FrontPos[PartyDirection][Var_DF24];
			item = unlink_item(&LevelMap[block].items, pos, -1);
			if (item && item != Var_E442) {
				got = 1;
				if (Var_E442 == -1)
					Var_E442 = item;
				if (InvSlot != 16) {
					hand = get_hand_item();
					if (item_fits_slot(item, InvCharacter, InvSlot)) {
						if (hand) {
							place_item(block, hand, pos);
							run_block_script(block, 4);
						}
						take_item(item);
						run_block_script(block, 8);
					} else {
						place_item(block, item, pos);
						run_block_script(block, 4);
					}
				} else if (item_fits_slot(item, InvCharacter, InvSlot)) {
					put_item(&Party[InvCharacter].inventory[16], -2, item, 0);
					draw_portrait(InvCharacter);
				} else {
					place_item(block, item, pos);
					run_block_script(block, 4);
				}
				SceneDirty = 1;
			} else {
				if (item) {
					put_item(&LevelMap[block].items, block, item, pos);
					run_block_script(block, 4);
				}
				Var_E442 = -1;
			}
		} while (!got && tries < 5);
	}
	recompute_ac(InvCharacter);
}

/* 1BB3:0DAE [asm] - the keyboard commands */
void far key_command(int key)
{
	register int dir;

	if (!InventoryMode)
		return;
	if (Var_CB76 == 5)
		return;
	dir = -1;
	switch (key) {
	case 0x34:
		wall_ahead_click();
		break;
	case 0x17:
		if (Var_EE3A != 2)
			use_item(InvCharacter, InvSlot);
		break;
	case 0x21:
		if (Var_EE3A != 2)
			key_drop(0);
		break;
	case 0x15:
		if (Var_EE3A != 2)
			key_drop(1);
		break;
	case 0x23:
		if (Var_EE3A != 2)
			key_pickup();
		break;
	case 0x12:
		dir = 0;
		break;
	case 0x20:
		dir = 1;
		break;
	case 0x2E:
		dir = 2;
		break;
	case 0x1F:
		dir = 3;
	}
	if (dir != -1 && Var_EE3A != 2)
		key_arrow(dir);
}

/* 1BB3:0E87 [asm] */
void far handle_input(void)
{
	int key;

	key = gui_process(GameGui);
	key_command(key);
}

/* 1BB3:0EAC [asm] - the main loop: never returns (the game ends with exit) */
void far game_loop(void)
{
	NextAmbient = GameTicks + roll_dice(1, 10, 3) * 18;
	level_start();
	show_mouse();
	PrevPartyBlock = PartyBlock;
	Var_6616 = 0;
	for (;;) {
		PrevPartyBlock = PartyBlock;
		check_party_alive(1);
		handle_input();
		run_timers();
		if (SceneDirty)
			draw_scene(1);
		if (NextAmbient < GameTicks) {
			NextAmbient = GameTicks + roll_dice(1, 10, 3) * 18;
			if (roll_dice(1, 2, -1))
				play_sound_at(0x1B, PartyBlock + roll_dice(1, 12, -1));
			else
				play_sound_at(0x1C, PartyBlock + roll_dice(1, 12, -1));
		}
		play_sound_here(0);
	}
}

/* 1BB3:0FC7 [asm] - a new game, or a saved one */
void far level_start(void)
{
	if (!Var_9316)
		new_game_start();
	else {
		if (!InventoryMode)
			take_item(HandItem);
		load_level(CurrentLevel, CurrentSubLevel);
		Var_9316 = 0;
	}
	view_nothing();
	BookOpen = 0;
	InvCharacter = 0;
	Var_EEF2 = TimerTicks;
	Var_DF2C = 0;
	Var_CB76 = 1;
	gui_main();
	Var_62A5 = 0;
	draw_play_screen(0);
	set_font(GameFont);
	Var_62A5 = 0;
	redraw_party();
	draw_scene(1);
	show_mouse();
	select_window(7);
}

/* 1BB3:1076 [asm] - a new game starts on level 4, block 0xAB, facing south */
void far new_game_start(void)
{
	CurrentLevel = 4;
	CurrentSubLevel = 0;
	load_level(4, 0);
	PartyBlock = 0xAB;
	PartyDirection = 2;
	if (!InventoryMode)
		take_item(0);
}
