/*
 * Eye of the Beholder II - START.EXE
 * Level script interpreter: module 14EC (Ghidra 24EC).
 *
 * Every level (LEVELx.INF) carries a block of bytecode, loaded at ScriptCode.
 * Each square of the 32x32 map may own a script (LevelBlock.script = offset in
 * ScriptCode). When something happens on a square - the party steps on it, an
 * item is dropped, a monster walks in... - the game calls run_block_script()
 * with a mask of events; the script runs if the square listens to one of them.
 *
 * Bytecode: one opcode byte (0xFF down to 0xE2) followed by its operands.
 * Words are little-endian 16-bit. Several opcodes take a "sub-opcode" which
 * re-uses opcode values as tags (0xE9 = one wall side, 0xF7 = all walls...).
 * Text strings live in a table at the start of ScriptCode:
 *     text = ScriptCode + *(int far *)ScriptCode  (see get_script_string).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 *
 * The whole module is rebuilt here, functions in address order as in the
 * original source file: compiled with Borland C++ 2.0
 *     bcc -c -ml -G -O -Z -y script.c
 * its code segment (11444 bytes, 24EC:000B-2CBE) and its initialised data
 * (467 bytes, DS:5A28-5BFA) are identical to START.EXE byte for byte
 * (tools/bcc.py, tools/segcmp.py). [asm] = such a function.
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern Item           Items[600];        /* 71CA */
extern Monster        Monsters[30];      /* 6E2A */
extern Character      Party[6];          /* E608 */
extern byte far      *ScriptCode;        /* 71C4 bytecode + text of the current level */

extern int            ScriptBlock;       /* A2CE square whose script is running */
extern unsigned       ScriptEvents;      /* EE22 events that triggered it */
extern char           ScriptEnd;         /* 946A set by op_end */
extern int            ScriptAbort;       /* E5A6 set when the level changes under our feet */
extern char           ScriptCharacter;   /* 931C party member selected by a condition, -1 = none */
extern char           ScriptResult;      /* 6D7C result of the last dialogue / flag 0xE4 */
extern int            EvalStack[30];     /* 92DA */
extern int            EvalSp;            /* 946C */
extern byte far      *CallStack[10];     /* E4D4 */
extern int            CallDepth;         /* E59E */

extern int            CurrentLevel;      /* 6DCE */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 0..3 */
extern unsigned long  LevelFlags[17];    /* 6DE2 one set of 32 flags per level */
extern unsigned long  GlobalFlags;       /* 6E26 */
extern volatile unsigned long TimerTicks;/* 667C incremented by the timer interrupt */
extern char           SceneDirty;        /* D0AA the 3D view must be redrawn */
extern signed char    ClassFlags[];      /* 1F9A per class: 8 thief, 0x31 warriors ... */

extern Missile        Thrown[10];        /* CC5C flying items and spells (eob2.h) */

/* Script handlers: one per opcode, in opcode order 0xFF, 0xFE, ... 0xE2.
 * The table is a local initialised array in run_block_script(); Borland
 * stores its initial value at DS:5AB1 and copies it with SCOPY@ on entry. */
typedef int (far *ScriptOp)(byte far *p);

/* ------------------------------------------------------------------ */
/* Routines from other modules (names are ours)                        */
/* ------------------------------------------------------------------ */
void block_changed(int block);                                   /* 1E24:0005 walls changed: update the square */
int  char_is_active(int ch, int what);                           /* 1E24:06E9 */
char far *get_script_string(byte far *text, int index);          /* 1E24:0869 */
int  roll_dice(char count, char sides, char add);                 /* 18D7:0505 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void play_sound_at(int sound, int block);                        /* 2A30:0014 */
void move_door(int block, int dir);                              /* 24EC:0209 (this module) */
void toggle_door(int block, int side);                           /* 24EC:0466 (this module) */
void teleport_party(int block);                                  /* 24EC:0076 (this module) */
void create_monster(int index, int block, int pos, int dir, int type,
                    int shape, int mode, int sub, int a, int b);  /* 1997:177E */
void move_monster(Monster far *m, int block, int pos);           /* 1997:04EA */
int  find_item(unsigned far *list, int item, int type, int count, int all); /* 1CBE:0387 */
int  unlink_item(unsigned far *list, int pos, int item);         /* 1CBE:02BF */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
int  create_item(int type);                                      /* 1CBE:092B */
void delete_items(int block, int type);                          /* 1CBE:04C6 */
void consume_hand_item(int a, int b);                             /* 1CBE:0101 */
void identify_list(unsigned far *list);                          /* 1CBE:042D */
void modify_hp(int ch, int amount);                              /* 1F55:0AC5 */
void inflict_damage(int ch, int a, int b, int c, int d, int e, int f); /* 1F55:143E */
void give_party_xp(int xp);                                      /* 1F55:0C0F */
void take_item(int item);                                    /* 18AE:0197 */
int  count_monsters(int type);                                   /* 24EC:22B4 (this module, after op_eval) */
int  count_party_items(int a, int b);                                 /* 1E24:0BBC */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void play_sequence(int seq);                                     /* 56DD:0BAE (overlay) */
void far run_block_script(int block, unsigned events);
void monsters_look(void);                                        /* 1997:033D */
void missiles_check(void);                                       /* 27B7:0EC1 */
int  mouse_in_rect(int x1, int y1, int x2, int y2);              /* 18D7:0AD7 */
void play_sound_here(int sound);                                 /* 2A30:0000 */
void set_timer(int timer, int ticks);
int  cursor_in_rect(int x1, int y1, int x2, int y2);             /* 3269:0213 waits for the mouse handler, then tests the cursor */
int  get_hand_item(void);                                        /* 1E24:0352 the item held (or the selected inventory slot) */
void place_item(int block, int item, int pos);                   /* 1CBE:0474 */
void draw_inventory_slot(int slot, int a, int b);                /* 1CBE:09A6 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
int  launch_item(int from, int item, int block, int pos, int dir, int type);   /* 1CBE:086A */
void launch_spell(char from, int spell, int block, int pos, int dir);          /* 20E2:084A */
char far *strcpy(char far *dst, char far *src);
void close_panels(void);                                         /* 1E24:0B5B closes the inventory / spell panels (?) */
void leave_level(void);                                          /* 559A:0EC5 (overlay) */
void save_level(int level);                                      /* 559A:02EA (overlay) LEVELS.TMP */
void more_off(void);                                             /* 31B0:0129 */
int  select_window(int w);                                       /* 31B0:0133 */
void load_level(int level, int sublevel);                        /* 53EC:17F5 (overlay) */
void end_dialogue(void);                                         /* 56DD:0D37 (overlay) */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void draw_lightning(void);                                       /* 4A5D:1DEC (overlay) */
void copy_page(int a, int b, int c, int d, int e, int f, int g, int h);   /* 2D92:000A */
void choose_character(void);                                         /* 56DD:0C1D (overlay) */
void choose_raise(void);                                         /* 56DD:1035 (overlay) */
int  make_room(int type, int value);                             /* 56DD:0667 (overlay) */
void npc_joins(int npc);                                         /* 56DD:071B (overlay) */
int  take_party_items(int a, int b);                                 /* 1E24:0212 */
void load_vcn(char far *name, char far *p);                      /* 53EC:0BCE (overlay) */
long xp_for_level(char cls, char part, char level);              /* 1B54:0515 -1: no such class part */
void give_xp(int ch, long xp);                                   /* 1F55:0B21 */
void start_dialogue(void);                                       /* 56DD:0CE1 (overlay) */
void clear_dialogue(void);                                       /* 56DD:01DD (overlay) */
void dialogue_text(int text, char far *button);                  /* 56DD:033E (overlay) */
void dialogue_picture(byte far *name, int a, int b, int c, int d); /* 56DD:0DDE (overlay) */
char dialogue_choice(int text, char far *b1, char far *b2, char far *b3); /* 56DD:0D73 (overlay) */
void view_nothing(void);                                         /* 4A5D:2127 (overlay) empty */
void redraw_party(void);                                         /* 1E24:0686 */
void free_monster_shapes(int first, int count);                  /* 53EC:050C (overlay) */
void draw_game_screen(void);                                     /* 1E24:0B88 (?) */
void load_monster_gfx_a(byte far *p, int first, int n);          /* 53EC:08AA (overlay) */
void load_monster_gfx_b(byte far *p, int first, int n);          /* 53EC:0948 (overlay) */
void load_monster_gfx_c(byte far *p, int first, int n);          /* 53EC:09E6 (overlay) */
char far *strupr(char far *s);
char far *strstr(char far *s, char far *sub);
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */

