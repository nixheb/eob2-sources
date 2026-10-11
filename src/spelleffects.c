/*
 * Eye of the Beholder II - START.EXE
 * The spell effects: the start, hit and end functions of the spell table
 * (Spells[], 39A6:0000): module 13A7 (Ghidra 23A7).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y spelleffects.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Item           Items[600];        /* 71CA */
extern Monster        Monsters[30];      /* 6E2A */
extern Character      Party[6];          /* E608 */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern char           BookCaster;        /* D0B0 */
extern signed char    SpellTarget;       /* CBC2 */
extern Character far *TargetChar;        /* E5A8 */
extern char           Var_E440;          /* E440 the place the caster aims from */
extern char           Var_DF38;          /* DF38 set when the spell fails */
extern int            Var_E5A2;          /* E5A2 */
extern char           SceneDirty;        /* D0AA */
extern char           Scratch[];         /* F16E */
extern byte far      *FlameShapes[];     /* EE24 */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern char           WallTypeFlags[256];/* 2268 */
extern char           Front;             /* 9444 */
extern ForceWall      ForceWalls[5];     /* 9346 the walls of force */
extern unsigned long  GameTicks;         /* 6684 */
extern char far      *ViewWalls[];       /* DEDC [13]: the block ahead */
extern signed char    WallSet[80];       /* 2178 per wall type; [46] the illusory wall, 0 under True Seeing */
extern char           CurrentSpell;      /* DF2A */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern unsigned long  PartyEffects;      /* 6DDC */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void end_all_effects(int ch);                                    /* 20E2:2AFB */
int  caster_level(int ch);                                       /* 20E2:0390 */
int  dex_ac_bonus(char dex);                                     /* 1B54:01BD */
int  sprintf(char far *buf, char far *fmt, ...);
void message(char far *text);                                    /* 18D7:0BE6 */
void draw_shape_mirrored(int page, byte far *shape, int x, int y, int flags);   /* 18D7:06EE */
void draw_shape(int page, byte far *shape, int x, int y, int flags);            /* 18D7:06AD */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
void spell_hits_block(int block, char from, char item, char a, char flags, char save, char saveType);  /* 1997:19A9 */
int  get_hand_item(void);                                        /* 1E24:0352 */
void take_item(int item);                                    /* 18AE:0197 */
void mouse_move_by(int a, int b);                                 /* 33C1:00A0 */
void launch_spell(char from, int spell, int block, int pos, int dir);   /* 20E2:084A */
int  spell_hit_damage(Missile far *t, int count, int sides, int add, int mult);  /* 20E2:0915 */
int  second_hand_free(Character far *c);                         /* 20E2:0379 */
int  new_spell_item_type(byte slots, byte b, char ac, char classes, char d0, char d1, char d2, int flags);  /* 20E2:023D */
int  new_spell_item(int flags, int icon, int value, int type);   /* 20E2:0305 */
void end_spell_effect(int spell, int ch, int verbose);           /* 20E2:0C6D */
void remove_char_timer(int ch, int what);                        /* 27B7:0D76 */
int  is_spell_item(unsigned item);                               /* 20E2:0044 */
void free_spell_item(int item);                                  /* 20E2:02C5 */
void monsters_targeted(int far *out, int block, int pos, char dir, char all, char nearest);  /* 1997:1AD2 */
int  spell_on_monster(Monster far *m, int effect, int magic, int save);   /* 1F55:16FB */
void set_char_status(int ch, int bit, char far *what, int save, long dur, int timerWhat, int once);  /* 1F55:15D6 */
void target_special(void);                                       /* 20E2:05EA */
void mouse_hide_rect(int x1, int y1, int x2, int y2);               /* 338B:0206 */
void mouse_show_rect(void);                                         /* 338B:02CA */
void streaks(int count, int spread, int step, int unused, int speed, int far *colours);  /* 20E2:2596 */
void damage_monster(Monster far *m, int from, int item, int a, int flags, int save, int saveType);  /* 1F55:12B8 */
void monsters_hit_update(void);                                  /* 1997:10D2 */
void far end_touch_spell(Character far *c);
int  monster_to_attack(char ch, int block);                      /* 1997:1929 */
void setmem(void far *p, unsigned n, char value);
void message(char far *text);
void no_effect(void);                                            /* 20E2:0032 */
void monster_hit(Monster far *m, int dmg, int a);                /* 1F55:104D */
void touch_attack(int from, int item, int a);                    /* 20E2:0DDB */
void modify_hp(int ch, int amount);                              /* 1F55:0AC5 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void start_timed_spell(int spell, int base, long unit, int perLevel, int mode);   /* 20E2:00A1 */
int  cleric_level(int ch);                                       /* 20E2:03F9 */
void neutralize_poison(int ch);                                  /* 20E2:005F */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait */
int  turn_monster(Monster far *m, int roll, int level);          /* 1E24:09DF */
int  random_char(void);                                          /* 20E2:2BC7 */
int  next_char(int ch);                                          /* 20E2:2C16 */
int  save_half(int ch, int dmg);                                 /* 1F55:0F50 */
void damage_character(int ch, int dmg);                          /* 1F55:14AC */
void redraw_party(void);                                         /* 1E24:0686 */

