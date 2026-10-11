/*
 * Eye of the Beholder II - START.EXE
 * The portraits of the party: the frame (spell effects), the face, the
 * hands, the hit points and food bars, the inventory and statistics pages
 * of a character: module 0901 (Ghidra 2901).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y portraits.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The portraits are drawn on page 2 then copied to the screen (page 0),
 * unless the page drawn on (Var_62A5) is already 2. On the main screen the
 * 6 portraits are at 176 + (8 or 80), (2, 54 or 106); with an inventory or
 * statistics page open only the selected character is drawn, large.
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Character      Party[6];          /* E608 */
extern unsigned long  PartyEffects;      /* 6DDC */
extern unsigned long  GlobalFlags;       /* 6E26 */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */
extern char           InvCharacter;      /* 9317 */
extern char           InvSlot;           /* E5A4 */
extern char           SelectedChar;      /* EE20 the portrait clicked first (to swap), -1 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            VideoMode;         /* 628A */
extern byte           InventoryMode;          /* 6655 */
extern char           HpBars;            /* E4C5 hit points shown as bars (else as numbers) */
extern byte           FrameLight, FrameDark, FrameFlat, Col_6D80, Col_6D81;   /* EEF0, E5C2, E606, 6D80, 6D81 colours */
extern byte           Col_92D8, Col_71AE, Col_71BF;   /* 92D8, 71AE, 71BF */
extern byte far      *CurFont;           /* F090 */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *DeadShape;         /* E400 the bones of a dead character */
extern byte far      *UnconsciousShape;  /* CB60 */
extern byte far      *InvisibleRemap;    /* EEE8 */
extern byte far      *PoisonRemap;       /* EEE4 */
extern byte far      *StoneRemap;        /* EE30 */
extern byte far      *NormalPalette;     /* CD12 */
extern byte far      *HandResultShape;   /* DF34 */
extern byte far      *DamageShape;       /* DF26 */
extern byte far      *HandShapes[2];     /* D03A the empty hands */
extern byte far      *HandShape2;        /* D082 the left hand when GlobalFlags & 0x80000000 */
extern byte far      *BookShape;         /* 71C0 (also: a hand that cannot be used) */
extern char far      *StatNames[6];      /* 1EFE */
extern char far      *RaceNames[];       /* 1F16 */
extern char far      *ClassNames[];      /* 1F46 */
extern char far      *ClassShortNames[]; /* 1F82 */
extern char far      *AlignNames[];  /* 1FA9 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void draw_frame3d(int x1, int y1, int x2, int y2, int c1, int c2, int fill);   /* 18D7:0A45 3D frame */
void draw_rect(int x1, int y1, int x2, int y2, int colour);  /* 18D7:0571 rectangle */
void put_shape(int page, byte far *shape, int x, int y, int flags);   /* 2DA3:004A */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);  /* 2D92:000A */
void set_font(byte far *font);                                   /* 3166:000A */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void draw_item_icon(int page, int item, int x, int y);           /* 1E24:07C8 */
int  hand_usable(int ch, int hand);                              /* 1E24:08AE */
void draw_inventory_slot(int slot, int frame, int a);            /* 1CBE:09A6 */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void print_number(int n, int x, int y, int fg, int bg);          /* 3060:000E */
void print_long(unsigned long n, int x, int y, int fg, int bg);  /* 3107:0008 */
int  long_width(unsigned long n);                                /* 18D7:09E2 */
int  number_width(int n);                                        /* 18D7:0A15 */
char class_part(char cls, char k);                               /* 1B54:0082 */
char far *format_strength(char str, char ext);                   /* 1B54:04C6 */
int  sprintf(char far *buf, char far *fmt, ...);
char far *strcpy(char far *dst, char far *src);
unsigned strlen(char far *s);
char far *itoa(int value, char far *s, int radix);
extern void (far     *Drv_6566)(int x1, int y1, int x2, int y2, int colour);   /* 6566 a line */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A fills */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_65DE)(byte far *pal);   /* 65DE colour remapping */
extern void (far     *Drv_6576)(int on);          /* 6576 */

