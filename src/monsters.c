/*
 * Eye of the Beholder II - START.EXE
 * The monsters: movement, perception, attacks, creation: module 0997
 * (Ghidra 1997).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y monsters.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern Monster        Monsters[30];      /* 6E2A */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern int            PartyBlock;        /* 6DD2 */
extern unsigned long  GlobalFlags;       /* 6E26 */
extern char           WallTypeFlags[256];/* 2268 */
extern char           WallDecoration[256];/* 21C8 -1: a door */
extern int            NoViewCopy;        /* E4D2 */
extern int            ViewBlock[];       /* 9446 [13]: the block in front of the party */
extern unsigned long  Countdown;         /* 668C counts down to 0 */
extern char           SceneDirty;        /* D0AA */
extern char           Var_92A6;          /* 92A6 */
extern signed char    SpellMissile[][4]; /* 2558 per monster spell: the spell, ... */
extern Spell far      Spells[];          /* 39A6:0000 */
extern Item           Items[600];        /* 71CA */
extern Character      Party[6];          /* E608 */
extern unsigned long  PartyEffects;      /* 6DDC */
extern int            PartyDirection;    /* 6DD4 */
extern int            CurrentLevel;      /* 6DCE */
extern int            CurrentSubLevel;   /* 6DD0 */
extern int            VideoMode;         /* 628A */
extern void (far     *Drv_654E)(void far *palette);   /* 654E video driver entry */
extern byte           Remap_23A8[];    /* 23A8 */
extern byte           Palette_23C8[];    /* 23C8 */
extern signed char    FaceFront[4];      /* 22B8 */
extern int            Var_E5A2;          /* E5A2 */
extern int            Var_EE1E;          /* EE1E */
extern int            StepCounter;          /* E5C8 */
extern signed char far SpellPlaces[4][8];/* 3A14:0000 the places hit by a spell, per party direction */
extern signed char far PlaceSeen2[][4];  /* 3A16:0000 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  block_changed(int block);                                   /* 1E24:0005 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
int  octant(int from, int to);                                   /* 2A68:0316 -1: the same block */
int  distance(int from, int to);                                 /* 2A68:0FD8 */
void run_block_script(int block, unsigned event);                /* 24EC:2BE9 */
void open_door(int block);                                       /* 24EC:0426 */
void play_sound_at(int sound, int block);                        /* 2A30:0014 */
void play_sound_here(int sound);                                 /* 2A30:0000 */
void clock_stop(int n);                                          /* 3187:003D */
void clock_start(int n);                                         /* 3187:0009 */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void launch_spell(char from, int spell, int block, int pos, int dir);   /* 20E2:084A */
int  create_item(int src);                                       /* 1CBE:092B */
int  launch_item(int from, int item, int block, int pos, int dir, int type);   /* 1CBE:086A */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
void lose_random_item(int ch);                                   /* 1CBE:0005 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void set_char_status(int ch, int bit, char far *what, int save, long dur, int timerWhat, int once);  /* 1F55:15D6 */
void damage_character(int ch, int dmg);                          /* 1F55:14AC */
void give_party_xp(int xp);                                      /* 1F55:0C0F */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
int  to_hit_target(int thac0, int ac);                           /* 1B54:05D3 */
void missiles_check(void);                                       /* 27B7:0EC1 */
void dranx_appears(void);                                        /* 51C5:13BE (overlay, stub 3AD7:0039) the dragon */
void finale_free_shapes(void);                                   /* 5B64:0000 (overlay, stub 3B10:0034) */
void play_finale(void);                                          /* 5B64:1326 (overlay, stub 3B10:0025) the end */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);
void far *memset(void far *p, int c, unsigned n);                /* 1000:712B */
void damage_monster(Monster far *m, int from, int item, int a, int flags, int save, int saveType);  /* 1F55:12B8 */
int  far monster_step(Monster far *m, int block, int dir);
int  far monster_can_attack(Monster far *m);
void far monster_hits(Monster far *m);
void far monster_dies(Monster far *m, int xp);
int  far monster_in_place(Monster far *m, int block, int pos, int middle);

