/*
 * Eye of the Beholder II - START.EXE
 * Items: lists, creation, deletion, inventory: module 0CBE (Ghidra 1CBE).
 *
 * The items lying on a square, or in the quiver, form a circular list,
 * doubly linked by Item.next / Item.prev; the list variable (LevelBlock.items,
 * Character.inventory[16]) holds its last item.
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y items.c
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
extern ItemType       ItemTypes[];       /* 946E */
extern int            CurrentLevel;      /* 6DCE */
extern char           Scratch[];         /* F16E */
extern char           InvCharacter;      /* 9317 */
extern char           InvSlot;           /* E5A4 */
extern char far       ItemNames[][35];   /* 37DA:0000 */
extern Spell far      Spells[];          /* 39A6:0000 (25 bytes each, see eob2.h) */
extern Missile        Thrown[10];        /* CC5C */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            SlotX[27];         /* 2127 */
extern unsigned char  SlotY[27];         /* 215D */
extern int            SlotMask[27];      /* 2362 what can go in a slot (ItemType.slots) */
extern int            VideoMode;         /* 628A */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 */
extern unsigned char  InventoryMode;     /* 6655 */
extern byte far      *LoadBuffer;          /* D0AC */
extern void (far *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A video driver entry 15 */
extern void (far *Drv_65D2)(int a, int b, int c, int d, byte far *e);      /* 65D2 video driver entry 37 */
extern signed char    ClassFlags[];      /* 1F9A */
extern signed char far HornSounds[];     /* 3A17:0000 (a far data segment) */
extern int            PartyBlock, PartyDirection;   /* 6DD2, 6DD4 */
extern int            LastThrownItem;    /* CC32 */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  sprintf(char far *buf, char far *fmt, ...);
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
int  get_hand_item(void);                                        /* 1E24:0352 */
void take_item(int item);                                        /* 18AE:0197 the item in hand and the mouse cursor */
void print_item_name(int item);                                  /* 1CBE:05D1 */
void draw_inventory_slot(int slot, int a, int b);                /* 1CBE:09A6 */
void inventory_click(int slot);                                     /* 1CBE:0CB7 */
void hand_click(int ch, int slot);                             /* 1CBE:0DA7 */
char far *strcpy(char far *dst, char far *src);
unsigned strlen(char far *s);
char far *get_script_string(char far *text, int index);          /* 1E24:0869 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void draw_frame3d(int x1, int y1, int x2, int y2, int c1, int c2, int fill);   /* 18D7:0A45 3D frame */
void draw_rect(int x1, int y1, int x2, int y2, int colour);  /* 18D7:0571 rectangle */
void print_number(int n, int x, int y, int fg, int bg);          /* 3060:000E */
void draw_item_icon(int page, int item, int x, int y);           /* 1E24:07C8 */
void recompute_ac(int ch);                                       /* 1F55:0CF7 */
void target_character(int ch);                                   /* 20E2:0565 */
void add_char_timer(char ch, long delay, char what, char mode);  /* 27B7:06B7 */
void modify_hp(int ch, int amount);                              /* 1F55:0AC5 */
void set_char_status(int ch, int bit, char far *what, int a, long b, int c, int d);   /* 1F55:15D6 */
void neutralize_poison(int ch);                                        /* 20E2:005F */
void cast_from_item(int ch, int spell, int hand);                /* 20E2:1C70 */
int  hand_usable(int ch, int hand);                              /* 1E24:08AE */
void attack_with(int ch, int slot, int item);                    /* 1F55:0869 */
void open_spell_book(int ch, int cleric);                        /* 20E2:0EF6 */
void eat_food(int ch);                                           /* 1BB3:07B1 */
void read_note(int value);                                    /* 56DD:0F79 (overlay) */
void damage_character(int ch, int damage);                       /* 1F55:14AC */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
void far run_block_script(int block, unsigned events);           /* 24EC:2BE9 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
int  far can_use_item(int ch, int item);
int  far hand_of_slot(int slot);

/* ------------------------------------------------------------------ */
/* Module 1CBE, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:3634...): names of the kinds of
 * rings, potions and wands, by Item.value (get_script_string) */
char RingNames[45]   = "Adornment\0Wizardry\0Sustenance\0Feather Fall\0";     /* 3634 */
char PotionNames[85] = "Giant Strength\0Healing\0Extra Healing\0Poison\0Vitality\0Speed\0Invisibility\0Cure Poison\0";  /* 3661 */
char WandNames[76]   = "Stick\0Lightning\0Frost\0Curing\0Fireball\0Starfire\0Magic Missile\0Dispel Magic\0";  /* 36B6 */
/* per inventory slot: 0 normal, 1 the quiver (a list), 2 a ring */
signed char SlotKind[27] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2 };   /* 3702 */
signed char SlotSize[3]  = { 16, 16, 8 };          /* 371D size of the frame, per kind */

