/*
 * Eye of the Beholder II - START.EXE
 * The characters: hits, damage, hit points, experience, armour class, death:
 * module 0F55 (Ghidra 1F55).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y party.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
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
extern ItemType       ItemTypes[];       /* 946E */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern unsigned long  PartyEffects;      /* 6DDC */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern char           SceneDirty;        /* D0AA */
extern signed char    PlaceSeen[4][4];   /* 2352 [party direction][place] */
extern char           WallClickType[256];/* 2218 */
extern char           Front;             /* 9444 */
extern char           LastHitMonster;    /* CB70 */
extern signed char    FrontPos[4][4];    /* 2342 */
extern unsigned long  GlobalFlags;       /* 6E26 */
extern byte far      *Font8;             /* F098 */
extern byte far      *GameFont;          /* 9342 */
extern signed char    ClassCount[];      /* 1EC8 number of classes of a (multi-)class */
extern char           Scratch[];         /* F16E */
extern signed char    ClassFlags[];      /* 1F9A */
extern int            Var_E5A2;          /* E5A2 */
extern signed char    ClassThac0Group[]; /* 1ED7 0 warrior 1 wizard 2 priest 3 rogue */
extern char           Var_EE3A;          /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */
extern char           BookOpen;         /* E441 1: the spell book is open (?) */
extern char           BookCaster;        /* D0B0 the character whose spell book is open */
extern char           GuiState[];        /* E5CA */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  str_to_hit_bonus(int ch);                                   /* 1B54:00F0 */
int  dex_to_hit_bonus(int ch);                                   /* 1B54:01A2 */
char thac0_target(char ch, char ac);                             /* 1B54:0569 the roll needed */
int  str_damage_bonus(int ch);                                   /* 1B54:0149 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void far inflict_damage(int ch, int from, int item, int a, int flags, int save, int saveType);
void far damage_monster(Monster far *m, int from, int item, int a, int flags, int save, int saveType);
int  far hit_monster(Monster far *m, int ch, int item, int a, int flags, int save, int saveType);
void far damage_character(int ch, int dmg);
int  block_ahead(int block, int dir);                            /* 1E24:012F */
int  monster_to_attack(char ch, int block);                      /* 1997:1929 the monster ch can reach, -1 */
void monster_dies(Monster far *m, int xp);                       /* 1997:114E */
int  is_spell_item(unsigned item);                               /* 20E2:0044 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
int  unlink_item(unsigned far *list, int pos, int item);         /* 1CBE:02BF */
int  launch_item(int from, int item, int block, int pos, int dir, int type);   /* 1CBE:086A */
void play_sound(int sound);                                      /* 2C9F:00AA */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
void end_spell_effect(int spell, int ch, int a);                 /* 20E2:0C6D */
void far monster_hit(Monster far *m, int dmg, int a);
void add_char_timer(char ch, long delay, char what, char mode);  /* 27B7:06B7 */
void redraw_party(void);                                         /* 1E24:0686 */
void kheldran_laughs(void);                                         /* 56DD:1774 (overlay) */
int  ask_yes_no(int style, int text, int no);                   /* 1E24:1027 */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);
void set_font(byte far *font);                                   /* 3166:000A */
void close_panels(void);                                         /* 1E24:0B5B */
int  load_game(int a, int b);                                    /* 559A:04A7 (overlay) */
void draw_hp(int ch);                                       /* 2901:03A5 */
char class_part(char cls, char k);                               /* 1B54:0082 the k-th class of a multi-class, -1 */
long xp_for_level(char cls, char part, char level);              /* 1B54:0515 -1: no such class part */
int  roll_level_hp(char ch, char parts);                         /* 1B54:01FB */
int  sprintf(char far *buf, char far *fmt, ...);
int  dex_ac_bonus(char dex);                                     /* 1B54:01BD */
int  hand_usable(int ch, int hand);                              /* 1E24:08AE */
int  cleric_level(int ch);                                       /* 20E2:03F9 */
int  char_saving_throw(int ch, int type);                        /* 1E24:069F */
int  save_table(char a, char level, char type);                  /* 1B54:000D */
void far monster_explodes(Monster far *m);
int  block_changed(int block);                                   /* 1E24:0005 */
void monster_flash(Monster far *m);                              /* 1997:106E */
int  distance(int from, int to);                                 /* 2A68:0FD8 */
int  monster_resists(Monster far *m);                            /* 1E24:0A76 */
int  has_ring(Character far *c, int kind);                       /* 1E24:01BE 1 or 0 */
void end_all_effects(int ch);                                    /* 20E2:2AFB */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void draw_face(int ch);                                       /* 2901:05AA redraws the character sheet (?) */
int  close_spell_book(void far *gui);                            /* 20E2:14A7 */
void view_burst(int block, int type, Missile far *t);            /* 20E2:1EF9 */
void monster_flee(Monster far *m);                               /* 1997:01D9 */