/* ------------------------------------------------------------------ */
/* Module 1997, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:302A...) */
/* the block offset of each octant (0 north, 1 north-east...)            302A */
signed char OctantStep[8] = {
	-32, -31, 1, 33, 32, 31, -1, -33
};
/* a diagonal step: [octant >> 1][facing], 0: none                       3032 */
signed char DiagonalStep[16] = {
	1, -32, 0, 0, 0, 32, 1, 0, 0, 0, -1, 32, -1, 0, 0, -32
};
/* the directions tried in turn when the way is blocked (two orders)     3042 */
int TurnOrderA[8] = {
	7, -6, 5, -4, 3, -2, 1, 0
};
int TurnOrderB[8] = {
	-7, 6, -5, 4, -3, 2, -1, 0
};
int TurnOrderSwitch = 0;                                                /* 3062 */
int TurnOrderCalls = 0;                                                 /* 3064 */
/* the place in its square of a monster arriving with this facing       3066 */
signed char PlaceOfFacing[4] = {
	0, 1, 3, 2
};
/* the places to try, by facing                                         306A */
signed char PlaceOrder[4][4] = {
	0, 1, 2, 3,
	0, 2, 1, 3,
	0, 1, 2, 3,
	3, 1, 2, 0
};
/* the place of a half-square monster, by the occupied places          307A */
signed char BigPlace[9] = {
	-1, 3, -1, -1, -1, -1, -1, -1, 0
};
/* the turn of a patrolling monster (modes 1-3: back, left, right), also
 * the side of a monster following a wall (modes 5, 6: left, right)    3083 */
signed char PatrolTurn[3] = { 2, -1, 1 };
/* can a monster strike from this place: [facing][place]                3086 */
signed char CanStrike[4][4] = {
	1, 1, 0, 0,
	0, 1, 0, 1,
	0, 0, 1, 1,
	1, 0, 1, 0
};
signed char TargetSide[4][4] = {
	2, 3, 0, 1,
	1, 2, 3, 0,
	0, 1, 2, 3,
	3, 0, 1, 2
};                                    /* 3096 */
signed char TargetOrder[48] = {
	1, 0, 3, 2, 5, 4,
	0, 1, 2, 3, 4, 5,
	5, 3, 1, 4, 2, 0,
	1, 3, 5, 0, 2, 4,
	4, 5, 2, 3, 0, 1,
	5, 4, 3, 2, 1, 0,
	0, 2, 4, 1, 3, 5,
	4, 2, 0, 5, 3, 1
};                                   /* 30A6 */
signed char TargetRow[4][4] = {
	0, 1, 0, 1,
	0, 0, 1, 1,
	1, 0, 1, 0,
	1, 1, 0, 0
};                                     /* 30D6 */
/* the places a front-row character reaches: [party dir][ch & 1][4]    30E6 */
signed char AttackPlaces[4][2][4] = {
	2, 3, 0, 1, 3, 2, 1, 0, 0, 2, 1, 3, 2, 0, 3, 1,
	1, 0, 3, 2, 0, 1, 2, 3, 3, 1, 0, 2, 1, 3, 2, 0
};
/* the places nearest to a place: [dir][place][4]                       3106 */
signed char NearPlaces[4][4][4] = {
	4, 2, 1, 3, 4, 3, 0, 2, 4, 0, 3, 1, 4, 1, 2, 0,
	4, 1, 2, 3, 4, 0, 3, 2, 4, 3, 0, 1, 4, 2, 1, 0,
	4, 2, 1, 3, 4, 3, 0, 2, 4, 0, 3, 1, 4, 1, 2, 0,
	4, 1, 2, 3, 4, 0, 3, 2, 4, 3, 0, 1, 4, 2, 1, 0
};

/* 1997:0004 [asm] - one step towards "dest": straight when possible,
 * else the directions of TurnOrderA or B (they swap every 10 calls). A
 * fleeing monster goes the other way */