/* Level timers: every "interval" ticks, the script of square "block" runs with event 0x80 */
typedef struct TimedEvent { int block; int interval; unsigned long next; } TimedEvent;
extern TimedEvent     TimedEvents[];     /* CBC8 */
extern int            TimedEventCount;   /* E4C8 */
extern unsigned long  GameTicks;         /* 6684 */
extern int            StepCounter;       /* E5C8 */
extern char           Var_DF24;          /* DF24 */
extern char           Scratch[];         /* F16E */
extern char           InventoryMode;     /* 6655 the inventory of a character is shown */
extern char           InvCharacter;      /* 9317 the character whose inventory is shown */
extern char           InvSlot;           /* E5A4 the selected slot (16 = quiver) */
extern char           WallDecoration[256];/* 21C8 decoration of a wall type, 0 = none */
extern char           WallClickType[256];/* 2218 what a click on the wall does, 1-based index in WallClick[] */
extern char           ItemSize[];        /* 2590 per item icon: > 14 = fits in a niche (?) */
extern char far      *ViewWalls[18];     /* DEDC &LevelMap[ViewBlock[i]].walls; [13] = the square in front */
extern char           Front;             /* 9444 the face of a square looking at the party */
extern int            ViewX;             /* 2500 x of the 3D view on the screen */
typedef struct DoorButton { char far *shape; byte x, y; } DoorButton;
extern DoorButton     DoorButtons[];     /* 931E the button drawn on doors */
extern char           CurrentSpell;      /* DF2A the spell being cast (read as a byte here) */
extern int            LastThrownItem;    /* CC32 the item that hit the square */
extern ItemType       ItemTypes[];       /* 946E */
extern char far       ItemNames[][35];   /* 37DA:0000 (a far data segment) */
extern int            CurrentSubLevel;   /* 6DD0 */
extern int            HandItem;          /* 6DD6 the item held by the mouse */
extern int            InDialogue;        /* EE38 a dialogue picture is shown (?) */
extern signed char    FrontPos[4][4];    /* 2342 positions in the square in front of the party, per direction */
extern int            Var_62A5;          /* 62A5 */
extern byte far      *ShrinkBuffer;          /* CBC4 */
extern void (far *Drv_6542)(int a, int b, int c, int d, byte far *e);   /* 6542 video driver entry 1 */
extern void (far *Drv_658A)(int a, int b, int c, int d, byte far *e);   /* 658A video driver entry 19 */
extern signed char    ClassCount[];      /* 1EC8 number of classes of a (multi-)class */
extern char           Var_DF2C;          /* DF2C */
extern char           Var_EE3A;          /* EE3A a mode of the inventory screen (?) */
extern int            TextColor;         /* 636D */
extern char           Var_6DE1;          /* 6DE1 */          /* DF2C */
extern char           Var_E4C6;          /* E4C6 set by launch_spell (spell from a character?) */
extern int            Var_DF4C;          /* DF4C 0x40 while a click is being checked (?) */

/* Doors being opened or closed one step per timer tick (timer_doors) */
typedef struct MovingDoor { int block; signed char dir; } MovingDoor;
extern MovingDoor     MovingDoors[3];    /* 929C */
extern char           WallTypeFlags[256];/* 2268  01 passable, 08 see-through, 10 door open, 20 door closed, 80 door */

typedef struct Decoration {
	byte shape[10];
	char next;
	byte flags;
	int  x[10];
	int  y[10];
} Decoration;
extern Decoration     Decorations[60];   /* D1CC */
extern char far      *DecoShapes[300];   /* DF50 */

/* ------------------------------------------------------------------ */
/* Module 24EC, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5A28...) */
int         TeleportCount = 0;                     /* 5A28 */
signed char DoorEndFlag[3] = { 0x20, 0, 0x10 };    /* 5A2A the flag that stops a door moving in direction dir, at [dir + 1] */
int         ForceChance[20] = {                    /* 5A2D out of 20, per strength (capped at 18) */
	1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10, 11, 12, 13 };