/* ------------------------------------------------------------------ */
/* Module 1F55, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5530...) */
/* the characters a missile may hit, per place seen (2 each, -1: none)    5530 */
signed char MissileTargets[8] = { 0, -1, 1, -1, 2, 4, 3, 5 };
/* the ammunition of a missile weapon, by type - 7 (7 bow: arrows, 14 sling: stones)   5538 */
signed char AmmoOf[8] = { 0x10, -1, -1, -1, -1, -1, -1, 0x12 };
/* the slots whose item counts for the armour class (armour, hands, ...)    5540 */
signed char ArmorSlots[5] = { 17, 0, 1, 18, 19 };
/* saving throw bonus of dwarves, gnomes and halflings, by constitution     5545 */
signed char ConSaveBonus[20] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5 };
/* pairs { attack flag, monster immunity }: an immune monster takes no damage
 * (0x10 thrown / 0x10 ..., 0x400 poison / 0x40 ...)                        5559 */
unsigned Immunities[12] = {
	0x0001, 0x0010, 0x0002, 0x0020, 0x0080, 0x0400,
	0x0020, 0x0800, 0x0040, 0x0080, 0x0400, 0x0040
};

/* 1F55:000C [asm] - does character ch hit monster mon with "item" (0: bare
 * hands)? melee: the strength bonus, else the dexterity one. ch < 0 (not a
 * character) always hits. Magical weapons are needed against some monsters. */
int far char_hits_monster(int ch, int mon, int item, int melee)
{
	int type, need, kind;
	int roll, bonus;

	if (ch < 0)
		return 1;
	if (item) {
		kind = Items[item].type;
		kind = ItemTypes[kind].flags & 0x7F;
	} else
		kind = 0;
	if (Monsters[mon].flags & 0x20)
		return 1;
	type = Monsters[mon].type;
	if (kind >= 1 && kind <= 3)
		bonus = Items[item].value;
	else
		bonus = 0;
	if ((kind >= 1 && kind <= 3) || item == 0) {
		if ((MonsterTypes[type].immunities & 0x200) && bonus <= 0)
			return 0;                             /* needs a +1 weapon */
		if ((MonsterTypes[type].immunities & 0x1000) && bonus <= 1)
			return 0;                             /* needs a +2 weapon */
	}
	if (melee)
		bonus += str_to_hit_bonus(ch);
	else
		bonus += dex_to_hit_bonus(ch);
	need = thac0_target(ch, MonsterTypes[type].ac) - bonus;
	roll = roll_dice(1, 20, 0);
	Monsters[mon].flags |= 1;                     /* the monster was attacked */
	if (PartyEffects & 0x8400L)                   /* bless + prayer */
		roll++;
	else if (Party[ch].effects & 0x1000L)
		roll++;
	roll = roll < 20 ? roll : 20;
	roll = roll > 1 ? roll : 1;
	return roll >= need;
}

/* 1F55:0192 [asm] - the damage of a hit: the weapon's dice (another set
 * against large creatures) + its magic bonus (+ strength in melee); bare
 * hands 1d2 */
