/*
 * Eye of the Beholder II - START.EXE
 * Transfer a party of Eye of the Beholder I (EOBDATA.SAV); overlay module
 * 4881 (Ghidra 5881).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y transfer.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * EOBDATA.SAV of EOB I starts with its 6 characters (243 bytes each), then
 * 64 bytes, then its items (14 bytes each, as in EOB II). The characters
 * keep their name, statistics, hit points, classes, levels, experience
 * (capped by class) and the mage spells of their book; the items are
 * converted one by one, and get the names of a similar item of EOB II.
 */
#include "eob2.h"

typedef unsigned char byte;

extern Character      Party[];           /* E608 */
extern Item           Items[];           /* 71CA */
extern ItemType       ItemTypes[];       /* 946E */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern char far      *ShrinkBuffer;      /* CBC4 */
extern char           MousePresent;          /* 6654 the keyboard moves the selection */
extern char           Scratch[];         /* F16E */
extern char far      *RaceNames[];       /* 1F16 */
extern char far      *ClassNames[];      /* 1F46 */
extern signed char    ClassCount[];      /* 1EC8 number of classes of a (multi-)class */
extern int            ClickUp[2];        /* 6622 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 */
extern byte far      *UnconsciousShape;  /* CB60 */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *Font8;             /* F098 */
extern char           Keys_2554[];       /* 2554 */
extern void (far     *Drv_655A)(void);   /* 655A starts the driver */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);    /* 657A fills */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern byte far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);    /* 6592 cuts a shape */

int  file_on_disk(char far *name);                               /* 2DCE:09B6 */
int  ask_yes_no(int style, int text, int no);                    /* 1E24:1027 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
char class_part(char cls, char k);                               /* 1B54:0082 */
void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
void farfree(void far *p);                                       /* 1000:7FF2 */
int  create_item(int type);                                      /* 1CBE:092B */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
int  read_key(void);                                             /* 3295:066D */
int  key_waiting(void);                                          /* 3295:0507 */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
void draw_frame3d(int x1, int y1, int x2, int y2, int light, int dark, int fill);   /* 18D7:0A45 */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
char far *strcat(char far *d, char far *s);                      /* 1000:74C4 */
void flash_box(int n);                                           /* 1E24:054E */
byte far *set_font(byte far *font);                              /* 3166:000A */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);      /* 18D7:067F */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
void message_box(int style, int text, char far *button, char far *keys, int colour);   /* 1E24:0E83 */
void movmem(void far *src, void far *dst, unsigned n);           /* 1000:714A */
void give_start_item(Character far *c, int type, int value, int slot);   /* 4EF4:0087 (overlay) */

/* EOB I spell number -> EOB II spell number (0: none); the cleric spells
 * of EOB I come after its 25 mage spells */
char far SpellConv[49] = {                                              /* 3500:0000 */
	0, 1, 2, 3, 4, 0, 5, 6, 10, 0, 11, 0, 12, 13, 0, 14,
	15, 16, 17, 18, 19, 20, 0, 0, 23, 24, 34, 36, 35, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 49, 48, 50, 51, 0, 53, 52, 54,
	55
};
/* the most experience a character keeps, by class part */
unsigned long far ExpCap[6] = { 999999L, 749999L, 899999L, 439999L, 899999L, 899999L };