/* 1CBE:0005 [asm] - a character loses one of his items that can be lost
 * (ItemType.flags & 0x80), starting from a random slot */
void far lose_random_item(int ch)
{
	int n, item;
	int slot;

	slot = roll_dice(1, 27, -1);
	for (n = 0; n < 27; n++) {
		if ((item = Party[ch].inventory[slot]) != 0 && (ItemTypes[Items[item].type].flags & 0x80)) {
			Party[ch].inventory[slot] = 0;
			if (Party[ch].raceSex & 1)
				sprintf(Scratch, "%s has lost her ", Party[ch].name);
			else
				sprintf(Scratch, "%s has lost his ", Party[ch].name);
			gui_printf(Scratch);
			print_item_name(item);
			gui_printf(".\r");
			break;
		}
		if (++slot >= 27)
			slot = 0;
	}
	draw_portrait(ch);
}

/* 1CBE:0101 [asm] - the item in a slot (-1: in the hand) is used up.
 * Returns its value. */
int far consume_hand_item(int ch, int slot)
{
	int item;

	if (slot == -1)
		item = get_hand_item();
	else
		item = Party[ch].inventory[slot];
	Items[item].block = -1;
	if (slot == -1)
		take_item(0);
	else {
		Party[ch].inventory[slot] = 0;
		if (Var_EE3A == 1)
			draw_inventory_slot(slot, 1, 0);
		else if (Var_EE3A == 0)
			draw_portrait(ch);
	}
	return Items[item].value;
}

/* 1CBE:01A1 [asm] */
void far refresh_selected_slot(void)
{
	if (Var_EE3A == 1)
		inventory_click(InvSlot);
	else if (Var_EE3A != 2)
		hand_click(InvCharacter, InvSlot);
}

/* 1CBE:01D4 [asm] - add an item at the end of a list */
void far put_item(unsigned far *list, int block, int item, int pos)
{
	int last, first;
	Item far *it, far *pf, far *pl;

	if (item == 0)
		return;
	it = &Items[item];
	if (*list == 0) {
		*list = item;
		it->prev = it->next = item;
		it->pos = pos;
		it->block = block;
		if (block >= 0)
			it->level = CurrentLevel;
		else
			it->level = -1;
	} else {
		last = *list;
		pl = &Items[last];
		first = Items[last].next;
		pf = &Items[first];
		it->block = block;
		it->pos = pos;
		if (block >= 0)
			it->level = CurrentLevel;
		else
			it->level = -1;
		it->prev = pf->prev;
		it->next = first;
		pf->prev = item;
		pl->next = item;
		*list = item;
	}
}

/* 1CBE:02BF [asm] - take out of a list the item "item", or (item -1) the
 * first one found at place "pos". Returns it, or 0. */
int far unlink_item(unsigned far *list, int pos, int item)
{
	int start, count;
	Item far *it;
	int cur, prev, next;

	cur = *list;
	if (cur == 0)
		return 0;
	count = 0;
	start = cur;
	while (cur != start || count == 0) {
		it = &Items[cur];
		if ((item == -1 && it->pos == pos) || item == cur) {
			next = it->next;
			prev = it->prev;
			Items[next].prev = prev;
			Items[prev].next = next;
			it->prev = 0;
			it->next = 0;
			it->block = 0;
			it->level = 0;
			if (*list == cur)
				*list = prev;
			if (*list == cur)
				*list = 0;                         /* it was alone */
			return cur;
		}
		count++;
		cur = it->prev;
	}
	return 0;
}