/* ------------------------------------------------------------------ */
/* Module 2901, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5D62...) */
signed char   HpColours[3] = { 3, 5, 8 };                   /* 5D62 good, wounded, dying */
unsigned char FrameX[2] = { 8, 80 };                        /* 5D65 */
signed char   FrameY[3] = { 2, 54, 106 };                   /* 5D67 */
signed char   HpX[2] = { 23, 95 };                          /* 5D6A */
unsigned char HpY[3] = { 46, 98, 150 };                     /* 5D6C */
signed char   FoodColours[3] = { 3, 5, 8 };                 /* 5D6F */
signed char   FaceX[2] = { 8, 80 };                         /* 5D72 */
signed char   FaceY[3] = { 11, 63, 115 };                   /* 5D74 */
signed char   HandX[2] = { 40, 112 };                       /* 5D77 */
unsigned char HandBoxY[6] = { 11, 27, 63, 79, 115, 131 };   /* 5D79 [row * 2 + hand] */
int           StatBoxX1[3] = { 0xB3, 0x110, 0x12D };        /* 5D7F the statistics page */
unsigned char StatBoxY1[3] = { 0x24, 0x33, 0x33 };          /* 5D85 */
int           StatBoxX2[3] = { 0x10F, 0x12C, 0x13E };       /* 5D88 */
unsigned char StatBoxY2[3] = { 0xA5, 0xA5, 0x93 };          /* 5D8E */
signed char   PortraitX[2] = { 1, 10 };                     /* 5D91 in bytes (8 pixels) */
signed char   PortraitTo[2] = { 23, 32 };                   /* 5D93 */
signed char   PortraitY[3] = { 2, 54, 106 };                /* 5D95 */

/* 2901:000C [asm] - the frame of a portrait: red (yellow under Haste)
 * for a protection, blue for a bonus to hit, both: dashes */
void far draw_portrait_frame(int ch)
{
	int colour;
	Character far *c;
	int prot, bonus, i, t;
	register int x, y;

	if (PartyEffects & 0x20000L)
		colour = 4;
	else
		colour = 6;
	x = FrameX[ch & 1];
	y = FrameY[ch >> 1];
	if (!Var_62A5)
		x += 0xB0;
	c = &Party[ch];
	prot = (c->effects & 0x4818L) || c->armorPool || c->unk146[0] || (PartyEffects & 0x20000L);
	bonus = (c->effects & 0x13000L) || (PartyEffects & 0x8420L);
	if (!prot && !bonus) {
		Drv_6566(x, y, x + 0x3E, y, FrameLight);
		Drv_6566(x, y + 0x31, x + 0x3E, y + 0x31, FrameDark);
		Drv_6566(x - 1, y, x - 1, y + 0x32, 12);
		Drv_6566(x + 0x3F, y, x + 0x3F, y + 0x32, 12);
	} else if (prot && !bonus)
		draw_rect(x, y, x + 0x3F, y + 0x31, colour);
	else if (bonus && !prot)
		draw_rect(x, y, x + 0x3F, y + 0x31, 5);
	else {
		t = x;
		for (i = 0; i < 0x40; i += 0x10) {
			x = t + i;
			if (prot) {
				Drv_6566(x, y, x + 7, y, colour);
				Drv_6566(x + 8, y + 0x31, x + 0xF, y + 0x31, colour);
			}
			if (bonus) {
				Drv_6566(x + 8, y, x + 0xF, y, 5);
				Drv_6566(x, y + 0x31, x + 7, y + 0x31, 5);
			}
		}
		x = t;
		t = y;
		for (i = 1; i < 0x30; i += 0xC) {
			y = t + i - 1;
			if (bonus) {
				Drv_6566(x, y + 1, x, y + 6, 5);
				Drv_6566(x + 0x3F, y + 7, x + 0x3F, y + 0xC, 5);
			}
			if (prot) {
				Drv_6566(x, y + 7, x, y + 0xC, colour);
				Drv_6566(x + 0x3F, y + 1, x + 0x3F, y + 6, colour);
			}
		}
	}
}

