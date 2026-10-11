/*
 * Eye of the Beholder II - START.EXE
 * Loading a level: overlay module 43EC (Ghidra 53EC), stub segment 2AF4.
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y loadlev.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * LEVELx.INF (LCW compressed, see tools/westwood.py) - every byte of the 16
 * files is accounted for by this format (checked by tools/infheader.py):
 *
 *   +0   u16 M                 offset of the monster section
 *   +2   sub-headers           one per variant of the level ("sub-level"),
 *                              each starting with the offset of the next one
 *                              (relative to +2), then:
 *        [EC maze[13] walls[13] (FF | xx palette[13])]  sounds[13]
 *        2 x door              EC|EA cps[13] slot type ? rects[3][8] 2 x (rect[8] x.w y.w) | 1 byte
 *        u16
 *        2 x monster graphics  EC loader slot cps[13] ? | 1 byte
 *        monster types         id ... (35 bytes, + 2 + 2n) ... FF
 *        decorations           EC count.w, then EC dec[13] cps[13] | xx type set deco ? flags
 *        timed events          (u16, u16 seconds) ... FFFF
 *        FFFF                  (never read)
 *   +M   u16 T                 offset of the trigger table
 *        EC (group, ticks)... FF + 30 x 14 bytes   monster placement (or 1 byte)
 *        bytecode              -> ScriptCode (texts at the end, see script.c)
 *   +T   u16 n, n x (block.w, events.w, script.w)
 */
#include "eob2.h"

typedef unsigned char byte;

typedef struct Decoration {
	byte          shape[10];     /* +0 the shape of each place, FF = none */
	char          next;          /* +A the next decoration of the chain */
	byte          flags;         /* +B */
	int           x[10], y[10];  /* +C */
} Decoration;

extern Decoration     Decorations[60];   /* D1CC */
extern int            DecorationCount;   /* E5C0 */
extern int far       *DecoRects;         /* D1C0 x, y, w, h of each decoration shape (.DEC) */
extern char far      *DecoShapes[300];   /* DF50 */
extern signed char    WallSet[256];      /* 2178 */
extern char           WallDecoration[256];   /* 21C8 */
extern char           WallClickType[256];   /* 2218 */
extern char           WallTypeFlags[256];   /* 2268 */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern char far *(far *Drv_658E)(int x, int y, int w, int h, void far *buf);   /* 658E cuts a shape */


