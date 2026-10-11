/*
 * Eye of the Beholder II - START.EXE
 * Utilities: map, party, inventory, hand: module 0E24 (Ghidra 1E24).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y util.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
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
extern char           SceneDirty;        /* D0AA */
extern int            ViewBlock[18];     /* 9446 the squares in sight */
extern char           WallTypeFlags[256];/* 2268 */
extern signed char    BlockStep[4];      /* 20E9 -32, +1, +32, -1 */
extern char           InvCharacter;      /* 9317 the selected character */
extern char           InvSlot;           /* E5A4 the selected inventory slot */
extern unsigned char  InventoryMode;     /* 6655 an inventory is shown: the "hand" is the selected slot */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics (?) */
extern int            InDialogue;        /* EE38 */
extern int            HandItem;          /* 6DD6 */
extern signed char    FrontPos[4][4];    /* 2342 */
typedef struct WallOfForce { int block; unsigned long when; } WallOfForce;
extern WallOfForce    ForceWalls[5];   /* 9346 walls of type 0x4A, removed when they expire */
extern unsigned long  PartyEffects;      /* 6DDC */
extern volatile unsigned long TimerTicks;/* 667C */
extern signed char    ClassThac0Group[]; /* 1ED7 0 warrior 1 wizard 2 priest 3 rogue */
extern byte far      *ItemIcons[];       /* CEE6 */
extern byte far      *MagicPalette;      /* CC26 */
extern byte far      *NormalPalette;     /* CD12 */
extern void (far *Drv_6576)(int on);        /* 6576 video driver entry 14 */
extern void (far *Drv_65DE)(byte far *pal); /* 65DE video driver entry 40 */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern ItemType       ItemTypes[];       /* 946E */
extern Monster        Monsters[30];      /* 6E2A */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 colours of the 3D frames */
extern char           BookOpen;          /* E441 */
extern char           GuiState[];        /* E5CA */
extern int            Var_62A5;          /* 62A5 */
extern int            GuiStyle;          /* 636F */
extern byte far      *CurFont;           /* F090 */
extern byte far      *Font8;             /* F098 */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern int            ClickDown[2], ClickUp[2]; /* 661A, 6622 where the mouse button went down / up (x, y) */
extern char far       MenuStrings[];     /* 38F7:0000 (a far data segment) */
extern signed char    BoxColours[];      /* 23CC colours of a flashing box, 0-terminated */
/* Uninitialised data of the module (DS:EF30-EF33, _BSS), in address order */
unsigned long NextBoxTick; /* EF30 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
void remove_force_wall(int n);                                        /* 23A7:098C ends the wall of force n */
void draw_hand(char ch, char hand);                           /* 2901:091F draws a hand of a portrait */
void draw_inventory_slot(int slot, int a, int b);                /* 1CBE:09A6 */
int  item_fits_slot(int item, int ch, int slot);                 /* 1CBE:0B90 */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
int  unlink_item(unsigned far *list, int pos, int item);         /* 1CBE:02BF */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void draw_rect2(int x1, int y1, int x2, int y2, int colour);   /* 2D9E:0000 */
int  saving_throw(Character far *c, int a, int level, int type, int race);   /* 1F55:0F8A */
void put_shape(int page, byte far *shape, int x, int y, int flags);    /* 2DA3:004A draws a shape */
int  can_use_item(int ch, int item);                             /* 1CBE:13AF can the character use this item (?) */
int  turn_result(int level, int undead);                         /* 1B54:05DE roll needed to turn, -1: destroyed */
void damage_monster(Monster far *m, int a, int b, int c, int d, int e, int f);   /* 1F55:12B8 */
int  octant(int from, int to);                                   /* 2A68:0316 direction from a square to another */
void draw_frame3d(int x1, int y1, int x2, int y2, int c1, int c2, int fill);     /* 18D7:0A45 draws a 3D frame */
void inventory_close(void far *gui);                                /* 1BB3:01F2 closes the inventory */
int  close_spell_book(void far *gui);                            /* 20E2:14A7 */
void load_vcn(char far *name, char far *p);                      /* 53EC:0BCE (overlay) */
void draw_play_screen(int scene);                                /* 53EC:1A19 (overlay) */
void copy_page(int a, int b, int c, int d, int e, int f, int g, int h);   /* 2D92:000A copies a screen area */
int  select_window(int w);                                       /* 31B0:0133 */
void set_font(byte far *font);                                   /* 3166:000A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void flush_keys(void);                                         /* 3295:0516 empties the input buffer (?) */
int  key_pressed(void);                                         /* 3295:041E an input is waiting (?) */
int  read_key(void);                                            /* 3295:066D key / mouse event */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);     /* 18D7:067F */
void wait_ticks(int ticks);                                    /* 2D9B:0003 waits */
void delay(unsigned ms);                                  /* 1000:6C86 (runtime, dos.h) */
unsigned strlen(char far *s);
char far *strchr(char far *s, int c);
int  block_ahead(int block, int dir);
int  get_hand_item(void);
void take_item(int item);                                        /* 18AE:0197 */
void far set_hand_item(int item);
int  char_is_active(int ch, unsigned what);