/* 2901:02DE [asm] - a bar: value out of max, w x h pixels */
void far draw_bar(register int x, register int y, int value, int max, int w, int h, int fg, int bg)
{
	int len;

	if (max < 1)
		return;
	if (value < 0)
		value = 0;
	if (value > max)
		value = max;
	len = w * value / max;
	if (len <= 0 && value > 0)
		len = 1;
	h--;
	draw_frame3d(x - 1, y - 1, x + w + 1, y + h + 1, FrameDark, FrameLight, -1);
	if (len > 0)
		Drv_657A(x, y, x + len, y + h, fg);
	if (len < w)
		Drv_657A(x + len, y, x + w, y + h, bg);
}

/* 2901:03A5 [asm] - the hit points: a bar (the option) or "nn of nn" */
void far draw_hp(int ch)
{
	int colour, hp, max, w, h;
	Character far *c;
	char buf[20];
	register int x, y;

	if (!char_is_active(ch, 1))
		return;
	c = &Party[ch];
	if (Var_EE3A && InvCharacter != ch)
		return;
	if (Var_EE3A == 0) {
		x = HpX[ch & 1];
		y = HpY[ch >> 1];
		w = 0x26;
		h = 3;
		if (!Var_62A5)
			x += 0xB0;
	} else {
		x = 0xFA;
		y = 0x10;
		w = 0x33;
		h = 5;
	}
	if (HpBars) {
		hp = c->hp + 10;
		max = c->hpMax + 10;
		colour = 0;
		if (max / 3 > hp)
			colour = 1;
		if (hp <= 10)
			colour = 2;
		if (Var_EE3A == 0)
			Drv_65F2("HP", x - 13, y - 1, 12, FrameFlat);
		draw_bar(x, y, hp, max, w, h, HpColours[colour], Col_6D80);
	} else {
		sprintf(buf, "%3d of %-3d", c->hp, c->hpMax);
		if (Var_EE3A == 0) {
			x -= 13;
			y--;
		}
		Drv_65F2(buf, x, y, 12, FrameFlat);
	}
}

/* 2901:0504 [asm] - the food bar (inventory page) */
void far draw_food(int ch)
{
	Character far *c;
	register int colour;

	if (Var_EE3A == 0)
		return;
	if (!char_is_active(ch, 1))
		return;
	c = &Party[ch];
	if (!(c->flags & 1))
		return;
	if (Var_EE3A && InvCharacter != ch)
		return;
	colour = 0;
	if (c->food < 33)
		colour = 1;
	if (c->food < 20)
		colour = 2;
	draw_bar(0xFA, 0x19, c->food, 100, 0x33, 5, FoodColours[colour], Col_6D80);
}

/* 2901:05AA [asm] - the face, recoloured when invisible, poisoned or
 * petrified, the bones of a dead character */
void far draw_face(int ch)
{
	Character far *c;
	int y;
	register int x;

	c = &Party[ch];
	if (!char_is_active(ch, 1))
		return;
	if (Var_EE3A == 0) {
		x = FaceX[ch & 1];
		y = FaceY[ch >> 1];
		if (!Var_62A5)
			x += 0xB0;
	} else {
		if (InvCharacter != ch)
			return;
		x = 0xB5;
		y = 3;
	}
	if (c->hp == -10)
		put_shape(Var_62A5, DeadShape, x, y, 0);
	else {
		if (c->effects & 0x140L) {
			Drv_65DE(InvisibleRemap);
			Drv_6576(1);
		}
		if (c->flags & 2) {
			Drv_65DE(PoisonRemap);
			Drv_6576(1);
		}
		if (c->flags & 8) {
			Drv_65DE(StoneRemap);
			Drv_6576(1);
		}
		put_shape(Var_62A5, c->face, x, y, 0);
		if (c->hp < 1)
			put_shape(Var_62A5, UnconsciousShape, x, y, 0);
		if ((c->flags & 2) || (c->flags & 8) || (c->effects & 0x140L)) {
			Drv_65DE(NormalPalette);
			Drv_6576(0);
		}
	}
}

/* 2901:072F [asm] - what a hand shows after an attack: the damage, or
 * -1 MISS, -2 HACK, -3 CAN'T REACH, -4 NO AMMO, -5 BASH */