typedef struct DoorButton { char far *shape; byte x, y; } DoorButton;
extern DoorButton     DoorButtons[6];    /* 931E the knobs of the doors */
extern char far      *DoorShapes[6];    /* EF06 slot * 3 + size */
typedef struct Accessory { char far *shape; char x, y; } Accessory;
extern Accessory      MonsterAccessories[36];   /* 936C */
extern char far      *MonsterGfx[36];   /* A2D0 slot * 18 + picture - 1 */
extern byte far      *Var_9318;          /* 9318 the colour variants of the monsters */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern char           Scratch[];         /* F16E */
extern int            Var_62A5;          /* 62A5 the page drawn on */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
int  file_on_disk(char far *name);                               /* 2DCE:09B6 */
int  set_page(int page);                                         /* 310B:0001 */
byte far *alloc_page(int page);                                  /* 2F3E:0009 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
extern char far      *WorkPage;          /* DF3A page 3 */
extern unsigned far  *WallLayouts;       /* E4CE */
extern byte           Remap_E566[16];    /* E566 the colours of the wall tiles */
extern byte           Remap_EEF6[16];    /* EEF6 the colours of the background tiles */
extern byte far      *VcnTiles;          /* CB72 */
extern int            VideoMode;         /* 628A */
extern unsigned       PageSegs[];        /* 62A7 */
extern Item           Items[600];        /* 71CA */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern int            CurrentLevel;      /* 6DCE */
extern byte far *(far *Drv_65E6)(byte far *table);   /* 65E6 the colour table of the mode */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
void load_cps_file(char far *name, int page, int dest, byte far *pal);   /* 3069:0044 */
void set_vcn_colours(byte far *walls, byte far *back);           /* 2A68:0380 (assembler) */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
typedef struct TimedEvent { int what; unsigned interval; unsigned long next; } TimedEvent;
extern TimedEvent     TimedEvents[];     /* CBC8 */
extern int            TimedEventCount;   /* E4C8 */
extern unsigned long  GameTicks;         /* 6684 */
extern unsigned long  LevelsVisited;     /* 6DD8 bit n-1: level n has a saved state */
extern MonsterType    MonsterTypes[];    /* CD1A */
void restore_level_state(int level);                             /* 559A:0288 (overlay) */
extern byte far      *Palette;           /* F168 */
extern byte far      *PoisonRemap;       /* EEE4 colour tables (256 bytes each) */
extern byte far      *InvisibleRemap;    /* EEE8 */
extern byte far      *StoneRemap;        /* EE30 */
extern byte far      *MagicPalette;      /* CC26 */
extern byte far      *NormalPalette;     /* CD12 */
/* Uninitialised data of the module (DS:6D44-6D5F, _BSS), in address order */
char VcnName[15];     /* 6D44 the .VCN loaded */
char WallSetName[13]; /* 6D53 */
extern int            StepCounter;       /* E5C8 */
extern int            Var_EE1E;          /* EE1E */
extern int            DoorKind[2];       /* 9366 */
extern int            DoorNoKnob[2];     /* EE34 */
extern void (far     *Drv_6552)(byte far *pal, byte far *table, int a, int b);   /* 6552 makes a colour table */
void play_sound(int sound);                                      /* 2C9F:00AA */
void load_sounds(char far *name);                                /* 2C9F:0009 */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
long load_file_into(char far *name, void far *buf, long n);      /* 3069:0000 */
void set_palette(byte far *pal);                                 /* 3111:000D */
typedef struct Timer {
	void (far *func)(int arg);   /* +0 */
	unsigned long interval;      /* +4 in game ticks */
	unsigned long next;          /* +8 */
	int           arg;           /* +C */
	char          unkE, active;  /* +E */
} Timer;
extern Timer          Timers[16];        /* 5BFC (2-5: the monster groups) */
extern Monster        Monsters[30];      /* 6E2A */
extern int            CurrentSubLevel;   /* 6DD0 */
extern byte far      *ScriptCode;        /* 71C4 */
extern char           FadeInPending;     /* EE3B */
void create_monster_at(int n, int unit, int block, int pos, int dir, int type, int slot, int mode, int wait, int drop1, int drop2);   /* 1997:1642 */
long unpack_picture(byte far *src, void far *dst);               /* 3069:0186 */
void load_picture(char far *name, byte far *pal, int a, int page, int mode);   /* 18D7:05C8 */
void clock_stop(int), clock_start(int);    /* 3187:003D / 0009 */
void draw_compass(int force);                                    /* 4A5D:1F7A (overlay) */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void farfree(void far *p);                                       /* 1000:7FF2 */
void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
void movmem(void far *src, void far *dest, unsigned n);
int  printf(char far *fmt, ...);
void exit(int status);                                           /* 1000:5A8D */
void system_shutdown(void);                                      /* 315F:0000 */
void far free_decorations(void);
void far load_decoration_file(char far *cps, char far *dec);
int far bit_set(char flags, int bit);
void far wall_type_setup();   /* defined in the old style: its callers pass ints */

void far free_door_shapes(void);
int far * far load_door_shapes(char far *cps, int slot, int far *p);
void far free_monster_shapes(int first, int count);
void far load_monster_accessories(char far *name, int base);
void far make_monster_colour_variants(char far *cps, int base);void far load_monster_gfx_a(char far *cps, int base, int acc);
void far load_monster_gfx_b(char far *cps, int base, int acc);
void far load_monster_gfx_c(char far *cps, int base, int acc);
void far expand_vcn(int split);
void far make_tile_mask(byte far *src, byte far *dst);
void far load_vcn(char far *name, char far *p);
void far relink_items(void);
void far load_maze(char far *name);
char far * far read_monster_types(char far *p);
int far * far read_timed_events(int far *p);
void far set_wall_flag2(int base, int on);
char far * far parse_level_header(char far *inf, int sub);
char far * far place_monsters(char far *p, int level);
void far load_level(int level, int sub);
void far draw_play_screen(int scene);


/* Initialised data of the module (DS:110A): x, y, w, h of the 6 pictures of
 * a monster graphics in its .CPS, for the 3 kinds of monster sizes */
