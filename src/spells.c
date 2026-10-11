/*
 * Eye of the Beholder II - START.EXE
 * Casting spells: the spell book, the targets, the spell missiles, the
 * effects and their end: module 10E2 (Ghidra 20E2).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y spells.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

/* Spell.flags
 *   0x0001 effect on the caster/target character       0x0002 recompute armour class
 *   0x0004 effect on the whole party (PartyEffects)    0x0008 timed (Spell.timing)
 *   0x0010 / 0x0030 refused if the character already has the effect
 *   0x0020 asks for a target character                 0x0040 all the party / refused if the party has it
 *   0x0080 target = the chosen character               0x0100 offensive: casting it breaks invisibility
 *   0x0200 effect on every active character            0x0400 touch spell: needs a free hand
 *   0x0800 (with 0x200) even unconscious characters    0x1000 special target (dispel magic, turn undead)
 *   0x2000 the hand holds the spell (flame blade)
 *
 * Spell.effect (Character.effects / PartyEffects):
 *   0x00001 armor        0x00002 detect magic    0x00008 shield            0x00010 blur
 *   0x00020 detect invisibility                   0x00040 invisibility / improved identify
 *   0x00100 improved invisibility                 0x00200 true seeing
 *   0x00400 bless        0x00800 protection from evil                     0x02000 slow poison
 *   0x04000 magical vestment                      0x08000 prayer (monsters -1 to hit)
 *   0x10000 haste        0x20000 mystic defense
 */

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern Item           Items[600];        /* 71CA */
extern Monster        Monsters[30];      /* 6E2A */
extern Character      Party[6];          /* E608 */
extern ItemType       ItemTypes[];       /* 946E */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern Missile        Thrown[10];        /* CC5C */
extern Spell far      Spells[];          /* 39A6:0000 */
extern signed char    SpellMissile[][4]; /* 2558 per monster spell: the spell, range, flags, ... */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern int            CurrentLevel;      /* 6DCE */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */
extern char           InvCharacter;      /* 9317 */
extern char           BookCaster;        /* D0B0 the character whose spell book is open */
extern char           Var_E4C6;          /* E4C6 1: cleric spells */
extern int            Var_71C8;          /* 71C8 casting from a scroll or a wand: hand + 1 */
extern signed char    SpellTarget;       /* CBC2 */
extern Character far *TargetChar;        /* E5A8 */
extern char           CurrentSpell;      /* DF2A spell being cast */
extern int            Var_E5A2;          /* E5A2 */
extern char           SceneDirty;        /* D0AA */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern byte far      *Shapes[];          /* CC34 */
extern void (far     *Drv_65D2)(int x, int y, int w, int h, byte far *buf);    /* 65D2 video driver */
extern void (far     *Drv_6542)(int x, int y, int w, int h, byte far *buf); /* 6542 */
extern void (far     *Drv_6546)(int x, int y, int w, int h, byte far *buf); /* 6546 */
extern void (far     *Drv_65B2)(int x, int y, int colour);      /* 65B2 a point */
extern void (far     *Drv_6566)(int x1, int y1, int x2, int y2, int colour);   /* 6566 a line */
extern signed char    PlaceSeen[4][4];   /* 2352 [party direction][place] */
extern unsigned long  PartyEffects;      /* 6DDC */
extern char           Scratch[];         /* F16E */
extern char           BookOpen;         /* E441 1: the spell book is open */
extern byte far      *LoadBuffer;          /* D0AC */
extern byte far      *BookShape;         /* 71C0 */
extern void far      *BookGui;           /* DF46 */
extern byte           GuiMageBook[];     /* 1FE1 */
extern byte           GuiClericBook[];   /* 2065 */
extern signed char far *BookSpells;      /* CC2A the memorized spells of the book's owner */
extern signed char    BookLevel;         /* CD16 the spell level shown */
extern signed char    BookSlot;          /* 936A */
extern signed char    BookPage;          /* D0AB 0 or 6 */
extern byte           FrameLight, Col_E444, FrameDark, Col_DF32, FrameFlat, Col_DF44;   /* EEF0, E444, E5C2, DF32, E606, DF44 colours */
extern byte           Col_9364, Col_929A, Col_6D81;   /* 9364, 929A, 6D81 */
extern byte far      *Shape_E4CA;        /* E4CA */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);    /* 657A fills */
extern signed char    FrontPos[4][4];    /* 2342 */
extern char           Var_E440;          /* E440 */
extern void (far     *SpellStart)(void); /* DF2E */
extern char           Var_DF38;          /* DF38 set by a spell to stop */
extern int            Var_CD18;          /* CD18 the spell cast (for the level scripts) */
extern int            MouseButton;          /* F0FE the mouse button */
extern char           GuiState[];        /* E5CA */
extern int            NoViewCopy;        /* E4D2 */
extern int            ViewBlock[18];     /* 9446 */
extern signed char    SquareRow[18];     /* 2330 0 = farthest row .. 3 = party row */
extern int            FloorObjectX[];    /* 23EC [(square * 5 + place) * 2] x, y */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern int            BurstX, BurstY;    /* CC2E, CC30 the centre of a burst */
extern int (far      *Drv_657E)(int x, int y);   /* 657E reads a pixel */
/* Uninitialised data: in bssef34.c (DS:EF34-EF4D) */
extern int SparkI;      /* EF34 */
extern int SparkColour; /* EF36 */
extern int SparkLive;   /* EF38 */
extern int SparkCX;     /* EF3A */
extern int SparkCY;     /* EF3C */
extern int SparkX;      /* EF3E */
extern int SparkY;      /* EF40 */
extern int SparkPage;   /* EF42 */
extern int SparkBack;   /* EF44 */
extern int Var_EF46;    /* EF46 */
extern int CharTries;   /* EF48 */
extern int Var_EF4A;    /* EF4A */
extern int BackColour;  /* EF4C */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void gui_main(void);                                         /* 1DFE:002F */
void gui_inventory(void);                                         /* 1DFE:00BD */
void gui_stats(void);                                         /* 1DFE:014B */
void gui_target(void);                                         /* 1DFE:021F */
void message(char far *text);                                    /* 18D7:0BE6 */
void swap_bytes(signed char far *a, signed char far *b);         /* 18D7:0808 */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
void remove_char_timer(int ch, int what);                        /* 27B7:0D76 */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
void add_char_timer(char ch, long delay, char what, char mode);  /* 27B7:06B7 */
int  class_part_index(int base, int cls);                        /* 1B54:009C */
void put_shape(int page, byte far *shape, int x, int y, int flags);   /* 2DA3:004A */
void clock_stop(int n);                                          /* 3187:003D */
void clock_start(int n);                                         /* 3187:0009 */
void mouse_hide_rect(int x1, int y1, int x2, int y2);               /* 338B:0206 */
void mouse_show_rect(void);                                         /* 338B:02CA */
void copy_page(int a, int b, int c, int d, int w, int h, int from, int to);  /* 2D92:000A */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void far apply_spell(int spell);
void monsters_targeted(int far *out, int block, int pos, char dir, char all, char nearest);  /* 1997:1AD2 */
int  char_hits_monster(int ch, int mon, int item, int melee);     /* 1F55:000C */
void damage_monster(Monster far *m, int from, int item, int a, int flags, int save, int saveType);  /* 1F55:12B8 */
int  monster_to_hit(Monster far *m, int ch);                     /* 1997:12EF */
void inflict_damage(int ch, int from, int item, int a, int flags, int save, int saveType);  /* 1F55:143E */
void monsters_hit_update(void);                                  /* 1997:10D2 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void recompute_ac(int ch);                                       /* 1F55:0CF7 */
void redraw_party(void);                                         /* 1E24:0686 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
int  monster_to_attack(char ch, int block);                      /* 1997:1929 */
int  sprintf(char far *buf, char far *fmt, ...);
void far missile_burst(Missile far *t);
void draw_frame3d(int x1, int y1, int x2, int y2, int c1, int c2, int fill);   /* 18D7:0A45 draws a 3D frame */
void print_number(int n, int x, int y, int fg, int bg);          /* 3060:000E */
void draw_compass(int a);                                        /* 3ABA:003E (overlay) */
void consume_hand_item(int ch, int slot);                        /* 1CBE:0101 */
void movmem(void far *src, void far *dest, unsigned n);
void play_sound(int sound);                                      /* 2C9F:00AA */
void run_block_script(int block, unsigned event);                /* 24EC:2BE9 */
char clamp(int v, int min, int max, int wrap);            /* 18D7:0B2F */
void far draw_spell_book(void);
void play_sound_at(int sound, int block);                        /* 2A30:0014 */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void draw_view_contents(int row);                                /* 3ABA:002F (overlay) */
void view_clip(int sq, int far *a, int far *b, int c);           /* 3ABA:0070 (overlay) */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
int  random_range(int min, int max);                                      /* 3064:0004 */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);      /* 18D7:067F */
void wait_tick(void);                                           /* 2D9B:0021 */
byte random_byte(void);                                         /* 33DC:0004 a random byte */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void far sparks(int scale, int spread, int count, int speed, int lift, signed char far *colours);
void far view_burst(int block, int type, Missile far *t);
void far draw_spell_book(void);
int  far cleric_level(int ch);
int  far caster_level(int ch);