/* ------------------------------------------------------------------ */
/* Module 1E24, in address order                                       */
/* ------------------------------------------------------------------ */

/* 1E24:0005 [asm] - a square changed: redraw if it is in sight */
int far block_changed(int block)
{
	int far *p;
	int i;

	for (p = ViewBlock, i = 0; i < 15; i++, p++)
		if (*p == block) {
			SceneDirty = 1;
			return 1;
		}
	return 0;
}

/* 1E24:0040 [asm] */
void far select_char(int ch)
{
	int old;

	if (InvCharacter != ch) {
		old = InvCharacter;
		InvCharacter = ch;
		draw_portrait(old);
		draw_portrait(InvCharacter);
	}
}

/* 1E24:0075 [asm] - empty (a stub kept from an earlier version) */
void far fn_1E24_0075(int a, int b)
{
	a++;
	b++;
}

/* 1E24:0086 [asm] - empty */
void far fn_1E24_0086(int a)
{
	a++;
}

/* 1E24:0091 [asm] - the square the party would walk into, or -1. Walking
 * into a wall of force (type 0x4A) ends it. */
int far get_walkable_block(int block, char dir)
{
	int wflags, wall;
	int i, next;

	next = block_ahead(block, dir);
	wall = LevelMap[next].walls[dir ^ 2];
	wflags = WallTypeFlags[wall];
	if (wall == 0x4A && block == PartyBlock) {
		for (i = 0; i < 5; i++)
			if (ForceWalls[i].block == next) {
				remove_force_wall(i);
				wflags = WallTypeFlags[0];
			}
	}
	/* HACK: walk through walls */
	/* if (!(wflags & 1))
		return -1;
	if (!(wflags & 1))
		return -1;
	if (LevelMap[next].flags & 7)
		return -1;*/
	return next;
}

/* 1E24:012F [asm] - the next square in a direction (the map wraps around) */
int far block_ahead(int block, int dir)
{
	return (block + BlockStep[dir]) & 0x3FF;
}

/* 1E24:0148 [asm] - the inventory slot of a character holding an item of
 * this type (-1: any) and value (-1: any), or -1 */
int far find_inventory_item(int ch, int type, int value)
{
	unsigned far *p;
	unsigned i;

	p = &Party[ch].inventory[0];
	for (i = 0; i < 27; p++, i++)
		if (*p && (Items[*p].type == type || type == -1) && (Items[*p].value == value || value == -1))
			return i;
	return -1;
}

/* 1E24:01BE [asm] - does the character wear a ring (type 0x2F, slots 25
 * and 26) of this kind? */
int far has_ring(Character far *c, int kind)
{
	int i, item;

	for (i = 25; i <= 26; i++) {
		if ((item = c->inventory[i]) == 0)
			continue;
		if (Items[item].type == 0x2F && Items[item].value == kind)
			return 1;
	}
	return 0;
}

/* 1E24:0212 [asm] - take away all the items of a type (and value) from the
 * party and the hand. Returns 1 if there was one. */