int MonsterRectsA[72] = {                /* 110A */
	0, 0, 7, 96,
	7, 40, 12, 56,
	19, 40, 12, 56,
	31, 0, 7, 96,
	0, 96, 7, 96,
	7, 96, 7, 96,
	14, 96, 5, 56,
	7, 0, 7, 40,
	14, 0, 7, 40,
	19, 96, 5, 56,
	24, 96, 5, 56,
	29, 96, 5, 56,
	14, 152, 3, 32,
	17, 160, 4, 24,
	21, 160, 4, 24,
	25, 152, 3, 32,
	28, 152, 3, 32,
	31, 152, 3, 32
};
int MonsterRectsB[72] = {                /* 119A */
	0, 0, 7, 96,
	7, 0, 7, 96,
	14, 0, 7, 96,
	21, 0, 7, 96,
	28, 0, 7, 96,
	0, 96, 7, 96,
	7, 96, 5, 56,
	12, 96, 5, 56,
	17, 96, 5, 56,
	22, 96, 5, 56,
	27, 96, 5, 56,
	32, 96, 5, 56,
	7, 152, 3, 32,
	10, 152, 3, 32,
	13, 152, 3, 32,
	16, 152, 3, 32,
	19, 152, 3, 32,
	22, 152, 3, 32
};
int MonsterRectsC[24] = {                /* 122A */
	0, 0, 10, 88,
	10, 0, 10, 88,
	20, 0, 10, 88,
	30, 0, 10, 88,
	0, 88, 10, 88,
	10, 88, 10, 88
};
void (far *MonsterLoaders[3])(char far *cps, int base, int acc) = {   /* 125A */
	load_monster_gfx_a, load_monster_gfx_b, load_monster_gfx_c
};
/* 53EC:0000 Frees the shapes of the decorations */
void far free_decorations(void)
{
	register int i;

	for (i = 0; i < 300; i++)
		if (DecoShapes[i]) {
			farfree(DecoShapes[i]);
			DecoShapes[i] = 0;
		}
	DecorationCount = 0;
}

/* 53EC:0050 Reads a .DEC file (its picture .CPS into page 3): the
 * decoration records stay in LoadBuffer + 16000, then the rectangles of the
 * shapes (DecoRects); wall_type_setup copies what the level uses */
void far load_decoration_file(char far *cps, char far *dec)
{
	long size;
	int count, rects, fd;
	Decoration far *d;
	int far *r;
	register int j, i;

	load_cps(cps, 3, 3);
	fd = open_file(dec, 1);
	read_file(fd, &count, 2L);
	size = (long)count * 52;
	read_file(fd, LoadBuffer + 16000, size);
	d = (Decoration far *)(LoadBuffer + 16000);
	for (i = 0; i < count; d++, i++)
		for (j = 0; j < 10; j++) {
			d->x[j] = d->x[j];           /* (a byte swap for other machines, empty here) */
			d->y[j] = d->y[j];
		}
	DecoRects = (int far *)(LoadBuffer + size + 16000);
	r = DecoRects;
	read_file(fd, &rects, 2L);
	read_file(fd, DecoRects, (long)rects << 3);
	for (i = 0; i < rects * 4; i++) {
		*r = *r;
		r++;
	}
	close_file(fd);
}

/* 53EC:01AA Bit bit of flags (not called) */
int far bit_set(char flags, int bit)
{
	return flags & (1 << bit) ? 1 : 0;
}

/* 53EC:01C8 A wall type of the level: its wall set, its click type, its
 * flags, and its chain of decorations (deco: the first record of the .DEC,
 * -1 none), copied into Decorations with their shapes cut from page 3 */
