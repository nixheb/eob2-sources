/*
 * Eye of the Beholder II - START.EXE
 * Timers: module 17B7 (Ghidra 27B7). The game keeps 16 timers (Timers[] at
 * DS:5BFC, see loop.c); run_timers() calls the function of every timer that
 * is due. They move the missiles, open the doors, animate the decorations,
 * make the party hungry...
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y timers.c
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
extern Missile        Thrown[10];        /* CC5C */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern char           SceneDirty;        /* D0AA */
extern int            Var_DF4C;          /* DF4C the event of a click / a hit on a wall */
extern char           WallTypeFlags[256];/* 2268 */
extern char           ItemSize[];        /* 2590 per item icon: >= 15 = small */
extern unsigned long  GameTicks;         /* 6684 */
typedef struct MovingDoor { int block; signed char dir; } MovingDoor;
extern MovingDoor     MovingDoors[3];    /* 929C */
extern int            AnimPhase;         /* 6D7E toggled every 10 ticks: animated decorations */
extern char far      *ViewWalls[18];     /* DEDC &LevelMap[ViewBlock[i]].walls */
extern char           Front;             /* 9444 the face of a square looking at the party */
extern ForceWall      ForceWalls[5];     /* 9346 the walls of force */
extern char           Scratch[];         /* F16E */
extern char           Var_EE3A;          /* EE3A a mode of the inventory screen (2: statistics ?) */
extern char           BookCaster;        /* D0B0 the character whose spell book is open */
extern char           BookOpen;          /* E441 */
extern char           SelectedChar;      /* EE20 the character whose portrait blinks (?) */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void draw_portrait(int ch);                                       /* 2901:0AF9 draws a portrait frame (?) */
int  unlink_item(unsigned far *list, int pos, int item);         /* 1CBE:02BF */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
int  wall_click(int block, int side);                            /* 24EC:0A81 */
int  block_has_monsters(int block);                              /* 24EC:01EB */
int  monster_in_place(Monster far *m, int block, int pos, int centre); /* 1997:1BAE monster_in_place */
int  thrown_hits_monster(Missile far *t, int m);                  /* 1F55:0368 */
int  thrown_hits_party(Missile far *t);                           /* 1F55:0279 */
void far run_block_script(int block, unsigned events);           /* 24EC:2BE9 */
void play_sound_here(int sound);                                 /* 2A30:0000 */
int  block_ahead(int block, int dir);                            /* 1E24:012F */
void missile_burst(Missile far *t);                               /* 20E2:1EDB */
void block_changed(int block);                                   /* 1E24:0005 */
void remove_force_wall(int n);                                   /* 23A7:098C a wall of force disappears */
void monsters_update(void);                                      /* 1997:1754 */
void scripts_update(void);                                       /* 24EC:000B */
void monsters_think(int group);                                  /* 1997:0F1C */
void end_spell_effect(int spell, int ch, int a);                 /* 20E2:0C6D */
void inflict_damage(int ch, int a, int b, int c, int d, int e, int f); /* 1F55:143E */
void draw_spell_book(void);                                    /* 20E2:1127 */
int  char_is_active(int ch, int what);                           /* 1E24:06E9 */
int  has_ring(Character far *c, int kind);                       /* 1E24:01BE 1 or 0 */
void draw_food(int ch);                                    /* 2901:0504 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  sprintf(char far *buf, char far *fmt, ...);
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */

/* ------------------------------------------------------------------ */
/* Module 27B7, in address order                                       */
/* ------------------------------------------------------------------ */

typedef struct Timer {
	void (far *func)(int arg);   /* +0 */
	unsigned long interval;      /* +4 in game ticks */
	unsigned long next;          /* +8 GameTicks value of the next call */
	int           arg;           /* +C */
	char          unkE;          /* +E */
	char          active;        /* +F */
} Timer;

void far timer_blink(int);
void far timer_thrown(int);
void far timer_monsters(int group);
void far timer_character(int ch);
void far timer_doors(int);
void far timer_decorations(int);
void far timer_hunger(int);
void far timer_monster_idle(int);