void far monster_walk_to(Monster far *m, int dest)
{
	int dir, b, from, diff;
	int far *order;
	int t;
	register int f, i;

	if (++TurnOrderCalls > 10) {
		TurnOrderCalls = 0;
		TurnOrderSwitch ^= 1;
	}
	order = TurnOrderA;
	if (TurnOrderSwitch)
		order = TurnOrderB;
	f = m->facing * 2;
	from = m->block;
	if ((dir = octant(from, dest)) == -1)
		return;
	if (m->flags & 8) {                           /* fleeing */
		if (--m->wait > 0)
			dir ^= 4;
		else {
			m->wait = 0;
			m->flags &= ~8;
		}
	}
	diff = (dir - f) & 7;
	if (from + OctantStep[dir] == dest) {
		if (!(dir & 1)) {
			monster_step(m, -1, dir >> 1);
			return;
		}
		t = (dir - 1) * 2;
		t += m->facing;
		t = DiagonalStep[t];
		if (t != 0)
			if (!(MonsterTypes[m->type].flags & 0x200L) || roll_dice(1, 4, 0) < 4) {
				t += from;
				if (monster_step(m, t, -1))
					return;
			}
	}
	if (diff) {
		if (dir & 1) {
			if (diff < 5)
				f++;
			else
				f--;
		} else {
			if (diff < 5)
				f += 2;
			else
				f -= 2;
		}
		f &= 7;
	}
	for (i = 7; i > -1; i--) {
		f = (f + order[i]) & 7;
		if (f & 1)
			b = 0;
		else
			b = block_ahead(from, f >> 1);
		if (b && monster_step(m, b, f >> 1))
			return;
	}
}

/* 1997:01D9 [asm] - a monster flees from the party (20 turns) */
void far monster_flee(Monster far *m)
{
	int dir;

	if (m->mode == 7 || m->mode == 8 || m->mode == 10)
		return;
	m->mode = 0;
	m->wait = 20;
	dir = (octant(m->block, PartyBlock) ^ 4) >> 1;
	m->flags |= 8;
	monster_step(m, -1, dir);
}

/* 1997:023C [asm] - the passive monsters (mode 8) of the level wake up and
 * hunt the party; the type of the last one may set a global flag */
void far wake_passive_monsters(void)
{
	Monster far *m;
	Monster far *last;
	int i;

	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->mode == 8) {
			m->mode = 0;
			m->dest = PartyBlock;
			last = m;
		}
	if (last->type == 7)
		GlobalFlags |= 0x40000L;
	if (last->type == 12)
		GlobalFlags |= 0x8000000L;
}

/* 1997:02AA [asm] - perception: a monster within 3 squares hunts the party
 * when it is not behind its back, or when it is adjacent */
void far monster_look(Monster far *m)
{
	int dist;
	int d;

	if (m->mode == 7 || m->mode == 8 || m->mode == 9 || m->mode == 10 || m->hp <= 0)
		return;
	if ((dist = distance(m->block, PartyBlock)) < 4) {
		d = octant(m->block, PartyBlock) - m->facing * 2 - 3;
		if (d < 0)
			d += 8;
		if (d > 2 || dist < 2) {
			m->mode = 0;
			m->dest = PartyBlock;
		}
	}
}

/* 1997:033D [asm] */
void far monsters_look(void)
{
	Monster far *m;
	int i;

	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		monster_look(m);
}

/* 1997:036C [asm] - the places taken in a block (a bit each), -1 when a
 * monster of another size is there, -2 when it is full */
int far occupied_places(int block, int big)
{
	int mask;
	Monster far *m;
	register int i, n;

	n = LevelMap[block].flags & 7;
	if (n == 4)
		return -2;
	mask = 0;
	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->block == block) {
			if (MonsterTypes[m->type].big != big)
				return -1;
			if (m->pos == 4 && !(m->flags & 0x20)) {
				if (MonsterTypes[m->type].big != 1)
					m->pos = PlaceOfFacing[m->facing];
				else
					m->pos = 0;
			}
			mask |= 1 << m->pos;
			if (--n == 0)
				break;
		}
	return mask;
}

/* 1997:0437 [asm] - a free place for monster m in a block, -1 none */
int far free_place(Monster far *m, int block)
{
	int place, big;
	signed char far *p;
	register int i, mask;

	big = MonsterTypes[m->type].big;
	if (big == 2)
		return -1;
	mask = occupied_places(block, big);
	if (mask < 0)
		return -1;
	if (big == 1) {
		if (mask == 9)
			return -1;
		place = BigPlace[mask];
		if (place != -1)
			m->facing = Var_92A6;
		return place;
	}
	p = PlaceOrder[m->facing];
	for (i = 0; i < 4; i++, p++) {
		place = m->facing ^ *p;
		if (!(mask & (1 << place)))
			return place;
	}
	return -1;
}

/* 1997:04EA [asm] - a monster goes to another block (-1: stays) and turns
 * (-1: no); the blocks' scripts get the events 0x400 (left) and 0x200 */