void far wall_type_setup(type, set, deco, click, flags)
int type;
char set;
int deco;
char click, flags;
{
	int far *r;
	register int s, i;

	WallSet[type] = set;
	WallDecoration[type] = DecorationCount + 1;
	WallClickType[type] = click;
	WallTypeFlags[type] = flags;
	WallTypeFlags[type] ^= 4;
	if (deco != -1) {
		while (deco != -1) {
			movmem(LoadBuffer + deco * 52 + 16000, &Decorations[DecorationCount], 52);
			for (i = 0; i < 10; i++) {
				s = Decorations[DecorationCount].shape[i];
				if (s == 0xFF)
					continue;
				if (DecoShapes[s])
					continue;
				r = DecoRects + s * 4;
				if (r[0] < 0 || r[1] < 0 || r[2] <= 0 || r[3] <= 0) {
					system_shutdown();
					printf("Error trying to make decoration %d\n x: %d y:%d w:%d h:%d", deco, r[0], r[1], r[2], r[3]);
					exit(0);
				}
				DecoShapes[s] = Drv_658E(r[0], r[1], r[2], r[3], 0L);
			}
			DecorationCount++;
			if ((deco = Decorations[DecorationCount - 1].next) != 0)
				Decorations[DecorationCount - 1].next = DecorationCount + 1;
			else
				deco = -1;
			if (s > 300 && s != 0xFF) {
				system_shutdown();
				printf("Too many decoration shapes loaded!\r");
				exit(0);
			}
			if (DecorationCount > 60) {
				system_shutdown();
				printf("Too many decoration types loaded!\r");
				exit(0);
			}
		}
	} else
		WallDecoration[type] = 0;
}

/* 53EC:039B Frees the shapes of the doors and of their knobs */
void far free_door_shapes(void)
{
	register int i;

	for (i = 0; i < 6; i++) {
		if (DoorShapes[i]) {
			farfree(DoorShapes[i]);
			DoorShapes[i] = 0;
		}
		if (DoorButtons[i].shape) {
			farfree(DoorButtons[i].shape);
			DoorButtons[i].shape = 0;
		}
	}
}

/* 53EC:0425 The door of slot (0 or 1): its picture into page 3, 3 shapes
 * (by distance), 2 knobs with their place; p: the rectangles in the INF.
 * Returns what follows */
int far * far load_door_shapes(char far *cps, int slot, int far *p)
{
	register int i;

	load_cps(cps, 3, 3);
	slot *= 3;
	for (i = 0; i < 3; i++) {
		DoorShapes[slot + i] = Drv_658E(p[0], p[1], p[2], p[3], 0L);
		p += 4;
	}
	for (i = 0; i < 2; i++) {
		DoorButtons[slot + i].shape = Drv_658E(p[0], p[1], p[2], p[3], 0L);
		p += 4;
		DoorButtons[slot + i].x = *p;
		p++;
		DoorButtons[slot + i].y = *p;
		p++;
	}
	Var_62A5 = 0;
	return p;
}

/* 53EC:050C Frees count monster pictures from first, with their accessories */
void far free_monster_shapes(int first, int count)
{
	register int i, end;

	end = first + count;
	for (i = first; i < end; i++) {
		if (MonsterGfx[i]) {
			farfree(MonsterGfx[i]);
			MonsterGfx[i] = 0;
		}
		if (MonsterAccessories[i].shape) {
			farfree(MonsterAccessories[i].shape);
			MonsterAccessories[i].shape = 0;
		}
	}
}

/* 53EC:05A0 The accessories of a monster graphics (name.DCR, if there is
 * one): 6 records of 6 bytes (x, y, w, h of the shape in page 3, its offset)
 * per picture, from base */
void far load_monster_accessories(char far *name, int base)
{
	int k, fd, count, n;
	byte w, h;
	register int off, j;

	sprintf(Scratch, "%s.dcr", name);
	if (!file_on_disk(Scratch))
		return;
	fd = open_file(Scratch, 1);
	read_file(fd, &count, 2L);
	read_file(fd, ShrinkBuffer, count * 36);
	close_file(fd);
	for (k = 0; k < count; k++)
		for (j = 0; j < 6; j++) {
			off = k * 36 + j * 6;
			n = base + off / 6;
			w = ShrinkBuffer[off + 2];
			h = ShrinkBuffer[off + 3];
			if (w > 0 && h > 0) {
				MonsterAccessories[n].shape = Drv_658E(ShrinkBuffer[off], ShrinkBuffer[off + 1], w, h, 0L);
				MonsterAccessories[n].x = ShrinkBuffer[off + 4];
				MonsterAccessories[n].y = ShrinkBuffer[off + 5];
			}
		}
}

/* 53EC:0700 The 2 colour variants of the 6 pictures of a monster (from
 * base): columns 302.. of the picture (lines 184-199) give, for each of the
 * 16 colours of the shape, its replacement in variant 1 and 2 */