/* ------------------------------------------------------------------ */
/* Module 20E2, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:55DC...) */
/* the little images drawn on a portrait, 4 fields of 2 bits per frame  55DC */
byte HandFx[8] = {
	64, 144, 228, 185, 110, 27, 6, 1
};
byte HandFxMask[4] = {
	192, 48, 12, 3
};                                           /* 55E4 */
byte HandFxShift[4] = {
	6, 4, 2, 0
};                                          /* 55E8 */
signed char HandFxPos[4][2] = {
	8, 6,
	28, 20,
	13, 24,
	13, 15
};                                  /* 55EC */
int SpellPortraitX[6] = {
	23, 32, 23, 32, 23, 32
};                                             /* 55F4 */
signed char SpellPortraitY[6] = {
	2, 2, 52, 52, 102, 102
};                                     /* 5600 */
/* the sparkles of the spell effect in the view: 11 frames, 16 sparkles */
unsigned long SparkleFrames[11] = {
	0x40000000L, 0x95000000L, 0xEA550000L, 0xBFAA5400L,
	0x6AFFA954L, 0x15AAFEA9L, 0x0055ABFEL, 0x000056ABL,
	0x00000156L, 0x00000001L, 0x00000000L
};                              /* 5606 */
unsigned long SparkleMask[16] = {
	0xC0000000L, 0x30000000L, 0x0C000000L, 0x03000000L,
	0x00C00000L, 0x00300000L, 0x000C0000L, 0x00030000L,
	0x0000C000L, 0x00003000L, 0x00000C00L, 0x00000300L,
	0x000000C0L, 0x00000030L, 0x0000000CL, 0x00000003L
};                                /* 5632 */
byte SparkleShift[16] = {
	30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};                                        /* 5672 */
byte SparkleX[16] = {
	80, 112, 48, 104, 32, 96, 56, 120, 128, 72, 88, 40, 96, 64, 112, 72
};                                            /* 5682 */
byte SparkleY[16] = {
	49, 43, 72, 23, 22, 72, 53, 27, 67, 46, 36, 40, 56, 28, 22, 68
};                                            /* 5692 */
/* can the character cast a touch spell (front row)                    56A2 */
signed char CanTouch[4] = {
	1, 1, 0, 0
};
/* the arrow buttons of the spell book: rows 1-5 from x + ArrowLeft to x + ArrowRight */
signed char ArrowLeft[5] = {                                            /* 56A6 */
	2, 1, 0, 2, 2
};
signed char ArrowRight[5] = {                                           /* 56AB */
	4, 5, 6, 4, 4
};
byte BookTabX[4] = {                                                    /* 56B0 */
	68, 98, 128, 144
};
byte BookTabY[3] = {                                                    /* 56B4 */
	130, 146, 152
};
signed char Table_56B7[18] = {
	0, 0, 1, 2, 1, 0, 0, 0, 1, 2, 1, 0, 0, 2, 0, 0,
	2, 0
};
signed char Table_56C9[14] = {
	2, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
};
signed char Table_56D7[4] = {
	7, 12, 15, 18
};
signed char Table_56DB[13] = {
	15, 5, 15, 5, 6, 5, 6, 8, 6, 8, 6, 8, 0
};
signed char Table_56E8[10] = {
	15, 9, 15, 9, 2, 10, 11, 10, 11, 0
};
signed char Table_56F2[11] = {
	5, 3, 5, 3, 3, 3, 3, 11, 3, 11, 0
};
signed char far *Tables_56FD[3] = { Table_56DB, Table_56E8, Table_56F2 };
int Table_5709[4] = {
	119, 103, 79, 63
};

/* 20E2:0006 [asm] - redraws the inventory screen as it is */
void far redraw_inventory(void)
{
	switch (Var_EE3A) {
	case 0:
		gui_main();
		break;
	case 1:
		gui_inventory();
		break;
	case 2:
		gui_stats();
	}
}

/* 20E2:0032 [asm] */
void far no_effect(void)
{
	message("no effect.\r");
}

/* 20E2:0044 [asm] - the items 11 to 17 are spells held in a hand */
int far is_spell_item(unsigned item)
{
	return item >= 11 && item <= 17;
}