/* Initialised data of the module (DS:5BFC...) */
Timer Timers[16] = {                               /* 5BFC */
	{ timer_blink,           9, 0,      0, 0, 0 },     /* the selected portrait blinks (off at start) */
	{ timer_thrown,          3, 0,      0, 0, 1 },     /* missiles */
	{ timer_monsters,       20, 0,      0, 0, 1 },     /* the monsters, in 4 groups */
	{ timer_monsters,       20, 7,      1, 0, 1 },     /* (staggered: the groups do not act on the same tick) */
	{ timer_monsters,       20, 14,     2, 0, 1 },
	{ timer_monsters,       20, 14,     3, 0, 1 },
	{ timer_character,      50, 0,      0, 0, 0 },     /* one per character (off at start) */
	{ timer_character,      50, 0,      1, 0, 0 },
	{ timer_character,      50, 0,      2, 0, 0 },
	{ timer_character,      50, 0,      3, 0, 0 },
	{ timer_character,      50, 0,      4, 0, 0 },
	{ timer_character,      50, 0,      5, 0, 0 },
	{ timer_doors,           5, 0,      0, 0, 1 },     /* doors in motion */
	{ timer_decorations,    10, 0,      0, 0, 1 },     /* animated decorations */
	{ timer_hunger,       1080, 0,      0, 0, 1 },     /* one game minute */
	{ timer_monster_idle,   25, 0,      0, 0, 1 },     /* idle monsters fidget */
};
int  TimerCount = 16;                              /* 5CFC */
char BlinkOn = 0;                                  /* 5CFE */
/* how a missile moves, per direction and place in the square: the new place,
 * + 0x80 when it goes on to the next square                          5CFF */
unsigned char MissileMove[4][4] = {
	{ 0x82, 0x83, 0x00, 0x01 },
	{ 0x01, 0x80, 0x03, 0x82 },
	{ 0x02, 0x03, 0x80, 0x81 },
	{ 0x81, 0x00, 0x83, 0x02 },
};

/* 27B7:000F [asm] - the frame of the selected character blinks */
void far timer_blink(int arg)
{
	int ch;

	BlinkOn ^= 1;
	if (BlinkOn) {
		ch = SelectedChar;
		SelectedChar = -1;
		draw_portrait(ch);
		SelectedChar = ch;
	} else
		draw_portrait(SelectedChar);
	arg++;                                        /* (unused) */
}

/* 27B7:0054 [asm] - a missile enters (block, pos). An item goes through an
 * open wall, or through a see-through one (bars) if it is small; otherwise
 * it hits the wall (event 0x10). A spell only needs an open wall. */
int far missile_enter(Missile far *t, int block, int pos)
{
	int item, small, pass, wall, wflags;

	wall = LevelMap[block].walls[t->dir ^ 2];
	wflags = WallTypeFlags[wall];
	if (t->active == 1) {
		item = t->item;
		small = ItemSize[Items[item].icon] >= 15;
		pass = (wflags & 1) || t->justLaunched || ((wflags & 2) && small);
		if (pass) {
			unlink_item(&LevelMap[t->block].items, 0, item);
			put_item(&LevelMap[block].items, block, item, pos | 4);
			t->block = block;
			t->pos = pos;
			t->range--;
			return 1;
		}
		Var_DF4C = 0x10;
		wall_click(block, t->dir);
	} else if ((wflags & 1) || t->block == block) {
		t->block = block;
		t->pos = pos;
		if (t->range != -1)
			t->range--;
		return 1;
	}
	return 0;
}

/* 27B7:0188 [asm] - does the missile hit someone in (block, pos)? */
int far missile_hit(Missile far *t, int block, int pos)
{
	Monster far *m;
	int i, b;

	b = block;
	if (t->justLaunched && (t->block != PartyBlock || t->caster >= 0)
	    && (!block_has_monsters(t->block) || t->caster < 0))
		return 0;
	if (t->active == 2)
		return t->hit(t);
	if (LevelMap[b].flags & 7) {
		for (i = 0, m = Monsters; i < 30; i++, m++)
			if (monster_in_place(m, b, pos, 1) && thrown_hits_monster(t, i))
				return 1;
	} else if (b == PartyBlock)
		return thrown_hits_party(t);
	return 0;
}