int far hit_damage(int ch, Monster far *m, int item, int type, char melee)
{
	ItemType far *it;
	int dmg;

	if (melee && ch != -1)
		dmg = str_damage_bonus(ch);
	else
		dmg = 0;
	if (item) {
		it = &ItemTypes[type];
		if (m && (MonsterTypes[m->type].flags & 1L))
			dmg += roll_dice(it->damageLarge[0], it->damageLarge[1], it->damage[2]);
		else
			dmg += roll_dice(it->damage[0], it->damage[1], it->damage[2]);
		dmg += Items[item].value;
	} else
		dmg += roll_dice(1, 2, 0);
	if (dmg < 0)
		return 0;
	return dmg;
}

/* 1F55:0251 [asm] - a bow or a sling (kinds 2 and 3) */
int far is_missile_weapon(int type)
{
	int kind;

	kind = ItemTypes[type].flags & 0x7F;
	return kind == 2 || kind == 3;
}

/* 1F55:0279 [asm] - a missile reaches the party: the characters on its side
 * (a missile flying along the party's axis may go on to the back row) */
int far thrown_hits_party(Missile far *t)
{
	int item, ch, place, both, hit;
	int idx, i;

	item = t->item;
	place = Items[item].pos & 3;
	place = PlaceSeen[PartyDirection][place];
	hit = 0;
	both = (t->dir == PartyDirection || t->dir == (PartyDirection ^ 2)) && place > 2;
	idx = place << 1;
	if (place > 2)
		idx += roll_dice(1, 2, -1);
	for (i = 0; i < 2; i++) {
		ch = MissileTargets[idx];
		idx ^= 1;
		if (char_is_active(ch, 3)) {
			inflict_damage(ch, -1, item, 0, 0x110, 5, 3);
			hit = 1;
		}
		if (place < 2 || !both)
			break;
	}
	return hit;
}

/* 1F55:0368 [asm] - a missile reaches monster mon (thrown by a character:
 * it must hit; by a monster or a trap: it does) */
int far thrown_hits_monster(Missile far *t, int mon)
{
	int item, flags;
	int caster;

	item = t->item;
	caster = t->caster;
	if (caster != -1 && !char_hits_monster(caster, mon, item, 0))
		return 0;
	if (caster == -1)
		flags = 0x110;
	else
		flags = 0x910;
	damage_monster(&Monsters[mon], caster, item, 0, flags, 5, 3);
	return 1;
}

/* 1F55:03E0 [asm] - character ch (front row) attacks the square ahead with
 * "item". Returns the damage, or -1 missed, -2 nothing / not possible,
 * -3 back row, -5 a wall broken with bare hands or a blunt weapon (?) */
int far melee_attack(int ch, int item)
{
	int mon, r, flags, click;
	int i, block;

	if (ch > 1)
		return -3;
	block = block_ahead(PartyBlock, PartyDirection);
	mon = monster_to_attack(ch, block);
	if (mon == -1) {
		click = WallClickType[LevelMap[block].walls[Front]];
		if (click == -1 || click == 8 || click == 9) {
			if (click == -1) {                    /* a wall that breaks */
				for (i = 0; i < 4; i++)
					if (WallClickType[LevelMap[block].walls[i]] == -1)
						LevelMap[block].walls[i]++;
				SceneDirty = 1;
			}
			if ((ItemTypes[Items[item].type].classes & 4) || item == 0)
				return -5;
			return -2;
		}
		return -1;
	}
	if (Monsters[mon].flags & 0x20) {
		monster_dies(&Monsters[mon], 1);
		gui_printf("The monster crumbles to dust!\r");
		return -2;
	}
	if (char_hits_monster(ch, mon, item, 1)) {
		flags = 0x100;
		if (is_spell_item(item))
			flags |= 1;
		LastHitMonster = mon;
		r = hit_monster(&Monsters[mon], ch, item, 1, flags, 5, 3);
		return r;
	}
	return -1;
}

/* 1F55:0563 [asm] - after a throw, put a new missile in the hand: from the
 * quiver, or from the belt (slots 24 to 22) an item of the same type */