void far draw_hand_result(int x, register int y, int result)
{
	char line1[6], line2[6];
	char dx, colour;

	line2[0] = 0;
	colour = 15;
	if (VideoMode == 0)
		colour = 2;
	if (result > -3 || result == -5)
		put_shape(Var_62A5, HandResultShape, x - 1, y, 0);
	else
		draw_frame3d(x, y, x + 0x1E, y + 0xF, Col_92D8, Col_71AE, Col_71BF);
	switch (result) {
	case -1:
		y++;
		strcpy(line1, "MISS");
		break;
	case -2:
		strcpy(line1, "HACK");
		break;
	case -5:
		strcpy(line1, "BASH");
		break;
	case -3:
		strcpy(line1, "CAN'T");
		strcpy(line2, "REACH");
		break;
	case -4:
		strcpy(line1, "NO");
		strcpy(line2, "AMMO");
		break;
	default:
		sprintf(line1, "%d", result);
	}
	if (line2[0] == 0) {
		dx = 16 - strlen(line1) * 6 / 2;
		Drv_65F2(line1, x + dx, y + 5, colour, 0);
	} else {
		dx = 16 - strlen(line1) * 6 / 2;
		Drv_65F2(line1, x + dx, y + 2, colour, 0);
		dx = 16 - strlen(line2) * 6 / 2;
		Drv_65F2(line2, x + dx, y + 9, colour, 0);
	}
}

/* 2901:091F [asm] - a hand of a portrait: the item, or the empty hand;
 * greyed out while it cannot be used */
void far draw_hand(char ch, char hand)
{
	int item;
	register int x, y;

	x = HandX[ch & 1];
	y = HandBoxY[(ch & 6) + hand];
	if (!Var_62A5)
		x += 0xB0;
	item = Party[ch].inventory[hand];
	hide_mouse();
	draw_frame3d(x, y, x + 0x1E, y + 0xF, FrameLight, FrameDark, FrameFlat);
	if (InventoryMode && ch == InvCharacter && hand == InvSlot)
		draw_rect(x, y, x + 0x1E, y + 0xF, 15);
	if (Party[ch].handResult[hand])
		draw_hand_result(x, y, Party[ch].handResult[hand]);
	else {
		if (item)
			draw_item_icon(Var_62A5, item, x + 8, y);
		else if (hand == 0 && (GlobalFlags & 0x80000000L))
			put_shape(Var_62A5, HandShape2, x + 8, y, 0);
		else
			put_shape(Var_62A5, HandShapes[hand], x + 8, y, 0);
		if ((Party[ch].busy & (1 << hand)) || !hand_usable(ch, hand) ||
		    Party[ch].hp <= 0 || (Party[ch].flags & 0xC))
			put_shape(Var_62A5, BookShape, x, y, 0);
	}
	show_mouse();
}