/* 27B7:025E [asm] - the missile falls (an item lands: event 4) */
void far missile_stop(Missile far *t)
{
	if (t->active == 1) {
		Items[t->item].pos &= 3;
		run_block_script(t->block, 4);
		play_sound_here(18);
	}
	t->active = 0;
	t->range = 0;
	t->dir = 0;
	t->block = 0;
	t->caster = 0;
	t->shape = 0;
	t->item = 0;
}

/* 27B7:02C1 [asm] - every 3 ticks the missiles move by half a square */
void far timer_thrown(int arg)
{
	int stop, i, bounce;
	Missile far *t;
	int pos, block;

	arg++;                                        /* (unused) */
	for (i = 0, t = Thrown; i < 10; i++, t++) {
		if (t->active != 0) {
			stop = t->range == 0;
			pos = MissileMove[t->dir][t->pos & 3];
			bounce = pos & 0x80;
			if (bounce) {
				pos &= 3;
				block = block_ahead(t->block, t->dir);
				t->justLaunched = 0;
			} else
				block = t->block;
			if (missile_enter(t, block, pos)) {
				if (bounce)
					run_block_script(block, 0x10);
				if (missile_hit(t, block, pos))
					stop = 1;
			} else {
				if ((t->flags & 0x20) && !missile_hit(t, t->block, t->pos))
					missile_burst(t);
				stop = 1;
			}
			if (stop)
				missile_stop(t);
			SceneDirty = 1;
		}
	}
}

/* 27B7:03F8 [asm] - the doors in motion (move_door) take one step */
void far timer_doors(int arg)
{
	int dir, side, wflags;
	int block, i;

	for (i = 0; i < 3; i++) {
		dir = MovingDoors[i].dir;
		block = MovingDoors[i].block;
		if (block != 0) {
			wflags = WallTypeFlags[LevelMap[block].walls[0]];
			side = (wflags & 8) ? 0 : 1;
			LevelMap[block].walls[side] = LevelMap[block].walls[side] + dir;
			LevelMap[block].walls[side + 2] = LevelMap[block].walls[side + 2] + dir;
			wflags = WallTypeFlags[LevelMap[block].walls[side]];
			block_changed(block);
			play_sound_here((wflags & 0x20) ? 5 : (dir == -1 ? 4 : 3));
			if ((wflags & 0x20) || (wflags & 0x10))
				MovingDoors[i].block = 0;
		}
	}
}

/* 27B7:050E [asm] - animated decorations (types 0x2C and 0x4A) change
 * picture every 10 ticks: redraw if one is in sight */
void far timer_decorations(int arg)
{
	int i;

	arg++;                                        /* (unused) */
	AnimPhase ^= 1;
	for (i = 0; i < 18; i++)
		if (ViewWalls[i][Front] == 0x2C || ViewWalls[i][Front] == 0x4A) {
			SceneDirty = 1;
			break;
		}
}

/* 27B7:055E [asm] - the walls of force that have expired disappear */
void far timed_events_update(void)
{
	int i;

	for (i = 0; i < 5; i++)
		if (ForceWalls[i].block && ForceWalls[i].expires < GameTicks)
			remove_force_wall(i);
}

/* 27B7:05A6 [asm] - called by the main loop */
void far run_timers(void)
{
	int i;
	Timer far *t;

	for (i = 0, t = Timers; i < TimerCount; i++, t++)
		if (t->active && t->next < GameTicks) {
			t->func(t->arg);
			if (t->active)                         /* the handler may have stopped itself */
				t->next = GameTicks + t->interval;
		}
	monsters_update();
	scripts_update();
	timed_events_update();
}

/* 27B7:0624 [asm] */
void far set_timer_arg(int n, int arg, int e, int active)
{
	Timer far *t;

	t = &Timers[n];
	t->active = active;
	t->next = GameTicks + t->interval;
	t->arg = arg;
	t->unkE = e;
}