void far move_monster(Monster far *m, int block, int dir)
{
	if (block != -1) {
		block_changed(m->block);
		if (LevelMap[m->block].flags & 7) {
			LevelMap[m->block].flags--;
			run_block_script(m->block, 0x400);
		}
		m->block = block;
		LevelMap[block].flags++;
		run_block_script(m->block, 0x200);
	}
	if (dir != -1) {
		m->facing = dir;
		block = m->block;
	}
	block_changed(block);
}

/* 1997:0585 [asm] - a monster steps into "block" (or only turns to "dir"
 * when block is -1 or its own). Doors: a monster type 0x1000 opens them,
 * type 1 with 0x04 smashes them. 0: not possible */
int far monster_step(Monster far *m, register int block, int dir)
{
	MonsterType far *mt;
	LevelBlock far *lb;
	char wall, place;
	register int from;

	from = m->block;
	mt = &MonsterTypes[m->type];
	if (block != from && block != -1) {
		if (m->flags & 8)
			if (distance(block, PartyBlock) < distance(m->block, PartyBlock))
				return 0;                         /* fleeing: never closer */
		if (block == PartyBlock)
			return 0;
		lb = &LevelMap[block];
		if (dir == -1)
			dir = m->facing;
		wall = lb->walls[dir ^ 2];
		if ((WallTypeFlags[wall] & 4) == 0) {
			if ((mt->flags & 0x1000L) && WallDecoration[wall] == -1) {
				if (WallTypeFlags[wall] & 0x20) {
					if ((mt->flags & 4L) && m->type == 1) {
						lb->walls[dir] = 0x48;    /* smashed */
						lb->walls[dir ^ 2] = 0x48;
					} else
						open_door(block);
				}
				if (dir != -1) {
					m->facing = dir;
					block_changed(m->block);
				}
				return 1;
			}
			return 0;
		}
		if ((lb->flags & 7) && block) {
			place = free_place(m, block);
			if (place == -1)
				return 0;
			m->pos = place;
		}
		move_monster(m, block, dir);
		dir = -1;
	}
	if (dir != -1)
		m->facing = dir;
	block_changed(from);
	if (NoViewCopy == 0 && MonsterTypes[m->type].moveSound)
		play_sound_at(MonsterTypes[m->type].moveSound, m->block);
	return 1;
}

/* 1997:0765 [asm] - a monster that was hit hunts the party */
void far monster_alert(Monster far *m)
{
	if ((m->flags & 1) && m->mode != 10) {
		if (m->mode == 8)
			wake_passive_monsters();
		else {
			m->mode = 0;
			m->dest = PartyBlock;
		}
	}
}

/* 1997:0797 [asm] - a monster takes a place in its block */
void far monster_settle(Monster far *m)
{
	int place;

	place = m->block;
	if ((LevelMap[place].flags & 7) == 1)
		m->pos = 4;
	else {
		place = free_place(m, place);
		if (place < 0)
			return;
		m->pos = place;
	}
	block_changed(m->block);
}

/* 1997:07F8 [asm] - melee: the party is in "block" (-1: the square ahead).
 * A monster strikes every other turn; seen by the party, it is animated */
int far monster_melee(Monster far *m, register int block)
{
	register int seen;

	if (block == -1)
		block = block_ahead(m->block, m->facing);
	if (block != PartyBlock)
		return 0;
	if (monster_can_attack(m)) {
		m->flags ^= 4;
		if (m->flags & 4) {
			seen = m->block == ViewBlock[13];
			if (seen) {
				clock_stop(2);                    /* the game clock stops */
				if (m->type == 4)
					play_sound_here(MonsterTypes[m->type].attackSound);
				m->attack = -2;
				Countdown = 0;
				draw_scene(1);
				m->attack = -1;
				if (m->type != 4)
					play_sound_here(MonsterTypes[m->type].attackSound);
				Countdown = 8;
				draw_scene(1);
			} else
				play_sound_here(MonsterTypes[m->type].attackSound);
			monster_hits(m);
			if (seen) {
				m->attack = 0;
				m->phase ^= 1;
				SceneDirty = 1;
				clock_start(2);
				Countdown = 8;
			}
		}
	} else
		monster_settle(m);
	return 1;
}

/* 1997:095C [asm] */
void far monster_cast(Monster far *m, int spell)
{
	launch_spell(-1, spell, m->block, m->pos, m->facing);
	spell = SpellMissile[spell][0];
	play_sound_at(Spells[spell].sound, m->block);
}

/* 1997:09B4 [asm] - does nothing */
void far fn_1997_09B4(Monster far *m, register int i)
{
	m++;
	i++;
}