/* 1CBE:0387 [asm] - look in a list for the item "item", or (-1) an item of
 * type "type" (-1: any). Items flying over the square (places 4-7) only
 * count with "all". Returns the item, or with "count" the number found. */
int far find_item(unsigned far *list, int item, int type, int count, int all)
{
	int first, start, n;
	Item far *it;
	int cur;

	cur = *list;
	if (cur == 0)
		return 0;
	first = 1;
	n = 0;
	start = cur;
	while (cur != start || first) {
		it = &Items[cur];
		if ((item == -1 && type == -1) || (item == -1 && it->type == type) || (type == -1 && cur == item)) {
			if (all != 0 || it->pos < 4 || it->pos > 7) {
				if (count)
					n++;
				else
					return cur;
			}
		}
		first = 0;
		cur = it->prev;
	}
	return n;
}

/* 1CBE:042D [asm] - identify all the items of a list */
void far identify_list(unsigned far *list)
{
	int start;
	Item far *it;
	int cur, first;

	cur = *list;
	if (cur) {
		first = 1;
		start = cur;
		while (cur != start || first) {
			it = &Items[cur];
			it->flags |= 0x40;
			first = 0;
			cur = it->prev;
		}
	}
}

/* 1CBE:0474 [asm] - put an item on a square, at the head of its list */
void far place_item(int block, int item, int pos)
{
	put_item(&LevelMap[block].items, block, item, pos);
	if (Items[item].prev != item)
		LevelMap[block].items = Items[item].prev;
}

/* 1CBE:04C6 [asm] - delete the items of a type (-1: all) lying on a square */
void far delete_items(int block, int type)
{
	int start, count;
	int cur, i;

	cur = LevelMap[block].items;
	if (cur == 0)
		return;
	LevelMap[block].items = 0;
	count = 0;
	start = cur;
	while (cur != start || count == 0) {
		if (Items[cur].type == type || type == -1) {
			Items[cur].block = -1;
			Items[cur].level = 0;
			i = cur;
			cur = Items[cur].prev;
			Items[i].prev = 0;
			Items[i].next = 0;
		} else {
			/* the others are put back on the square */
			i = cur;
			cur = Items[cur].prev;
			Items[i].prev = 0;
			Items[i].next = 0;
			put_item(&LevelMap[block].items, block, i, Items[i].pos);
		}
		count++;
	}
}

/* 1CBE:05D1 [asm] - print the name of an item: identified scrolls, rings,
 * potions and wands get "Mage Scroll of Fireball"..., weapons and armour
 * "+2 Long Sword" or "-1 Cursed Long Sword" */
void far print_item_name(int item)
{
	char value, kind;
	Item far *it;
	char far *unid, far *name, far *what, far *sub;
	char buf[60];

	it = &Items[item];
	unid = ItemNames[it->nameUnid];
	name = ItemNames[it->name];
	kind = ItemTypes[it->type].flags & 0x7F;
	value = it->value;
	what = 0;
	sub = 0;
	if ((it->flags & 0x40) && strlen(name) == 0) {
		switch (kind & 0x7F) {
		case 0:
		case 1:
		case 2:
		case 3:                                   /* weapons, armour */
			if (value) {
				if (value < 0)
					sprintf(buf, "%d Cursed %s", value, unid);
				else
					sprintf(buf, "+%d %s", value, unid);
			} else
				strcpy(buf, unid);
			break;
		case 9:
			what = "Mage Scroll";
			sub = Spells[value].name;
			break;
		case 10:
			what = "Cleric Scroll";
			sub = Spells[value].name;
			break;
		case 16:
			what = "Ring";
			sub = get_script_string(RingNames, value);
			break;
		case 14:
			what = "Potion";
			sub = get_script_string(PotionNames, value);
			break;
		case 18:
			if (value != 5)                       /* (the Starfire wand is not called a wand) */
				what = "Wand";
			sub = get_script_string(WandNames, value);
			break;
		default:
			strcpy(buf, unid);
			break;
		}
		if (what || sub) {
			if (!what)
				sprintf(buf, "%s", sub);
			else if (sub)
				sprintf(buf, "%s of %s", what, sub);
		}
	} else if (it->flags & 0x40)
		strcpy(buf, name);
	else
		strcpy(buf, unid);
	gui_printf(buf);
}