/* 27B7:066D [asm] - (re)start or stop a timer */
void far set_timer(int n, int active)
{
	Timer far *t;

	t = &Timers[n];
	t->active = active;
	t->next = GameTicks + t->interval;
}

/* 27B7:06A8 [asm] - every 20 ticks, one of the 4 groups of monsters acts */
void far timer_monsters(int group)
{
	monsters_think(group);
}

/* 27B7:06B7 [asm] - something ends for character ch in "delay" ticks (see
 * Character.timerWhat). The character's timer (Timers[6 + ch]) is set to the
 * nearest end. mode 0: a new entry; else an entry for the same thing is
 * reused if there is one. */
void far add_char_timer(char ch, long delay, char what, char mode)
{
	Timer far *t;
	Character far *c;
	unsigned long when;
	int  n, found;
	int  i, slot;

	n = ch + 6;
	when = GameTicks + delay;
	t = &Timers[n];
	c = &Party[ch];
	if (!t->active) {
		t->interval = delay;
		c->timers[0] = when;
		c->timerWhat[0] = what;
		set_timer(n, 1);
	} else {
		if (t->next > when)
			t->next = when;
		if (!mode) {
			for (i = 0; i < 10; i++)
				if (c->timers[i] == 0) {
					c->timers[i] = when;
					c->timerWhat[i] = what;
					break;
				}
		} else {
			for (found = 0, slot = -1, i = 0; i < 10 && !found; i++) {
				if (slot == -1 && c->timers[i] == 0)
					slot = i;
				if (found == 0 && c->timerWhat[i] == what) {
					slot = i;
					found = 1;
				}
			}
			/* ORIGINAL BUG: with ten entries in use and none for "what", slot
			 * is still -1: timers[-1] and timerWhat[-1] overwrite the last two
			 * inventory slots and the end of timers[9] */
			c->timers[slot] = when;
			c->timerWhat[slot] = what;
		}
	}
}

/* 27B7:083B [asm] - a hand can be used again (kind 0: attack, 1: ..., 2: ...) */
void far hand_ready(int ch, int kind, int hand)
{
	switch (kind) {
	case 0:
	case 2:
		Party[ch].busy ^= 1 << hand;
	case 1:                                       /* (cases 0 and 2 go on here) */
		Party[ch].handResult[hand] = 0;
		draw_portrait(ch);
	}
}

/* 27B7:0885 [asm] - the character's timer: everything that ends now ends,
 * then the timer is set for the next end */
void far timer_character(int ch)
{
	unsigned long min, when, delay;
	Character far *c;
	int  i, what;

	c = &Party[ch];
	for (i = 0; i < 10; i++) {
		when = c->timers[i];
		if (when && when <= GameTicks) {
			c->timers[i] = 0;
			what = c->timerWhat[i];
			if (what < 0)
				end_spell_effect(-what, ch, 1);
			else
				switch (c->timerWhat[i]) {
				case 0:
					hand_ready(ch, 0, 0);
					break;
				case 1:
					hand_ready(ch, 0, 1);
					break;
				case 2:
					hand_ready(ch, 1, 0);
					if (c->effects & 0x10000L)            /* haste */
						delay = 9;
					else
						delay = 36;
					add_char_timer(ch, delay, 4, 1);
					break;
				case 3:
					hand_ready(ch, 1, 1);
					if (c->effects & 0x10000L)
						delay = 9;
					else
						delay = 36;
					add_char_timer(ch, delay, 5, 1);
					break;
				case 4:
					hand_ready(ch, 2, 0);
					break;
				case 5:
					hand_ready(ch, 2, 1);
					break;
				case 6:
					c->lastDamage = 0;
					draw_portrait(ch);
					break;
				case 7:
					sprintf(Scratch, "%s no longer has giant strength.\r", c->name);
					gui_printf(Scratch);
					c->str = c->strMax;
					c->strExt = c->strExtMax;
					if (Var_EE3A == 2)
						draw_portrait(ch);
					break;
				case 12:
					c->effects &= ~0x10000L;
					gui_printf("%s slows down.\r", c->name);
					draw_portrait(ch);
					break;
				case 8:                               /* poison: 1d5? damage every 546 ticks */
					if (c->flags & 2) {
						inflict_damage(ch, 0, 0, 5, 0x400, 5, 3);
						add_char_timer(ch, 0x222L, 8, 1);
					} else {
						c->flags &= ~2;
						draw_portrait(ch);
					}
					break;
				case 9:
					if (c->flags & 4) {
						sprintf(Scratch, "\x06\x04%s is no longer paralyzed!\r", c->name);
						gui_printf(Scratch);
						c->flags &= ~4;
						draw_portrait(ch);
					}
					break;
				case 11:
					if (c->busy & 4) {
						c->busy &= ~4;
						if (BookCaster == ch && BookOpen)
							draw_spell_book();
					}
					break;
				}
		}
	}
	min = 0xFFFFFFFFL;
	for (i = 0; i < 10; i++)
		if (c->timers[i] < min && c->timers[i] != 0)
			min = c->timers[i];
	if (min != 0xFFFFFFFFL)
		Timers[ch + 6].interval = min - GameTicks;
	else {
		Timers[ch + 6].interval = min;
		set_timer(ch + 6, 0);
	}
}