void far make_monster_colour_variants(char far *cps, int base)
{
	byte c;
	int col, n, k, i, v, old;
	byte idx[16];
	byte pal[16];
	register int j, var;

	old = set_page(2);
	load_cps(cps, 3, 3);
	for (i = 0; i < 6; i++) {
		n = i + base;
		mem_copy(MonsterGfx[n] + 4, pal, 16L);
		col = i * 3 + 302;
		for (j = 0; j < 16; j++) {
			c = *(alloc_page(Var_62A5) + (j + 184) * 320 + col);
			for (k = 0; k < 16; k++)
				if (pal[k] == c) {
					idx[j] = k;
					break;
				}
			if (k == 16)
				idx[j] = 0;
		}
		for (var = 1; var < 3; var++) {
			mem_copy(MonsterGfx[n] + 4, pal, 16L);
			for (j = 0; j < 16; j++) {
				c = (alloc_page(Var_62A5) + (j + 184) * 320 + col)[var];
				if (idx[j])
					pal[idx[j]] = c;
			}
			v = base < 18 ? i : i + 6;
			mem_copy(pal, Var_9318 + (v << 5) + (var - 1 << 4), 16L);
		}
	}
	set_page(old);
}

/* 53EC:08AA The 6 pictures of a monster graphics from base, cut with the
 * rectangles of MonsterRectsA, its colour variants and its accessories (acc) */
void far load_monster_gfx_a(char far *cps, int base, int acc)
{
	int count;
	register int k, i;

	load_cps(cps, 3, 3);
	count = 6;
	for (i = 0, k = 0; i < count; k += 4, i++)
		MonsterGfx[base + i] = Drv_658E(MonsterRectsA[k], MonsterRectsA[k + 1], MonsterRectsA[k + 2], MonsterRectsA[k + 3], 0L);
	make_monster_colour_variants(cps, base);
	if (acc)
		load_monster_accessories(cps, base);
	Var_62A5 = 0;
}

/* 53EC:0948 The same with MonsterRectsB */
void far load_monster_gfx_b(char far *cps, int base, int acc)
{
	int count;
	register int k, i;

	load_cps(cps, 3, 3);
	count = 6;
	for (i = 0, k = 0; i < count; k += 4, i++)
		MonsterGfx[base + i] = Drv_658E(MonsterRectsB[k], MonsterRectsB[k + 1], MonsterRectsB[k + 2], MonsterRectsB[k + 3], 0L);
	make_monster_colour_variants(cps, base);
	if (acc)
		load_monster_accessories(cps, base);
	Var_62A5 = 0;
}

/* 53EC:09E6 The same with MonsterRectsC */
void far load_monster_gfx_c(char far *cps, int base, int acc)
{
	register int k, i;

	load_cps(cps, 3, 3);
	for (i = 0, k = 0; i < 6; k += 4, i++)
		MonsterGfx[base + i] = Drv_658E(MonsterRectsC[k], MonsterRectsC[k + 1], MonsterRectsC[k + 2], MonsterRectsC[k + 3], 0L);
	make_monster_colour_variants(cps, base);
	if (acc)
		load_monster_accessories(cps, base);
	Var_62A5 = 0;
}

/* 53EC:0A7B (EGA) Expands the 4-bit tiles in WorkPage to bytes, from the end,
 * through the colour table of the walls (Remap_E566) or, for the tiles of
 * the background (split, marked in ShrinkBuffer), of the background */
void far expand_vcn(int split)
{
	char far *dst;
	register int i;

	dst = WorkPage;
	dst += 63999;
	memset(ShrinkBuffer, 0, 2000);
	for (i = 0; i < 330; i++)
		ShrinkBuffer[WallLayouts[i]] = 1;
	for (i = 31999; i >= 0; i--)
		if (split && ShrinkBuffer[i / 32]) {
			*dst = Remap_E566[WorkPage[i] & 0xF];
			dst--;
			*dst = Remap_E566[(WorkPage[i] & 0xF0) >> 4];
			dst--;
		} else {
			*dst = Remap_EEF6[WorkPage[i] & 0xF];
			dst--;
			*dst = Remap_EEF6[(WorkPage[i] & 0xF0) >> 4];
			dst--;
		}
}