/* 20E2:005F [asm] - the poison ends */
void far neutralize_poison(int ch)
{
	Party[ch].flags &= ~2;
	Party[ch].effects &= ~0x2000L;
	remove_char_timer(ch, -34);
	draw_portrait(ch);
}

/* 20E2:00A1 [asm] - the timer that ends a spell: base + per level */
void far start_timed_spell(int spell, int base, long unit, int perLevel, int mode)
{
	long delay;
	register int level;

	if (Var_E4C6 == 1)
		level = cleric_level(BookCaster);
	else
		level = caster_level(BookCaster);
	delay = unit * base + unit * (perLevel * level);
	add_char_timer(SpellTarget, delay, -spell, mode);
}

/* 20E2:0120 [asm] - sorts the memorized spells of each spell level: the
 * ready ones first, then the cast ones, the empty slots last */
void far sort_spells(int ch)
{
	int lvl, changed, i;
	signed char far *q;
	signed char far *p;
	register int a, b;

	p = Party[ch].mageSpells[0];
	for (lvl = 0; lvl < 16; p += 10, lvl++)
		do {
			changed = 0;
			for (i = 0; i < 9; i++) {
				q = p + i;
				a = q[0];
				b = q[1];
				if (a == 0)
					a = 80;
				else if (a < 0)
					a = -a + 40;
				if (b == 0)
					b = 80;
				else if (b < 0)
					b = -b + 40;
				if (a > b) {
					swap_bytes(q, q + 1);
					changed = 1;
				}
			}
		} while (changed);
}

/* 20E2:01DF [asm] - the end of a spell cast from a scroll or a wand */
void far end_item_cast(void)
{
	Var_71C8--;
	Party[BookCaster].busy ^= 1 << Var_71C8;
	add_char_timer(BookCaster, 18L, Var_71C8 + 2, 1);
	draw_portrait(BookCaster);
	BookCaster = Var_EF46;
	Var_E4C6 = Var_EF4A;
	Var_71C8 = 0;
	redraw_inventory();
}

/* 20E2:023D [asm] - a free item type (51 to 56) for a spell held in a hand */
int far new_spell_item_type(byte slots, byte b, char ac, char classes, char d0, char d1, char d2, int flags)
{
	ItemType far *t;
	register int i;

	t = &ItemTypes[51];
	for (i = 51; i < 57; i++, t++)
		if (t->ac == -30)
			break;
	if (i == 57)
		return -1;
	t->slots = slots;
	t->hands = 0;
	t->unk2 = b;
	t->ac = ac;
	t->classes = classes;
	t->damage[0] = t->damageLarge[0] = d0;
	t->damage[1] = t->damageLarge[1] = d1;
	t->damage[2] = t->damageLarge[2] = d2;
	t->flags = flags;
	return i;
}

/* 20E2:02C5 [asm] */
void far free_spell_item(int item)
{
	ItemTypes[Items[item].type].ac = -30;
	Items[item].block = -2;
	Items[item].level = -1;
}

/* 20E2:0305 [asm] - a free item (11 to 16) for a spell held in a hand */
int far new_spell_item(int flags, int icon, int value, int type)
{
	Item far *p;
	register int i;

	p = &Items[11];
	for (i = 11; i < 17; i++, p++)
		if (p->block == -2)
			break;
	if (i == 17)
		return -1;
	p->flags = flags + 0x20;
	p->icon = icon;
	p->type = type;
	p->value = value;
	p->next = p->prev = p->nameUnid = p->name = p->block = Items[0].pos = 0;
	return i;
}

/* 20E2:0379 [asm] */
int far second_hand_free(Character far *c)
{
	return c->inventory[1] == 0;
}

/* 20E2:0390 [asm] - the mage level of a caster (a scroll or a wand: 9;
 * a monster: 5, 9 from level 7) */
int far caster_level(register int ch)
{
	int part;

	if (Var_71C8)
		return 9;
	if (ch == -1) {
		if (CurrentLevel < 7)
			return 5;
		return 9;
	}
	if ((part = class_part_index(1, Party[ch].cls)) > -1)
		return Party[ch].level[part];
	return 1;
}

/* 20E2:03F9 [asm] - the cleric level (a paladin casts from level 9, as level - 8) */
int far cleric_level(register int ch)
{
	register int part;

	if (Var_71C8)
		return 9;
	if (ch == -1) {
		if (CurrentLevel < 7)
			return 5;
		return 9;
	}
	if ((part = class_part_index(2, Party[ch].cls)) > -1)
		return Party[ch].level[part];
	if ((part = class_part_index(4, Party[ch].cls)) > -1) {
		if (Party[ch].level[part] > 8)
			return Party[ch].level[part] - 8;
		else
			return 1;
	}
	return 1;
}

/* 20E2:04A5 [asm] */
void far choose_target(void)
{
	message("Cast spell on which character? ");
	gui_target();
}

/* 20E2:04BC [asm] - one frame of the glow on a portrait */
void far draw_portrait_fx(int ch, int frame)
{
	int y, n;
	register int i, x;

	draw_portrait(ch);
	for (i = 0; i < 4; i++) {
		n = (HandFx[frame] & HandFxMask[i]) >> HandFxShift[i];
		if (n--) {
			if (Var_EE3A == 0) {
				x = SpellPortraitX[ch] * 8;
				y = SpellPortraitY[ch];
			} else {
				x = 0xB5;
				y = 3;
			}
			y += HandFxPos[i][1];
			x += HandFxPos[i][0] - 8;
			put_shape(0, Shapes[n], x, y, 0);
		}
	}
}

/* 20E2:0565 [asm] - the glow on the portrait of the target(s) (-1: all) */
void far target_character(register int ch)
{
	int first, last;
	register int k;

	if (ch == -1) {
		first = 0;
		last = 5;
	} else
		first = last = ch;
	for (k = 0; k < 8; k++) {
		for (ch = first; ch <= last; ch++)
			if ((Party[ch].flags & 1) && (Var_EE3A == 0 || InvCharacter == ch))
				draw_portrait_fx(ch, k);
		delay_ticks(2);
	}
	for (ch = first; ch <= last; ch++)
		draw_portrait(ch);
}