/* 1997:09C3 [asm] - a ranged attack: the party straight ahead, at most 3
 * squares, nothing between. The attacks of the type are taken in turn
 * (mode 1) or at random (2): < 0 a thrown item, < 20 a spell, 20 the
 * mind flayer's psychic blast (the whole party paralyzed) */
int far monster_ranged_attack(Monster far *m)
{
	MonsterType far *mt;
	int dir, mode, count, k, item;
	signed char far *p;
	register int block, i;

	mt = &MonsterTypes[m->type];
	if (m->ammo == 0)
		return 0;
	if (roll_dice(1, 3, 0) > m->cooldown) {
		m->cooldown++;
		return 0;
	}
	if (distance(m->block, PartyBlock) > 3 || octant(m->block, PartyBlock) != m->facing * 2)
		return 0;
	dir = m->facing;
	for (block = block_ahead(m->block, dir); block != PartyBlock; block = block_ahead(block, dir)) {
		if ((WallTypeFlags[LevelMap[block].walls[dir ^ 2]] & 3) == 0 || (LevelMap[block].flags & 7))
			return 0;
	}
	mode = mt->attackMode;
	count = mt->attackCount;
	p = mt->rangedAttack;
	switch (mode) {
	case 1:
		k = m->cycle;
		if (++m->cycle >= count)
			m->cycle = 0;
		break;
	case 2:
		k = roll_dice(1, count, -1);
		break;
	}
	p += k;
	if (*p < 0) {
		item = create_item(*p < 0 ? -*p : *p);
		if (item)
			if (!launch_item(-1, item, m->block, m->pos, m->facing, Items[item].type))
				Items[item].block = -1;
	} else if (*p < 20)
		monster_cast(m, *p);
	else
		switch (*p) {
		case 20:                                  /* psychic mind blast */
			play_sound_at(0x67, m->block);
			gui_printf("The party is hit with a psychic mind blast!\r");
			for (i = 0; i < 6; i++)
				set_char_status(i, 4, "paralyzed", 1, 5L, 9, 1);
		}
	if (m->ammo != 0xFF)
		m->ammo--;
	m->cooldown = 0;
	return 1;
}

/* 1997:0C33 [asm] - mode 0: hunt the party. A monster that reaches its
 * destination without finding the party waits (mode 5 or 6) */
void far monster_hunt(Monster far *m)
{
	MonsterType far *mt;
	register int dir, block;

	mt = &MonsterTypes[m->type];
	dir = octant(m->block, PartyBlock);
	if (!(mt->flags & 0x800L) || (dir & 1) == 1)
		dir = m->facing;
	else
		dir >>= 1;
	block = block_ahead(m->block, dir);
	if (m->dest == block && block != PartyBlock)
		m->mode = roll_dice(1, 2, -1) + 5;
	else if (!monster_ranged_attack(m))
		if (!monster_melee(m, block)) {
			m->attack = 0;
			monster_walk_to(m, m->dest);
			if (MonsterTypes[m->type].flags & 8L)
				monster_melee(m, -1);
		}
}

/* 1997:0D33 [asm] - modes 1-3: straight on; blocked, turn by "turn" quarters */
void far monster_patrol(Monster far *m, int turn)
{
	int block;

	block = block_ahead(m->block, m->facing);
	if (!monster_step(m, block, -1)) {
		m->facing += turn;
		m->facing &= 3;
		monster_step(m, -1, m->facing);
	}
}

/* 1997:0D96 [asm] - modes 5, 6: follow a wall on one side (-1 left, 1 right) */
void far monster_follow_wall(Monster far *m, register int side)
{
	int ahead, block, open, face;
	register int dir;

	if (m->follow >= 0) {
		if (m->follow == 0)
			monster_patrol(m, -side);
		dir = (m->facing + side) & 3;
		block = block_ahead(m->block, dir);
		face = FaceFront[dir];
		open = WallTypeFlags[LevelMap[block].walls[face]] & 4;
		if (m->follow == 0) {
			if (!open)
				m->follow = -1;
			return;
		}
		if (open) {
			monster_step(m, -1, dir);
			m->follow = -1;
			return;
		}
	}
	ahead = block_ahead(m->block, m->facing);
	if (!monster_step(m, ahead, -1)) {
		dir = (m->facing - side) & 3;
		monster_step(m, -1, dir);
		m->follow = 0;
	} else
		m->follow = 1;
}

