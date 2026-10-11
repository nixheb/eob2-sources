/*
 * Eye of the Beholder II - START.EXE
 * Saving and loading: the 6 games (EOBDATA0-5.SAV) and the levels already
 * visited (LEVELS.TMP); overlay module 459A (Ghidra 559A).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y save.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A level, as kept in LEVELS.TMP (2130 bytes, at (level - 1) * 2130): the
 * walls of its 1024 squares packed with LCW (1200 bytes at most), its 30
 * monsters (900 bytes), its 5 walls of force (30 bytes, times made relative).
 * A saved game: its name (20 bytes), the 6 characters, the state of the
 * level (92 bytes from CurrentLevel), the 600 items, all of LEVELS.TMP
 * (17 levels), 3 bytes of options, 96 bytes (DS:979E).
 */
#include "eob2.h"

typedef unsigned char byte;
typedef struct MovingDoor { int block; signed char dir; } MovingDoor;

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern Character      Party[];           /* E608 */
extern Item           Items[];           /* 71CA */
extern Monster        Monsters[];        /* 6E2A */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern ForceWall      ForceWalls[5];     /* 9346 */
extern Missile        Thrown[10];        /* CC5C */
extern MovingDoor     MovingDoors[3];    /* 929C */
extern unsigned char  WallTypeFlags[256];   /* 2268 */
extern signed char    FrontPos[4][4];    /* 2342 */
extern unsigned long  GameTicks;         /* 6684 */
extern unsigned long  LevelsVisited;     /* 6DD8 bit n-1: level n has a saved state */
extern int            CurrentLevel;      /* 6DCE */
extern int            CurrentSubLevel;   /* 6DD0 */
extern int            PartyBlock;        /* 6DD2 */
extern int            PartyDirection;    /* 6DD4 */
extern int            HandItem;          /* 6DD6 */
extern int            SoundEnabled;      /* 65FA */
extern int            Var_65FC;          /* 65FC (saved with the options) */
extern char           HpBars;            /* E4C5 */
extern byte           Var_979E[0x60];    /* 979E */
extern char           Var_9316;          /* 9316 0: a new game, -1 loading, 1 loaded */
extern char           SceneDirty;        /* D0AA */
extern unsigned char  InventoryMode;     /* 6655 */
extern char           SaveMask;          /* EE3E bit n: EOBDATAn.SAV exists */
extern char           SaveNames[6][20];  /* EE3F */
extern int            MenuX, MenuY;      /* EEEC, EEEE the slot menu */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern char           Scratch[];         /* F16E */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern byte far      *WorkPage;          /* DF3A page 3 */
extern byte far      *GameFont;          /* 9342 */
extern unsigned char  FrameFlat;         /* E606 */
extern int            ClickDown[2];      /* 661A */
extern int            ClickUp[2];        /* 6622 */
extern char           Keys_2554[];       /* 2554 */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);    /* 657A fills */
extern byte far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);    /* 6592 cuts a shape */

int  compress_lcw(void far *src, void far *dst, int n);          /* 3248:000A (assembler) */
unsigned unpack_lcw(void far *src, void far *dst, int max);      /* 33EC:0004 (assembler) */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
void farfree(void far *p);                                       /* 1000:7FF2 */
int  read_key(void);                                             /* 3295:066D */
int  key_pressed(void);                                          /* 3295:041E */
void quit(int code);                                             /* 18D7:0AC3 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void far *normalize(void far *p);                                /* 3374:0071 */
void clock_start(int n);                                         /* 3187:0009 */
void clock_stop(int n);                                          /* 3187:003D */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
long write_file(int fd, void far *buf, long n);                  /* 2DCE:05DE */
long seek_file(int fd, long pos, int from);                      /* 2DCE:0750 */
int  file_on_disk(char far *name);                               /* 2DCE:09B6 */
void drives_search_off(void);                                    /* 2DCE:15EC */
void drives_search_on(void);                                     /* 2DCE:15F6 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void clip_window(int page, int x, int y, int w, int h);          /* 310C:0009 */
void message_box(int style, int text, char far *button, char far *keys, int colour);   /* 1E24:0E83 */
int  ask_yes_no(int style, int text, int no);                    /* 1E24:1027 */
int  edit_line(char far *text, char x, char y, char max, char fg, char bg, char cursorBg);   /* 18D7:0004 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void load_picture(char far *name, byte far *pal, int a, int page, int mode);   /* 18D7:05C8 */
void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
void load_level(int level, int sublevel);                        /* 53EC:17F5 (overlay) */
void draw_play_screen(int scene);                                /* 53EC:1A19 (overlay) */
byte far *set_font(byte far *font);                              /* 3166:000A */
void redraw_party(void);                                         /* 1E24:0686 */
void take_item(int item);                                        /* 18AE:0197 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
void run_block_script(int block, unsigned events);               /* 24EC:2BE9 */
void draw_frame(int x, int y, int w, int h, int pressed, int hollow);   /* 1E24:0AB2 */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);      /* 18D7:067F */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
int far restore_game(int slot);
int far write_save(int slot);
void far leave_level(void);
void far read_save_names(void);
int far slot_menu(int x, int y, int mode);