/* 27B7:0C75 [asm] - set the timer of every character to his nearest end
 * (after a game is loaded...) */
void far refresh_char_timers(void)
{
	Character far *c;
	Timer far *t;
	int  n;
	unsigned long min;
	int  i, ch;

	for (ch = 0, c = Party; ch < 6; ch++, c++) {
		if (char_is_active(ch, 1)) {
			min = 0xFFFFFFFFL;
			for (i = 0; i < 10; i++)
				if (c->timers[i] < min && c->timers[i] != 0)
					min = c->timers[i];
			n = ch + 6;
			t = &Timers[n];
			if (min != 0xFFFFFFFFL) {
				t->interval = min - GameTicks;
				set_timer(n, 1);
			} else {
				t->interval = min;
				set_timer(n, 0);
			}
		}
	}
}

/* 27B7:0D76 [asm] - forget everything of kind "what" pending for a character */
void far remove_char_timer(int ch, int what)
{
	Character far *c;
	int  i;

	c = &Party[ch];
	for (i = 0; i < 10; i++)
		if (c->timerWhat[i] == what) {
			c->timerWhat[i] = 0;
			c->timers[i] = 0;
		}
	refresh_char_timers();
}

/* 27B7:0DCF [asm] - everybody alive gets a bit hungrier every game minute */
void far timer_hunger(int arg)
{
	Character far *c = Party;
	int  i;

	for (i = 0; i < 6; i++, c++)
		/* ORIGINAL BUG: has_ring() answers 1 or 0, so ">= 0" is always true:
		 * the ring of sustenance (kind 2) never stops the hunger */
		if (has_ring(c, 2) >= 0 && c->food != 0 && (c->flags & 1) && c->hp > -10) {
			c->food--;
			draw_food(i);
		}
}

/* 27B7:0E2C [asm] - idle monsters fidget: one time in two, a new offset of
 * 1-2 pixels in x (high nibble) and y (low nibble) */
void far timer_monster_idle(int arg)
{
	Monster far *m;
	int  i;

	for (i = 0, m = Monsters; i < 30; i++, m++)
		if (m->mode != 10 && m->mode != 7 && (m->flags & 0x20) == 0 && roll_dice(1, 2, 0) == 1) {
			m->idle = roll_dice(1, 2, 0) << 4;
			m->idle += roll_dice(1, 2, 0);
			block_changed(m->block);
		}
}

/* 27B7:0EC1 [asm] - after the party moved: the missiles flying over its
 * new square may hit it */
void far missiles_check(void)
{
	Missile far *t;
	int  i, hit;

	for (t = Thrown, i = 0; i < 10; i++, t++)
		if (t->active) {
			hit = missile_hit(t, t->block, t->pos);
			if (hit)
				missile_stop(t);
		}
}