/* 20E2:05EA [asm] - sparkles in the view */
void far target_special(void)
{
	int x, y, k;
	register int i, n;

	clock_stop(2);                    /* the game clock stops */
	mouse_hide_rect(0, 0, 0xB0, 0x78);
	copy_page(0, 0, 0, 0, 0x16, 0x78, 0, 2);
	for (i = 0; i < 16; i++)
		Drv_65D2(SparkleX[i] >> 3, SparkleY[i], 2, 16, ShrinkBuffer + (i << 8));
	for (k = 0; k < 11; k++) {
		Var_62A5 = 2;
		for (i = 0; i < 16; i++) {
			x = SparkleX[i];
			y = SparkleY[i];
			Drv_6542(SparkleX[i] >> 3, SparkleY[i], 2, 16, ShrinkBuffer + (i << 8));
		}
		for (i = 0; i < 16; i++) {
			n = (SparkleFrames[k] & SparkleMask[i]) >> SparkleShift[i];
			x = SparkleX[i];
			y = SparkleY[i];
			if (n--)
				put_shape(2, Shapes[n], x, y, 0);
		}
		Var_62A5 = 0;
		mouse_hide_rect(0, 0, 0xB0, 0x78);
		copy_page(0, 0, 0, 0, 0x16, 0x78, 2, 0);
		mouse_show_rect();
		delay_ticks(2);
	}
	for (i = 0; i < 16; i++)
		Drv_6546(SparkleX[i] >> 3, SparkleY[i], 2, 16, ShrinkBuffer + (i << 8));
	mouse_show_rect();
	clock_start(2);
}

/* 20E2:07DF [asm] - the character chosen as the target (a button of the
 * GUI: +38 the character, +4 the button) */
int far target_chosen(byte far *button)
{
	SpellTarget = button[0x38];
	if (SpellTarget != -1) {
		TargetChar = &Party[SpellTarget];
		if ((TargetChar->flags & 1) == 0)
			return *(int far *)(button + 4);
		apply_spell(CurrentSpell);
	} else {
		message("\rSpell aborted.\r");
		if (Var_71C8)
			end_item_cast();
		else {
			redraw_inventory();
			draw_spell_book();
		}
	}
	return *(int far *)(button + 4);
}

/* 20E2:084A [asm] - a spell missile (fireball, lightning...) */
void far launch_spell(char from, register int spell, int block, int pos, int dir)
{
	Missile far *t;
	register int i;

	t = Thrown;
	for (i = 0; i < 10; i++, t++)
		if (t->active == 0)
			break;
	if (i == 10)
		return;
	t->active = 2;
	t->justLaunched = 1;
	t->flags = SpellMissile[spell][2];
	t->dir = dir;
	t->range = SpellMissile[spell][1];
	t->block = block;
	t->pos = pos;
	t->item = spell;
	t->shape = SpellMissile[spell][3];
	t->caster = from;
	spell = SpellMissile[spell][0];
	t->hit = Spells[spell].end;
	SceneDirty = 1;
}

/* 20E2:0915 [asm] - a spell missile bursts: the monsters of the block (all
 * of them with flags 2, the nearest with 1) or the party (all with 2, else
 * the character on that side). The missile flags: 4 a to-hit roll, 8 a save
 * for half (type 4, else none), 0x10 hits even the caster's side, 0x40
 * bursts (fn 1EDB). Returns 1 when something was hit */
int far spell_hit_damage(Missile far *t, int count, int sides, int add, register int mult)
{
	char dmg, f1, f2, f4, ch, hit, f10, save, saveType;
	int list[5];
	int far *p;
	int place;
	register int dflags;

	f10 = t->flags & 0x10;
	f1 = t->flags & 1;
	f2 = t->flags & 2;
	f4 = t->flags & 4;
	if (t->flags & 8) {
		save = 4;
		saveType = 0;
	} else {
		save = 5;
		saveType = 3;
	}
	dflags = Spells[SpellMissile[t->item][0]].damage;
	if (t->caster >= 0)
		dflags |= 0x800;
	hit = 0;
	if (mult <= 0)
		mult = 1;
	if ((LevelMap[t->block].flags & 7) && (t->caster >= 0 || f10)) {
		monsters_targeted(list, t->block, t->pos, t->dir, f2, f1);
		Var_E5A2 = 1;
		for (p = list; *p != -1; p++) {
			dmg = roll_dice(count, sides, add) * mult;
			if (!f4 || char_hits_monster(t->caster, *p, 0, 0)) {
				damage_monster(&Monsters[*p], 0, 0, dmg, dflags, save, saveType);
				hit = 1;
			}
		}
		monsters_hit_update();
	} else if (t->block == PartyBlock && (t->caster < 0 || f10)) {
		if (f2) {
			for (ch = 0; ch < 6; ch++)
				if (char_is_active(ch, 1))
					/* the to-hit roll is made for the first monster of the level */
					if (monster_to_hit(Monsters, ch) || !f4) {
						dmg = roll_dice(count, sides, add) * mult;
						hit = 1;
						inflict_damage(ch, 0, 0, dmg, dflags, save, saveType);
					}
		} else {
			place = t->pos & 3;
			place = PlaceSeen[PartyDirection][place];
			/* ORIGINAL BUG: character 6 does not exist (Party[6] is read
			 * past the end of the party) */
			if (place > 2 && (char_is_active(5, 1) || char_is_active(6, 1)) && roll_dice(1, 2, -1))
				ch = place + 2;
			else
				ch = place;
			if (t->item != 0 || !(Party[ch].effects & 8L)) {
				if (Party[ch].flags & 1)
					if (monster_to_hit(Monsters, ch) || !f4) {
						dmg = roll_dice(count, sides, add) * mult;
						hit = 1;
						inflict_damage(ch, 0, 0, dmg, dflags, save, saveType);
					}
			} else
				hit = 1;                          /* the Shield spell stops magic missiles */
		}
	}
	if (hit && (t->flags & 0x40))
		missile_burst(t);
	else if (t->item == 4)
		hit = 0;
	return hit;
}

/* 20E2:0C6D [asm] - a spell effect ends (verbose: "<name>'s <spell> spell
 * expires.") */
void far end_spell_effect(int spell, register int ch, int verbose)
{
	Spell far *s;
	Character far *c;
	register int i;

	s = &Spells[spell];
	c = &Party[ch];
	if (verbose) {
		sprintf(Scratch, "%s's %s spell expires.\r", c->name, s->name);
		message(Scratch);
	}
	if (s->end)
		s->end(c);
	if (s->flags & 1)
		c->effects &= ~s->effect;
	if (s->flags & 4)
		PartyEffects &= ~s->effect;
	if (s->flags & 0x200)
		for (i = 0; i < 6; i++)
			if (char_is_active(i, 1) && (char_is_active(i, 2) || (s->flags & 0x800)))
				Party[i].effects &= ~s->effect;
	if (s->flags & 2)
		recompute_ac(SpellTarget);
	if (verbose) {
		if (s->flags & 0x20A0)
			draw_portrait(ch);
		else if (s->flags & 0x40)
			redraw_party();
	}
}

/* 20E2:0DDB [asm] - a touch spell on the monster ahead: from the front
 * rank, with a to-hit roll. item -1: hit points max - 1d4 */