void far reload_hand(int ch, int slot, int type, int quiver)
{
	int item;
	int i;

	if (quiver && Party[ch].inventory[16]) {
		Party[ch].inventory[slot] = unlink_item(&Party[ch].inventory[16], 0, -1);
		return;
	}
	for (i = 24; i >= 22; i--) {
		item = Party[ch].inventory[i];
		/* ORIGINAL BUG (?): "!=" - an item of ANOTHER type is taken; harmless
		 * as long as the type is -1 (any) */
		if (item != 0 && (Items[item].type != type || type == -1)) {
			Party[ch].inventory[slot] = item;
			Party[ch].inventory[i] = 0;
			return;
		}
	}
}

/* 1F55:0628 [asm] - throw the item of a hand (kind 2: daggers, darts...) */
int far throw_item(int ch, int slot, int item)
{
	int row;

	row = ch;
	if (row > 3)
		row -= 2;
	if (launch_item(ch, item, PartyBlock, FrontPos[PartyDirection][row], PartyDirection, Items[item].type)) {
		play_sound(11);
		Party[ch].inventory[slot] = 0;
		reload_hand(ch, slot, -1, 0);
		SceneDirty = 1;
	}
	return 0;
}

/* 1F55:06AD [asm] - shoot with a bow or a sling: arrows come from a hand
 * then the quiver, stones from anywhere in the inventory. -4: no ammunition */
int far fire_missile(int ch, int item)
{
	int type, ammo, need, row;
	int i;

	type = Items[item].type;
	need = AmmoOf[type - 7];
	ammo = 0;
	if (need == 0x10) {                           /* arrows */
		ammo = Party[ch].inventory[0];
		if (ammo == 0 || Items[ammo].type != 0x10)
			ammo = 0;
		else
			Party[ch].inventory[0] = 0;
		if (ammo == 0) {
			ammo = Party[ch].inventory[1];
			if (ammo == 0 || Items[ammo].type != 0x10)
				ammo = 0;
			else
				Party[ch].inventory[1] = 0;
		}
		if (ammo == 0 && Party[ch].inventory[16])
			ammo = unlink_item(&Party[ch].inventory[16], 0, -1);
	} else {
		for (i = 0; i < 27; i++)
			if (Items[Party[ch].inventory[i]].type == need) {
				ammo = Party[ch].inventory[i];
				Party[ch].inventory[i] = 0;
				if (i < 2)
					draw_portrait(ch);
				break;
			}
	}
	if (ammo) {
		row = ch;
		if (row > 3)
			row -= 2;
		/* (written here as a flat index, unlike in throw_item) */
		if (launch_item(ch, ammo, PartyBlock, FrontPos[0][row + PartyDirection * 4], PartyDirection, type)) {
			play_sound(type == 7 ? 26 : 11);
			SceneDirty = 1;
		}
		return 0;
	}
	return -4;
}

/* 1F55:0869 [asm] - a character attacks with the item of a hand (slot 0/1).
 * The hand shows the result, then is busy for 18 ticks (36 more after a hit,
 * see timer_character). Attacking ends Invisibility. */
void far attack_with(int ch, int slot, int item)
{
	int r;
	int type;

	if (item)
		type = Items[item].type;
	else
		type = 0;
	r = 0;
	if (Party[ch].effects & 0x40L)
		end_spell_effect(10, ch, 1);
	switch (ItemTypes[type].flags & 0x7F) {
	case 1:
		r = melee_attack(ch, item);
		if (r == 0)
			r = -1;
		play_sound(32);
		break;
	case 2:
		r = throw_item(ch, slot, item);
		break;
	case 3:
		r = fire_missile(ch, item);
		draw_portrait(ch);
		break;
	}
	if (r > 0) {
		if (Items[item].flags & 8) {              /* drains life */
			Party[ch].hp += r;
			draw_portrait(ch);
		}
		if (Items[item].flags & 0x10)             /* used up by the hit */
			Party[ch].inventory[slot] = 0;
		monster_hit(&Monsters[LastHitMonster], r, 1);
	}
	Party[ch].busy ^= 1 << slot;
	Party[ch].handResult[slot] = r;
	draw_portrait(ch);
	if (r < -2)
		add_char_timer(ch, 18L, slot, 1);
	else
		add_char_timer(ch, 18L, slot + 2, 1);
}