/* 1CBE:086A [asm] - throw an item from (block, pos) towards dir: it flies
 * 12 squares at most. "from": the character, < 0 a monster. */
int far launch_item(int from, int item, int block, int pos, int dir, int type)
{
	Missile far *t;
	int i;

	for (t = Thrown, i = 0; i < 10; i++, t++)
		if (t->active == 0)
			break;
	if (i == 10)
		return 0;
	put_item(&LevelMap[block].items, block, item, pos | 4);
	t->active = 1;
	t->justLaunched = 1;
	t->flags = 0;
	t->dir = dir;
	t->range = 12;
	t->block = block;
	t->pos = pos;
	t->item = item;
	t->shape = type;
	t->caster = from;
	t->justLaunched = 1;
	t->hit = 0;
	play_sound(type != 7 ? 11 : 26);
	return 1;
}

/* 1CBE:092B [asm] - a new item, a copy of the item "src" (a model), in the
 * first free entry of Items[] (block -1). Returns it, or 0. */
int far create_item(int src)
{
	Item far *p, far *it;
	int i, found;

	it = &Items[src];
	if (it->block == -1)
		return 0;
	for (i = 1, found = 0, p = &Items[1]; i < 600; i++, p++)
		if (p->block == -1) {
			found = 1;
			break;
		}
	if (!found)
		return 0;
	mem_copy(it, p, 14L);
	return i;
}

/* 1CBE:09A6 [asm] - draw a slot of the inventory shown (frame, number of
 * arrows in the quiver, the item) */
void far draw_inventory_slot(int slot, int frame, int page)
{
	int x, y, item, n, saved;

	saved = Var_62A5;
	Var_62A5 = page;
	x = SlotX[slot];
	y = SlotY[slot];
	item = Party[InvCharacter].inventory[slot];
	if (frame) {
		frame = SlotKind[slot];                   /* (the variable is reused) */
		if (VideoMode == 0)
			draw_frame3d(x - 1, y - 1, x + SlotSize[frame], y + SlotSize[frame], 3, 1, frame != 1 ? FrameFlat : -1);
		else
			draw_frame3d(x - 1, y - 1, x + SlotSize[frame], y + SlotSize[frame], FrameLight, FrameDark, frame != 1 ? FrameFlat : -1);
		if (frame == 1) {
			Drv_657A(0xE3, 0x41, 0xEE, 0x45, 12);
			n = find_item(&Party[InvCharacter].inventory[slot], -1, -1, 1, 1);
			if (n < 10)
				x = 0xE9;
			else
				x = 0xE3;
			print_number(n, x, 0x41, 15, 0);
			x = SlotX[slot];
		}
		if (InventoryMode && InvSlot == slot)
			draw_rect(x - 1, y - 1, x + SlotSize[frame], y + SlotSize[frame], 15);
	}
	if (item && SlotKind[slot] != 1) {
		if (SlotKind[slot] != 2)
			draw_item_icon(page, item, x, y);
		else
			draw_item_icon(page, item, x - 4, y - 4);   /* rings are small */
	}
	Var_62A5 = saved;
}

/* 1CBE:0B5B [asm] */
void far fn_1CBE_0B5B(int show)
{
	if (show)
		Drv_65D2(0x16, 0, 0x12, 0xA8, LoadBuffer);
	draw_portrait(InvCharacter);
}

/* 1CBE:0B90 [asm] - may this item go in this slot? (with the messages) */
int far item_fits_slot(int item, int ch, int slot)
{
	int mask, kind;

	if (slot == 17 && item && !can_use_item(ch, item)) {
		sprintf(Scratch, "%s can't wear that type of armor.\r", Party[ch].name);
		gui_printf(Scratch);
		return 0;
	}
	if (slot < 2 && (Items[Party[ch].inventory[slot]].flags & 0x20)) {
		kind = ItemTypes[Items[Party[ch].inventory[slot]].type].flags & 0x7F;
		if (kind >= 1 && kind <= 3)
			gui_printf("%s cannot release the weapon!  It is cursed!\r", Party[ch].name);
		return 0;                                 /* a cursed item does not leave the hand */
	}
	if (item)
		mask = ItemTypes[Items[item].type].slots;
	else
		mask = -1;
	if (!(SlotMask[slot] & mask)) {
		gui_printf("You can't put that item there.\r");
		return 0;
	}
	return 1;
}