/* Initialised data of the module (DS:1346...) */
long SaveSizes[7] = { 20, 0x816, 0x5C, 0x20D0, 0x8D72, 3, 0x60 };      /* 1346 the parts of a saved game */

/* 559A:0000 [asm] - the current level into dst (LEVELS.TMP format) */
void far pack_level(byte far *dst)
{
	byte far *p;
	byte far *q;
	int n;
	register int i, k;

	p = dst + 1200;
	q = ShrinkBuffer;
	for (i = 0; i < 1024; i++)
		for (k = 0; k < 4; k++) {
			*q = LevelMap[i].walls[k];
			q++;
		}
	n = compress_lcw(ShrinkBuffer, dst, 0x1000);
	if (n > 1200) {
		Var_62A5 = 0;
		sprintf(Scratch, "Map compression failure: size of map = %d", n);
		Drv_65F2(Scratch, 0, 0, 15, 0);
		read_key();
		quit(0);
	}
	mem_copy(Monsters, p, 900L);
	p += 900;
	clock_stop(2);
	for (i = 0; i < 5; i++)
		if (ForceWalls[i].expires > GameTicks)
			ForceWalls[i].expires -= GameTicks;
		else
			ForceWalls[i].expires = 1;
	mem_copy(ForceWalls, p, 30L);
	for (i = 0; i < 5; i++)
		ForceWalls[i].expires += GameTicks;
	clock_start(2);
}

/* 559A:0186 [asm] - the reverse; the monsters are counted in their squares */
void far unpack_level(byte far *src)
{
	byte far *p;
	byte far *q;
	register int i, k;

	q = ShrinkBuffer;
	p = src + 1200;
	unpack_lcw(src, q, 0x1000);
	for (i = 0; i < 1024; i++)
		for (k = 0; k < 4; k++) {
			LevelMap[i].walls[k] = *q;
			q++;
		}
	mem_copy(p, Monsters, 900L);
	for (i = 0; i < 30; i++)
		LevelMap[Monsters[i].block].flags++;
	p += 900;
	mem_copy(p, ForceWalls, 30L);
	clock_stop(2);
	for (i = 0; i < 5; i++)
		ForceWalls[i].expires += GameTicks;
	clock_start(2);
}

/* 559A:0288 [asm] */
void far restore_level_state(int level)
{
	register int fd;

	fd = open_file("LEVELS.TMP", 1);
	seek_file(fd, (level - 1) * 2130U, 0);
	read_file(fd, WorkPage, 2130L);
	close_file(fd);
	unpack_level(WorkPage);
}

/* 559A:02EA [asm] */
void far save_level(int level)
{
	byte far *buf;
	register int fd;

	buf = normalize(WorkPage + 0xC800);
	pack_level(buf);
	fd = open_file("LEVELS.TMP", 3);
	seek_file(fd, (level - 1) * 2130U, 0);
	write_file(fd, buf, 2130L);
	close_file(fd);
	LevelsVisited |= 1 << (level - 1);
}

/* 559A:037A [asm] - the timers of the characters, made relative to now */
void far timers_to_relative(void)
{
	Character far *c;
	register int k, ch;

	c = Party;
	for (ch = 0; ch < 6; ch++, c++)
		if (char_is_active(ch, 1))
			for (k = 0; k < 10; k++)
				if (c->timers[k] > GameTicks)
					c->timers[k] -= GameTicks;
				else if (c->timers[k])
					c->timers[k] = 1;
}