/* 1F55:09FE [asm] - is somebody still able to act? If not and "ask": the
 * game is over - load a saved game or quit. Returns 1 when all are dead. */
int far check_party_alive(int ask)
{
	int i, n;

	for (n = i = 0; i < 6; i++)
		n += char_is_active(i, 13);
	if (n == 0 && ask) {
		redraw_party();
		if (GlobalFlags & 0x100000L)
			kheldran_laughs();
		if (!ask_yes_no(14, 67, 1)) {
			system_shutdown();
			exit(0);
		}
		set_font(Font8);
		close_panels();
		if (!load_game(0, 0)) {
			system_shutdown();
			exit(0);
		}
		set_font(GameFont);
		n = 1;
	}
	return !n;
}

/* 1F55:0AC5 [asm] - heal (or hurt) a character, at most up to his maximum */
void far modify_hp(int ch, int amount)
{
	Character far *c;

	if (char_is_active(ch, 3)) {
		c = &Party[ch];
		c->hp += amount;
		if (c->hp > c->hpMax)
			c->hp = c->hpMax;
		draw_hp(ch);
		draw_portrait(ch);
	}
}

/* 1F55:0B21 [asm] - experience for one character: shared between his
 * classes; a class goes up a level when its experience is above the next
 * level's */
void far level_up(int ch, int k);

void far give_xp(int ch, long xp)
{
	int k, cls;
	long need;

	cls = Party[ch].cls;
	xp /= ClassCount[cls];
	for (k = 0; k < 3; k++) {
		if (class_part(cls, k) == -1)
			continue;
		Party[ch].experience[k] += xp;
		need = xp_for_level(cls, k, Party[ch].level[k] + 1);
		if (need == -1)
			continue;
		if (Party[ch].experience[k] > need)
			level_up(ch, k);
	}
}

/* 1F55:0C0F [asm] - experience for the party, shared between the living */
void far give_party_xp(int xp)
{
	int i, n;

	n = 0;
	for (i = 0; i < 6; i++)
		if (char_is_active(i, 3))
			n++;
	if (n > 0) {
		xp /= n;
		for (i = 0; i < 6; i++)
			if (char_is_active(i, 3))
				give_xp(i, xp);
	}
}

/* 1F55:0C6C [asm] */
void far level_up(int ch, int k)
{
	int hp;

	Party[ch].level[k]++;
	hp = roll_level_hp(ch, 1 << k);
	Party[ch].hp += hp;
	Party[ch].hpMax += hp;
	sprintf(Scratch, "\x06\x01%s has gained a level.\x06\x0f\r", Party[ch].name);
	draw_portrait(ch);
	gui_printf(Scratch);
	play_sound(23);
}

/* 1F55:0CF7 [asm] - the armour class of a character: 10 + dexterity, the
 * armour pieces his class may wear, the best ring of protection (unless the
 * armour is magical), then the spells: Armor (6), Shield (4), Magical
 * Vestment (5, better with the level); never below -10 */