/* 1997:0EB3 [asm] - modes 7, 10: wait, then hunt */
void far monster_wait(Monster far *m)
{
	if (m->wait && --m->wait == 0)
		m->mode = 0;
}

/* 1997:0ED5 [asm] - all the monsters of a block turn with a big one */
void far big_monster_update(int block, int dir)
{
	Monster far *m;
	int i;

	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->block == block && m->facing != dir) {
			m->facing = dir;
			m->skip = 1;
		}
}

/* 1997:0F1C [asm] - the turn of the monsters of one group (timer_monsters) */
void far monsters_think(int group)
{
	Monster far *m;
	register int i;

	m = Monsters;
	for (i = 0; i < 30; i++, m++) {
		if (m->unit == group) {
			if (m->hp <= 0 || (m->flags & 0x20))
				continue;
			if (m->skip) {
				m->skip = 0;
				continue;
			}
			monster_look(m);
			if (m->mode > 0)
				monster_alert(m);
			switch (m->mode) {
			case 0:
				monster_hunt(m);
				break;
			case 1:
			case 2:
			case 3:
				monster_patrol(m, PatrolTurn[m->mode - 1]);
				break;
			case 5:
			case 6:
				monster_follow_wall(m, PatrolTurn[m->mode - 4]);
				break;
			case 7:
			case 10:
				monster_wait(m);
				break;
			}
			if (m->mode != 4 && m->mode != 7 && m->mode != 8)
				m->phase ^= 1;
		}
		if (MonsterTypes[m->type].big == 1)
			big_monster_update(m->block, m->facing);
	}
	missiles_check();
}

/* 1997:106E [asm] - a flash of the screen when an invisible monster is hit */
void far monster_flash(Monster far *m)
{
	clock_stop(2);                    /* the game clock stops */
	if (VideoMode == 0)
		Drv_654E(Palette_23C8);
	else
		Drv_654E(Remap_23A8);
	Countdown = 0;
	draw_scene(1);
	m->flags &= ~2;
	Countdown = 1;
	clock_start(2);
	SceneDirty = 1;
}

/* 1997:10D2 [asm] - after a spell: the monsters hit are shown, the dead die */
void far monsters_hit_update(void)
{
	Monster far *m;
	register int i, changed;

	m = Monsters;
	draw_scene(1);
	/* ORIGINAL BUG: "changed" is never set to 0 (whatever DI holds) */
	for (i = 0; i < 30; i++, m++)
		if (m->flags & 2) {
			m->flags &= ~2;
			changed = 1;
			if (m->hp <= 0)
				monster_dies(m, 1);
		}
	if (changed) {
		SceneDirty = 1;
		Countdown = 1;
	} else
		SceneDirty = 0;
	Var_E5A2 = 0;
}

/* 1997:114E [asm] - a monster dies: it may drop its items, the party gets
 * the experience. On level 16 (sublevel 1) the dragon Dran: the first form
 * comes back as the second, the second ends the game */
void far monster_dies(Monster far *m, int xp)
{
	register int pos, item;

	m->hp = -1;
	if ((pos = m->pos) == 4)
		pos = roll_dice(1, 4, -1);
	if (m->drop1 != 0 && roll_dice(1, 10, 0) == 1) {
		item = create_item(m->drop1);
		put_item(&LevelMap[m->block].items, m->block, item, pos);
	}
	if (m->drop2 != 0) {
		item = create_item(m->drop2);
		put_item(&LevelMap[m->block].items, m->block, item, pos);
	}
	if (xp)
		give_party_xp(MonsterTypes[m->type].xp);
	if (CurrentLevel == 16 && CurrentSubLevel == 1 && (MonsterTypes[m->type].flags & 4L)) {
		if (m->type == 0) {
			m->hp = 150;
			m->cycle = 0;
			m->ammo = -1;
			m->slot++;
			m->type++;
			dranx_appears();
		} else {
			finale_free_shapes();
			play_finale();
			system_shutdown();
			exit(0);
		}
	} else {
		move_monster(m, 0, -1);
		if (m->mode == 8)
			wake_passive_monsters();
	}
}

/* 1997:12AF [asm] - only the monsters at the front of their square (or in
 * the middle, or big) can strike */
int far monster_can_attack(Monster far *m)
{
	if (m->pos == 4 || MonsterTypes[m->type].big == 1)
		return 1;
	return CanStrike[m->facing][m->pos];
}