/* Initialised data of the module (DS:16CE...) */
char Transferable[60] = {                                               /* 16CE by item type */
	1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
	1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1,
	0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 5881:0000 [asm] - ask for the disk until EOBDATA.SAV is found (or
 * cancel: 0); the characters are left at LoadBuffer, the items follow */
Item far * far read_eob1_save(void)
{
	byte far *p;
	register int ok, fd;

	do {
		ok = file_on_disk("EOBDATA.SAV");
		if (!ok) {
			show_mouse();
			ok = ask_yes_no(15, 0x44, 1) - 1;
			hide_mouse();
		}
	} while (!ok);
	if (ok == -1)
		return 0;
	p = LoadBuffer;
	fd = open_file("EOBDATA.SAV", 1);
	read_file(fd, p, 0x5B2L);
	p += 0x5B2;
	read_file(fd, p, 0x40L);
	read_file(fd, p, 7000L);
	close_file(fd);
	return (Item far *)p;
}

/* 5881:00C6 [asm] - the mage spells of a book of EOB I */
unsigned long far convert_spells(unsigned long book)
{
	int i, s;
	unsigned long r;

	for (r = i = 0; i < 25; i++)
		if (book & (1L << i)) {
			s = SpellConv[i + 1] - 1;
			if (s != -1)
				r |= 1L << s;
		}
	return r;
}

/* 5881:0133 [asm] */
void far convert_characters(void)
{
	byte far *old;
	Character far *c;
	byte far *shape;
	int k;
	register int x, i;

	old = LoadBuffer;
	c = Party;
	for (i = 0; i < 6; old += 243, c++, i++) {
		mem_copy(old, c, 27L);
		mem_copy(old + 0x1F, &c->raceSex, 20L);
		mem_copy(old + 0x77, c->inventory, 54L);
		c->mageBook = convert_spells(*(unsigned long far *)(old + 0x73));
		c->hpMax = (char)old[0x1C];
		c->ac = old[0x1D];
		c->flags &= 1;
		c->hp = c->hpMax;
		c->food = 100;
		if (c->portrait < 0)
			c->portrait = (c->portrait < 0 ? -c->portrait : c->portrait) + 43;
		for (k = 0; k < 3; k++)
			if ((x = class_part(c->cls, k)) != -1)
				if (ExpCap[x] < c->experience[k])
					c->experience[k] = ExpCap[x];
	}
	load_cps("CHARGENA", 3, 3);
	c = Party;
	for (i = 0; i < 6; i++, c++)
		if (c->flags) {
			shape = Drv_6592(c->portrait % 10 * 4, c->portrait / 10 * 32, 4, 32, 0L);
			mem_copy(shape, c->face, 1400L);
			farfree(shape);
		}
}

/* 5881:037F [asm] - an item of EOB I made into a new item: its index
 * (0 if it cannot be transferred).
 * ORIGINAL BUG: for an empty slot (0) or an item that cannot be
 * transferred, "p->level = 0" goes through p never set (it still holds
 * the item of the previous call). */
int far convert_item(int idx, Item far *olditems)
{
	Item far *p;
	Item far *q;
	Item far *o;
	int item;
	register int i, found;

	o = olditems + idx;
	item = 0;
	found = 0;
	if (idx && Transferable[o->type]) {
		item = create_item(1);
		p = &Items[item];
		p->flags = o->flags | 0x40;
		p->icon = o->icon;
		p->type = o->type;
		p->level = -1;
		switch (p->type) {
		case 0x23:
			if (p->type == 0x23)
				o->value += 25;
		case 0x22:
			p->value = SpellConv[o->value];
			if (p->value == 0) {
				p->block = -1;
				return 0;
			}
			break;
		case 0x30:
			if (o->value == 5) {
				memset(p, 0, 14);
				p->block = -1;
				item = 0;
				return item;
			}
			p->value = o->value;
			p->flags = o->flags & 0x3F;
			p->flags += 3;
			break;
		case 0x27:
			p->value = o->value - 1;
			break;
		case 0x12:
			p->icon = 0x13;
			p->value = o->value;
			break;
		default:
			p->value = o->value;
		}
		switch (ItemTypes[p->type].flags & 0x7F) {
		case 1:
		case 2:
		case 3:
			if (p->value)
				p->flags |= 0x80;
			break;
		case 5:
		case 6:
		case 9:
		case 10:
		case 14:
		case 16:
		case 18:
			p->flags |= 0x80;
		}
		q = &Items[1];
		for (i = 1; i < 600; i++, q++) {
			if (i == 60 || i == 62 || i == 63 || i == 83)
				continue;
			if (q->level != 0 || q->block == -2)
				continue;
			if (q->type == p->type && q->icon == p->icon) {
				p->nameUnid = q->nameUnid;
				p->name = q->name;
				found = 1;
				break;
			}
		}
		if (!found) {
			q = &Items[1];
			for (i = 1; i < 600; i++, q++) {
				if (i == 60 || i == 62 || i == 63 || i == 83)
					continue;
				if (q->level != 0 || q->block == -2)
					continue;
				if (q->type == p->type) {
					p->nameUnid = q->nameUnid;
					p->name = q->name;
					found = 1;
					break;
				}
			}
		}
		if (!found) {
			Drv_655A();
			sprintf(ShrinkBuffer, "Item conversion matching failure\rType:   %d\rIcon:   %d\rSpec:   %d\rFlags:   %d",
				o->type, o->icon, o->value, o->flags);
			Drv_65F2(ShrinkBuffer, 0, 0, 15, 12);
			read_key();
			memset(p, 0, 14);
			p->block = -1;
			item = 0;
		}
	}
	p->level = 0;                                 /* sic: p may be unset */
	return item;
}

/* 5881:06D7 [asm] - the inventory of a character (the quiver is a list) */
void far convert_inventory(Character far *c, Item far *olditems)
{
	int newi, first, firstTime;
	Item far *q;
	register int cur, slot;

	for (slot = 0; slot < 27; slot++)
		if (slot == 16) {
			cur = first = c->inventory[slot];
			c->inventory[slot] = 0;
			firstTime = 1;
			while (cur && (cur != first || firstTime)) {
				firstTime = 0;
				newi = convert_item(cur, olditems);
				put_item(&c->inventory[slot], -2, newi, 0);
				q = olditems;
				q += cur;
				cur = q->prev;
			}
		} else
			c->inventory[slot] = convert_item(c->inventory[slot], olditems);
}

/* 5881:07B5 [asm] - a character in the transfer screen (selected: its
 * name in another colour).
 * ORIGINAL BUG: the levels after the first one all show level[1]. */
void far draw_transfer_char(int i, int selected)
{
	int k;
	Character far *c;
	char buf[10];
	int fg1, fg2;
	register int y, x;

	y = i / 2 * 40;
	x = i % 2 * 159;
	c = &Party[i];
	hide_mouse();
	Drv_657A(x + 4, y + 0x18, x + 0x24, y + 0x39, 12);
	draw_frame3d(x + 0x28, y + 0x18, x + 0x9D, y + 0x39, FrameLight, FrameDark, FrameFlat);
	if (c->flags & 1) {
		put_shape(Var_62A5, c->face, x + 4, y + 0x19, 0);
		if (selected) {
			fg1 = 6;
			fg2 = 15;
		} else {
			put_shape(Var_62A5, UnconsciousShape, x + 4, y + 0x19, 0);
			fg1 = 15;
			fg2 = 12;
		}
		print_text(c->name, x + 0x2C, y + 0x1B, fg1, 0);
		Drv_65F2(RaceNames[c->raceSex], x + 0x2B, y + 0x24, fg2, 0);
		Drv_65F2(ClassNames[c->cls], x + 0x2B, y + 0x2B, fg2, 0);
		sprintf(Scratch, "Level: %d", c->level[0]);
		for (k = 1; k < ClassCount[c->cls]; k++) {
			sprintf(buf, " / %d", c->level[1]);   /* sic */
			strcat(Scratch, buf);
		}
		Drv_65F2(Scratch, x + 0x2B, y + 0x32, fg2, 0);
	}
	show_mouse();
}

int ButtonX[2] = { 9, 0x120 };                                          /* 170A */
char far *ButtonText[2] = { "CANCEL", "OK" };                           /* 170E */
int Selected = -1;                                                      /* 1716 */
int TransferRects[32] = {                                               /* 1718 */
	4, 0x18, 0x9D, 0x38, 0xA2, 0x18, 0x13C, 0x39,
	4, 0x40, 0x9D, 0x60, 0xA2, 0x40, 0x13C, 0x60,
	4, 0x68, 0x9D, 0x89, 0xA2, 0x68, 0x13C, 0x89,
	4, 0x94, 0x2E, 0x9E, 0x110, 0x94, 0x13A, 0x9E
};

/* 5881:09C6 [asm] - the keyboard selection: 0-5 a character, 6 CANCEL, 7 OK */
void far select_transfer(register int b)
{
	if (Selected > 5 && Selected != b) {
		hide_mouse();
		Drv_65F2(ButtonText[Selected - 6], ButtonX[Selected - 6], 0x97, 15, 0);
		show_mouse();
	}
	if (b < 6)
		flash_box(b + 14);
	else if (Selected != b) {
		if (Selected < 6)
			flash_box(-1);
		hide_mouse();
		Drv_65F2(ButtonText[b - 6], ButtonX[b - 6], 0x97, 6, 0);
		show_mouse();
	}
	Selected = b;
}

/* 5881:0A75 [asm] */
void far draw_transfer_screen(void)
{
	register int i;

	Var_62A5 = 2;
	set_font(GameFont);
	Drv_655A();
	draw_frame3d(0, 0, 0x13F, 0xA2, FrameLight, FrameDark, FrameFlat);
	Drv_65F2("You may only take four characters on your adventure.", 5, 3, 15, 0);
	Drv_65F2("Select OK when you are finished choosing your party.", 5, 10, 15, 0);
	for (i = 0; i < 6; i++)
		draw_transfer_char(i, 0);
	draw_frame3d(4, 0x94, 0x2E, 0x9F, FrameLight, FrameDark, FrameFlat);
	draw_frame3d(0x110, 0x94, 0x13A, 0x9F, FrameLight, FrameDark, FrameFlat);
	print_text("CANCEL", 9, 0x97, 15, 0);
	print_text("OK", 0x120, 0x97, 15, 0);
	Var_62A5 = 0;
	hide_mouse();
	copy_page(0, 0, 0, 0, 40, 200, 2, 0);
	show_mouse();
	read_key();
}

/* 5881:0BC5 [asm] - which box was clicked, -1 none */
int far transfer_hit(void)
{
	register int i;

	for (i = 0; i < 32; i += 4)
		if (in_rect(ClickUp[0], ClickUp[1], TransferRects[i], TransferRects[i + 1], TransferRects[i + 2], TransferRects[i + 3]))
			return i >> 2;
	return -1;
}

/* 5881:0C17 [asm] - choose the 4 characters: their mask, 0 if cancelled */
int far choose_transfer(void)
{
	int sel, cur, hit, n, k, result, x;
	register int done, key;

	show_mouse();
	draw_transfer_screen();
	cur = sel = done = 0;
	for (;;) {
		if (key_waiting()) {
			key = read_key() & 0x8FF;
			switch (key) {
			case 0x41:
				if ((hit = transfer_hit()) != -1) {
					cur = hit;
					done = 1;
				}
				break;
			case 0x60:
				cur -= 2;
				if (cur < 0)
					cur += 8;
				break;
			case 0x62:
				cur += 2;
				if (cur > 8)
					cur -= 8;
				break;
			case 0x5C:
			case 0x66:
				cur ^= 1;
				break;
			case 0x2B:
			case 0x3D:
				done = 1;
				break;
			case 0x6E:
				done = 1;
				cur = 6;
			}
		}
		if (MousePresent)
			select_transfer(cur);
		if (!done)
			continue;
		if (cur < 6 && (Party[cur].flags & 1)) {
			sel ^= 1 << cur;
			draw_transfer_char(cur, sel & (1 << cur));
		}
		done = 0;
		if (cur < 6)
			continue;
		x = (cur - 6) * 268 + 4;
		hide_mouse();
		draw_frame3d(x, 0x94, x + 0x2A, 0x9F, FrameFlat, FrameFlat, -1);
		wait_ticks(5);
		draw_frame3d(x, 0x94, x + 0x2A, 0x9F, FrameLight, FrameDark, -1);
		show_mouse();
		if (cur != 6) {
			n = 0;
			for (k = 0; k < 6; k++)
				if (sel & (1 << k))
					n++;
			if (n < 4)
				message_box(16, 0x45, "OK", Keys_2554, 6);
			if (n > 4)
				message_box(16, 0x46, "OK", Keys_2554, 6);
		}
		if (n == 4 || cur == 6)
			break;
	}
	set_font(Font8);
	if (cur == 7) {
		message_box(16, 0x47, "OK", Keys_2554, 6);
		result = sel;
	} else
		result = 0;
	hide_mouse();
	return result;
}

/* 5881:0E66 [asm] - empty a character, but keep its number and portrait
 * buffer */
void far clear_character(Character far *c)
{
	int id;
	byte far *face;

	face = c->face;
	id = c->id;
	memset(c, 0, sizeof(Character));
	c->face = face;
	c->id = id;
}

/* 5881:0EB4 [asm] - 1: a party transferred and ready */
int far transfer_party(void)
{
	Item far *items;
	byte far *face;
	int i, sel;
	register int k, j;

	if ((items = read_eob1_save()) == 0)
		return 0;
	convert_characters();
	if ((sel = choose_transfer()) == 0) {
		for (i = 0; i < 6; i++)
			clear_character(&Party[i]);
		return 0;
	}
	for (j = i = 0; i < 6; i++)
		if (sel & (1 << i)) {
			face = Party[j].face;
			movmem(&Party[i], &Party[j], sizeof(Character));
			Party[j].face = face;
			movmem(Party[i].face, Party[j].face, 1400);
			j++;
		}
	clear_character(&Party[4]);
	clear_character(&Party[5]);
	for (i = 0; i < 4; i++)
		convert_inventory(&Party[i], items);
	for (i = 0; i < 4; i++)
		for (k = 2; k < 16; k++)
			if (Party[i].inventory[k] == 0) {
				give_start_item(&Party[i], 0x5D, -1, 2);
				return 1;
			}
	Party[0].inventory[2] = 0;
	give_start_item(&Party[0], 0x5D, -1, 2);
	return 1;
}
