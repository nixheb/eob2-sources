/*
 * Eye of the Beholder II - START.EXE
 * A new party: the starting equipment, spells and statistics; overlay
 * module 3EF4 (Ghidra 4EF4).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y newparty.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * After the character generation (the first 4 characters):
 * show_entering_game, then for each one its equipment by class (a kit drawn
 * at random, then by class group), its first spells, its maximum
 * statistics; the shapes of the character generation are freed.
 */
#include "eob2.h"

extern Character      Party[];           /* E608 */
extern Item           Items[];           /* 71CA */
extern ItemType       ItemTypes[];       /* 946E */
extern signed char    ClassFlags[];      /* 1F9A 1 fighter, 2 mage, 4 cleric, 8 thief, 0x10 paladin, 0x20 ranger */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern void far      *CgShapes[17];      /* 0752 shapes of the character generation */
extern void far      *CgButtonList[19];      /* 6D82 */
extern unsigned char far *PortraitShapes[]; /* D0B2 */

void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
int  create_item(int type);                                      /* 1CBE:092B */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void recompute_ac(int ch);                                       /* 1F55:0CF7 */
void init_chargen(void);                                         /* 4C72:24F3 (overlay) */
void farfree(void far *p);                                       /* 1000:7FF2 */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */

/* The kits: a count, then that many choices of two items (type, value,
 * slot; a type -1 ends early) */
int far Kit0[] = { 3, 36, 0, 17, -1, 0, 0, 56, 1, 17, 31, 0, 1, 23, 1, 17, 31, 1, 1 };   /* 3436:0000 */
int far Kit1[] = { 1, 2, 0, 17, -1, 0, 0 };
int far Kit2[] = { 2, 56, 1, 17, 31, 0, 1, 23, 1, 17, 31, 0, 1 };
int far Kit3[] = { 2, 1, 1, 17, 31, 1, 1, 1, 0, 17, 31, 2, 1 };

/* Initialised data of the module (DS:0AF2...) */
int far *StartKits[4] = { Kit0, Kit1, Kit2, Kit3 };                    /* 0AF2 */
int KitOfClass[15] = { 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 3, 2, 0, 0, 2 };   /* 0B02 */

/* 4EF4:0000 [asm] */
void far show_entering_game(void)
{
	int old;

	copy_page(0, 0, 20, 0, 20, 128, 2, 2);
	old = Var_62A5;
	Var_62A5 = 2;
	print_text("  Entering game.\r  Please wait.", 168, 32, 15, 0);
	Var_62A5 = old;
	hide_mouse();
	copy_page(20, 0, 18, 64, 20, 128, 2, 0);
	show_mouse();
}

/* 4EF4:0087 [asm] - give an item of this type to c, in its slot, or else
 * in the first free one from 2 (value -1: the item's own) */
void far give_start_item(Character far *c, int type, int value, int slot)
{
	register int item, i;

	if (type > 0) {
		item = create_item(type);
		Items[item].flags |= 0x40;
		if (value != -1)
			Items[item].value = value;
		if ((ItemTypes[Items[item].type].flags & 0x7F) < 4 && value)
			Items[item].flags |= 0x80;
		if (c->inventory[slot] == 0)
			c->inventory[slot] = item;
		else
			for (i = 2; i < 16; i++)
				if (c->inventory[i] == 0) {
					c->inventory[i] = item;
					return;
				}
	}
}

/* 4EF4:014A [asm] */
void far give_start_items(void)
{
	int i, cf;
	int far *p;
	int value, r;
	Character far *c;
	register int type, k;

	for (i = 0; i < 4; i++) {
		c = &Party[i];
		c->flags = 1;
		c->food = 100;
		c->id = i;
		p = StartKits[KitOfClass[c->cls]];
		r = roll_dice(1, *p++, -1);
		p += r * 6;
		for (k = 0; k < 2; k++) {
			if (*p == -1)
				break;
			give_start_item(c, p[0], p[1], p[2]);
			p += 3;
		}
		cf = ClassFlags[c->cls];
		r = roll_dice(1, 2, -1);
		value = 0;
		if (cf & 0x31) {
			if (c->raceSex >> 1 == 3) {
				type = 0x16;
				value = 1;
			} else if (r)
				type = 0x22;
			else {
				type = 5;
				value = 1;
			}
		} else if (cf & 4) {
			if (r) {
				type = 0x1A;
				value = 1;
			} else
				type = 0x1A;
		} else if (cf & 8) {
			value = 1;
			if (c->raceSex >> 1 == 3)
				type = 0x16;
			else
				type = 5;
		} else if (r) {
			type = 4;
			value = 1;
		} else
			type = 3;
		give_start_item(c, type, value, 0);
		give_start_item(c, 10, -1, 2);
		give_start_item(c, 10, -1, 2);
		give_start_item(c, 0x18, 2, 2);
		if (ClassFlags[c->cls] & 2) {
			give_start_item(c, 7, -1, 1);
			give_start_item(c, 0x15, 4, 2);
			give_start_item(c, 0x15, 13, 2);
		}
		if (ClassFlags[c->cls] & 0x14) {
			if (c->cls != 2)
				give_start_item(c, 8, -1, 1);
			else
				give_start_item(c, 0x1B, -1, 1);
			give_start_item(c, 0x14, 0x31, 1);
		}
		if (ClassFlags[c->cls] & 8)
			give_start_item(c, 6, -1, 1);
		if (i == 0)
			give_start_item(c, 0x5D, -1, 2);
		recompute_ac(i);
	}
}