/* 1CBE:0CB7 [asm] - a click on a slot of the inventory: swap with the hand
 * (the quiver gives or takes one arrow) */
void far inventory_click(int slot)
{
	int item;
	int hand;

	item = Party[InvCharacter].inventory[slot];
	hand = get_hand_item();
	if (!item_fits_slot(hand, InvCharacter, slot))
		return;
	if (slot != 16) {
		take_item(item);
		Party[InvCharacter].inventory[slot] = hand;
		draw_inventory_slot(slot, 1, 0);
		recompute_ac(InvCharacter);
	} else if (hand == 0) {
		item = unlink_item(&Party[InvCharacter].inventory[16], 0, -1);
		draw_inventory_slot(slot, 1, 0);
		take_item(item);
	} else {
		put_item(&Party[InvCharacter].inventory[16], -2, hand, 0);
		draw_inventory_slot(slot, 1, 0);
		take_item(0);
	}
}

/* 1CBE:0DA7 [asm] - a click on a hand of a portrait: swap with the hand
 * (a cursed item stays) */
void far hand_click(int ch, int slot)
{
	int hand, type, item;
	unsigned char mask;

	item = Party[ch].inventory[slot];
	if (Items[item].flags & 0x20)
		return;
	hand = get_hand_item();
	type = Items[hand].type;
	if (hand)
		mask = ItemTypes[type].slots;
	else
		mask = 0xFF;
	if (SlotMask[slot] & mask) {
		take_item(item);
		Party[ch].inventory[slot] = hand;
		draw_portrait(ch);
	}
	recompute_ac(ch);
}

/* how the drinker feels, per potion (Item.value)                     3720 */
char PotionFeel[94] = "much stronger\0better\0much better\0ill for a moment\0satiated\0fast and agile\0transparent\0better\0";

/* 1CBE:0E4F [asm] - drink the potion in a slot (-1: in the hand) */
void far drink_potion(int ch, int slot)
{
	Character far *c;
	int kind;

	c = &Party[ch];
	kind = consume_hand_item(ch, slot);
	play_sound(10);
	switch (kind) {
	case 0:                                       /* giant strength */
		target_character(ch);
		c->str = 22;
		c->strExt = 0;
		add_char_timer(ch, (roll_dice(1, 4, 0) + 4) * 546, 7, 1);
		break;
	case 1:                                       /* healing */
		target_character(ch);
		modify_hp(ch, roll_dice(2, 4, 2));
		break;
	case 2:                                       /* extra healing */
		target_character(ch);
		modify_hp(ch, roll_dice(3, 8, 3));
		break;
	case 3:                                       /* poison (a save may avoid it) */
		set_char_status(ch, 2, "poisoned", 0, 1L, 8, 1);
		c->effects &= ~0x2000L;                   /* slow poison is over */
		break;
	case 4:                                       /* vitality */
		target_character(ch);
		c->food = 100;
		if (Var_EE3A)
			draw_portrait(ch);
		break;
	case 5:                                       /* speed */
		target_character(ch);
		c->effects |= 0x10000L;
		add_char_timer(ch, (roll_dice(1, 4, 0) + 4) * 546, 12, 1);
		play_sound(100);
		draw_portrait(ch);
		break;
	case 6:                                       /* invisibility */
		target_character(ch);
		c->effects |= 0x40L;
		draw_portrait(ch);
		break;
	case 7:                                       /* cure poison */
		target_character(ch);
		neutralize_poison(ch);
		break;
	}
	if (kind != 3 || (c->flags & 2) == 0) {   /* (a resisted poison: "ill for a moment") */
		sprintf(Scratch, "%s feels %s!\r", c->name, get_script_string(PotionFeel, kind));
		gui_printf(Scratch);
	}
}

/* the spell of each wand (Item.value); 5, Starfire, is special   377E */
signed char WandSpells[8] = { 0, 0x11, 0x17, 0x31, 0x0D, -1, 4, 0x0C };