void far recompute_ac(int ch)
{
	int type, max, a, b, usable, dexac;
	Character far *c;
	int i, item;

	c = &Party[ch];
	dexac = dex_ac_bonus(c->dex);
	c->ac = dexac + 10;
	for (i = 0; i < 4; i++) {
		item = c->inventory[ArmorSlots[i]];
		if (!item)
			continue;
		if (i == 2 && !hand_usable(ch, 1))
			continue;                             /* a shield needs the second hand */
		type = Items[item].type;
		usable = (char)(ItemTypes[type].classes & ClassFlags[c->cls]);
		if (!(ItemTypes[type].flags & 0x7F) && usable) {
			if (i < 1 || i > 2 || type == 0x1B || type == 0x39) {
				c->ac += ItemTypes[type].ac;
				c->ac -= Items[item].value;
			}
		}
	}
	item = c->inventory[17];
	if (Items[item].value == 0) {                 /* no magical armour: the rings of protection */
		a = 0;
		b = 0;
		item = c->inventory[25];
		if (item && !(ItemTypes[Items[item].type].flags & 0x7F))
			a = Items[item].value;
		item = c->inventory[26];
		if (item && !(ItemTypes[Items[item].type].flags & 0x7F))
			b = Items[item].value;
		a = a > b ? a : b;
		c->ac = c->ac - a;
	}
	if (c->armorPool > 0) {
		if (c->ac > dexac + 6)
			c->ac = dexac + 6;
		else
			c->armorPool = 0;
	}
	if ((c->effects & 8L) && c->ac > 4)
		c->ac = 4;
	if (c->effects & 0x4000L) {
		max = 5;
		if (cleric_level(ch) > 5)
			max += (cleric_level(ch) - 5) / 3;
		if (c->ac > max)
			c->ac = max;
	}
	if (c->ac < -10)
		c->ac = -10;
}

/* 1F55:0F50 [asm] - half damage on a successful saving throw (type 4) */
int far save_half(int ch, int dmg)
{
	int need;

	need = char_saving_throw(ch, 4);
	if (roll_dice(1, 20, 0) >= need)
		dmg >>= 1;
	return dmg;
}

/* 1F55:0F8A [asm] - a saving throw; dwarves and halflings (races 3, 5) and
 * gnomes (4) get a bonus from their constitution against some types */
int far saving_throw(Character far *c, int a, int level, int type, int race)
{
	int need;

	if (type == 5)
		return 0;
	need = save_table(a, level, type);
	switch (race) {
	case 3:
	case 5:
		if (type == 1 || type == 0 || type == 4)
			need -= ConSaveBonus[c->con];
		break;
	case 4:
		if (type == 4 || type == 1)
			need -= ConSaveBonus[c->con];
	}
	return roll_dice(1, 20, 0) >= need;
}

/* 1F55:1024 [asm] - damage after a successful save: types 0 and 1 half, 3 none */
int far saved_damage(int type, int dmg)
{
	switch (type) {
	case 0:
		dmg >>= 1;
		break;
	case 1:
		dmg >>= 1;
		break;
	case 3:
		dmg = 0;
	}
	return dmg;
}

/* 1F55:104D [asm] - a monster loses hit points (it stops fleeing, it has
 * been hit); a monster type with flag 0x2000 dies at once */
void far monster_hit(Monster far *m, int dmg, int a)
{
	m->hp -= dmg;
	m->flags &= ~8;
	m->flags |= 1;
	if (MonsterTypes[m->type].flags & 0x2000L) {
		monster_explodes(m);
		block_changed(m->block);
		m->hp = 0;
	} else if (block_changed(m->block)) {
		m->flags |= 2;
		if (Var_E5A2)
			return;
		monster_flash(m);
	}
	if (m->hp <= 0)
		monster_dies(m, a);
	else if (distance(m->block, PartyBlock) < 4)
		m->dest = PartyBlock;
}

/* 1F55:1112 [asm] - the damage done to a monster. flags 0x100: by a
 * character's weapon (ch, item, a = melee), else dice (ch, item, a = count,
 * sides, add). A save (type "save", 5: none) reduces it (saved_damage);
 * flags 0x10: thrown (a thrown thing that is not a missile weapon does 1);
 * flags 1: a spell (magic resistance). Then the immunities of the monster. */