/* 4EF4:041E [asm] - the mages know 9 spells, the clerics and paladins
 * have one spell (29) memorized */
void far give_start_spells(void)
{
	register int i;

	for (i = 0; i < 4; i++) {
		if (ClassFlags[Party[i].cls] & 2)
			Party[i].mageBook = 0x81CB6L;
		if (ClassFlags[Party[i].cls] & 0x14)
			Party[i].clericSpells[0][0] = 0x1D;
	}
}

/* 4EF4:0480 [asm] */
void far set_max_stats(void)
{
	Character far *c;
	register int i;

	c = Party;
	for (i = 0; i < 4; i++, c++) {
		c->strMax = c->str;
		c->strExtMax = c->strExt;
		c->intelMax = c->intel;
		c->wisMax = c->wis;
		c->dexMax = c->dex;
		c->conMax = c->con;
		c->chaMax = c->cha;
		c->hpMax = c->hp;
	}
}

/* 4EF4:04E6 [asm] */
static void far free_shapes_0752(void)
{
	register int i;

	for (i = 0; i < 17; i++)
		if (CgShapes[i]) {
			farfree(CgShapes[i]);
			CgShapes[i] = 0;
		}
}

/* 4EF4:052F [asm] - the portraits, then the other shapes of the
 * character generation */
void far free_chargen_shapes(void)
{
	register int i;

	for (i = 0; i < 67; i++)
		farfree(PortraitShapes[i]);
	free_shapes_0752();
	for (i = 0; i < 19; i++)
		farfree(CgButtonList[i]);
}

/* 4EF4:057A [asm] */
void far start_new_party(void)
{
	show_entering_game();
	give_start_items();
	give_start_spells();
	set_max_stats();
	free_chargen_shapes();
	hide_mouse();
}

/* The test party (make_test_party) */
char far *DefNames[4] = { "ALLABAR", "WILLIAM", "CIRCE", "APHRODITE" };   /* 0B20 */
int DefPortrait[4] = { 23, 7, 31, 29 };                                 /* 0B30 */
int DefClass[4] = { 2, 5, 3, 4 };                                       /* 0B38 */
int DefRace[4] = { 0, 0, 3, 1 };                                        /* 0B40 */
int DefAlign[4] = { 0, 1, 1, 1 };                                       /* 0B48 */
int DefLevel[4] = { 6, 7, 7, 7 };                                       /* 0B50 */

/* 4EF4:0598 [asm] - a ready-made party, without the character
 * generation (a test, called by nobody?).
 * ORIGINAL BUG: 6 characters from tables of 4: the 5th and the 6th get
 * the bytes that follow (a name pointer made of DefPortrait...) */
void far make_test_party(void)
{
	Character far *c;
	register int i;

	init_chargen();
	for (i = 0; i < 4; i++) {		/* HACK: 4, the original 6 overflowed the tables */
		c = &Party[i];
		c->raceSex = DefRace[i];
		c->cls = DefClass[i];
		c->alignment = DefAlign[i];
		c->portrait = DefPortrait[i];
		c->str = 18;
		c->intel = 18;
		c->wis = 18;
		c->dex = 18;
		c->con = 18;
		c->cha = 18;
		c->hp = 30;
		c->ac = 6;
		c->level[0] = DefLevel[i];
		c->experience[0] = 69000L;
		strcpy(c->name, DefNames[i]);
		mem_copy(PortraitShapes[c->portrait], c->face, 1400L);
	}
	start_new_party();
}