/* 53EC:0B6C (EGA) A 2-bit mask of the tiles: 3 where a pixel is not 0 */
void far make_tile_mask(byte far *src, byte far *dst)
{
	register unsigned i;

	for (i = 0; i < 47500; i++) {
		*dst = 0;
		if (*src & 0xF0)
			*dst |= 0xC0;
		if (*src & 0xF)
			*dst |= 0x30;
		if (src[1] & 0xF0)
			*dst |= 0xC;
		if (src[1] & 0xF)
			*dst |= 3;
		src += 2;
		dst++;
	}
}

/* 53EC:0BCE The tiles of the walls (name.VCN, or the last one: name 0) and
 * their two colour tables; in EGA they are expanded into pages 4 and 6 */
void far load_vcn(char far *name, char far *p)
{
	int seg;
	byte far *tbl;
	register int i;

	if (!name)
		strcpy(Scratch, VcnName);
	else
		sprintf(Scratch, "%s.%s", name, "VCN");
	load_cps_file(Scratch, 3, 3, 0L);
	mem_copy(WorkPage + 2, Remap_E566, 16L);
	mem_copy(WorkPage + 18, Remap_EEF6, 16L);
	if (VideoMode == 6) {
		tbl = Drv_65E6(0L);
		Drv_65E6(tbl);
		for (i = 1; i < 16; i++) {
			Remap_E566[i] = tbl[Remap_E566[i]];
			Remap_EEF6[i] = tbl[Remap_EEF6[i]];
		}
	}
	mem_copy(WorkPage + 34, VcnTiles, 47500L);
	set_vcn_colours(Remap_E566, Remap_EEF6);
	if (VideoMode == 5) {
		mem_copy(VcnTiles, WorkPage, 32000L);
		expand_vcn(1);
		Drv_654A(3, 4, 0L);
		mem_copy(VcnTiles + 32000, WorkPage, 32000L);
		expand_vcn(0);
		seg = PageSegs[6];
		PageSegs[6] = 0xABE8;
		Drv_654A(3, 6, 0L);
		PageSegs[6] = seg;
		load_cps_file(Scratch, 3, 3, 0L);
		make_tile_mask(WorkPage + 34, VcnTiles);
	}
	*p = *p;
	strcpy(VcnName, Scratch);
}

/* 53EC:0DD9 Rebuilds the lists of the items lying in the new level */
void far relink_items(void)
{
	Item far *it;
	register int i;

	for (i = 0; i < 1024; i++)
		LevelMap[i].items = 0;
	it = Items;
	for (i = 0; i < 600; it++, i++)
		if (it->level == CurrentLevel && it->block > 0)
			put_item(&LevelMap[it->block].items, it->block, i, it->pos);
}

/* 53EC:0E4C LEVELx.MAZ: a 6-byte header, then 4 wall bytes per square
 * (or the state saved in LEVELS.TMP when the level was visited) */
void far load_maze(char far *name)
{
	long far *src;
	unsigned long dst;
	register int i, fd;

	memset(LevelMap, 0, 10240);
	fd = open_file(name, 1);
	read_file(fd, LevelMap, 6L);
	read_file(fd, WorkPage, 4096L);
	close_file(fd);
	if (LevelsVisited & (1 << (CurrentLevel - 1)))
		restore_level_state(CurrentLevel);
	else {
		src = (long far *)WorkPage;
		dst = (unsigned long)(LevelBlock far *)LevelMap;
		for (i = 0; i < 1024; i++) {
			*(long far *)dst = *src;
			src++;
			dst = dst + 10;
		}
	}
}

/* 53EC:0F2B The monster types of the level (id, then 35 to 41 bytes),
 * until FF; returns what follows */
char far * far read_monster_types(char far *p)
{
	MonsterType far *mt;
	int id;
	register int i, j;
	register int k;

	while ((id = *p++) != -1) {
		mt = &MonsterTypes[id];
		mt->ac = *p;
		p++;
		mt->thac0 = *p;
		p++;
		mt->level = *p;
		p++;
		mt->hpDice[0] = *p;
		p++;
		mt->hpDice[1] = *p;
		p++;
		mt->hpDice[2] = *p;
		p++;
		mt->attacks = *p;
		p++;
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++) {
				mt->damage[i][j] = *p;
				p++;
			}
		mt->immunities = *(int far *)p;
		p += 2;
		mt->flags = *(int far *)p;
		p += 2;
		mt->kinds = *(int far *)p;
		p += 2;
		*(long far *)&mt->xp = *(int far *)p;
		p += 2;
		mt->big = *p;
		p++;
		mt->attackSound = *p;
		p++;
		mt->moveSound = *p;
		p++;
		mt->ammo = *p;
		p++;
		if (*p++ != -1) {
			mt->attackMode = *p;
			p++;
			mt->attackCount = *p;
			p++;
			for (k = 0; k < mt->attackCount; k++) {
				mt->rangedAttack[k] = *p;
				p += 2;
			}
		}
		mt->undead = *p;
		p++;
		mt->resist = *p;
		p++;
		for (i = 0; i < 3; i++) {
			mt->accessory[i] = *p;
			p++;
		}
	}
	return p;
}