int far hit_monster(Monster far *m, int ch, int item, int a, int flags, int save, int saveType)
{
	int who, it, how;
	MonsterType far *mt;
	int dmg, i;

	if (flags & 0x100) {
		who = ch;
		it = item;
		how = a;
		dmg = hit_damage(who, m, it, Items[it].type, how);
	} else
		dmg = roll_dice(ch, item, a);
	mt = &MonsterTypes[m->type];
	if (save != 5 && saving_throw((Character far *)m, 0, mt->level, save, 6))
		dmg = saved_damage(saveType, dmg);
	if ((flags & 0x110) == 0x110 && !is_missile_weapon(Items[it].type))
		dmg = 1;
	/* ORIGINAL BUG: ItemTypes is indexed by the item number, not by its type
	 * (Items[it].type): the "blunt weapon" bit (classes & 4) of a random item
	 * type decides whether a monster immune to 0x100 takes half damage */
	if ((flags & 0x100) && (mt->immunities & 0x100) && (ItemTypes[it].classes & 4) == 0)
		dmg >>= 1;
	if ((flags & 0x100) && (mt->immunities & 0x2000)) {
		if (Items[it].value < 3)               /* needs a +3 weapon */
			dmg >>= 2;
		if (Items[it].value == 3)
			dmg >>= 1;
		if (dmg == 0)
			dmg = Items[it].value;
	} else if (mt->immunities & 0x2000)
		dmg >>= 1;
	if ((flags & 1) && monster_resists(m))
		dmg = 0;
	for (i = 0; i < 12; i += 2)
		if ((Immunities[i] & flags) && (mt->immunities & Immunities[i + 1]))
			dmg = 0;
	return dmg;
}

/* 1F55:12B8 [asm] - hit_monster, then the monster loses the hit points
 * (flags 0x800 passed on to monster_hit) */
void far damage_monster(Monster far *m, int from, int item, int a, int flags, int save, int saveType)
{
	int lethal;
	int dmg;

	dmg = hit_monster(m, from, item, a, flags, save, saveType);
	lethal = flags & 0x800;
	if (dmg > 0)
		monster_hit(m, dmg, lethal);
}

/* 1F55:1304 [asm] - the damage done to a character: as hit_monster (the
 * attacker's weapon with flags 0x100, else dice); flags 4: a fall (a ring
 * of feather fall, 3, protects), 0x400: poison (Slow Poison protects) */
int far hit_character(Character far *c, int ch, int item, int a, int flags, int save, int saveType)
{
	int who, it, how;
	int dmg;

	if (flags & 0x100) {
		who = ch;
		it = item;
		how = a;
		dmg = hit_damage(who, 0, it, Items[it].type, how);
	} else
		dmg = roll_dice(ch, item, a);
	/* ORIGINAL BUG: the race is passed as raceSex (race * 2 + sex): the
	 * constitution bonus of dwarves, gnomes and halflings (3, 4, 5) goes to
	 * female elves (3) and to half-elves (4, 5) instead */
	if (save != 5 && saving_throw(c, ClassThac0Group[c->cls], c->level[0], save, c->raceSex))
		dmg = saved_damage(saveType, dmg);
	if ((flags & 0x110) == 0x110 && !is_missile_weapon(Items[it].type))
		dmg = 1;
	if ((flags & 4) && has_ring(c, 3))
		dmg = 0;
	if ((flags & 0x400) && (c->effects & 0x2000L))
		dmg = 0;
	else if (flags & 0x400) {
		sprintf(Scratch, "\x06\x06%s feels the effects of poison!\r", c->name);
		gui_printf(Scratch);
	}
	return dmg;
}

/* 1F55:143E [asm] - a character takes the damage of an attack */
void far inflict_damage(int ch, int from, int item, int a, int flags, int save, int saveType)
{
	int dmg;

	dmg = hit_character(&Party[ch], from, item, a, flags, save, saveType);
	if (dmg)
		damage_character(ch, dmg);
}

/* 1F55:147F [asm] */
void far character_dies(int ch, Character far *c)
{
	c->hp = -10;
	c->flags &= 1;
	c->food = 0;
	end_all_effects(ch);
	play_sound(22);
}

/* 1F55:14AC [asm] - a character loses hit points: dead at -10; the Armor
 * spell absorbs them until its pool is empty; the spell book of a character
 * who falls is closed */