signed char DoorButtonOf[32] = {                   /* 5A55 per door type: its button (DoorButtons) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 3 };

/* 24EC:000B [asm] - the level's timed scripts */
void far scripts_update(void)
{
	TimedEvent far *t = TimedEvents;
	int i;

	for (i = 0; i < TimedEventCount; i++, t++) {
		if (t->next < GameTicks) {
			run_block_script(t->block, 0x80);
			t->next = t->interval + GameTicks;
			SceneDirty = 1;
		}
	}
}

/* 24EC:0076 [asm] */
void far teleport_party(int block)
{
	int i;

	monsters_look();                              /* the monsters may notice the party */
	i = PartyBlock;
	PartyBlock = block;
	run_block_script(i, 2);                       /* the party leaves the old square */
	if (++TeleportCount > 3) {
		gui_printf("\r");
		TeleportCount = 0;
	}
	run_block_script(block, 1);                   /* ... and enters the new one */
	if (LevelMap[block].walls[0] == 0x1A)
		for (i = 0; i < 4; i++)
			LevelMap[block].walls[i] = 0;
	monsters_look();                              /* the monsters may notice the party */
	StepCounter++;
	Var_DF24 = -1;
	SceneDirty = 1;
	missiles_check();
}

/* 24EC:0108 [asm] - is the mouse on one of the decorations of the chain? */
int far deco_clicked(int deco)
{
	int x, y, shape, w, h, hit;

	if (Var_DF4C != 0x40)
		return 1;
	hit = 0;
	do {
		deco--;
		x = Decorations[deco].x[1] - 3;
		y = Decorations[deco].y[1] - 3;
		shape = Decorations[deco].shape[1];
		w = DecoShapes[shape][2] * 8 + 6;
		h = DecoShapes[shape][1] + 6;
		if (Decorations[deco].flags & 1)
			x = 0xB0 - x - w;
		hit |= mouse_in_rect(x, y, x + w, y + h);
		deco = Decorations[deco].next;
	} while (deco > 0 && !hit);
	return hit;
}

/* 24EC:01EB [asm] */
int far block_has_monsters(int block)
{
	return (LevelMap[block].flags & 7) != 0;
}

/* 24EC:0209 [asm] - start moving a door (dir +1 opens, -1 closes). The door
 * wall is on sides 0 and 2, or 1 and 3; its type steps by dir until it reaches
 * the fully open / closed type. At most three doors move at the same time: a
 * fourth one jumps directly to its final state. */
void far move_door(int block, int dir)
{
	char wall, side;
	int  free, slot, mask, back;
	int  i;

	free = slot = -1;
	side = (WallTypeFlags[LevelMap[block].walls[0]] & 8) ? 0 : 1;
	wall = LevelMap[block].walls[side];
	mask = DoorEndFlag[dir + 1];
	if (WallTypeFlags[wall] & mask)
		return;                                   /* already there */
	for (i = 0; i < 3; i++) {
		if (MovingDoors[i].block == block) {
			slot = i;
			break;
		}
		if (MovingDoors[i].block == 0 && free == -1)
			free = i;
	}
	if (slot == -1 && free == -1) {
		/* no free slot: the door is moved at once */
		side = (WallTypeFlags[LevelMap[block].walls[0]] & 8) ? 0 : 1;
		wall = LevelMap[block].walls[side];
		mask = DoorEndFlag[dir + 1];
		while (!(WallTypeFlags[wall] & mask))
			wall += dir;
		LevelMap[block].walls[side]     = wall;
		LevelMap[block].walls[side ^ 2] = wall;
		block_changed(block);
		return;
	}
	if (slot == -1)
		slot = free;
	MovingDoors[slot].block = block;
	MovingDoors[slot].dir   = dir;
	back = -dir;
	mask = DoorEndFlag[back + 1];
	if (WallTypeFlags[wall] & mask) {
		/* the door leaves its end position: first step now, with its sound */
		LevelMap[block].walls[side]     = LevelMap[block].walls[side] + dir;
		LevelMap[block].walls[side ^ 2] = LevelMap[block].walls[side ^ 2] + dir;
		play_sound_here(dir == -1 ? 4 : 3);
	}
	set_timer(12, 1);
}

/* 24EC:0426 [asm] */
void far open_door(int block)
{
	move_door(block, 1);
}

/* 24EC:0439 [asm] - a door does not close on monsters or on the party */
void far close_door(int block)
{
	if (!(LevelMap[block].flags & 7) && PartyBlock != block)
		move_door(block, -1);
}

/* 24EC:0466 [asm] - open a closed door, close an open one; a moving door
 * turns back. ("side" is only used as a variable.) */
void far toggle_door(int block, int side)
{
	int dir, i;

	if (LevelMap[block].flags & 7)
		return;
	if (PartyBlock == block)
		return;
	for (dir = i = 0; i < 3; i++)
		if (MovingDoors[i].block == block)
			dir = -MovingDoors[i].dir;
	if (dir == 0) {
		side = (WallTypeFlags[LevelMap[block].walls[0]] & 8) ? 0 : 1;
		if (WallTypeFlags[LevelMap[block].walls[side]] & 1)
			dir = -1;
		else
			dir = 1;
	}
	move_door(block, dir);
}

/* 24EC:051C [asm] - "force" a stuck door (clicked by the player): the
 * strongest active character rolls 1d20 against ForceChance[strength].
 * Returns 0 when the click was not on the door. */
int far force_door(int block, int side)
{
	int str, i, best;

	if (mouse_in_rect(0x28, 0x10, 0x88, 0x58) || Var_DF4C != 0x40) {
		best = -1;
		for (i = 0; i < 6; i++) {
			if (char_is_active(i, 13)) {
				if (best >= 0) {
					/* ORIGINAL BUG: the candidate is given the exceptional strength
					 * (18/xx) of the current best, not his own */
					if (Party[best].str + Party[best].strExt <= Party[i].str + Party[best].strExt)
						best = i;
				} else
					best = i;
			}
		}
		if (best == -1) {
			/* ORIGINAL BUG (?): the message is formatted but never shown */
			sprintf(Scratch, "You are not capable of forcing the door.\r");
		} else {
			str = Party[best].str;
			if (str > 18)
				str = 18;
			if (roll_dice(1, 20, 0) <= ForceChance[str]) {
				gui_printf("\x06\x04You force the door.\r");
				if (LevelMap[block].walls[side] == 0x33) {
					LevelMap[block].walls[side]     = 8;
					LevelMap[block].walls[side ^ 2] = 8;
				} else {
					LevelMap[block].walls[side]     = 0x12;
					LevelMap[block].walls[side ^ 2] = 0x12;
				}
				open_door(block);
			} else
				gui_printf("\x06\x06You try to force the door but fail.\r");
		}
		return 1;
	}
	return 0;
}

int far pry_door(int block, int side);

/* 24EC:06A8 [asm] - click on a door: on its button the door opens or closes */
int far door_click(int block, int side)
{
	int x, y, type, w, h, dx;
	char far *shape;
	int k;

	type = ViewWalls[13][Front];             /* the wall in front of the party */
	k = DoorButtonOf[type];
	shape = DoorButtons[k].shape;
	dx = ViewX;
	x = DoorButtons[k].x + dx - 4;
	y = DoorButtons[k].y - 4;
	w = shape[2] * 8 + 8;
	h = shape[1] + 8;
	if (mouse_in_rect(x, y, x + w, y + h) || Var_DF4C != 0x40) {
		toggle_door(block, side);
		play_sound(6);
		return 1;
	}
	return pry_door(block, side);
}

/* 24EC:0789 [asm] - click on a decoration: event 0x40 for the square's script */
int far deco_click(int block, int side)
{
	int deco;

	deco = WallDecoration[LevelMap[block].walls[side]];
	if (deco_clicked(deco)) {
		run_block_script(block, 0x40);
		return 1;
	}
	return 0;
}

/* 24EC:07D3 [asm] - lever / switch: the wall type goes to the next one */
int far switch_up(int block, int side)
{
	int deco;

	deco = WallDecoration[LevelMap[block].walls[side]];
	if (deco_clicked(deco)) {
		LevelMap[block].walls[side]++;
		SceneDirty = 1;
		run_block_script(block, Var_DF4C);
		return 1;
	}
	return 0;
}

/* 24EC:0831 [asm] - ... or to the previous one */
int far switch_down(int block, int side)
{
	int deco;

	deco = WallDecoration[LevelMap[block].walls[side]];
	if (deco_clicked(deco)) {
		LevelMap[block].walls[side]--;
		SceneDirty = 1;
		run_block_script(block, Var_DF4C);
		return 1;
	}
	return 0;
}

/* 24EC:088F [asm] - a niche (items at position 8 of the square): take the
 * item (event 8) or put the held one there (event 4) */
int far niche_click(int block, int side)
{
	int deco;
	int item;

	deco = WallDecoration[LevelMap[block].walls[side]];
	if (deco_clicked(deco)) {
		if (InventoryMode && InvSlot == 16) {
			/* the quiver is selected: only arrows go there */
			item = unlink_item(&LevelMap[block].items, 8, -1);
			if (Items[item].type == 16) {
				put_item(&Party[InvCharacter].inventory[16], -2, item, 0);
				draw_inventory_slot(16, 1, 0);
			} else {
				gui_printf("You can't put that item there.\r");
				place_item(block, item, 8);
			}
			return 1;
		}
		if (get_hand_item() == 0) {
			item = unlink_item(&LevelMap[block].items, 8, -1);
			if (item) {
				run_block_script(block, 8);
				take_item(item);
				SceneDirty = 1;
			}
		} else if (ItemSize[Items[get_hand_item()].icon] > 14) {
			put_item(&LevelMap[block].items, block, get_hand_item(), 8);
			run_block_script(block, 4);
			take_item(0);
			SceneDirty = 1;
		} else
			gui_printf("The item is too large to fit.\r");
		return 1;
	}
	return 0;
}

/* 24EC:0A0D [asm] - a door that cannot be opened from here */
int far pry_door(int block, int side)
{
	if ((cursor_in_rect(0x28, 0x10, 0x88, 0x58) || Var_DF4C != 0x40)
	    && (WallTypeFlags[LevelMap[block].walls[side]] & 0x20)) {
		gui_printf("No one is able to pry this door open.\r");
		return 1;
	}
	return 0;
}

/* 24EC:0A64 [asm] - a wall that only runs the script */
int far wall_event(int block, int side)
{
	side++;                                       /* (unused) */
	run_block_script(block, Var_DF4C);
	return 1;
}

/* 24EC:0A81 [asm] - the player clicked on side "side" of square "block" (or
 * an event Var_DF4C happened there): the wall type decides what it does. */
int far wall_click(int block, int side)
{
	int mask, r;
	int (far *click[10])(int block, int side) = {
		door_click, deco_click, switch_up, switch_down, force_door,
		pry_door, wall_event, deco_click, wall_event, niche_click
	};
	int type;

	side ^= 2;                                    /* the side facing the party */
	type = LevelMap[block].walls[side];
	if (!WallClickType[type])
		return 0;
	mask = ((LevelMap[block].flags & 0xF8) >> 3) | 0xE0;
	if (!(Var_DF4C & mask))
		return 0;
	if (WallClickType[type] - 1 < 0)
		return 0;
	r = click[WallClickType[type] - 1](block, side);
	Var_DF4C = 0;
	SceneDirty = 1;
	/* ORIGINAL BUG: no return value here (AX still holds r) */
}

/* 24EC:0B21 [asm] - click on the wall in front of the party */
void far click_front_wall(byte far *p)
{
	int block;

	p += 60;                                      /* (unused) */
	Var_DF4C = 0x40;
	block = block_ahead(PartyBlock, PartyDirection);
	wall_click(block, PartyDirection);
}

/* ------------------------------------------------------------------ */
/* Script opcodes                                                      */
/* ------------------------------------------------------------------ */

/* FF  F7 block.w type.b          all four sides
 *     E9 block.w side.b type.b   one wall side
 *     ED dir.b                   turn the party            24EC:0B55 [asm] */
int far op_set_wall(byte far *p)
{
	byte far *start = p;
	char v;
	int  i, block;

	switch ((signed char)*p++) {
	case -0x09:                                   /* F7 */
		block = *(int far *)p;  p += 2;
		v = *p++;
		for (i = 0; i <= 3; i++)
			LevelMap[block].walls[i] = v;
		block_changed(block);
		break;
	case -0x17:                                   /* E9 */
		block = *(int far *)p;  p += 2;
		v = *p++;
		LevelMap[block].walls[v] = *p++;
		block_changed(block);
		break;
	case -0x13:                                   /* ED */
		v = *p++;
		PartyDirection = v;
		break;
	}
	return p - start;
}

/* FE  F7 block.w a.b b.b         all sides: a <-> b (decided on side 0)
 *     E9 block.w side.b a.b b.b  one side: a <-> b
 *     EA block.w                 open / close the door      24EC:0C24 [asm] */
int far op_toggle_wall(byte far *p)
{
	int  a, b;
	byte far *start = p;
	int  block, side;

	switch ((signed char)*p++) {
	case -0x09:                                   /* F7 */
		block = *(int far *)p;  p += 2;
		a = (signed char)*p++;
		b = (signed char)*p++;
		if (LevelMap[block].walls[0] == a) {
			LevelMap[block].walls[0] = b;
			LevelMap[block].walls[1] = b;
			LevelMap[block].walls[2] = b;
			LevelMap[block].walls[3] = b;
		} else {
			LevelMap[block].walls[0] = a;
			LevelMap[block].walls[1] = a;
			LevelMap[block].walls[2] = a;
			LevelMap[block].walls[3] = a;
		}
		block_changed(block);
		break;
	case -0x17:                                   /* E9 */
		block = *(int far *)p;  p += 2;
		side = (signed char)*p++;
		a = (signed char)*p++;
		b = (signed char)*p++;
		if (LevelMap[block].walls[side] == a)
			LevelMap[block].walls[side] = b;
		else
			LevelMap[block].walls[side] = a;
		block_changed(block);
		break;
	case -0x16:                                   /* EA */
		block = *(int far *)p;  p += 2;
		toggle_door(block, -1);
		break;
	}
	return p - start;
}

/* FD block.w   24EC:0DA8 [asm] */
int far op_open_door(byte far *p)
{
	int  block;
	byte far *start = p;

	block = *(int far *)p;  p += 2;
	open_door(block);
	return p - start;
}

/* FC block.w   24EC:0DDF [asm] */
int far op_close_door(byte far *p)
{
	int  block;
	byte far *start = p;

	block = *(int far *)p;  p += 2;
	close_door(block);
	return p - start;
}

/* FB unused.b index.b block.w pos.b dir.b type.b shape.b mode.b sub.b a.w b.w   24EC:0E16 [asm] */
int far op_create_monster(byte far *p)
{
	int  index, block, pos, dir, type, shape, mode, sub, a, b;
	byte far *start = p;

	p++;                                          /* the first operand byte is not used */
	index = (signed char)*p++;
	block = *(int far *)p;  p += 2;
	pos   = (signed char)*p++;
	dir   = (signed char)*p++;
	type  = (signed char)*p++;
	shape = (signed char)*p++;
	mode  = (signed char)*p++;
	sub   = (signed char)*p++;
	a     = *(int far *)p;  p += 2;
	b     = *(int far *)p;  p += 2;
	create_monster(index, block, pos, dir, type, shape, mode, sub, a, b);
	return p - start;
}

/* Where a missile starts in the square, per direction (two choices) */
signed char LaunchPos[4][2] = { { 2, 3 }, { 0, 2 }, { 1, 0 }, { 3, 1 } };   /* 5A9D */

/* E9  DF spell.w block.w dir.b which.b    cast a spell from the square
 *     EC item.w  block.w dir.b which.b    throw a new item (only the low byte of
 *                                         spell / item is used)    24EC:0EE2 [asm] */
int far op_launch(byte far *p)
{
	char what, dir, which, saved, spell;
	byte far *start = p;
	int  item, block;

	spell = *p == 0xDF;
	p++;
	what = *p;  p += 2;
	block = *(int far *)p;  p += 2;
	dir   = *p++;
	which = *p++;
	if (!spell) {
		item = create_item(what);
		if (item) {
			if (!launch_item(-1, item, block, LaunchPos[dir][which], dir, Items[item].type))
				Items[item].block = -1;           /* could not be thrown: the item is lost */
		}
	} else {
		saved = Var_E4C6;
		Var_E4C6 = 0;
		launch_spell(-1, what, block, LaunchPos[dir][which], dir);
		Var_E4C6 = saved;
	}
	return p - start;
}

/* FA  E8 src.w dst.w              teleport the party
 *     F3 src.w dst.w              move the monsters standing on src
 *     F5 lvl src.w lvl dst.w      move all the items
 *     E1 type.w lvl src.w lvl dst.w   move the items of one type
 * lvl = EB for "this level", else level.b. The party's teleport runs scripts
 * (leaving / entering squares): the state of the running script is saved
 * around it.                                                  24EC:0FED [asm] */
int far op_move(byte far *p)
{
	Monster far *m;
	Missile far *t;
	int  pos, sub, src, item, type, srcLevel, dstLevel;
	int  result, block, events, end, ch, depth;
	byte far *start = p;
	int  i, dst;

	sub = (signed char)*p;
	p++;
	if (sub == -0x1F) {                           /* E1 */
		type = *(int far *)p;  p += 2;
	} else
		type = -1;
	if (sub != -0x1F && sub != -0x0B) {
		src = *(int far *)p;  p += 2;
		dst = *(int far *)p;  p += 2;
	}
	switch (sub) {
	case -0x18:                                   /* E8 the party */
		result = ScriptResult;
		block  = ScriptBlock;
		events = ScriptEvents;
		end    = ScriptEnd;
		ch     = ScriptCharacter;
		depth  = CallDepth;
		teleport_party(dst);
		ScriptResult    = result;
		ScriptBlock     = block;
		ScriptEvents    = events;
		ScriptEnd       = end;
		ScriptCharacter = ch;
		if (!ScriptAbort)
			CallDepth = depth;
		Var_DF2C = 0;
		break;
	case -0x0D:                                   /* F3 monsters */
		m = Monsters;
		for (i = 0; i < 30; m++, i++)
			if (m->block == src)
				move_monster(m, dst, m->pos);
		break;
	case -0x0B:                                   /* F5 items */
	case -0x1F:                                   /* E1 items of one type */
		if (*p++ == 0xEB)
			srcLevel = CurrentLevel;
		else {
			srcLevel = (signed char)*p;
			p++;
		}
		src = *(int far *)p;  p += 2;
		if (*p++ == 0xEB)
			dstLevel = CurrentLevel;
		else {
			dstLevel = (signed char)*p;
			p++;
		}
		dst = *(int far *)p;  p += 2;
		if (srcLevel == CurrentLevel) {
			while ((item = find_item(&LevelMap[src].items, -1, type, 0, 1)) != 0) {
				pos = Items[item].pos;
				unlink_item(&LevelMap[src].items, 0, item);
				if (dstLevel == CurrentLevel)
					put_item(&LevelMap[dst].items, dst, item, pos);
				else {
					Items[item].level = dstLevel;
					Items[item].block = dst;
					if (pos < 8)
						Items[item].pos = pos & 3;
				}
			}
			/* items flying over the square */
			t = Thrown;
			for (i = 0; i < 10; t++, i++)
				if (t->active == 1 && t->block == src) {
					if (dstLevel == CurrentLevel)
						t->block = dst;
					else
						t->active = 0;
				}
		} else {
			for (i = 0; i < 600; i++)
				if (Items[i].level == srcLevel && Items[i].block == src) {
					Items[i].level = dstLevel;
					Items[i].block = dst;
				}
		}
		break;
	}
	SceneDirty = 1;
	return p - start;
}

/* F9 ch.b block.w pos.b   a thief: one small item of a character goes to the
 * square (ch = -1: a random character). Hands only with Var_EE3A, never the
 * quiver.                                                      24EC:12CF [asm] */
int far op_steal_item(byte far *p)
{
	int  block, item, n, pos;
	byte far *start = p;
	int  slot, ch;

	ch = (signed char)*p;
	p++;
	block = *(int far *)p;  p += 2;
	pos = (signed char)*p;
	p++;
	if (ch == -1)
		ch = roll_dice(1, 6, -1);
	while (!Party[ch].flags) {
		ch++;
		if (ch > 5)
			ch = 0;
	}
	if (Var_EE3A && InvCharacter == ch)
		return p - start;                         /* his inventory is on the screen */
	item = 0;
	slot = roll_dice(1, 27, 0);
	for (n = 0; n < 27; n++) {
		if ((Var_EE3A || slot > 1) && slot != 16) {
			item = Party[ch].inventory[slot];
			if (item && ItemSize[Items[item].icon] >= 15)
				break;
		}
		slot++;
		if (slot >= 27)
			slot = 0;
	}
	if (n < 27 && item) {
		Party[ch].inventory[slot] = 0;
		put_item(&LevelMap[block].items, block, item, pos);
	}
	return p - start;
}

/* F8 text.w colour.b unused.b   a message; %s in the text = the character chosen
 * by the last condition, or a random one                       24EC:1412 [asm] */
int far op_message(byte far *p)
{
	int  saved, index, off;
	byte far *start = p;
	char far *text;
	int  ch;

	index = *(int far *)p;  p += 2;
	saved = TextColor;
	TextColor = (signed char)*p;
	p++;
	p++;
	if (ScriptCharacter == -1) {
		ch = roll_dice(1, 6, -1);
		while (!char_is_active(ch, 3)) {
			ch++;
			if (ch > 5)
				ch = 0;
		}
	} else
		ch = ScriptCharacter;
	off = *(int far *)ScriptCode;
	text = get_script_string(ScriptCode + off, index);
	gui_printf(text, Party[ch].name);
	TextColor = saved;
	gui_printf("\r");
	return p - start;
}

/* F7  EF bit.b                   flag of the level
 *     F0 bit.b                   global flag
 *     F3 monster.b bit.b         flag of a monster (and its AI state is reset)
 *     E4                         ScriptResult = 1
 *     D1                         Var_6DE1 = 0                  24EC:14E8 [asm] */
int far op_set_flags(byte far *p)
{
	int  sub, bit;
	byte far *start = p;
	int  m;

	sub = (signed char)*p;
	p++;
	switch (sub) {
	case -0x11:                                   /* EF */
		bit = (signed char)*p;
		p++;
		LevelFlags[CurrentLevel] |= 1L << bit;
		break;
	case -0x10:                                   /* F0 */
		bit = (signed char)*p;
		p++;
		GlobalFlags |= 1L << bit;
		break;
	case -0x0D:                                   /* F3 */
		m = (signed char)*p;
		p++;
		bit = (signed char)*p;
		p++;
		Monsters[m].flags |= 1 << bit;
		Monsters[m].mode = 0;
		break;
	case -0x1C:                                   /* E4 */
		ScriptResult = 1;
		break;
	case -0x2F:                                   /* D1 */
		Var_6DE1 = 0;
		break;
	}
	return p - start;
}

/* F6 sound.b block.w   block 0: heard everywhere, else from that square   24EC:15D8 [asm] */
int far op_play_sound(byte far *p)
{
	byte far *start = p;
	char sound;
	int  block;

	sound = *p;
	p++;
	block = *(int far *)p;  p += 2;
	if (block)
		play_sound_at(sound, block);
	else
		play_sound(sound);
	return p - start;
}

/* F5  EF bit.b  /  F0 bit.b      clear a flag of the level / a global flag
 *     E4                         ScriptResult = 0
 *     D1                         Var_6DE1 = 1
 * (the byte after the sub-opcode is always read: E4 and D1 give it back) 24EC:162E [asm] */
int far op_clear_flags(byte far *p)
{
	int  sub, bit;
	byte far *start = p;

	sub = (signed char)*p;
	p++;
	bit = (signed char)*p;
	p++;
	switch (sub) {
	case -0x11:                                   /* EF */
		LevelFlags[CurrentLevel] &= ~(1L << bit);
		break;
	case -0x10:                                   /* F0 */
		GlobalFlags &= ~(1L << bit);
		break;
	case -0x1C:                                   /* E4 */
		ScriptResult = 0;
		p--;
		break;
	case -0x2F:                                   /* D1 */
		Var_6DE1 = 1;
		p--;
		break;
	}
	return p - start;
}

/* F4 ch.b amount.b   ch = -1: the whole party                  24EC:16D3 [asm] */
int far op_modify_hp(byte far *p)
{
	byte far *start = p;
	int  ch, amount;

	ch = (signed char)*p;
	p++;
	amount = (signed char)*p;
	p++;
	if (ch == -1)
		for (ch = 0; ch < 6; ch++)
			modify_hp(ch, amount);
	else
		modify_hp(ch, amount);
	return p - start;
}

/* F3 ch.b a.b b.b c.b d.b e.b f.b   damage (dice, saving throw...) ch = -1: the party   24EC:1735 [asm] */
int far op_damage(byte far *p)
{
	int  ch, a, b, c, d, e, f;
	byte far *start = p;

	ch = (signed char)*p;
	p++;
	a = (signed char)*p;
	p++;
	b = (signed char)*p;
	p++;
	c = (signed char)*p;
	p++;
	d = (signed char)*p;
	p++;
	e = (signed char)*p;
	p++;
	f = (signed char)*p;
	p++;
	if (ch == -1)
		for (ch = 0; ch < 6; ch++)
			inflict_damage(ch, a, b, c, d, e, f);
	else
		inflict_damage(ch, a, b, c, d, e, f);
	return p - start;
}

/* F2 target.w   24EC:17F6 [asm] */
int far op_jump(byte far *p)
{
	byte far *start = p;

	p = ScriptCode + *(int far *)p;
	return p - start;
}

/* F1   24EC:182A [asm] - (the p++ has no effect: the script ends) */
int far op_end(byte far *p)
{
	ScriptEnd = 1;
	CallDepth = 0;
	p++;
	return 0;
}

/* F0   24EC:183F [asm] - returning with an empty stack ends the script */
int far op_return(byte far *p)
{
	byte far *start = p;

	if (CallDepth != 0)
		p = CallStack[--CallDepth];
	else
		ScriptEnd = 1;
	return p - start;
}

/* EF target.w   24EC:1888 [asm] - at most 10 nested calls, deeper ones are ignored */
int far op_call(byte far *p)
{
	int  target;
	byte far *start = p;

	target = *(int far *)p;
	p += 2;
	if (CallDepth < 10) {
		CallStack[CallDepth++] = p;
		p = ScriptCode + target;
	}
	return p - start;
}

/* (the four "PUSH ...; continue;" below keep their own increment in the
 * original code: a plain "EvalStack[EvalSp] = x; EvalSp++; continue;" is
 * merged by the compiler into the EvalSp++ at the end of the loop. A
 * do { } while (0) block is the only form found that BC++ 2.0 leaves alone:
 * a macro like this one.) */
#define PUSH(x)  do { EvalStack[EvalSp] = (x); EvalSp++; } while (0)

/* EE <expression> EE else.w
 * The condition is in reverse Polish notation: values are pushed on EvalStack,
 * FF..F8 are operators. A false result jumps to "else".        24EC:18E7 [asm]
 *
 *   values                                   operators (pop b, pop a, push a op b)
 *   E4 ScriptResult      E0 ScriptEvents     FF ==   FE !=   FD <    FC <=
 *   ED party direction   E8 characters able  FB >    FA >=   F9 &&   F8 ||
 *   F7 block.w     wall type (side 0)        E9 side.b block.w   wall type
 *   F5 a.b b.b block.w   find_item           F2 type.w block.w   find_item
 *   F3 FF block.w  monsters on the square    F3 xx t.b t.b .. FF  monsters of types
 *   F1 F5 a.w b.w  count_party_items(a, b)        F1 xx block.w       the party is there
 *   F0 bit.b / EF bit.b  global / level flag
 *   E7 E1/F5/F6/D0 n.b name/CF n.b name   the item in hand: type, index, value,
 *                  unidentified / identified name contains "name"
 *   CE n.b   a character has portrait n (ScriptCharacter = him)
 *   DD race.b / DC mask.b   a character of this race / class (from a random one)
 *   DB count.b sides.b add.b  dice           D7 F5/E1/F6/DC  the item that hit:
 *   DF the spell being cast                     index, type, value, kind
 *   DA nobody can act (all absent or invisible)    D2 value.w   a number      */
int far op_eval(byte far *p)
{
	int  op;
	Item far *it;
	int  v, w, x, i, ch, n, y;
	byte far *start = p;
	char len;
	char name[36], word[36];
	int  a, b;

	while ((op = (signed char)*p++) != -0x12) {
		switch (op) {
		case -0x1C:                               /* E4 */
			EvalStack[EvalSp] = ScriptResult;
			break;
		case -0x20:                               /* E0 */
			EvalStack[EvalSp] = ScriptEvents;
			break;
		case -0x13:                               /* ED */
			EvalStack[EvalSp] = PartyDirection;
			break;
		case -0x18:                               /* E8 */
			for (n = i = 0; i < 6; i++)
				if (char_is_active(i, 15))
					n++;
			EvalStack[EvalSp] = n;
			break;
		case -0x09:                               /* F7 */
			v = *(int far *)p;  p += 2;
			EvalStack[EvalSp] = LevelMap[v].walls[0];
			break;
		case -0x0B:                               /* F5 */
			y = (signed char)*p;
			p++;
			x = (signed char)*p;
			p++;
			v = *(int far *)p;  p += 2;
			v = find_item(&LevelMap[v].items, -1, x, 1, y);
			EvalStack[EvalSp] = v;
			break;
		case -0x0D:                               /* F3 */
			if (*p++ == 0xFF) {
				v = *(int far *)p;  p += 2;
				EvalStack[EvalSp] = LevelMap[v].flags & 7;
				break;
			}
			n = 0;
			while (*p != 0xFF)
				n += count_monsters((signed char)*p++);
			p++;
			PUSH(n);
			continue;
		case -0x0E:                               /* F2 */
			w = *(int far *)p;  p += 2;
			v = *(int far *)p;  p += 2;
			EvalStack[EvalSp] = find_item(&LevelMap[v].items, w, -1, 0, 0);
			break;
		case -0x0F:                               /* F1 */
			if (*p++ == 0xF5) {
				v = *(int far *)p;  p += 2;
				w = *(int far *)p;  p += 2;
				EvalStack[EvalSp] = count_party_items(v, w);
				break;
			}
			v = *(int far *)p;  p += 2;
			PUSH(PartyBlock == v);
			continue;
		case -0x10:                               /* F0 */
			v = (signed char)*p;
			p++;
			if (GlobalFlags & (1L << v)) {
				EvalStack[EvalSp] = 1;
				break;
			}
			PUSH(0);
			continue;
		case -0x11:                               /* EF */
			v = (signed char)*p;
			p++;
			if (LevelFlags[CurrentLevel] & (1L << v)) {
				EvalStack[EvalSp] = 1;
				break;
			}
			PUSH(0);
			continue;
		case -0x19:                               /* E7 the item in hand */
			w = get_hand_item();
			it = &Items[w];
			switch ((signed char)*p++) {
			case -0x1F:                           /* E1 */
				EvalStack[EvalSp] = it->type;
				break;
			case -0x0B:                           /* F5 */
				EvalStack[EvalSp] = w;
				break;
			case -0x0A:                           /* F6 */
				EvalStack[EvalSp] = it->value;
				break;
			case -0x30:                           /* D0 */
				len = *p;
				p++;
				strcpy(name, ItemNames[it->nameUnid]);
				strupr(name);
				strcpy(word, p);
				strupr(word);
				p += len;
				EvalStack[EvalSp] = strstr(name, word) != 0;
				break;
			case -0x31:                           /* CF */
				len = *p;
				p++;
				strcpy(name, ItemNames[it->name]);
				strupr(name);
				strcpy(word, p);
				strupr(word);
				p += len;
				EvalStack[EvalSp] = strstr(name, word) != 0;
				break;
			default:
				continue;
			}
			break;
		case -0x17:                               /* E9 */
			w = (signed char)*p;
			p++;
			v = *(int far *)p;  p += 2;
			EvalStack[EvalSp] = LevelMap[v].walls[w];
			break;
		case -0x32:                               /* CE */
			w = 0;
			v = (signed char)*p;
			p++;
			for (i = 0; i < 6; i++)
				if (char_is_active(i, 5) && Party[i].portrait == v) {
					w = 1;
					ScriptCharacter = i;
					break;
				}
			EvalStack[EvalSp] = w;
			break;
		case -0x23:                               /* DD */
			w = (signed char)*p;
			p++;
			ch = roll_dice(1, 6, 0);
			for (a = i = 0; i < 6; i++) {
				if (++ch > 5)
					ch = 0;
				if (char_is_active(ch, 5) && (Party[ch].raceSex >> 1) == w) {
					ScriptCharacter = ch;
					a = 1;
					break;
				}
			}
			EvalStack[EvalSp] = a;
			break;
		case -0x24:                               /* DC */
			w = (signed char)*p;
			p++;
			ch = roll_dice(1, 6, 0);
			for (a = i = 0; i < 6; i++) {
				if (++ch > 5)
					ch = 0;
				if (char_is_active(ch, 5) && (w & ClassFlags[Party[ch].cls])) {
					ScriptCharacter = ch;
					a = 1;
					break;
				}
			}
			EvalStack[EvalSp] = a;
			break;
		case -0x25:                               /* DB */
			v = (signed char)*p;
			p++;
			x = (signed char)*p;
			p++;
			w = (signed char)*p;
			p++;
			EvalStack[EvalSp] = roll_dice(v, x, w);
			break;
		case -0x29:                               /* D7 the item that hit the square */
			switch ((signed char)*p++) {
			case -0x0B:                           /* F5 */
				EvalStack[EvalSp] = LastThrownItem;
				break;
			case -0x1F:                           /* E1 */
				EvalStack[EvalSp] = Items[LastThrownItem].type;
				break;
			case -0x0A:                           /* F6 */
				EvalStack[EvalSp] = Items[LastThrownItem].value;
				break;
			case -0x24:                           /* DC */
				EvalStack[EvalSp] = ItemTypes[Items[LastThrownItem].type].flags & 0x7F;
				break;
			default:
				continue;
			}
			break;
		case -0x21:                               /* DF */
			EvalStack[EvalSp] = CurrentSpell;
			break;
		case -0x01:                               /* FF */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a == b;
			break;
		case -0x02:                               /* FE */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a != b;
			break;
		case -0x03:                               /* FD */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a < b;
			break;
		case -0x04:                               /* FC */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a <= b;
			break;
		case -0x05:                               /* FB */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a > b;
			break;
		case -0x06:                               /* FA */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a >= b;
			break;
		case -0x07:                               /* F9 */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a && b;
			break;
		case -0x08:                               /* F8 */
			b = EvalStack[--EvalSp];
			a = EvalStack[--EvalSp];
			EvalStack[EvalSp] = a || b;
			break;
		case -0x26:                               /* DA */
			b = 0;
			for (a = 0; a < 6; a++)
				if ((Party[a].flags & 1) == 1 && !(Party[a].effects & 0x40)) {
					b = 1;
					break;
				}
			EvalStack[EvalSp] = !b;
			break;
		case -0x2E:                               /* D2 */
			a = *(int far *)p;  p += 2;
			EvalStack[EvalSp] = a;
			break;
		default:
			continue;
		}
		EvalSp++;
	}
	v = EvalStack[--EvalSp];
	if (v)
		p += 2;
	else
		p = ScriptCode + *(int far *)p;
	return p - start;
}

/* 24EC:22B4 [asm] - monsters of a type alive on this sub-level */
int far count_monsters(int type)
{
	int i, n;

	n = 0;
	for (i = 0; i < 30; i++)
		if (Monsters[i].type == type && Monsters[i].sublevel == CurrentSubLevel && Monsters[i].hp >= 0)
			n++;
	return n;
}

/* ED  FF                    the item in hand is used up
 *     FE block.w            delete all the items lying on a square
 *     xx block.w            ... only those of type xx           24EC:2305 [asm] */
int far op_delete_item(byte far *p)
{
	byte far *start = p;
	char sub;
	int  block;

	sub = *p;
	p++;
	switch (sub) {
	case -1:
		consume_hand_item(0, -1);
		break;
	case -2:
		block = *(int far *)p;  p += 2;
		delete_items(block, -1);
		break;
	default:
		block = *(int far *)p;  p += 2;
		delete_items(block, sub);
		break;
	}
	return p - start;
}

/* Loaders of the monster graphics (by the kind of file), for EC 00 */
void (far *MonsterGfxLoader[3])(byte far *p, int first, int n) = {   /* 5AA5 */
	load_monster_gfx_a, load_monster_gfx_b, load_monster_gfx_c };

/* EC  E5 level.b sub.b block.w dir.b   go to another level (dir FF: unchanged)
 *     xx kind.b index.b name[13]       load monster graphics: MonsterGfxLoader[kind]
 *                                      (shapes index * 18 ...)    24EC:2376 [asm] */
int far op_load_level(byte far *p)
{
	char level, sub, dir, kind;
	byte far *start = p;
	int  block;

	close_panels();
	kind = *p;
	p++;
	level = *p;
	p++;
	if (kind == (char)0xE5) {
		sub = *p;
		p++;
		block = *(int far *)p;  p += 2;
		dir = *p;
		p++;
		PartyBlock = block;
		if (dir != -1)
			PartyDirection = dir;
		leave_level();
		save_level(CurrentLevel);
		more_off();
		select_window(7);
		load_level(level, sub);
		if (InDialogue)
			end_dialogue();
		view_nothing();
		teleport_party(block);
		ScriptEnd = 1;
		ScriptAbort = 1;
		SceneDirty = 1;
		redraw_party();
		CallDepth = 0;
		return 0;
	}
	block = (signed char)*p;
	p++;
	free_monster_shapes(block * 18, 18);
	MonsterGfxLoader[level](p, block * 18, 1);
	p += 13;
	view_nothing();
	draw_game_screen();
	return p - start;
}

/* EB  E2 xp.w    experience for the party                       24EC:24A9 [asm] */
int far op_experience(byte far *p)
{
	int  xp;
	byte far *start = p;

	if (*p++ == 0xE2) {
		xp = *(int far *)p;  p += 2;
		give_party_xp(xp);
	}
	return p - start;
}

/* EA type.w block.w pos.b flags.b [value.b] [b2.b] [icon.b]   a new item
 * block -1: in the hand (or in front of the party if the hand is full)
 * block -2: in front of the party                              24EC:24ED [asm] */
int far op_create_item(byte far *p)
{
	byte far *start = p;
	char pos, flags;
	int  item, block;

	item = *(int far *)p;  p += 2;
	block = *(int far *)p;  p += 2;
	pos = *p;
	p++;
	flags = *p;
	p++;
	item = create_item(item);
	if (flags & 1) {
		Items[item].value = *p;
		p++;
	}
	if (flags & 2) {
		Items[item].flags = *p;
		p++;
	}
	if (flags & 4) {
		Items[item].icon = *p;
		p++;
	}
	if (item) {
		if (block == -1) {
			if (HandItem == 0)
				take_item(item);
			else
				put_item(&LevelMap[PartyBlock].items, PartyBlock, item, FrontPos[PartyDirection][roll_dice(1, 2, -1)]);
		} else if (block == -2)
			put_item(&LevelMap[PartyBlock].items, PartyBlock, item, FrontPos[PartyDirection][roll_dice(1, 2, -1)]);
		else
			put_item(&LevelMap[block].items, block, item, pos);
	}
	return p - start;
}

/* E8  F1 n.b    turn the party by n quarters
 *     F5 n.b    turn the missiles flying over this square         24EC:2641 [asm] */
int far op_turn(byte far *p)
{
	Missile far *t;
	int  sub;
	byte far *start = p;
	int  i, n;

	sub = (signed char)*p;
	p++;
	n = (signed char)*p;
	p++;
	switch (sub) {
	case -0x0F:                                   /* F1 */
		PartyDirection = (PartyDirection + n) & 3;
		Var_DF24 = -1;
		SceneDirty = 1;
		break;
	case -0x0B:                                   /* F5 */
		t = Thrown;
		for (i = 0; i < 10; t++, i++)
			if (t->active && t->block == ScriptBlock)
				t->dir = (t->dir + n) & 3;
		break;
	}
	return p - start;
}

/* E7 block.w   identify the items of the square - and, when the party stands
 * there, all its items                                         24EC:26D8 [asm] */
int far op_identify(byte far *p)
{
	Character far *c;
	int  block, k;
	byte far *start = p;
	int  slot, item;

	block = *(int far *)p;  p += 2;
	if (block == PartyBlock) {
		c = Party;
		for (k = 0; k < 6; k++, c++) {
			c = &Party[k];
			if (c->flags & 1) {
				for (slot = 0; slot < 27; slot++) {
					item = c->inventory[slot];
					if (item)
						Items[item].flags |= 0x40;
				}
				identify_list(&c->inventory[16]);    /* the quiver is a list */
			}
		}
	}
	identify_list(&LevelMap[block].items);
	return p - start;
}

/* E5 ticks.w   wait (busy loop on the timer)                   24EC:2799 [asm] */
int far op_delay(byte far *p)
{
	unsigned long end;
	byte far *start = p;

	end = TimerTicks + *(int far *)p;
	p += 2;
	while (end > TimerTicks)
		;
	return p - start;
}

/* E6 n.b   an animated sequence (finale...)                    24EC:27EB [asm] */
int far op_sequence(byte far *p)
{
	byte far *start = p;

	play_sequence((signed char)*p++);
	return p - start;
}

/* E4   redraw the 3D view now. (Returns p - p: the opcode has no operand.)  24EC:281E [asm] */
int far op_draw_scene(byte far *p)
{
	draw_scene(1);
	return p - p;
}

/* E3  D5                         open the dialogue window
 *     D6                         clear it
 *     D4                         close it
 *     F8 text.w button.w         a text (of the dialogue table) and one button
 *     D3 name[13] a.b b.w c.w d.w   a picture
 *     D8 text.w b1.w b2.w b3.w   a question with up to three answers: the
 *                                answer goes to ScriptResult        24EC:2839 [asm] */
int far op_dialogue(byte far *p)
{
	byte far *start = p;
	char far *b1, far *b2, far *b3;
	int  text;

	switch ((signed char)*p++) {
	case -0x2B:                                   /* D5 */
		start_dialogue();
		break;
	case -0x2A:                                   /* D6 */
		clear_dialogue();
		break;
	case -0x2C:                                   /* D4 */
		end_dialogue();
		break;
	case -0x08:                                   /* F8 */
		b1 = get_script_string(ScriptCode + *(int far *)ScriptCode, *(int far *)(p + 2));
		dialogue_text(*(int far *)p, b1);
		p += 4;
		break;
	case -0x2D:                                   /* D3 */
		dialogue_picture(p, (signed char)p[13], *(int far *)(p + 14), *(int far *)(p + 16), *(int far *)(p + 18));
		p += 20;
		break;
	case -0x28:                                   /* D8 */
		text = *(int far *)p;  p += 2;
		b1 = get_script_string(ScriptCode + *(int far *)ScriptCode, *(int far *)p);
		p += 2;
		b2 = get_script_string(ScriptCode + *(int far *)ScriptCode, *(int far *)p);
		p += 2;
		b3 = get_script_string(ScriptCode + *(int far *)ScriptCode, *(int far *)p);
		p += 2;
		ScriptResult = dialogue_choice(text, b1, b2, b3);
		break;
	}
	return p - start;
}

/* E2 n.w   special effects                                     24EC:29B0 [asm]
 *   0  four flashes of the 3D view           1  choose_character
 *   2  the character ScriptResult goes up one level in each of his classes
 *   3  choose_raise                          4  if make_room(33, 5): npc_joins(4)
 *   5  take_party_items(46, 5) and (46, 6)       6  load_vcn(0, 0) */
int far op_special(byte far *p)
{
	byte far *start = p;
	int  n, k;
	unsigned long end;
	long xp;
	int  classes;

	n = *(int far *)p;  p += 2;
	switch (n) {
	case 0:
		draw_scene(1);
		Var_62A5 = 2;
		hide_mouse();
		Drv_658A(9, 0, 4, 0x78, ShrinkBuffer);
		show_mouse();
		for (k = 0; k < 4; k++) {
			end = GameTicks + 1;
			draw_lightning();
			hide_mouse();
			copy_page(9, 0, 9, 0, 4, 0x78, 2, 0);
			show_mouse();
			Drv_6542(9, 0, 4, 0x78, ShrinkBuffer);
			while (end > GameTicks)
				;
		}
		Var_62A5 = 0;
		SceneDirty = 1;
		break;
	case 1:
		choose_character();
		break;
	case 2:
		classes = ClassCount[Party[ScriptResult].cls];
		for (k = 0; k < classes; k++) {
			xp = xp_for_level(Party[ScriptResult].cls, k, Party[ScriptResult].level[k] + 1);
			if (xp != -1) {
				xp = xp - Party[ScriptResult].experience[k];
				give_xp(ScriptResult, xp + 1);
			}
		}
		break;
	case 3:
		choose_raise();
		break;
	case 4:
		if (make_room(33, 5))
			npc_joins(4);
		break;
	case 5:
		take_party_items(46, 5);
		take_party_items(46, 6);
		break;
	case 6:
		load_vcn(0, 0);
		break;
	}
	return p - start;
}

/* ------------------------------------------------------------------ */
/* The interpreter                                                     */
/* ------------------------------------------------------------------ */

/* 24EC:2BE9 [asm] - run the script of square "block" if it listens to one of
 * the events: the low 3 bits of LevelBlock.flags count the monsters, the
 * others are the events the square reacts to (the top three always). */
void far run_block_script(int block, unsigned events)
{
	byte far *p;
	int  script;
	unsigned flags, mask;
	/* one handler per opcode, FF down to E2 (initial value at DS:5AB1, copied on entry) */
	ScriptOp ops[30] = {
		op_set_wall,    op_toggle_wall,  op_open_door,  op_close_door,     /* FF FE FD FC */
		op_create_monster, op_move,      op_steal_item, op_message,        /* FB FA F9 F8 */
		op_set_flags,   op_play_sound,   op_clear_flags, op_modify_hp,     /* F7 F6 F5 F4 */
		op_damage,      op_jump,         op_end,        op_return,         /* F3 F2 F1 F0 */
		op_call,        op_eval,         op_delete_item, op_load_level,    /* EF EE ED EC */
		op_experience,  op_create_item,  op_launch,     op_turn,           /* EB EA E9 E8 */
		op_identify,    op_sequence,     op_delay,      op_draw_scene,     /* E7 E6 E5 E4 */
		op_dialogue,    op_special                                         /* E3 E2       */
	};
	int  op;

	script = LevelMap[block].script;
	if (script == 0)
		return;
	flags = LevelMap[block].flags;
	mask = ((flags & 0xFFF8) >> 3) | 0xE0;
	if (!(mask & events))
		return;
	ScriptResult    = 0;
	ScriptBlock     = block;
	ScriptEvents    = events;
	ScriptEnd       = 0;
	ScriptCharacter = -1;
	ScriptAbort     = 0;
	p = ScriptCode + script;
	do {
		op = (signed char)*p;
		p++;
		if (op < 0 && op > -31)                   /* FF .. E2 */
			p += ops[-(op + 1)](p);
	} while (!ScriptEnd && !ScriptAbort);
}