int far take_party_items(int type, int value)
{
	int slot, ch;
	int item, found;

	found = ch = 0;
	for (; ch < 6; ch++) {
		if (!char_is_active(ch, 1))
			continue;
		while ((slot = find_inventory_item(ch, type, value)) != -1) {
			item = Party[ch].inventory[slot];
			Items[item].block = -1;
			Party[ch].inventory[slot] = 0;
			found = 1;
			if (InDialogue == 0) {
				if (Var_EE3A == 0 && slot < 2 && ch < 5)
					draw_hand(ch, slot);
				if (Var_EE3A == 1 && InvCharacter == ch)
					/* ORIGINAL BUG: the character is passed as the slot to redraw */
					draw_inventory_slot(ch, slot, 1);
			}
		}
	}
	if (!InventoryMode) {
		item = get_hand_item();
		if ((type == -1 || Items[item].type == type) && (value == -1 || Items[item].value == value)) {
			take_item(0);
			Items[item].block = -1;
			found = 1;
		}
	}
	return found;
}

/* 1E24:0352 [asm] - the item held: in the mouse "hand", or in the selected
 * slot when an inventory is shown */
int far get_hand_item(void)
{
	if (!InventoryMode)
		return HandItem;
	return Party[InvCharacter].inventory[InvSlot];
}

/* 1E24:037C [asm] */
void far set_hand_item(int item)
{
	if (!InventoryMode)
		HandItem = item;
	else if (item_fits_slot(item, InvCharacter, InvSlot)) {
		Party[InvCharacter].inventory[InvSlot] = item;
		if (Var_EE3A == 0)
			draw_portrait(InvCharacter);
		else
			draw_inventory_slot(InvSlot, 1, 0);
	} else {
		/* it does not fit there: it falls in front of the party */
		gui_printf("You can't put that item there.\r");
		put_item(&LevelMap[PartyBlock].items, PartyBlock, item, FrontPos[PartyDirection][roll_dice(1, 2, -1)]);
		SceneDirty = 1;
	}
}

/* 1E24:043C [asm] - a character leaves the party: all his items fall in
 * front of the party */
void far drop_character(int ch)
{
	Character far *c;
	int i, item;

	c = &Party[ch];
	c->flags = 0;
	for (i = 0; i < 27; i++)
		if (i != 16 && (item = c->inventory[i]) != 0) {
			put_item(&LevelMap[PartyBlock].items, PartyBlock, item, FrontPos[PartyDirection][roll_dice(1, 2, -1)]);
			c->inventory[i] = 0;
		}
	while (c->inventory[16]) {                   /* the quiver is a list */
		item = unlink_item(&c->inventory[16], 0, -1);
		put_item(&LevelMap[PartyBlock].items, PartyBlock, item, FrontPos[PartyDirection][roll_dice(1, 2, -1)]);
	}
	c->inventory[16] = 0;
	if (InvCharacter == ch)
		InvCharacter = 0;
	refresh_char_timers();
}

/* Screen boxes that can flash (portraits, buttons...)                5C5C */
int Boxes[20][4] = {
	{ 183,   1, 247,  52 }, { 255,   1, 319,  52 },     /* the 6 portraits */
	{ 183,  53, 247, 104 }, { 255,  53, 319, 104 },
	{ 183, 105, 247, 156 }, { 255, 105, 319, 156 },
	{  16,  63,  48,  96 }, {  80,  63, 112,  96 },
	{  16, 127,  48, 160 }, {  80, 127, 112, 160 },
	{ 176,  66, 208,  97 }, { 208,  66, 240,  97 },
	{ 240,  66, 272,  97 }, { 272,  66, 304,  97 },
	{   4,  24,  36,  57 }, { 163,  24, 195,  57 },
	{   4,  64,  36,  97 }, { 163,  64, 195,  97 },
	{   4, 104,  36, 137 }, { 163, 104, 195, 137 },
};
int FlashingBox = -1;                              /* 54FC */
int BoxFrame = 0;                                  /* 54FE */

/* 1E24:054E [asm] - make box n flash: called again and again, it changes
 * the colour of the frame once per tick; another n stops the previous one */
void far flash_box(int n)
{
	int x1, y1, x2, y2;

	if (FlashingBox != n) {
		if (FlashingBox != -1) {
			hide_mouse();
			draw_rect2(Boxes[FlashingBox][0], Boxes[FlashingBox][1], Boxes[FlashingBox][2], Boxes[FlashingBox][3], 12);
			show_mouse();
		}
		BoxFrame = 0;
		FlashingBox = n;
		NextBoxTick = TimerTicks;
	} else if (FlashingBox != -1 && TimerTicks > NextBoxTick) {
		if (BoxColours[BoxFrame] == 0)
			BoxFrame = 0;
		x1 = Boxes[FlashingBox][0];
		y1 = Boxes[FlashingBox][1];
		x2 = Boxes[FlashingBox][2];
		y2 = Boxes[FlashingBox][3];
		hide_mouse();
		draw_rect2(x1, y1, x2, y2, BoxColours[BoxFrame]);
		show_mouse();
		BoxFrame++;
		NextBoxTick = TimerTicks + 1;
	}
}