/* 559A:042F [asm] */
void far timers_to_absolute(void)
{
	Character far *c;
	register int k, ch;

	c = Party;
	for (ch = 0; ch < 6; ch++, c++)
		if (char_is_active(ch, 1))
			for (k = 0; k < 10; k++)
				if (c->timers[k])
					c->timers[k] += GameTicks;
	refresh_char_timers();
}

/* 559A:04A7 [asm] - the menu of the saved games at (x, y): 1 if loaded */
int far load_game(register int x, int y)
{
	int slot, ox, oy, ok;
	register int bad;

	ox = Dims[11].x;
	oy = Dims[11].y;
	clip_window(11, ox + (x >> 3), oy + y, Dims[11].w, Dims[11].h);
	read_save_names();
	do {
		slot = slot_menu(x, y, 1);
		bad = slot != 6 && !(1 << slot & SaveMask);
		if (bad)
			message_box(11, 0x41, "OK", Keys_2554, 6);
	} while (bad);
	if (slot == 6)
		ok = 0;
	else {
		ok = restore_game(slot);
		if (!ok)
			message_box(11, 0x10, "OK", Keys_2554, 6);
	}
	clip_window(11, ox, oy, Dims[11].w, Dims[11].h);
	return ok;
}

/* 559A:05A4 [asm] - save in a slot, with a name typed in: 1 if saved */
int far save_game(int x, int y)
{
	int slot, ok, ox, oy, ly, result;
	register int lx, len;

	ox = Dims[11].x;
	oy = Dims[11].y;
	clip_window(11, ox + (x >> 3), oy + y, Dims[11].w, Dims[11].h);
	read_save_names();
	do {
		slot = slot_menu(x, y, 0);
		ok = slot == 6 || !(1 << slot & SaveMask);
		if (!ok)
			ok = ask_yes_no(11, 0x37, 1);
	} while (!ok);
	if (slot == 6)
		result = 0;
	else {
		ly = y + slot * 17 + 23;
		lx = (x + 1) * 8;
		do {
			do {
				hide_mouse();
				Drv_657A(lx - 2, ly, lx + 0xA0, ly + 8, FrameFlat);
				len = edit_line(SaveNames[slot], x + 1, ly, 19, 2, 0, 8);
				show_mouse();
			} while (len == -1);
			if (!strlen(SaveNames[slot]))
				message_box(11, 0x36, "OK", Keys_2554, 6);
		} while (!strlen(SaveNames[slot]));
		hide_mouse();
		Drv_657A(lx - 2, ly, lx + 0xA0, ly + 8, FrameFlat);
		print_text(SaveNames[slot], (x + 1) * 8, ly, 15, 0);
		show_mouse();
		result = write_save(slot);
		if (!result) {
			message_box(11, 0x0F, "OK", Keys_2554, 6);
			message_box(11, 0x42, "OK", Keys_2554, 6);
		}
	}
	clip_window(11, ox, oy, Dims[11].w, Dims[11].h);
	return result;
}

/* 559A:07E5 [asm] - write EOBDATAn.SAV (slot -1: FINAL.SAV): 1 if all
 * was written */
int far write_save(int slot)
{
	int lvl;
	long n;
	register int ok, fd;

	lvl = CurrentLevel - 1;
	leave_level();
	fd = open_file("LEVELS.TMP", 1);
	read_file(fd, WorkPage, 0x8D72L);
	close_file(fd);
	pack_level(WorkPage + lvl * 2130);
	LevelsVisited |= 1 << lvl;
	if (slot != -1)
		sprintf(Scratch, "EOBDATA%d.SAV", slot);
	else
		strcpy(Scratch, "FINAL.SAV");
	fd = open_file(Scratch, 2);
	ok = 1;
	if (ok) {
		n = 20;
		ok &= write_file(fd, SaveNames[slot], n) == n;
		timers_to_relative();
		n = 0x816;
		ok &= write_file(fd, Party, n) == n;
		timers_to_absolute();
		n = 0x5C;
		ok &= write_file(fd, &CurrentLevel, n) == n;
		n = 0x20D0;
		ok &= write_file(fd, Items, n) == n;
		n = 0x8D72;
		ok &= write_file(fd, WorkPage, n) == n;
		ok &= write_file(fd, &Var_65FC, 1L) == 1;
		ok &= write_file(fd, &SoundEnabled, 1L) == 1;
		ok &= write_file(fd, &HpBars, 1L) == 1;
		n = 0x60;
		ok &= write_file(fd, Var_979E, n) == n;
		close_file(fd);
		load_picture("INVENT", 0L, 5, 3, 2);
	}
	return ok;
}