/* 53EC:1199 The timed events: (what, seconds) until FFFF; the seconds
 * become ticks. Returns what follows */
int far * far read_timed_events(int far *p)
{
	register int v, w;

	TimedEventCount = 0;
	while ((w = *p) != -1) {
		p++;
		TimedEvents[TimedEventCount].what = w;
		v = *p * 18;
		TimedEvents[TimedEventCount].interval = v;
		TimedEvents[TimedEventCount].next = GameTicks + v;
		TimedEventCount++;
		p++;
	}
	p++;
	return p;
}

/* 53EC:1219 Sets (on) or clears flag 2 of the wall types base * 10 + 3 to
 * + 11, but the middle one (not called) */
void far set_wall_flag2(int base, int on)
{
	register int i;

	base = base * 10 + 3;
	for (i = 0; i < 9; i++) {
		if (i == 4)
			continue;
		if (on)
			WallTypeFlags[base + i] |= 2;
		else
			WallTypeFlags[base + i] &= ~2;
	}
}

/* 53EC:1257 Reads the sub-header sub of the level (inf: the INF after its
 * first word): maze, walls, palettes, sounds, doors, monster graphics and
 * types, decorations, timed events. Returns the name of the wall set */
char far * far parse_level_header(char far *inf, int sub)
{
	int n, fd;
	char far *p;
	register int i;

	play_sound(0);
	p = inf;
	for (i = 0; i < sub; i++)
		p = inf + *(int far *)p;
	p += 2;
	if (*p++ == (char)0xEC) {
		load_maze(p);
		p += 13;
		sprintf(Scratch, "%s.%s", p, "VMP");
		fd = open_file(Scratch, 1);
		read_file(fd, WallLayouts, 2L);
		read_file(fd, WallLayouts, 6000L);
		close_file(fd);
		if (VideoMode != 5)
			sprintf(Scratch, "%s.PAL", p);
		else
			sprintf(Scratch, "%s.EGA", p);
		strcpy(WallSetName, p);
		p += 13;
		if (*p++ != (char)0xFF) {
			if (VideoMode != 5)
				sprintf(Scratch, "%s.PAL", p);
			else
				sprintf(Scratch, "%s.EGA", p);
			p += 13;
		}
		load_file_into(Scratch, Palette, 0x300L);
		movmem(Palette + 0x2A0, ShrinkBuffer, 0x60);
		memset(Palette + 0x2A0, 0x3F, 0x60);
		Drv_6552(Palette, PoisonRemap, 4, 0x4B);
		Drv_6552(Palette, InvisibleRemap, 12, 200);
		Drv_6552(Palette, StoneRemap, 10, 0x55);
		Drv_6552(Palette, MagicPalette, 11, 0x7D);
		movmem(ShrinkBuffer, Palette + 0x2A0, 0x60);
		Drv_6552(Palette, NormalPalette, 12, 0x55);
		if (VideoMode != 3)
			set_palette(Palette);
	}
	delay_ticks(1);
	load_sounds(p);
	p += 13;
	free_door_shapes();
	free_monster_shapes(0, 36);
	free_decorations();
	for (i = 0; i < 2; i++)
		if (*p == (char)0xEC || *p == (char)0xEA) {
			set_wall_flag2(p[14], *p == (char)0xEA);
			p++;
			DoorKind[p[13]] = p[14];
			DoorNoKnob[p[13]] = p[15];
			p = (char far *)load_door_shapes(p, p[13], (int far *)(p + 16));
		} else
			p++;
	Var_EE1E = *(int far *)p;
	p += 2;
	StepCounter = 0;
	for (i = 0; i < 2; i++)
		if (*p++ == (char)0xEC) {
			MonsterLoaders[p[0]](p + 2, p[1] * 18, p[15]);
			p += 16;
		}
	p = read_monster_types(p);
	if (*p++ == (char)0xEC) {
		n = *(int far *)p;
		p += 2;
		for (i = 0; i < n; i++)
			if (*p++ == (char)0xEC) {
				load_decoration_file(p, p + 13);
				p += 26;
			} else {
				wall_type_setup(*p, p[1], p[2], p[3], p[4]);
				p += 5;
			}
	}
	p = (char far *)read_timed_events((int far *)p);
	return WallSetName;
}