/* 1E24:0686 [asm] */
void far redraw_party(void)
{
	int i;

	for (i = 0; i < 6; i++)
		draw_portrait(i);
}

/* 1E24:069F [asm] - a saving throw of a character (see party.c) */
int far char_saving_throw(int ch, int type)
{
	Character far *c;

	c = &Party[ch];
	return saving_throw(c, ClassThac0Group[c->cls], c->level[0], type, c->raceSex >> 1);
}

/* 1E24:06E9 [asm] - tests on a character; "what" is a set of conditions:
 *   1 present            2 alive (hp > -10) and not 8    4 hp > 0 and not 8
 *   8 not 4 nor 8        0x20 not paralyzed (4)          0x10 not poisoned (2)
 *   0x40 not starving */
int far char_is_active(int ch, unsigned what)
{
	Character far *c;
	int r;

	r = 1;
	c = &Party[ch];
	if (what & 1)
		r &= c->flags & 1;
	if (what & 2)
		r &= c->hp > -10 && (c->flags & 8) == 0;
	if (what & 4)
		r &= c->hp > 0 && (c->flags & 8) == 0;
	if (what & 8)
		r &= (c->flags & 0x0C) == 0;
	if (what & 0x20)
		r &= (c->flags & 4) == 0;
	if (what & 0x10)
		r &= (c->flags & 2) == 0;
	if (what & 0x40)
		r &= c->food > 0;
	return r;
}

/* 1E24:07C8 [asm] - draw the icon of an item; with Detect Magic (party
 * effect 2) magical items (flag 0x80) are drawn with another palette */
void far draw_item_icon(int page, int item, int x, int y)
{
	int icon;
	int magic;

	icon = Items[item].icon;
	magic = (PartyEffects & 2) && (Items[item].flags & 0x80);
	if (magic) {
		Drv_65DE(MagicPalette);
		Drv_6576(1);
	}
	put_shape(page, ItemIcons[icon], x, y, 0);
	if (magic) {
		Drv_65DE(NormalPalette);
		Drv_6576(0);
	}
}

/* 1E24:0869 [asm] - string number "index" of a table of 0-terminated strings */
char far * far get_script_string(char far *text, int index)
{
	char far *s;

	if (index == -1)
		s = 0;
	else
		for (s = text; index != 0; index--)
			while (*s++ != 0)
				;
	return s;
}

/* 1E24:08AE [asm] - can the character use hand "hand"? The second hand is
 * free only if the first holds no two-handed item (ItemType.hands == 2) */
int far hand_usable(int ch, int hand)
{
	int r0, item1, r1, type0, type1;
	int item0;

	item0 = Party[ch].inventory[0];
	r0 = can_use_item(ch, item0);
	type0 = Items[item0].type;
	if (hand == 0)
		return item0 == 0 || r0 != 0;
	item1 = Party[ch].inventory[1];
	r1 = can_use_item(ch, item1);
	type1 = Items[item1].type;
	if (item0 && ItemTypes[type0].hands == 2)
		return 0;
	if (item1 == 0)
		return 1;
	if ((ItemTypes[type1].flags & 0x7F) > 0 && (ItemTypes[type1].flags & 0x7F) < 4)
		if (ItemTypes[type1].hands == 0)
			return r1;
		else
			return 0;                             /* a weapon that needs both hands (?) */
	return r1;
}

/* 1E24:098B [asm] - does the character hold an item of kind 6 in a hand? */
int far holds_kind6(Character far *c)
{
	int i, type;

	for (i = 0; i <= 1; i++)
		if (c->inventory[i]) {
			type = Items[c->inventory[i]].type;
			if (ItemTypes[type].flags == 6)
				return 1;
		}
	return 0;
}

/* 1E24:09DF [asm] - Turn Undead on one monster: the roll needed comes from
 * the cleric level and the undead type (MonsterType +0x29); -1 = destroyed */