/* 559A:0A80 [asm] - read EOBDATAn.SAV: 1 if loaded */
int far restore_game(int slot)
{
	int fd, ok, redo, pos;
	char far *p;
	byte far *shape;
	Character far *c;
	byte far *faces[6];
	register int i, k;

	p = WorkPage;
	sprintf(Scratch, "EOBDATA%d.SAV", slot);
	fd = open_file(Scratch, 1);
	ok = 1;
	if (ok) {
		for (i = 0; i < 7; i++) {
			ok &= read_file(fd, p, SaveSizes[i]) == SaveSizes[i];
			p += SaveSizes[i];
		}
		close_file(fd);
	}
	if (ok) {
		leave_level();
		p = WorkPage + 20;
		for (k = 0; k < 6; k++)
			faces[k] = Party[k].face;
		mem_copy(p, Party, SaveSizes[1]);
		p += SaveSizes[1];
		for (k = 0; k < 6; k++)
			Party[k].face = faces[k];
		timers_to_absolute();
		mem_copy(p, &CurrentLevel, SaveSizes[2]);
		p += SaveSizes[2];
		mem_copy(p, Items, SaveSizes[3]);
		p += SaveSizes[3];
		fd = open_file("LEVELS.TMP", 1);
		close_file(fd);
		fd = open_file("LEVELS.TMP", 2);
		write_file(fd, p, SaveSizes[4]);
		close_file(fd);
		p += SaveSizes[4];
		Var_65FC = *p++;
		SoundEnabled = *p++;
		HpBars = *p++;
		mem_copy(p, Var_979E, SaveSizes[6]);
		p += SaveSizes[6];
		load_cps("CHARGENA", 3, 3);
		redo = 0;
		for (i = 0; i < 6; i++) {
			c = &Party[i];
			if (c->flags && c->portrait >= 0) {
				shape = Drv_6592(c->portrait % 10 * 4, c->portrait / 10 * 32, 4, 32, 0L);
				mem_copy(shape, c->face, 1400L);
				farfree(shape);
			} else
				redo = 1;
		}
		if (redo) {
			load_cps("OUTPORTS", 5, 3);
			redo = 0;
			for (i = 0; i < 6; i++) {
				c = &Party[i];
				if (c->flags && c->portrait < 0) {
					shape = Drv_6592(-(c->portrait + 1) * 4, 0, 4, 32, 0L);
					mem_copy(shape, c->face, 1400L);
					farfree(shape);
				}
			}
		}
		if (Var_9316 != -1) {
			load_level(CurrentLevel, CurrentSubLevel);
			draw_play_screen(0);
			SceneDirty = 1;
			Var_62A5 = 0;
			set_font(GameFont);
			redraw_party();
			if (!InventoryMode)
				take_item(HandItem);
			else if (HandItem) {
				pos = FrontPos[PartyDirection][roll_dice(1, 2, -1)];
				put_item(&LevelMap[PartyBlock].items, PartyBlock, HandItem, pos);
			}
		}
		Var_9316 = 1;
	} else
		Var_9316 = 0;
	return ok;
}

/* 559A:0EC5 [asm] - before the level is kept: the flying items fall, the
 * moving doors end their move */
void far leave_level(void)
{
	int dir, side;
	register int i, block;

	for (i = 0; i < 30; i++)
		Monsters[i].attack = 0;
	for (i = 0; i < 10; i++) {
		if (Thrown[i].active == 1) {
			Items[Thrown[i].item].pos &= 3;
			run_block_script(Items[Thrown[i].item].block, 4);
			Thrown[i].active = 0;
		}
		if (Thrown[i].active == 2)
			Thrown[i].active = 0;
	}
	for (i = 0; i < 3; i++) {
		block = MovingDoors[i].block;
		dir = MovingDoors[i].dir;
		if (block) {
			side = WallTypeFlags[LevelMap[block].walls[0]] & 8 ? 0 : 1;
			while ((WallTypeFlags[LevelMap[block].walls[side]] & 0x30) == 0) {
				LevelMap[block].walls[side] = LevelMap[block].walls[side] + dir;
				LevelMap[block].walls[side + 2] = LevelMap[block].walls[side + 2] + dir;
			}
			MovingDoors[i].block = 0;
		}
	}
}