void far touch_attack(register int from, int item, int a)
{
	int block;
	register int mon;

	/* ORIGINAL BUG: CanTouch has 4 entries: for the characters 5 and 6 the
	 * next table is read (2 and 1, not 0): they may touch from the back */
	if (CanTouch[BookCaster]) {
		block = block_ahead(PartyBlock, PartyDirection);
		mon = monster_to_attack(BookCaster, block);
		if (mon != -1) {
			if (char_hits_monster(BookCaster, mon, 0, 1)) {
				if (from == -1) {
					a = Monsters[mon].hpMax - roll_dice(1, 4, 0);
					from = 0;
					item = 0;
				}
				damage_monster(&Monsters[mon], from, item, a, 0x801, 4, 2);
			}
		} else {
			sprintf(Scratch, "%s missed the monster.\r", Party[BookCaster].name);
			message(Scratch);
		}
	} else {
		sprintf(Scratch, "%s must be in the front ranks to hit!\r", Party[BookCaster].name);
		message(Scratch);
	}
}

/* 20E2:0EF6 [asm] - opens the spell book of a character (mage or cleric),
 * on the spell level and slot he had */
int far open_spell_book(int ch, char cleric)
{
	BookLevel = Party[ch].bookLevel;
	BookSlot = Party[ch].bookSlot;
	BookPage = Party[ch].bookPage;
	BookCaster = ch;
	Var_E4C6 = cleric == 1;
	if (Var_E4C6) {
		BookGui = GuiClericBook;
		BookSpells = Party[ch].clericSpells[0];
	} else {
		BookGui = GuiMageBook;
		BookSpells = Party[ch].mageSpells[0];
	}
	if (*(BookSpells + BookLevel * 10 + BookPage + BookSlot) <= 0) {
		for (BookSlot = 0; BookSlot < 10; BookSlot++)
			if (*(BookSpells + BookLevel * 10 + BookSlot) > 0) {
				if (BookSlot > 5) {
					BookPage = 6;
					BookSlot -= 6;
				}
				break;
			}
		if (BookSlot == 10) {
			BookPage = 0;
			BookSlot = 6;
		}
	}
	if (!BookOpen)
		Drv_65D2(8, 0x79, 0x0E, 0x38, LoadBuffer + 25000);
	BookOpen = 1;
	gui_main();
	draw_spell_book();
	return 1;
}

/* 20E2:102F [asm] - an arrow button of the spell book (down: pointing down) */
void far draw_book_arrow(register int x, int y, int down)
{
	register int i;

	if (down) {
		Drv_65B2(x + 3, y + 5, 12);
		for (i = 1; i < 6; i++)
			Drv_6566(x + ArrowLeft[i - 1], y + (5 - i), x + ArrowRight[i - 1], y + (5 - i), 12);
	} else {
		Drv_65B2(x + 3, y, 12);
		for (i = 1; i < 6; i++)
			Drv_6566(x + ArrowLeft[i - 1], y + i, x + ArrowRight[i - 1], y + i, 12);
	}
}

/* 20E2:10E5 [asm] */
void far draw_book_tabs(void)
{
	register int i, j;

	for (j = 0; j < 3; j++)
		for (i = 0; i < 4; i++)
			put_shape(Var_62A5, BookShape, BookTabX[i], BookTabY[j], 0);
}

/* 20E2:1127 [asm] - draws the spell book: the 6 level tabs, the spells of
 * the page (the selected one inverted), ABORT SPELL */
void far draw_spell_book(void)
{
	int x, c1, c2, c3, grey, white;
	register int i, spell;

	Var_62A5 = 2;
	mouse_hide_rect(0x40, 0x78, 0xB0, 0xB1);
	copy_page(8, 0x79, 8, 0x79, 0x0E, 0x38, 0, 2);
	mouse_show_rect();
	for (i = 0; i < 6; i++) {
		c1 = BookLevel == i ? FrameLight : Col_E444;
		c2 = BookLevel == i ? FrameDark : Col_DF32;
		c3 = BookLevel == i ? FrameFlat : Col_DF44;
		x = i * 18 + 0x44;
		draw_frame3d(x, 0x79, x + 0x11, 0x81, c1, c2, c3);
		print_number(i + 1, i * 18 + 0x4B, 0x7B, 12, 0);
	}
	draw_frame3d(0x44, 0x82, 0xAF, 0xB0, FrameLight, FrameDark, FrameFlat);
	draw_frame3d(0x44, 0xA8, 0x91, 0xB0, Col_9364, Col_929A, Col_6D81);
	draw_frame3d(0x92, 0xA8, 0x9F, 0xB0, Col_9364, Col_929A, Col_6D81);
	draw_frame3d(0xA0, 0xA8, 0xAF, 0xB0, Col_9364, Col_929A, Col_6D81);
	draw_book_arrow(0x96, 0xA9, 0);
	draw_book_arrow(0xA5, 0xA9, 1);
	c3 = FrameFlat;
	grey = 8;
	white = 15;
	for (i = 0; i < 7; i++) {
		spell = *(BookSpells + BookLevel * 10 + i + BookPage);
		if (BookSlot != i) {
			if (spell >= 0 && i < 6 && i + BookPage < 9)
				Drv_65F2(((char far * far *)BookGui)[spell], 0x47, i * 6 + 0x84, white, c3);
			else if (i == 6)
				Drv_65F2("ABORT SPELL", 0x4A, 0xAA, 12, Col_6D81);
		} else if (spell >= 0 && i < 6 && i + BookPage < 9)
			Drv_65F2(((char far * far *)BookGui)[spell], 0x47, i * 6 + 0x84, white, grey);
		else {
			Drv_657A(0x45, 0xA9, 0x90, 0xAF, grey);
			if (i == 6)
				Drv_65F2("ABORT SPELL", 0x4A, 0xAA, white, grey);
		}
	}
	if (Party[BookCaster].busy & 4)
		draw_book_tabs();
	if ((BookSpells + BookLevel * 10)[6] <= 0)
		put_shape(2, Shape_E4CA, 0x92, 0xA8, 0);
	Var_62A5 = 0;
	mouse_hide_rect(0x44, 0x78, 0xB0, 0xB1);
	copy_page(8, 0x79, 8, 0x79, 0x0E, 0x38, 2, 0);
	mouse_show_rect();
}

/* 20E2:14A7 [asm] - closes the spell book */
int far close_spell_book(byte far *button)
{
	BookOpen = 0;
	mouse_hide_rect(0x40, 0x78, 0xB0, 0xB1);
	Drv_6546(8, 0x79, 0x0E, 0x38, LoadBuffer + 25000);
	mouse_show_rect();
	draw_compass(1);
	redraw_inventory();
	return *(int far *)(button + 4);
}