/* 2901:0AF9 [asm] - a portrait, or the inventory / statistics page */
void far draw_portrait(int ch)
{
	Character far *c;
	byte far *font;
	int i, x, y, page;
	unsigned long exp;
	char buf[6];
	int nameColour, statusColour;
	register int part, level;

	if (!char_is_active(ch, 1))
		return;
	c = &Party[ch];
	if (c->flags & 0xE) {
		nameColour = 8;
		statusColour = 6;
	} else {
		nameColour = 12;
		statusColour = 15;
	}
	switch (Var_EE3A) {
	case 0:
		hide_mouse();
		font = CurFont;
		set_font(GameFont);
		x = PortraitX[ch & 1];
		y = PortraitY[ch >> 1];
		copy_page(0x16, 0xA8, x, y, 8, 0x18, 2, 2);
		copy_page(0x1E, 0xA8, x, y + 0x18, 8, 0x1A, 2, 2);
		page = Var_62A5;
		Var_62A5 = 2;
		x <<= 3;
		if (SelectedChar == ch)
			Drv_65F2("Swapping", x + 2, y + 2, 8, FrameFlat);
		else
			Drv_65F2(c->name, x + 2, y + 2, nameColour, FrameFlat);
		draw_face(ch);
		draw_hand(ch, 0);
		draw_hand(ch, 1);
		draw_hp(ch);
		if (char_is_active(ch, 2))
			draw_portrait_frame(ch);
		if (c->lastDamage > 0) {
			itoa(c->lastDamage, buf, 10);
			put_shape(2, DamageShape, x + 0xD, y + 0x1E, 0);
			Drv_65F2(buf, x + 0x22 - strlen(buf) * 3, y + 0x2A, !VideoMode ? 12 : 15, 0);
		}
		Var_62A5 = page;
		if (!Var_62A5)
			copy_page(x >> 3, y, PortraitTo[ch & 1], y, 8, 0x32, 2, 0);
		set_font(font);
		show_mouse();
		return;
	case 1:
	case 2:
		if (InvCharacter != ch)
			return;
		copy_page(0x16, 0, 0, 0, 0x12, 0xA8, 2, 2);
		Var_62A5 = 2;
		draw_face(ch);
		print_text(c->name, 0xDB, 6, statusColour, FrameFlat);
		draw_hp(ch);
		draw_food(ch);
		if (Var_EE3A == 1) {
			if (c->hp + 10 == 0)
				print_text("DEAD", 0xF7, 0x9E, 6, Col_6D81);
			else if ((unsigned)(c->hp + 9) < 10)
				print_text("UNCONSCIOUS", 0xE2, 0x9E, 6, Col_6D81);
			else if ((c->flags & 2) && (c->effects & 0x2000L))
				print_text("POISON (SLOW)", 0xDC, 0x9E, 6, Col_6D81);
			else if (c->flags & 2)
				print_text("POISONED", 0xEB, 0x9E, 6, Col_6D81);
			else if (c->flags & 4)
				print_text("PARALYZED", 0xE8, 0x9E, 6, Col_6D81);
			else if (c->flags & 8)
				print_text("PETRIFIED", 0xE8, 0x9E, 6, Col_6D81);
			for (i = 0; i < 27; i++)
				draw_inventory_slot(i, 0, 2);
			draw_inventory_slot(16, 1, 2);
			draw_inventory_slot(InvSlot, 1, 2);
		} else {
			for (i = 0; i < 3; i++)
				Drv_657A(StatBoxX1[i], StatBoxY1[i], StatBoxX2[i], StatBoxY2[i], Col_6D81);
			print_text("CHARACTER INFO", 0xB7, 0x2A, 15, Col_6D81);
			Drv_65F2(ClassNames[c->cls], 0xB7, 0x37, 12, Col_6D81);
			Drv_65F2(AlignNames[c->alignment], 0xB7, 0x3E, 12, Col_6D81);
			Drv_65F2(RaceNames[c->raceSex], 0xB7, 0x45, 12, Col_6D81);
			for (i = 0; i < 6; i++)
				Drv_65F2(StatNames[i], 0xB7, i * 7 + 0x52, 12, Col_6D81);
			Drv_65F2("ARMOR CLASS", 0xB7, 0x7C, 12, Col_6D81);
			Drv_65F2("EXP", 0xEF, 0x8A, 12, Col_6D81);
			Drv_65F2("LVL", 0x116, 0x8A, 12, Col_6D81);
			Drv_65F2(format_strength(c->str, c->strExt), 0x113, 0x52, 15, Col_6D81);
			print_number(c->intel, 0x113, 0x59, 15, Col_6D81);
			print_number(c->wis, 0x113, 0x60, 15, Col_6D81);
			print_number(c->dex, 0x113, 0x67, 15, Col_6D81);
			print_number(c->con, 0x113, 0x6E, 15, Col_6D81);
			print_number(c->cha, 0x113, 0x75, 15, Col_6D81);
			print_number(c->ac, 0x113, 0x7C, 15, Col_6D81);
			for (i = 0; i < 3; i++) {
				part = class_part(c->cls, i);
				if (part == -1)
					continue;
				Drv_65F2(ClassShortNames[part], 0xB7, i * 7 + 0x91, 12, Col_6D81);
				exp = c->experience[i];
				level = c->level[i];
				print_long(exp, 0xF7 - long_width(exp), i * 7 + 0x91, 15, Col_6D81);
				print_number(level, 0x11E - number_width(level), i * 7 + 0x91, 15, Col_6D81);
			}
		}
		Var_62A5 = 0;
		hide_mouse();
		copy_page(0x16, 0, 0x16, 0, 0x12, 0xA8, 2, 0);
		show_mouse();
		copy_page(0, 0, 0x16, 0, 0x12, 0xA8, 2, 2);
	}
}