/* ------------------------------------------------------------------ */
/* Module 23A7, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5890...) */
/* the flames of Burning Hands: shape (< 0 mirrored), x, y               5890 */
signed char FlameShape[6] = { 3, -3, 2, -2, 1, -1 };
byte FlameX[6] = { 0, 152, 24, 120, 56, 88 };
signed char FlameY[6] = { 64, 64, 56, 56, 56, 56 };
/* Ice Storm also hits the 4 blocks around (block offsets)              58A2 */
signed char IceStormAround[4] = { -32, 32, 1, -1 };
/* the colours of the Cone of Cold streaks                              58A6 */
int ConeColours[8] = { 15, 9, 15, 9, 2, 10, 2, 0 };
/* the 7 blocks of the Cone of Cold, per party direction (offsets)      58B6 */
signed char ConeNorth[7] = { -32, -64, -63, -65, -96, -97, -95 };
signed char ConeEast[7] = { 1, 2, -30, 34, 3, -29, 35 };
signed char ConeSouth[7] = { 32, 64, 63, 65, 96, 95, 97 };
signed char ConeWest[7] = { -1, -2, 30, -34, -3, 29, -35 };
signed char far *ConeBlocks[4] = { ConeNorth, ConeEast, ConeSouth, ConeWest };   /* 58D2 */

/* 23A7:0002 [asm] - all the spell effects of the party end */
void far spell_dispel_magic(void)
{
	register int i;

	for (i = 0; i < 6; i++)
		if (char_is_active(i, 1))
			end_all_effects(i);
}

/* 23A7:002C [asm] - the items in the target's hands are identified */
void far spell_improved_identify(void)
{
	register int i, item;

	for (i = 0; i < 2; i++) {
		item = TargetChar->inventory[i];
		if (item)
			Items[item].flags |= 0x40;
	}
}

/* 23A7:0060 [asm] - Armor: AC 6, absorbs 8 + level points */
void far spell_armor(void)
{
	TargetChar->armorPool = caster_level(BookCaster) + 8;
	if (dex_ac_bonus(TargetChar->dex) + 6 >= TargetChar->ac) {
		sprintf(Scratch, "%s already has an armor class better than AC 6.", TargetChar->name);
		message(Scratch);
	}
}

/* 23A7:00C3 [asm] - flames, then 1d3 + 2 per level on the block ahead */
void far spell_burning_hands(void)
{
	int block, level;
	register int i;

	for (i = 0; i < 6; i++)
		if (FlameShape[i] < 0)
			draw_shape_mirrored(0, FlameShapes[(FlameShape[i] < 0 ? -FlameShape[i] : FlameShape[i]) - 1], FlameX[i], FlameY[i], 0);
		else
			draw_shape(0, FlameShapes[FlameShape[i] - 1], FlameX[i], FlameY[i], 0);
	delay_ticks(2);
	level = caster_level(BookCaster);
	block = block_ahead(PartyBlock, PartyDirection);
	spell_hits_block(block, 1, 3, level << 1, 0x21, 4, 0);
	SceneDirty = 1;
}