void far damage_character(int ch, int dmg)
{
	Character far *c;

	c = &Party[ch];
	if (char_is_active(ch, 3)) {
		if (c->tempHp) {
			c->tempHp -= dmg;
			if (c->tempHp < 0)
				c->tempHp = 0;
		}
		c->hp -= dmg;
		c->lastDamage = dmg;
		if (c->hp <= -10)
			character_dies(ch, c);
		else
			play_sound(21);
		if (c->armorPool > 0) {
			c->armorPool -= dmg;
			if (c->armorPool <= 0) {
				c->armorPool = 0;
				end_spell_effect(1, ch, 1);
			}
		}
		if (Var_EE3A == 0)
			draw_portrait(ch);
		else {
			hide_mouse();
			draw_face(ch);
			show_mouse();
		}
		if (c->hp <= 0 && BookOpen == 1 && BookCaster == ch)
			close_spell_book(GuiState);
		add_char_timer(ch, 18L, 6, 1);
	}
}

/* 1F55:15D6 [asm] - a character gets a status (flags bit: 2 poisoned,
 * 4 paralyzed, 8 petrified) unless he saves (type "save", 5: no save);
 * "once": not again if he has it. With a duration (in half minutes) a
 * timer "timerWhat" ends it. Prints "<name> is <what>!" */
void far set_char_status(int ch, int bit, char far *what, int save, long dur, int timerWhat, int once)
{
	if ((Party[ch].flags & bit) && once)
		return;
	if (!char_is_active(ch, 3))
		return;
	if (save != 5 && char_saving_throw(ch, save))
		return;
	if (bit & 8) {                                /* petrified: the spells end */
		end_all_effects(ch);
		Party[ch].flags &= 1;
		Party[ch].flags |= 8;
	} else
		Party[ch].flags |= bit;
	if ((bit & 0xC) && BookOpen && BookCaster == ch)
		close_spell_book(GuiState);
	if (dur)
		add_char_timer(ch, dur * 546, timerWhat, 1);
	draw_portrait(ch);
	sprintf(Scratch, "\x06\x06%s is %s!\r", Party[ch].name, what);
	gui_printf(Scratch);
}

/* 1F55:16FB [asm] - a spell effect on a monster, unless it saves (-> 0) or,
 * for a magical one, resists (-> -1): 0 hold person, 1 hold monster, 2 hold
 * (any), 3 death (not 8-immune), 4 death, 5 removed, 6 fear (monster_flee) */
int far spell_on_monster(Monster far *m, int effect, int magic, int save)
{
	MonsterType far *mt;
	int r;

	mt = &MonsterTypes[m->type];
	if (magic == 1 && (monster_resists(m) || (mt->immunities & 0x10)))
		return -1;
	if (saving_throw((Character far *)m, 0, mt->level, save, 6))
		return 0;
	switch (effect) {
	case 0:
	case 1:
	case 2:
		if (effect == 1)
			r = (mt->kinds & 2L) ? 1 : 0;
		else if (effect == 0)
			r = (mt->kinds & 1L) ? 1 : 0;
		else
			r = 1;
		if (r && !(mt->immunities & 2)) {
			m->mode = 10;                     /* held */
			m->wait = 15;
		}
		break;
	case 3:
		if (!(mt->immunities & 8))
			monster_hit(m, 1000, 1);
		break;
	case 4:
		monster_hit(m, 1000, 1);
		break;
	case 5:
		m->flags |= 0x20;
		SceneDirty = 1;
		break;
	case 6:
		if (!(mt->immunities & 4))
			monster_flee(m);
		break;
	}
	return 1;
}

/* 1F55:1854 [asm] - a monster that dies at once when hit (MonsterType flag
 * 0x2000, the gas spore ?) explodes: 6d6 to the whole party when it is near */
void far monster_explodes(Monster far *m)
{
	int i;

	m->flags |= 2;
	if (distance(m->block, PartyBlock) < 2) {
		view_burst(PartyBlock, 2, (Missile far *)0);
		for (i = 0; i < 6; i++)
			inflict_damage(i, 6, 6, 0, 8, 1, 0);
	} else
		view_burst(m->block, 2, (Missile far *)0);
	m->flags &= ~2;
}