/* 1997:12EF [asm] - the attack roll of a monster against character ch: a
 * natural 20 always hits */
int far monster_to_hit(Monster far *m, int ch)
{
	int type, need;
	register int roll;

	type = m->type;
	roll = roll_dice(1, 20, 0);
	if (roll != 20) {
		if (Party[ch].effects & 0x800L)           /* protection from evil */
			roll -= 2;
		if (Party[ch].effects & 0x10L)            /* blur */
			roll -= 2;
		if (PartyEffects & 0x8000L)               /* prayer */
			roll--;
	}
	need = to_hit_target(MonsterTypes[type].thac0, Party[ch].ac);
	return roll >= need || roll == 20;
}

/* 1997:1397 [asm] - all the attacks of a monster on one character, then
 * its special effects: item lost, poison, paralysis, petrification */
void far monster_attack_char(int ch, Monster far *m)
{
	int dmg, n;
	register int i, type;

	dmg = 0;
	type = m->type;
	n = MonsterTypes[type].attacks;
	for (i = 0; i < n; i++)
		/* ORIGINAL BUG: the loop counter i is passed instead of ch: attack i
		 * is rolled against the armour class of character i */
		if (monster_to_hit(m, i))                 /* sic */
			dmg += roll_dice(MonsterTypes[type].damage[i][0], MonsterTypes[type].damage[i][1], MonsterTypes[type].damage[i][2]);
	if (dmg > 0) {
		if ((MonsterTypes[type].flags & 0x80L) && roll_dice(1, 4, -1) != 3)
			lose_random_item(ch);
		damage_character(ch, dmg);
		if (MonsterTypes[type].flags & 0x10L) {
			set_char_status(ch, 2, "poisoned", 0, 1L, 8, 1);
			Party[ch].effects &= ~0x2000L;        /* slow poison ends */
		}
		if (MonsterTypes[type].flags & 0x20L)
			set_char_status(ch, 4, "paralyzed", 2, 5L, 9, 1);
		if (MonsterTypes[type].flags & 0x8000L)
			set_char_status(ch, 8, "PETRIFIED", 2, 0L, 0, 1);
	}
}

/* 1997:153D [asm] - a monster strikes: the characters it can reach depend
 * on the directions and on its place (front or back row); 0x140 (blur,
 * displacement ?) makes a character hard to hit; type flag 0x4000: it hits
 * all the characters of the row */
void far monster_hits(Monster far *m)
{
	int side, row, end;
	Character far *c;
	register int i, base;

	side = TargetSide[PartyDirection][m->facing];
	base = side * 12;
	row = m->pos != 4 ? TargetRow[m->facing][m->pos] : roll_dice(1, 2, -1);
	if (row)
		base += 6;
	end = base + 6;
	for (i = base; i < end; i++) {
		c = &Party[TargetOrder[i]];
		if (char_is_active(TargetOrder[i], 3)) {
			if ((c->effects & 0x140L) && roll_dice(1, 20, 0) >= 5)
				continue;
			monster_attack_char(TargetOrder[i], m);
			if (!(MonsterTypes[m->type].flags & 0x4000L))
				break;
		}
	}
}

/* 1997:1642 [asm] - (re)creates monster n */
void far create_monster_at(register int n, int unit, register int block, int pos, int dir, int type,
	int slot, int mode, int wait, int drop1, int drop2)
{
	Monster far *m;
	MonsterType far *mt;

	m = &Monsters[n];
	mt = &MonsterTypes[type];
	memset(m, 0, 30);
	if (block) {
		unit <<= 1;
		if (n & 1)
			unit++;
		m->cooldown = roll_dice(1, 3, 0);
		m->type = type;
		m->ammo = mt->ammo;
		m->cycle = 0;
		m->unit = unit;
		m->pos = pos;
		m->slot = slot;
		m->mode = mode;
		m->wait = wait;
		m->facing = dir;
		m->colour = n % 3;
		m->hpMax = m->hp = roll_dice(mt->hpDice[0], mt->hpDice[1], mt->hpDice[2]);
		m->drop1 = drop1;
		m->drop2 = drop2;
		m->sublevel = CurrentSubLevel;
		move_monster(m, block, dir);
	}
}

/* 1997:1754 [asm] */
void far monsters_update(void)
{
	if (Var_EE1E && StepCounter > Var_EE1E) {
		run_block_script(0, 0x20);
		StepCounter = 0;
	}
}