/* 23A7:019D [asm] - the item in hand is shown again (its glow) */
void far spell_detect_magic(void)
{
	take_item(get_hand_item());
	mouse_move_by(0, 0);
}

/* 23A7:01BB [asm] */
void far end_detect_magic(void)
{
	take_item(get_hand_item());
	mouse_move_by(0, 0);
}

/* 23A7:01D9 [asm] */
void far spell_magic_missile(void)
{
	launch_spell(BookCaster, 0, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:01FA [asm] - 1d4+1, one more missile every 2 levels */
int far hit_magic_missile(Missile far *t)
{
	return spell_hit_damage(t, 1, 4, 1, (caster_level(t->caster) - 1) >> 1);
}

/* 23A7:022D [asm] - the hand holds a charge: 1d8 + level */
void far spell_shocking_grasp(void)
{
	int hand;
	register int type, item;

	hand = second_hand_free(TargetChar);
	type = new_spell_item_type(0, 0, 0, 0x0F, 1, 8, caster_level(BookCaster), 1);
	if (type != -1)
		item = new_spell_item(0x10, 0x52, 0, type);
	if (type != -1 && item != -1)
		TargetChar->inventory[hand] = item;
	else {
		message("The spell fails!\r");
		end_spell_effect(6, SpellTarget, 0);
		remove_char_timer(SpellTarget, -6);
		Var_DF38 = 1;
	}
}

/* 23A7:02E4 [asm] - the charge leaves the hand */
void far end_touch_spell(Character far *c)
{
	register int i, item;

	for (i = 0; i < 2; i++) {
		item = c->inventory[i];
		if (is_spell_item(item)) {
			free_spell_item(item);
			c->inventory[i] = 0;
			return;
		}
	}
}

/* 23A7:032B [asm] */
void far spell_acid_arrow(void)
{
	launch_spell(BookCaster, 1, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:034D [asm] - 2d4, times level / 3 */
int far hit_acid_arrow(Missile far *t)
{
	return spell_hit_damage(t, 2, 4, 0, caster_level(t->caster) / 3);
}

/* 23A7:0382 [asm] */
void far spell_fireball(void)
{
	launch_spell(BookCaster, 2, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:03A4 [asm] - 1d6 per level */
int far hit_fireball(Missile far *t)
{
	return spell_hit_damage(t, 1, 6, 0, caster_level(t->caster));
}

/* 23A7:03D3 [asm] */
void far spell_hold_person(void)
{
	launch_spell(BookCaster, 3, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:03F5 [asm] - Hold Person hits the monsters of the block, or (cast
 * by a monster) 1d4 characters from a random one */
int far hit_hold_person(Missile far *t)
{
	int list[5];
	int far *p;
	int held, k;
	register int ch, n;

	if (t->block != PartyBlock) {
		held = 0;
		p = list;
		monsters_targeted(p, t->block, t->pos, t->dir, 1, 1);
		for (; *p != -1; p++)
			held |= spell_on_monster(&Monsters[*p], 0, 1, 4);
	} else {
		n = roll_dice(1, 4, 0);
		ch = roll_dice(1, 6, 0);
		for (k = 0; k < 6; k++) {
			if (char_is_active(ch, 3)) {
				set_char_status(ch, 4, "held", 4, 5L, 9, 1);
				n--;
			}
			if (n == 0)
				break;
			if (++ch >= 6)
				ch = 0;
		}
		held = 1;
	}
	return held;
}

/* 23A7:04FA [asm] */
void far spell_lightning_bolt(void)
{
	launch_spell(BookCaster, 4, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:051C [asm] - 1d6 per level */
int far hit_lightning_bolt(Missile far *t)
{
	return spell_hit_damage(t, 1, 6, 0, caster_level(t->caster));
}

/* 23A7:054B [asm] - the hand holds a charge: (level / 2) d6, the caster
 * gains the hit points */
void far spell_vampiric_touch(void)
{
	int hand;
	register int type, item;

	hand = second_hand_free(TargetChar);
	type = new_spell_item_type(0, 0, 0, 0x0F, caster_level(BookCaster) >> 1, 6, 0, 1);
	if (type != -1)
		item = new_spell_item(0x18, 0x53, 0, type);
	if (type != -1 && item != -1)
		TargetChar->inventory[hand] = item;
	else {
		message("The spell fails!\r");
		end_spell_effect(18, SpellTarget, 0);
		remove_char_timer(SpellTarget, -18);
		Var_DF38 = 1;
	}
}

/* 23A7:0604 [asm] - the hit points gained above the maximum are lost */
void far end_vampiric_touch(Character far *c)
{
	if (c->hp > c->hpMax)
		c->hp = c->hpMax;
	end_touch_spell(c);
}

/* 23A7:062B [asm] - the monsters of the block ahead flee */
void far spell_fear(void)
{
	Monster far *m;
	register int i, block;

	target_special();
	block = block_ahead(PartyBlock, PartyDirection);
	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->block == block)
			spell_on_monster(m, 6, 1, 4);
}

/* 23A7:0689 [asm] */
void far spell_ice_storm(void)
{
	launch_spell(BookCaster, 5, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:06AB [asm] - 1d6 per level on the block and the 4 around */
int far hit_ice_storm(Missile far *t)
{
	int r;
	register int i, saved;

	r = spell_hit_damage(t, 1, 6, 0, caster_level(t->caster));
	if (r)
		for (i = 0; i < 4; i++) {
			saved = t->block;
			t->block += IceStormAround[i];
			t->block &= 0x3FF;
			spell_hit_damage(t, 1, 6, 0, caster_level(t->caster));
			t->block = saved;
		}
	return r;
}

/* 23A7:073C [asm] - the cursed items of the target (but the quiver) are
 * no longer cursed, and identified */
void far spell_remove_curse(void)
{
	register int item, i;

	for (i = 0; i < 27; i++)
		if (i != 16) {
			item = TargetChar->inventory[i];
			if (item && (Items[item].flags & 0x20) && !is_spell_item(item)) {
				Items[item].flags &= ~0x20;
				Items[item].flags |= 0x40;
			}
		}
}

/* 23A7:079F [asm] - the cone: 7 blocks ahead, level d4 + level */
void far spell_cone_of_cold(void)
{
	int list[5];
	int far *q;
	int block;
	signed char far *p;
	register int level, k;

	level = caster_level(BookCaster);
	mouse_hide_rect(0, 0, 0xB0, 0x78);
	streaks(0x96, 0x32, 0x0A, 1, 0x64, ConeColours);
	mouse_show_rect();
	p = ConeBlocks[PartyDirection];
	Var_E5A2 = 1;
	for (k = 0; k < 7; k++, p++) {
		block = (PartyBlock + *p) & 0x3FF;
		monsters_targeted(list, block, 4, PartyDirection, 1, 1);
		for (q = list; *q != -1; q++)
			damage_monster(&Monsters[*q], level, 4, level, 0x41, 4, 0);
	}
	monsters_hit_update();
}

/* 23A7:0894 [asm] */
void far spell_hold_monster(void)
{
	launch_spell(BookCaster, 6, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:08B6 [asm] */
int far hit_hold_monster(Missile far *t)
{
	int list[5];
	int far *p;
	register int held;

	held = 0;
	p = list;
	monsters_targeted(p, t->block, t->pos, t->dir, 1, 1);
	for (; *p != -1; p++)
		held |= spell_on_monster(&Monsters[*p], 1, 1, 4);
	return held;
}

/* 23A7:0925 [asm] - the first monster in line ahead, up to "range" blocks,
 * stopped by a wall; -1 none */
int far monster_in_line(register int range)
{
	int mon;
	register int block;

	block = PartyBlock;
	mon = -1;
	while (mon == -1 && range != 0) {
		block = block_ahead(block, PartyDirection);
		mon = monster_to_attack(BookCaster, block);
		range--;
		if ((WallTypeFlags[LevelMap[block].walls[Front]] & 1) == 0) {
			range = 0;
			mon = -1;
		}
	}
	return mon;
}

/* 23A7:098C [asm] - a wall of force disappears */
void far remove_force_wall(register int k)
{
	setmem(LevelMap[ForceWalls[k].block].walls, 4, 0);
	ForceWalls[k].block = 0;
	SceneDirty = 1;
}

/* 23A7:09D0 [asm] - a wall of force on the empty block ahead (5 at most:
 * the oldest goes), for (level / 2 + 1) half-minutes */
void far spell_wall_of_force(void)
{
	int slot;
	unsigned long oldest;
	register int block, i;

	block = block_ahead(PartyBlock, PartyDirection);
	if (LevelMap[block].walls[0] || LevelMap[block].walls[1] || LevelMap[block].walls[2] ||
	    LevelMap[block].walls[3] || (LevelMap[block].flags & 7)) {
		message("The spell fails\r");
		return;
	}
	oldest = -1L;
	slot = 0;
	for (i = 0; i < 5; i++) {
		if (ForceWalls[i].block == 0)
			break;
		if (ForceWalls[i].expires < oldest) {
			oldest = ForceWalls[i].expires;
			slot = i;
		}
	}
	if (i == 5) {
		remove_force_wall(slot);
		i = slot;
	}
	setmem(LevelMap[block].walls, 4, 0x4A);
	ForceWalls[i].block = block;
	ForceWalls[i].expires = GameTicks + (caster_level(BookCaster) * 546 >> 1) + 546;
	SceneDirty = 1;
}

/* 23A7:0B25 [asm] - the monster ahead dies (save), a wall of force ahead goes */
void far spell_disintegrate(void)
{
	register int mon;

	mon = monster_in_line(1);
	if (mon != -1)
		spell_on_monster(&Monsters[mon], 4, 1, 4);
	if (ViewWalls[13][Front] == 0x4A) {
		setmem(ViewWalls[13], 4, 0);
		SceneDirty = 1;
	}
}

/* 23A7:0B89 [asm] */
void far spell_flesh_to_stone(void)
{
	int block;
	register int mon;

	target_special();
	block = block_ahead(PartyBlock, PartyDirection);
	mon = monster_to_attack(BookCaster, block);
	if (mon != -1)
		spell_on_monster(&Monsters[mon], 5, 1, 4);
	else
		message("No monster within reach!\r");
}

/* 23A7:0BF1 [asm] */
void far spell_stone_to_flesh(void)
{
	if (TargetChar->flags & 8)
		TargetChar->flags &= ~8;
	else
		no_effect();
}

/* 23A7:0C0D [asm] - the illusory walls are no longer drawn */
void far spell_true_seeing(void)
{
	WallSet[46] = 0;
	SceneDirty = 1;
}

/* 23A7:0C1C [asm] */
void far end_true_seeing(void)
{
	WallSet[46] = 1;
	SceneDirty = 1;
}

/* 23A7:0C2B [asm] - Finger of Death / Slay Living: death, or 2d8+1 if saved */
void far spell_finger_of_death(void)
{
	Monster far *m;
	register int mon;

	mon = monster_in_line(2);
	if (mon != -1) {
		m = &Monsters[mon];
		if (!spell_on_monster(m, 3, 1, 4))
			monster_hit(m, roll_dice(2, 8, 1), 1);
	}
}

/* 23A7:0C98 [asm] - a monster with less than 90 hit points is held, no save */
void far spell_power_word_stun(void)
{
	register int mon;

	mon = monster_in_line(2);
	if (mon != -1 && Monsters[mon].hp < 90)
		spell_on_monster(&Monsters[mon], 2, 1, 5);
}

/* 23A7:0CDE [asm] - 1d8 by touch */
void far spell_cause_light_wounds(void)
{
	touch_attack(1, 8, 0);
}

/* 23A7:0CF6 [asm] */
void far spell_cure_light_wounds(void)
{
	modify_hp(SpellTarget, roll_dice(1, 8, 0));
}

/* 23A7:0D1A [asm] - Aid: 1d8 more hit points (lost first), +1 to hit */
void far spell_aid(void)
{
	int hp;

	if (char_is_active(SpellTarget, 3)) {
		if (TargetChar->tempHp > 0) {
			gui_printf("%s is already under the effect of an aid spell.\r", TargetChar->name);
			play_sound(0x4F);
		} else {
			hp = roll_dice(1, 8, 0);
			TargetChar->tempHp = hp;
			TargetChar->hp += hp;
			TargetChar->effects |= 0x1000L;
			return;
		}
	} else
		no_effect();
	end_spell_effect(39, SpellTarget, 0);
	remove_char_timer(SpellTarget, -39);
}

/* 23A7:0DC9 [asm] */
void far end_aid(Character far *c)
{
	c->hp -= c->tempHp;
	c->tempHp = 0;
	c->effects &= ~0x1000L;
}

/* 23A7:0DEF [asm] - the hand holds a flaming blade: 1d4+4 */
void far spell_flame_blade(void)
{
	int hand;
	register int type, item;

	hand = second_hand_free(TargetChar);
	type = new_spell_item_type(0, 0, 0, 0x0F, 1, 4, 4, 1);
	if (type != -1)
		item = new_spell_item(0, 0x54, 0, type);
	if (type != -1 && item != -1)
		TargetChar->inventory[hand] = item;
	else {
		message("The spell fails!\r");
		/* ORIGINAL BUG: copied from Shocking Grasp: spell 6 is ended, not
		 * Flame Blade (40) */
		end_spell_effect(6, SpellTarget, 0);
		remove_char_timer(SpellTarget, -6);
		Var_DF38 = 1;
	}
}

/* 23A7:0E99 [asm] - the poison waits (1 + cleric level) x 32760 ticks */
void far spell_slow_poison(void)
{
	if (Party[SpellTarget].flags & 2) {
		TargetChar->effects |= 0x2000L;
		start_timed_spell(CurrentSpell, 1, 32760L, cleric_level(BookCaster), 1);
	} else
		no_effect();
}

/* 23A7:0EF2 [asm] */
void far end_slow_poison(Character far *c)
{
	c->effects &= ~0x2000L;
}

/* 23A7:0F08 [asm] - all the party is no longer hungry */
void far spell_create_food(void)
{
	register int i;

	for (i = 0; i < 6; i++)
		if (char_is_active(i, 3))
			Party[i].food = 100;
}

/* 23A7:0F38 [asm] - up to 4 characters can move again */
void far spell_remove_paralysis(void)
{
	register int i, n;

	n = 4;
	for (i = 0; i < 6; i++)
		if ((Party[i].flags & 4) && n) {
			Party[i].flags &= ~4;
			n--;
		}
}

/* 23A7:0F71 [asm] */
void far spell_cure_serious_wounds(void)
{
	modify_hp(SpellTarget, roll_dice(2, 8, 1));
}

/* 23A7:0F95 [asm] */
void far spell_cause_serious_wounds(void)
{
	touch_attack(2, 8, 1);
}

/* 23A7:0FAE [asm] */
void far spell_neutralize_poison(void)
{
	if (TargetChar->flags & 2)
		neutralize_poison(SpellTarget);
	else
		no_effect();
}

/* 23A7:0FD1 [asm] */
void far spell_cause_critical_wounds(void)
{
	touch_attack(3, 8, 3);
}

/* 23A7:0FEA [asm] */
void far spell_cure_critical_wounds(void)
{
	modify_hp(SpellTarget, roll_dice(3, 8, 3));
}

/* 23A7:100E [asm] */
void far spell_flame_strike(void)
{
	launch_spell(BookCaster, 7, PartyBlock, Var_E440, PartyDirection);
}

/* 23A7:1030 [asm] - 6d8 */
int far hit_flame_strike(Missile far *t)
{
	return spell_hit_damage(t, 6, 8, 0, 0);
}

/* 23A7:104F [asm] - an elf cannot be raised */
void far spell_raise_dead(void)
{
	if (TargetChar->hp == -10 && (TargetChar->raceSex >> 1) != 1) {
		TargetChar->hp = 1;
		draw_portrait(SpellTarget);
	} else
		no_effect();
}

/* 23A7:1084 [asm] - by touch: the monster keeps 1d4 hit points */
void far spell_harm(void)
{
	touch_attack(-1, -1, -1);
}

/* 23A7:1097 [asm] */
void far spell_heal(void)
{
	if (TargetChar->hpMax - TargetChar->hp > 0)
		modify_hp(SpellTarget, TargetChar->hpMax - TargetChar->hp);
	else
		no_effect();
}

/* 23A7:10C7 [asm] - a monster spell: 12 points */
int far hit_twelve(Missile far *t)
{
	return spell_hit_damage(t, 0, 0, 12, 1);
}

/* 23A7:10E6 [asm] - Turn Undead on the monsters of the block ahead (each
 * one tried only once, flag 0x10) */
void far spell_turn_undead(void)
{
	Monster far *m;
	int block, n;
	int list[5];
	int roll, level;
	register int i, turned;

	turned = 0;
	block = block_ahead(PartyBlock, PartyDirection);
	n = LevelMap[block].flags & 7;
	if (n) {
		monsters_targeted(list, block, 4, 4, 1, 1);
		level = cleric_level(BookCaster);
		roll = roll_dice(1, 20, 0);
		for (i = 0; i < 5; i++) {
			if (list[i] == -1)
				break;
			m = &Monsters[list[i]];
			if ((MonsterTypes[m->type].kinds & 4L) && !(m->flags & 0x10)) {
				Var_E5A2 = 1;
				m->flags |= 0x10;
				turned |= turn_monster(m, roll, level);
			}
		}
		if (turned) {
			play_sound(0x5F);
			monsters_hit_update();
		}
		Var_E5A2 = 0;
	}
}

/* 23A7:11F6 [asm] - a monster's disintegration ray on the party.
 * ORIGINAL BUG: the test of save_half is inverted and repeated on the same
 * character: he is disintegrated as soon as a saving throw succeeds */
int far hit_disintegrate_party(Missile far *t)
{
	register int ch;

	if (t->block != PartyBlock)
		return 0;
	ch = random_char();
	while (ch != -1)
		if (!save_half(ch, 1)) {
			damage_character(ch, 300);
			sprintf(Scratch, "%s has been disintegrated!\r", Party[ch].name);
			gui_printf(Scratch);
			break;
		}
	return 1;
}

/* 23A7:126A [asm] - a death spell: 1d4 characters below level 8 die */
int far hit_death_spell(Missile far *t)
{
	register int ch, n;

	if (t->block != PartyBlock)
		return 0;
	n = roll_dice(1, 4, 0);
	ch = random_char();
	gui_printf("\x06\x06The party has been hit by a death spell!\r");
	while (ch != -1 && n > 0) {
		if (Party[ch].level[0] < 8) {
			damage_character(ch, 300);
			n--;
		}
		ch = next_char(ch);
	}
	return 1;
}

/* 23A7:12E1 [asm] - a monster's Cause Serious Wounds: 3d8+3 */
int far hit_cause_serious(Missile far *t)
{
	int dmg;
	register int ch;

	if (t->block != PartyBlock)
		return 0;
	ch = random_char();
	if (ch != -1) {
		gui_printf("\x06\x06%s has been hit by cause serious wounds.\r", Party[ch].name);
		dmg = roll_dice(3, 8, 3);
		damage_character(ch, dmg);
	}
	return 1;
}

/* 23A7:1343 [asm] - a petrifying gaze.
 * ORIGINAL BUG: save_half(ch, 2) is 1 or 2, never 0: nobody is petrified */
int far hit_petrify(Missile far *t)
{
	register int ch;

	if (t->block != PartyBlock)
		return 0;
	for (ch = random_char(); ch != -1; ch = next_char(ch))
		if (!save_half(ch, 2)) {
			set_char_status(ch, 8, "PETRIFIED", 5, 0L, 0, 1);
			break;
		}
	return 1;
}

/* 23A7:13A8 [asm] - 2 hit points per level */
void far spell_lay_on_hands(void)
{
	modify_hp(SpellTarget, Party[BookCaster].level[0] * 2);
}

/* 23A7:13CD [asm] - a breath: 12d10+6, or 4d10+6 under Mystic Defense (which ends) */
int far hit_breath(Missile far *t)
{
	register int r;

	if (PartyEffects & 0x20000L) {
		r = spell_hit_damage(t, 4, 10, 6, 0);
		if (r) {
			PartyEffects &= ~0x20000L;
			redraw_party();
		}
	} else
		r = spell_hit_damage(t, 12, 10, 6, 0);
	return r;
}

/* 23A7:143D [asm] - a monster spell: 18 points */
int far hit_eighteen(Missile far *t)
{
	return spell_hit_damage(t, 0, 0, 18, 0);
}