/* 20E2:1506 [asm] - a level tab clicked */
int far book_level_click(byte far *button)
{
	BookLevel = button[0x38];
	BookPage = 0;
	for (BookSlot = 0; BookSlot < 6; BookSlot++)
		if (*(BookSpells + BookLevel * 10 + BookSlot) > 0)
			break;
	draw_spell_book();
	Party[BookCaster].bookLevel = BookLevel;
	Party[BookCaster].bookSlot = BookSlot;
	Party[BookCaster].bookPage = BookPage;
	return *(int far *)(button + 4);
}

/* 20E2:158B [asm] */
void far book_slot_absolute(void)
{
	BookSlot += BookPage;
	if (BookSlot == 12 || (BookSlot == 6 && BookPage == 0))
		BookSlot = 9;
	if (BookSlot > 8)
		BookSlot = 8;
}

/* 20E2:15BD [asm] */
void far book_slot_relative(void)
{
	if (BookSlot < 6)
		BookPage = 0;
	else if (BookSlot != 9) {
		BookPage = 6;
		BookSlot -= 6;
	} else {
		BookSlot = 6;
		if ((BookSpells + BookLevel * 10)[6] > 0)
			BookPage = 6;
		else
			BookPage = 0;
	}
}

/* 20E2:1608 [asm] - the other page of the level (spells 7 to 9) */
int far book_turn_page(byte far *button)
{
	if ((BookSpells + BookLevel * 10)[6] > 0 || BookPage == 6) {
		BookPage ^= 6;
		BookSlot = 0;
	}
	Party[BookCaster].bookPage = BookPage;
	Party[BookCaster].bookSlot = BookSlot;
	draw_spell_book();
	return *(int far *)(button + 4);
}

/* 20E2:1665 [asm] - a scroll is used up, a wand loses a charge */
void far use_charged_item(register int ch, int slot)
{
	int charges;
	register int item;

	item = Party[ch].inventory[slot];
	switch (Items[item].type) {
	case 0x1A:
	case 0x22:
	case 0x23:
	case 0x3A:
		consume_hand_item(ch, slot);
		break;
	case 0x30:
	case 0x3E:
		if (Items[item].value == 5)
			break;
		charges = Items[item].flags & 0x3F;
		if (--charges == 0)
			consume_hand_item(ch, slot);
		else
			Items[item].flags--;
	}
}

/* 20E2:170E [asm] - the caster (BookCaster) casts a spell: from the book
 * (the spell leaves the memorized list) or from an item in a hand */
void far cast_spell(int spell, register int hand)
{
	Spell far *s;
	int k;
	register int pos;

	CurrentSpell = spell;
	s = &Spells[spell];
	if ((Spells[spell].flags & 0x100) && (Party[BookCaster].effects & 0x40L))
		end_spell_effect(10, BookCaster, 1);          /* an offensive spell ends Invisibility */
	pos = BookCaster;
	if (pos > 3)
		pos -= 2;
	Var_E440 = FrontPos[PartyDirection][pos];
	if (s->flags & 0x400) {
		if (Party[BookCaster].inventory[0] && Party[BookCaster].inventory[1]) {
			message("You must have a free hand for this spell.\r");
			return;
		}
		if (is_spell_item(Party[BookCaster].inventory[0]) || is_spell_item(Party[BookCaster].inventory[1])) {
			message("You can't have two of this spell type active.\r");
			return;
		}
	}
	if (CurrentSpell != 0x3E) {
		if (Var_71C8 == 0) {
			k = *(BookSpells + BookLevel * 10 + BookPage + BookSlot);
			if (BookSlot + BookPage < 8)
				movmem(BookSpells + BookLevel * 10 + BookSlot + BookPage + 1,
				       BookSpells + BookLevel * 10 + BookSlot + BookPage, 8 - (BookSlot + BookPage));
			(BookSpells + BookLevel * 10)[8] = -k;    /* cast: kept at the end until rest */
			if (*(BookSpells + BookLevel * 10 + BookPage + BookSlot) <= 0) {
				BookSlot--;
				if (BookSlot == -1 && BookPage != 0) {
					BookPage = 0;
					BookSlot = 5;
				} else if (BookSlot == -1)
					BookSlot = 6;
			}
		} else if (hand != -1)
			use_charged_item(BookCaster, hand);
	}
	sprintf(Scratch, "%s casts %s.\r", Party[BookCaster].name, s->name);
	gui_printf(Scratch);
	if (s->flags & 0x20)
		choose_target();
	else {
		SpellTarget = BookCaster;
		TargetChar = &Party[BookCaster];
		apply_spell(spell);
	}
}

/* 20E2:199C [asm] - the effect of a spell on its target */
void far apply_spell(register int spell)
{
	Spell far *s;
	int far *timing;
	register int i;

	s = &Spells[spell];
	SpellStart = s->start;
	play_sound(s->sound);
	if (s->flags & 0xA0)
		target_character(SpellTarget);
	else if (s->flags & 0x40)
		target_character(-1);
	else if (s->flags & 0x1000)
		target_special();
	if (s->flags & 0x20) {
		gui_printf(TargetChar->name);
		gui_printf("\rok\r");
	}
	if ((s->flags & 0x30) && (TargetChar->effects & s->effect)) {
		sprintf(Scratch, "%s is already under the effect of a %s spell.\r", TargetChar->name, s->name);
		message(Scratch);
		goto done;
	}
	if ((s->flags & 0x50) && (PartyEffects & s->effect)) {
		sprintf(Scratch, "The party is already under the effect of a %s spell.\r", s->name);
		message(Scratch);
		goto done;
	}
	if (s->flags & 8) {
		timing = s->timing;
		start_timed_spell(spell, timing[0], timing[1], timing[2], timing[3]);
	}
	Var_DF38 = 0;
	if (SpellStart)
		SpellStart();
	if (Var_DF38)
		return;
	if (s->flags & 1)
		TargetChar->effects |= s->effect;
	if (s->flags & 4)
		PartyEffects |= s->effect;
	if (s->flags & 0x200)
		for (i = 0; i < 6; i++)
			if (char_is_active(i, 1) && (char_is_active(i, 2) || (s->flags & 0x800)))
				Party[i].effects |= s->effect;
	if (s->flags & 2)
		recompute_ac(SpellTarget);
	if (s->flags & 0x20A0)
		draw_portrait(SpellTarget);
	else if (s->flags & 0x40)
		redraw_party();
done:
	if (Var_71C8)
		end_item_cast();
	else {
		Party[BookCaster].busy |= 4;
		add_char_timer(BookCaster, 72L, 11, 1);
		redraw_inventory();
		draw_spell_book();
	}
	Var_CD18 = spell;
	run_block_script(PartyBlock, 0x800);          /* the level script may react */
	Var_CD18 = 0;
}

/* 20E2:1C70 [asm] - casting from a scroll or a wand in a hand */
void far cast_from_item(char ch, register int spell, register int hand)
{
	Var_EF46 = BookCaster;
	Var_EF4A = Var_E4C6;
	Var_71C8 = hand + 1;
	BookCaster = ch;
	Var_E4C6 = spell > 33;
	cast_spell(spell, hand);
}