/* 1997:177E [asm] - a new monster (from a script): not where the party
 * is, not in a full block; it takes a free slot, else the slot of the
 * farthest monster (not one placed by the INF, 0x40) */
void far create_monster(int unit, int block, int pos, int dir, int type, int slot, int mode,
	int wait, int drop1, int drop2)
{
	int far_, d, n;
	register int i, best;

	n = LevelMap[block].flags & 7;
	if (n == 7)
		return;
	if (PartyBlock == block)
		return;
	if (n != 0) {
		if (MonsterTypes[type].big != 0)
			return;
		if (pos == 4)
			return;
	}
	for (i = 0; i < 30; i++)
		if (Monsters[i].block == block) {
			if (Monsters[i].pos == 4)
				return;
			if (MonsterTypes[Monsters[i].type].big != 0)
				return;
		}
	best = -1;
	for (far_ = i = 0; i < 30; i++) {
		if (Monsters[i].hp <= 0) {
			best = i;
			break;
		}
		if ((Monsters[i].flags & 0x40) == 0)
			if ((d = distance(Monsters[i].block, PartyBlock)) > far_) {
				far_ = d;
				best = i;
			}
	}
	if (best == -1)
		return;
	if (Monsters[best].hp > 0)
		monster_dies(&Monsters[best], 0);
	create_monster_at(best, unit, block, pos, dir, type, slot, mode, wait, drop1, drop2);
}

/* 1997:18D2 [asm] - the monster in each place of a block (-1: none) */
void far monsters_by_place(int far *place, register int block)
{
	Monster far *m;
	register int i;

	for (i = 0; i < 6; i++)
		place[i] = -1;
	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->block == block)
			place[m->pos] = i;
}

/* 1997:1929 [asm] - the monster a front-row character can strike in the
 * block ahead, -1 none */
int far monster_to_attack(char ch, int block)
{
	int place[6];
	signed char far *p;
	register int i;

	monsters_by_place(place, block);
	if (place[4] != -1)
		return place[4];
	p = AttackPlaces[PartyDirection][ch & 1];
	for (i = 0; i < 4; i++, p++)
		if (place[*p] != -1)
			return place[*p];
	return -1;
}

/* 1997:19A9 [asm] - a spell hits all the monsters of a block */
void far spell_hits_block(int block, char from, char item, char a, char flags, char save, char saveType)
{
	int place[6];
	signed char far *p;
	register int i;

	monsters_by_place(place, block);
	Var_E5A2 = 1;
	place[5] = -1;
	p = SpellPlaces[PartyDirection];
	for (i = 0; i < 6; i++, p++)
		if (place[*p] != -1)
			damage_monster(&Monsters[place[*p]], from, item, a, flags, save, saveType);
	monsters_hit_update();
}

/* 1997:1A4A [asm] - the monster nearest to a place of a block */
int far monster_near_place(int block, int pos, int dir)
{
	signed char far *p;
	int best, bestk;
	Monster far *m;
	register int k, i;

	best = -1;
	bestk = 5;
	for (i = 0, m = Monsters; i < 30; i++, m++) {
		p = NearPlaces[dir][pos];
		if (m->block == block) {
			if (m->pos == pos)
				return i;
			for (k = 0; k < 4; k++)
				if (m->pos == p[k] && k < bestk) {
					bestk = k;
					best = i;
				}
		}
	}
	return best;
}

/* 1997:1AD2 [asm] - the monsters hit in a block (list ended by -1): all
 * of them, the nearest to a place, or the first one in the place */
void far monsters_targeted(int far *out, register int block, int pos, char dir, char all, char nearest)
{
	Monster far *m;
	int middle;
	register int i;

	if (pos < 4)
		middle = PlaceSeen2[dir][pos];
	else
		middle = 1;
	if (all) {
		for (i = 0, m = Monsters; i < 30; i++, m++)
			if (m->block == block && (m->pos != 4 || middle))
				*out++ = i;
	} else if (nearest)
		*out++ = monster_near_place(block, pos, dir);
	else
		for (i = 0, m = Monsters; i < 30; i++, m++)
			if (monster_in_place(m, block, pos, middle)) {
				*out++ = i;
				break;
			}
	*out = -1;
}

/* 1997:1BAE [asm] */
int far monster_in_place(Monster far *m, int block, int pos, int middle)
{
	return m->block == block && (m->pos == pos || (m->pos == 4 && middle));
}