int far turn_monster(Monster far *m, int roll, int level)
{
	int undead;
	int need;

	undead = MonsterTypes[m->type].undead;
	need = turn_result(level, undead);
	if (need != -1) {
		if (roll >= need) {
			m->mode = 0;
			m->flags |= 8;                         /* flees */
			m->wait = 40;
			m->facing = (octant(m->block, PartyBlock) ^ 4) >> 1;
			return 1;
		}
	} else {
		damage_monster(m, 0, 0, 500, 0x200, 5, 3);
		return 1;
	}
	return 0;
}

/* 1E24:0A76 [asm] - magic resistance (MonsterType +0x2A, in %) */
int far monster_resists(Monster far *m)
{
	int resist;

	resist = MonsterTypes[m->type].resist;
	return roll_dice(1, 100, 0) < resist;
}

/* 1E24:0AB2 [asm] - a 3D frame (pressed: flat), filled unless "hollow" */
void far draw_frame(int x, int y, int w, int h, int pressed, int hollow)
{
	int x1, y1, x2, y2, c1, c2;

	hide_mouse();
	x1 = x;
	y1 = y;
	x2 = x + w - 1;
	y2 = y + h - 1;
	if (pressed) {
		c2 = FrameFlat;
		c1 = FrameFlat;
	} else {
		c1 = FrameLight;
		c2 = FrameDark;
	}
	draw_frame3d(x1, y1, x2, y2, c1, c2, -1);
	x1++;
	y1++;
	x2--;
	y2--;
	draw_frame3d(x1, y1, x2, y2, FrameLight, FrameDark, hollow ? -1 : FrameFlat);
	show_mouse();
}

/* 1E24:0B5B [asm] - close the inventory / the spell book */
void far close_panels(void)
{
	if (Var_EE3A)
		inventory_close(GuiState);
	if (BookOpen)
		close_spell_book(GuiState);
}

/* 1E24:0B88 [asm] */
void far draw_game_screen(void)
{
	load_vcn(0, 0);
	Var_62A5 = 0;
	hide_mouse();
	draw_play_screen(1);
	redraw_party();
	show_mouse();
}

/* 1E24:0BBC [asm] - how many characters carry an item of this type and value */
int far count_party_items(int type, int value)
{
	int i, n;

	for (i = n = 0; i < 6; i++)
		if (char_is_active(i, 1) && find_inventory_item(i, type, value) != -1)
			n++;
	return n;
}

/* 1E24:0BF9 [asm] - take away up to "count" items of a type whose value is
 * <= (mode -1), == (0) or >= (1) "value". Returns how many were taken. */
int far remove_items(int type, int value, int mode, int count)
{
	int again, left, ch;
	int slot, item;

	left = count;
	again = 1;
	while (left && again) {
		again = 0;
		for (ch = 0; ch < 6; ch++) {
			if (!char_is_active(ch, 1))
				continue;
			for (slot = 0; slot < 27; slot++) {
				if (slot == 16)
					continue;
				item = Party[ch].inventory[slot];
				if (Items[item].type == type) {
					if ((mode == -1 && Items[item].value <= value) || (mode == 0 && Items[item].value == value)
					    || (mode == 1 && Items[item].value >= value)) {
						again = 1;
						Party[ch].inventory[slot] = 0;
						Items[item].block = -1;
						left--;
					}
				}
				if (left == 0)
					break;
			}
			if (left == 0)
				break;
		}
	}
	return count - left;
}

/* 1E24:0D19 [asm] - open the text window with a text of the menu table */
void far open_text_box(int style, int text)
{
	int old;
	byte far *font;

	old = GuiStyle;
	select_window(style);
	font = CurFont;
	set_font(Font8);
	if (WinW <= 22 && WinH <= 84) {
		hide_mouse();
		copy_page(WinX, WinY, 0, WinH, WinW, WinH, 0, 2);   /* save what is under it */
		show_mouse();
	}
	Var_62A5 = 2;                                 /* draw on page 2... */
	draw_frame(0, 0, WinW << 3, WinH, 0, 0);
	print_text(get_script_string(MenuStrings, text), 5, 5, 15, 0);
	Var_62A5 = 0;
	hide_mouse();
	copy_page(0, 0, WinX, WinY, WinW, WinH, 2, 0);       /* ...then copy it to the screen */
	show_mouse();
	select_window(old);
	set_font(font);
}