/* 20E2:1CB1 [asm] - a click in the spell book: a spell (0-5), the arrows
 * (6: with the mouse button, up or down) or ABORT SPELL (7) */
int far book_click(byte far *button)
{
	int slot, dir, k;
	register int i, any;

	slot = *(int far *)(button + 0x38);
	if (slot == 6) {
		/* ORIGINAL BUG: "any" is never set to 0 */
		for (i = 0; i < 10; i++)
			if ((BookSpells + BookLevel * 10)[i] > 0) {
				any = 1;
				break;
			}
		if (!any)
			goto end;
		if (MouseButton == 1)
			dir = -1;
		else if (MouseButton == 2)
			dir = 1;
		BookSlot += BookPage;
		if (BookSlot == 12 || (BookSlot == 6 && BookPage == 0))
			BookSlot = 9;
		do {
			BookSlot += dir;
			BookSlot = clamp(BookSlot, 0, 9, 1);
		} while (*(BookSpells + BookLevel * 10 + BookSlot) <= 0 && BookSlot != 9);
		if (BookSlot < 6)
			BookPage = 0;
		else if (BookSlot != 9) {
			BookPage = 6;
			BookSlot -= 6;
		} else {
			BookSlot = 6;
			BookPage = 6;
		}
		if (BookPage == 6 && (BookSpells + BookLevel * 10)[6] <= 0)
			BookPage = 0;
		draw_spell_book();
		goto save;
	}
	if (*(BookSpells + BookLevel * 10 + BookPage + slot) <= 0 && slot != 7)
		goto save;
	if (slot < 6 && BookPage + slot < 9)
		BookSlot = slot;
	else if (slot != 7)
		goto end;
	if (BookSlot < 6 && BookPage + BookSlot < 9) {
		if (Party[BookCaster].busy & 4)
			goto end;
		draw_spell_book();
		k = *(BookSpells + BookLevel * 10 + BookPage + BookSlot);
		k += Var_E4C6 == 1 ? 33 : 0;
		cast_spell(k, 0);
	} else if ((BookSlot == 6 && slot == 7) || slot == 6)
		close_spell_book(GuiState);
save:
	Party[BookCaster].bookSlot = BookSlot;
	Party[BookCaster].bookPage = BookPage;
end:
	return *(int far *)(button + 4);
}

/* 20E2:1EDB [asm] - a spell missile bursts on its block */
void far missile_burst(Missile far *t)
{
	view_burst(t->block, t->item, t);
}

/* 20E2:1EF9 [asm] - a burst in the view (fireball...): sparks drawn over
 * the view, if the block can be seen */
void far view_burst(register int block, int type, Missile far *t)
{
	signed char far *colours;
	int row, a, b, kind, r;
	register int i;

	if (NoViewCopy) {
		play_sound_at(0x23, PartyBlock);
		return;
	}
	kind = Table_56C9[type];
	colours = Tables_56FD[kind];
	for (i = 0; i < 18; i++)
		if (ViewBlock[i] == block)
			break;
	if (i > 17)
		return;
	if ((r = Table_56B7[i]) == 0) {
		play_sound_at(0x23, PartyBlock);
		return;
	}
	if (r == 1 && (t->dir & 1) != (PartyDirection & 1)) {
		play_sound_at(0x23, PartyBlock);
		return;
	}
	row = SquareRow[i];
	view_clip(i, &a, &b, 5);
	if (b < a)
		return;
	if (t)
		t->active = 0;
	draw_scene(1);
	if (t)
		t->active = 2;
	Var_62A5 = 2;
	Drv_657A(0, 0, 0xB0, 0x78, 0);
	BackColour = Drv_657E(0, 0);
	draw_view_contents(Table_56D7[row]);
	Var_62A5 = 0;
	view_clip(i, &a, &b, 5);
	hide_mouse();
	BurstX = FloorObjectX[(i * 5 + 4) * 2] + 0x58;
	BurstY = 0x30;
	play_sound(0x23);
	switch (row) {
	case 0:
		sparks(3, 0x93, 0x23, 0x14, 7, colours);
		break;
	case 1:
		sparks(2, 0x93, 0x23, 0x14, 7, colours);
		break;
	case 2:
		sparks(1, 0x93, 0x23, 0x14, 7, colours);
		break;
	case 3:
		sparks(0, 0x1CC, 0x32, 0x14, 4, colours);
	}
	show_mouse();
}

/* 20E2:2117 [asm] - sparks: they fly, fall, bounce and change colour
 * (colours[life >> 8], 0: dead); kept in ShrinkBuffer */
byte random_byte(void);                                         /* 33DC:0004 a random byte */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void far sparks(int scale, int spread, int count, int speed, int lift, signed char far *colours)
{
	int far *px;
	int far *py;
	int far *vx;
	int far *vy;
	int far *under;
	int far *rate;
	int far *life;
	int far *base;
	int x2, y1, y2, top, left, right;
	register int ymax, x1;

	clock_stop(2);                    /* the game clock stops */
	ymax = Table_5709[scale];
	if (scale)
		scale--;
	base = (int far *)ShrinkBuffer;
	top = 0;
	left = -100;
	right = 0x114;
	y1 = Dims[5].y;
	y2 = y1 + Dims[5].h - 1;
	x1 = Dims[5].x * 8;
	x2 = x1 + Dims[5].w * 8;
	px = base;
	py = base + 150;
	vx = base + 300;
	vy = base + 450;
	under = base + 600;
	rate = base + 750;
	life = base + 900;
	SparkPage = Var_62A5;
	if (count > 150)
		count = 150;
	SparkCX = BurstX;
	SparkCY = BurstY;
	for (SparkI = 0; SparkI < count; SparkI++) {
		px[SparkI] = py[SparkI] = 0;
		vx[SparkI] = random_range(0, spread) - (spread >> 1);
		vy[SparkI] = random_range(0, spread) - (spread >> 1) - (spread >> (8 - lift));
		rate[SparkI] = random_range(0x400 / speed, 0x800 / speed);
		life[SparkI] = scale << 8;
	}
	SparkLive = 2;
	while (SparkLive) {
		if (SparkLive != 2)
			for (SparkI = count - 1; SparkI >= 0; SparkI--) {
				SparkX = (px[SparkI] >> 6 >> scale) + SparkCX;
				SparkY = (py[SparkI] >> 6 >> scale) + SparkCY;
				if (SparkY > ymax)
					SparkY = ymax;
				if (in_rect(SparkX, SparkY, x1, y1, x2, y2))
					Drv_65B2(SparkX, SparkY, under[SparkI]);
			}
		SparkLive = 0;
		for (SparkI = 0; SparkI < count; SparkI++) {
			if (vx[SparkI] > 0)
				vx[SparkI]--;
			else
				vx[SparkI]++;
			px[SparkI] += vx[SparkI];
			vy[SparkI] += 5;
			py[SparkI] += vy[SparkI];
			life[SparkI] += rate[SparkI];
			SparkX = (px[SparkI] >> 6 >> scale) + SparkCX;
			SparkY = (py[SparkI] >> 6 >> scale) + SparkCY;
			if (SparkY >= ymax || SparkY < top)
				vy[SparkI] = 0 - (vy[SparkI] >> 1);
			if (SparkX >= right || SparkX < left)
				vx[SparkI] = 0 - (vx[SparkI] >> 1);
			if (SparkY > ymax)
				SparkY = ymax;
			Var_62A5 = 2;
			SparkBack = Drv_657E(SparkX, SparkY);
			Var_62A5 = 0;
			under[SparkI] = Drv_657E(SparkX, SparkY);
			SparkColour = colours[life[SparkI] >> 8];
			if (SparkColour) {
				SparkLive = 1;
				if (SparkBack == BackColour && in_rect(SparkX, SparkY, x1, y1, x2, y2))
					Drv_65B2(SparkX, SparkY, SparkColour);
			} else
				rate[SparkI] = 0;
		}
		wait_tick();
		wait_tick();
	}
	clock_start(2);
	Var_62A5 = SparkPage;
}