/* 1CBE:1082 [asm] */
void far use_wand(int ch, int slot)
{
	int spell;
	int value;

	value = Items[Party[ch].inventory[slot]].value;
	if (value == 0) {
		gui_printf("The wand has no apparent magical effect\r");
		return;
	}
	if (value != 5) {
		spell = WandSpells[value];
		cast_from_item(ch, spell, slot);
	} else
		cast_from_item(ch, 0x40, slot);
}

char *HornMessages[4] = {                          /* 3786 */
	"A bellowing sound comes from the horn.\r",
	"A hollow sound comes from the horn.\r",
	"A melodious sound comes from the horn.\r",
	"An eerie sound comes from the horn.\r",
};

/* 1CBE:10EC [asm] - blow a horn (value 1-4) */
void far blow_horn(int ch, int slot)
{
	int n;

	n = Items[Party[ch].inventory[slot]].value - 1;
	gui_printf(HornMessages[n]);
	play_sound(HornSounds[n]);
}

/* 1CBE:1140 [asm] - a character uses the item of a slot (a hand: right
 * click). What it does depends on the kind of item (ItemType.flags & 0x7F). */
void far use_item(register int ch, int slot)
{
	int type, hand, kind, value;
	Character far *c;
	register int item;

	c = &Party[ch];
	if (!char_is_active(ch, 13))
		return;
	item = c->inventory[slot];
	hand = hand_of_slot(slot);
	if (hand != -1) {
		if (!hand_usable(ch, hand))
			return;
		if (Var_EE3A == 0 && (Party[ch].busy & (1 << slot)))
			return;                               /* the hand is still busy */
	}
	if (!can_use_item(ch, item)) {
		gui_printf(c->name);
		gui_printf(" can not use this item.\r");
		return;
	}
	if (item == 0 && hand == -1)
		return;
	type = Items[item].type;
	value = Items[item].value;
	kind = ItemTypes[type].flags & 0x7F;
	switch (kind) {
	case 0:
	case 16:
		gui_printf("This item automatically used when worn.\r");
		break;
	case 1:
	case 2:
	case 3:                                       /* weapons: attack */
		if (Var_EE3A == 0)
			attack_with(ch, slot, item);
		break;
	case 5:
	case 6:                                       /* spell book, holy symbol */
		if (Var_EE3A == 0)
			open_spell_book(ch, kind == 6);
		break;
	case 7:                                       /* food */
		if (InventoryMode)
			eat_food(ch);
		break;
	case 4:
	case 8:
	case 12:
	case 13:
	case 15:
		gui_printf("This item is not used in this way.\r");
		break;
	case 9:                                       /* mage scroll */
		if (Var_EE3A == 0)
			cast_from_item(ch, value, hand);
		break;
	case 10:                                      /* cleric scroll */
		if (Var_EE3A == 0)
			cast_from_item(ch, value, hand);
		break;
	case 11:
		read_note(value);
		break;
	case 14:
		drink_potion(ch, slot);
		break;
	case 18:
		if (Var_EE3A == 0)
			use_wand(ch, slot);
		break;
	case 19:
		blow_horn(ch, slot);
		break;
	case 20:
		if (value == 1)
			damage_character(ch, 200);
		else
			cast_from_item(ch, 0x37, slot);
		consume_hand_item(ch, slot);
		break;
	}
	/* the square ahead reacts to the item used (event 0x100) - for the
	 * weapons of kind 1, only when used by the front row */
	if (kind != 1 || ch < 2) {
		LastThrownItem = item;
		run_block_script(block_ahead(PartyBlock, PartyDirection), 0x100);
		LastThrownItem = 0;
	}
}

/* 1CBE:13AF [asm] - may the character's class use this item? */
int far can_use_item(int ch, int item)
{
	int r;

	if (item)
		r = (char)(ItemTypes[Items[item].type].classes & ClassFlags[Party[ch].cls]);
	else
		r = 1;
	return r;
}

/* 1CBE:13F8 [asm] - slot 0 or 1 is a hand, the others -1 */
int far hand_of_slot(int slot)
{
	return slot > 1 ? -1 : slot;
}