/* 1E24:0E22 [asm] - close it: restore what was under it */
void far close_text_box(int style)
{
	int old;

	old = GuiStyle;
	select_window(style);
	if (WinW <= 22 && WinH <= 84) {
		hide_mouse();
		copy_page(0, WinH, WinX, WinY, WinW, WinH, 2, 0);
		show_mouse();
	}
	select_window(old);
}

/* 1E24:0E83 [asm] - a message in the text window, with one button: waits
 * for a click on it or for one of the keys */
void far message_box(int style, int text, char far *button, char far *keys, int colour)
{
	int old, y, w, waiting;
	byte far *font;
	int x, key;

	old = GuiStyle;
	font = CurFont;
	set_font(Font8);
	select_window(style);
	open_text_box(style, text);
	x = (WinX << 3) + (WinW << 3);
	x -= (strlen(button) << 3) + 16;
	y = WinY + WinH - 19;
	w = (strlen(button) << 3) + 7;
	draw_frame(x, y, w, 14, 0, 0);
	hide_mouse();
	print_text(button, x + 4, y + 3, colour, 0);
	show_mouse();
	flush_keys();
	do {
		key = read_key() & 0x8FF;
		waiting = (key == 0x41 || key == 0x42) ? !in_rect(ClickUp[0], ClickUp[1], x, y, x + w, y + 14)
		                                       : !strchr(keys, key);
	} while (waiting);
	draw_frame(x, y, w, 14, 1, 1);                /* the button is pressed */
	wait_ticks(5);
	draw_frame(x, y, w, 14, 0, 1);
	select_window(old);
	set_font(font);
	close_text_box(style);
}

char *YesNo[2] = { "Yes", "No" };                  /* 5500 */

/* 1E24:1027 [asm] - a question with "Yes" and "No" buttons; no is the
 * default when "no" is 1. Returns 1 for yes. */
int far ask_yes_no(int style, int text, int no)
{
	int old, key, waiting;
	int bx[2];
	int y, w, tx, last, colour;
	byte far *font;
	int i;

	old = GuiStyle;
	font = CurFont;
	set_font(Font8);
	select_window(style);
	open_text_box(style, text);
	bx[0] = (WinX << 3) + 8;
	bx[1] = (WinX << 3) + (WinW << 3);
	bx[1] -= 40;
	y = WinY + WinH - 21;
	w = 32;
	last = -1;
	no ^= 1;
	for (i = 0; i < 2; i++)
		draw_frame(bx[i], y, w, 14, 0, 0);
	flush_keys();
	do {
		waiting = 1;
		if (!InventoryMode) {                     /* the mouse pointer selects */
			if (in_rect(ClickDown[0], ClickDown[1], bx[0], y, bx[0] + w, y + 14))
				no = 0;
			if (in_rect(ClickDown[0], ClickDown[1], bx[1], y, bx[1] + w, y + 14))
				no = 1;
		}
		if (key_pressed()) {
			key = read_key() & 0x8FF;
			switch (key) {
			case 0x41:
			case 0x42:                            /* mouse buttons */
				if (in_rect(ClickUp[0], ClickUp[1], bx[0], y, bx[0] + w, y + 14)) {
					no = 0;
					waiting = 0;
				}
				if (in_rect(ClickUp[0], ClickUp[1], bx[1], y, bx[1] + w, y + 14)) {
					no = 1;
					waiting = 0;
				}
				break;
			case 0x5C:
			case 0x66:                            /* arrows */
				no ^= 1;
				break;
			case 0x2B:
			case 0x3D:                            /* enter */
				waiting = 0;
				break;
			case 0x16:                            /* Y */
				no = 0;
				waiting = 0;
				break;
			case 0x33:                            /* N */
				no = 1;
				waiting = 0;
				break;
			}
		}
		if (no != last) {
			for (i = 0; i < 2; i++) {
				tx = bx[i] + (w >> 1) - (strlen(YesNo[i]) << 2) + 1;
				if (no == i)
					colour = 6;
				else
					colour = 15;
				hide_mouse();
				print_text(YesNo[i], tx, y + 3, colour, 0);
				show_mouse();
			}
			last = no;
		}
	} while (waiting);
	draw_frame(bx[no], y, w, 14, 1, 1);
	delay(5);
	draw_frame(bx[no], y, w, 14, 0, 1);
	select_window(old);
	set_font(font);
	close_text_box(style);
	return no ^ 1;
}