/* 20E2:2596 [asm] - streaks from the borders of the view to its centre */
void far streaks(int count, int spread, register int step, int unused, int speed, int far *colours)
{
	int colour, live, cx, cy, x, y, savedPage, back, frame;
	int far *px;
	int far *py;
	int far *vx;
	int far *vy;
	int far *under;
	int far *rate;
	int far *life;
	int far *delay;
	int far *base;
	int dist, v, decel, d;
	register int i;

	Var_62A5 = 2;
	Drv_657A(0, 0, 0xB0, 0x78, 0);
	BackColour = Drv_657E(0, 0);
	draw_view_contents(7);
	Var_62A5 = 0;
	base = (int far *)ShrinkBuffer;
	px = base;
	py = base + 150;
	vx = base + 300;
	vy = base + 450;
	under = base + 600;
	rate = base + 750;
	life = base + 900;
	delay = base + 1050;
	spread <<= 6;
	clock_stop(2);                    /* the game clock stops */
	savedPage = Var_62A5;
	if (count > 150)
		count = 150;
	cx = 0x58;
	cy = 0x30;
	for (i = 0; i < count; i++) {
		dist = random_range(spread >> 2, spread);
		v = d = 0;
		while (d < dist) {
			v += step;
			d += v;
		}
		switch (random_byte() & 3) {
		case 0:
			px[i] = 0x20;
			py[i] = d;
			vx[i] = v;
			vy[i] = 0;
			break;
		case 1:
			px[i] = d;
			py[i] = 0x20;
			vx[i] = 0;
			vy[i] = v;
			break;
		case 2:
			px[i] = 0x20;
			py[i] = -d;
			vx[i] = v;
			vy[i] = 0;
			break;
		case 3:
			px[i] = -d;
			py[i] = 0x20;
			vx[i] = 0;
			vy[i] = v;
		}
		if (random_byte() & 1) {
			vx[i] *= -1;
			vy[i] *= -1;
		}
		rate[i] = random_range(0x400 / speed, 0x800 / speed);
		life[i] = 0;
		delay[i] = random_range(0, speed >> 2);
	}
	live = 2;
	frame = 0;
	while (live) {
		if (live != 2)
			for (i = count - 1; i >= 0; i--) {
				x = (px[i] >> 6) + cx;
				y = (py[i] >> 6) + cy;
				Drv_65B2(x, y, under[i]);
			}
		live = 0;
		decel = (step >> 1) + (step >> 2) + (step >> 3);
		for (i = 0; i < count; i++) {
			if (delay[i])
				delay[i]--;
			else {
				if (px[i] > 0) {
					if (vx[i] > 0)
						vx[i] -= step;
					else
						vx[i] -= decel;
				} else if (vx[i] < 0)
					vx[i] += step;
				else
					vx[i] += decel;
				if (py[i] > 0) {
					if (vy[i] > 0)
						vy[i] -= step;
					else
						vy[i] -= decel;
				} else if (vy[i] < 0)
					vy[i] += step;
				else
					vy[i] += decel;
				px[i] += vx[i];
				py[i] += vy[i];
				life[i] += rate[i];
			}
			x = (px[i] >> 6) + cx;
			y = (py[i] >> 6) + cy;
			if ((speed >> 2) > frame)
				back = 0;
			else {
				Var_62A5 = 2;
				back = Drv_657E(x, y);
				Var_62A5 = 0;
			}
			under[i] = Drv_657E(x, y);
			colour = colours[life[i] >> 8];
			if (colour) {
				live = 1;
				if (back == BackColour && delay[i] == 0)
					Drv_65B2(x, y, colour);
			} else
				rate[i] = 0;
		}
		wait_tick();
		frame++;
	}
	Var_62A5 = savedPage;
	unused++;
	clock_start(2);
}

/* 20E2:2AFB [asm] - all the spell effects of a character end (death,
 * petrification) */
void far end_all_effects(register int ch)
{
	Character far *c;
	register int i;

	c = &Party[ch];
	c->effects = 0;
	for (i = 0; i < 4; i++)
		(&c->armorPool)[i] = 0;
	for (i = 0; i < 10; i++) {
		if (c->timerWhat[i] < 0)
			end_spell_effect(-c->timerWhat[i], ch, 0);
		c->timers[i] = 0;
		c->timerWhat[i] = 0;
	}
	refresh_char_timers();
	recompute_ac(ch);
	c->busy = 0;
	c->handResult[0] = 0;
	c->handResult[1] = 0;
	c->lastDamage = 0;
	c->str = c->strMax;
	c->strExt = c->strExtMax;
	redraw_party();
}

/* 20E2:2BC7 [asm] - a random active character, -1 none.
 * ORIGINAL BUG: the search goes up to 6 (> 6, not > 5): Party[6] is read */
int far random_char(void)
{
	register int i;

	i = roll_dice(1, 6, 0);
	for (CharTries = 0; CharTries < 6; CharTries++) {
		if (char_is_active(i, 3))
			return i;
		if (++i > 6)
			i = 0;
	}
	return -1;
}

/* 20E2:2C16 [asm] - the next active character after ch (CharTries goes on
 * from random_char: it is not counted in the loop) */
int far next_char(int ch)
{
	register int i;

	i = ch + 1;
	CharTries++;
	while (CharTries < 6) {
		if (char_is_active(i, 3))
			return i;
		if (++i > 6)
			i = 0;
	}
	return -1;
}