/* 559A:1037 [asm] - which slots are used, and their names */
void far read_save_names(void)
{
	int i;
	register int fd;

	drives_search_off();
	for (i = 0; i < 6; i++) {
		sprintf(Scratch, "EOBDATA%d.sav", i);
		if (file_on_disk(Scratch)) {
			SaveMask |= 1 << i;
			fd = open_file(Scratch, 1);
			read_file(fd, SaveNames[i], 20L);
			close_file(fd);
		} else {
			SaveMask &= ~(1 << i);
			memset(SaveNames[i], 0, 20);
		}
	}
	drives_search_on();
}

char far *SlotTexts[4] = { "Cancel", "Empty Slot", "Save Game", "Load Game" };   /* 1362 */

/* 559A:10F1 [asm] - a line of the slot menu (6: Cancel); state 0: only
 * the text, 1 raised, 2 pressed */
void far draw_slot(int slot, int state, char colour)
{
	int w, h;
	char far *text;
	register int x, y;

	if (slot < 0)
		return;
	if (slot < 6) {
		x = MenuX + 4;
		y = MenuY + slot * 17 + 20;
		w = 0xA7;
		h = 14;
		if (1 << slot & SaveMask)
			text = SaveNames[slot];
		else
			text = SlotTexts[1];
	} else {
		x = MenuX + 0x76;
		y = MenuY + 0x7E;
		w = 0x35;
		h = 14;
		text = SlotTexts[0];
	}
	if (state)
		draw_frame(x, y, w, h, state - 1, 0);
	hide_mouse();
	print_text(text, x + 4, y + 3, colour, 0);
	show_mouse();
}

/* 559A:11D8 [asm] - the line under (x, y), -1 none */
int far slot_at(int x, int y)
{
	int found;
	register int i, sy;

	if (InventoryMode)
		return -1;
	found = -1;
	for (i = 0; i < 6; i++) {
		sy = MenuY + i * 17 + 20;
		if (in_rect(x, y, MenuX + 4, sy, MenuX + 0xA7, sy + 14)) {
			found = i;
			break;
		}
	}
	if (in_rect(x, y, MenuX + 0x76, MenuY + 0x7E, MenuX + 0xAB, MenuY + 0x8C))
		found = 6;
	return found;
}

/* 559A:1275 [asm] - the slot menu (mode 0 save, 1 load): the line chosen,
 * 6 Cancel */
int far slot_menu(int x, int y, int mode)
{
	int shown, done, key, hit;
	register int sel, i;

	hide_mouse();
	Var_62A5 = 2;
	MenuX = MenuY = 0;
	draw_frame(0, 0, 0xB0, 0x90, 0, 0);
	print_text(SlotTexts[mode + 2], 0x34, 5, 15, 0);
	for (i = 0; i < 7; i++)
		draw_slot(i, 1, 15);
	copy_page(0, 0, x >> 3, y, 22, 0x90, 2, 0);
	show_mouse();
	Var_62A5 = 0;
	MenuX = x;
	MenuY = y;
	shown = -1;
	sel = 0;
	done = 0;
	do {
		if (!key_pressed()) {
			if ((hit = slot_at(ClickDown[0], ClickDown[1])) != -1)
				sel = hit;
		} else {
			key = read_key() & 0x8FF;
			switch (key) {
			case 0x41:
				if ((hit = slot_at(ClickUp[0], ClickUp[1])) != -1) {
					sel = hit;
					done = 1;
				}
				break;
			case 0x60:
				if (--sel < 0)
					sel = 6;
				break;
			case 0x62:
				if (++sel > 6)
					sel = 0;
				break;
			case 0x2B:
			case 0x3D:
				done = 1;
			}
		}
		if (shown != sel) {
			draw_slot(shown, 0, 15);
			draw_slot(sel, 0, 6);
			shown = sel;
		}
	} while (!done);
	draw_slot(sel, 2, 6);
	wait_ticks(5);
	draw_slot(sel, 1, 6);
	return sel;
}