/* 53EC:169A The monster section of the INF: the speed of the 4 monster
 * timers ((group, ticks) until FF), then the 30 monsters (14 bytes each,
 * only if the level was never visited). Returns what follows */
char far * far place_monsters(char far *p, int level)
{
	register int i, g;

	while ((g = *p++) != -1) {
		g <<= 1;
		Timers[g + 2].interval = *p;
		Timers[g + 3].interval = *p;
		p++;
	}
	for (i = 0; i < 4; i++)
		if (i & 1)
			Timers[i + 2].next = GameTicks + (Timers[i + 2].interval >> 1);
		else
			Timers[i + 2].next = GameTicks + Timers[i + 2].interval;
	if (!(LevelsVisited & (1 << (level - 1)))) {
		memset(Monsters, 0, 900);
		for (i = 0; i < 30; i++) {
			if (*p != -1) {
				create_monster_at(p[0], p[1], *(int far *)(p + 2), p[4], p[5], p[6], p[7], p[8], p[9],
				                  *(int far *)(p + 10), *(int far *)(p + 12));
				Monsters[*p].flags |= 0x40;
			}
			p += 14;
		}
	} else
		p += 420;
	return p;
}

/* 53EC:17F5 Loads level (its variant sub): LEVELx.INF, the header, the
 * monsters, the script and its triggers, the tiles */
void far load_level(int level, int sub)
{
	int t, fd, n, size, used;
	char far *p;
	char far *name;
	register int i, b;

	clock_stop(2);
	CurrentLevel = level;
	CurrentSubLevel = sub;
	sprintf(Scratch, "%s%d.%s", "LEVEL", level, "INF");
	fd = open_file(Scratch, 1);
	read_file(fd, &size, 2L);
	read_file(fd, WorkPage, size);
	close_file(fd);
	size = unpack_picture(WorkPage, LoadBuffer);
	name = parse_level_header((char far *)LoadBuffer + 2, sub);
	t = *(int far *)LoadBuffer;
	p = (char far *)LoadBuffer + t;
	t = *(int far *)p;
	p += 2;
	if (*p++ == (char)0xEC)
		p = place_monsters(p, level);
	else if (!(LevelsVisited & (1 << (level - 1))))
		memset(Monsters, 0, 900);
	used = p - (char far *)LoadBuffer;
	size = t - used;
	mem_copy(p, ScriptCode, size);
	Var_62A5 = 2;
	relink_items();
	p = (char far *)LoadBuffer + t;
	n = *(int far *)p;
	p += 2;
	for (i = 0; i < n; i++) {
		b = *(int far *)p;
		p += 2;
		LevelMap[b].flags |= *(int far *)p;
		p += 2;
		LevelMap[b].script = *(int far *)p;
		p += 2;
	}
	load_vcn(name, 0L);                  /* sic: load_vcn writes *p = *p through 0 */
	load_picture("INVENT", 0L, 5, 3, 2);
	clock_start(2);
	FadeInPending = 1;
	Var_62A5 = 0;
}

/* 53EC:1A19 Redraws the play screen: PLAYFLD, the compass, the view
 * (scene, unless a fade is pending), then INVENT back in page 5 */
void far draw_play_screen(int scene)
{
	int old;

	load_picture("PLAYFLD", 0L, 5, 3, 2);
	old = set_page(2);
	draw_compass(1);
	if (scene && !FadeInPending)
		draw_scene(0);
	set_page(old);
	hide_mouse();
	copy_page(0, 0, 0, 0, 40, 200, 2, 0);
	show_mouse();
	load_picture("INVENT", 0L, 5, 3, 2);
}
