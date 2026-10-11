/*
 * Eye of the Beholder II - START.EXE
 * The camp menu: rest, memorize / pray for spells, scribe scrolls, drop an
 * NPC, the options, load / save / quit; overlay module 3F5F (Ghidra 4F5F).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y camp.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The menus are built from two tables: CampMenus (DS:0B98: a title, a
 * window, the first of their buttons and how many) and CampButtons (far,
 * 343E:0000: the place, the key and the text of each button). Each button
 * is a copy of Buttons[0] (gui.c) pointing to its CampButtons entry (in
 * page0/page1); its texts are strings of MenuStrings.
 */
#include "eob2.h"

typedef unsigned char byte;

/* A menu of the camp (5 bytes) */
typedef struct CampMenu {
	char          title;         /* +0 the string of MenuStrings */
	char          window;        /* +1 0: no window drawn */
	char          first;         /* +2 the first of its buttons (CampButtons) */
	char          count;         /* +3 */
	char          colour;        /* +4 of the title */
} CampMenu;

/* A button of the camp menus (10 bytes) */
typedef struct MenuButton {
	char          text;          /* +0 the string of MenuStrings, 0 = none */
	int           x;             /* +1 */
	unsigned char y, w, h;       /* +3 */
	int           key;           /* +6 */
	int           flags;         /* +8 1 framed, 2 can be selected (arrows, mouse), 4 centred */
} MenuButton;

/* the CampButtons entry of a button (kept in Button.page0/page1) */
#define TEMPLATE(b)  (*(MenuButton far * far *)&(b)->page0)

extern Character      Party[6];          /* E608 */
extern Item           Items[600];        /* 71CA */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern signed char    ClassFlags[];      /* 1F9A */
extern char far      *GuiMageBook[];     /* 1FE1 the names of the mage spells */
extern char far      *GuiClericBook[];   /* 2065 the names of the cleric spells */
extern Button         Buttons[];         /* 39B8 Buttons[0]: the model of the buttons */
extern char far       MenuStrings[];     /* 38F7:0000 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            VideoMode;         /* 628A */
extern int            GuiStyle;          /* 636F the current window */
extern char           GuiDirty;          /* 6389 redraw the list */
extern int            SoundEnabled;      /* 65FA */
extern int            Var_65FC;          /* 65FC the music (saved with the options) */
extern int            ClickDown[2];      /* 661A */
extern int            ClickUp[2];        /* 6622 */
extern unsigned char  InventoryMode;     /* 6655 the keyboard moves in the inventory */
/* Uninitialised data: in bss6c74.c (DS:6C74-6D43) */
extern int CampPortraits;           /* 6C74 portraits were greyed (redraw them) */
extern char CampSpellLevel[32];     /* 6C76 the level of each spell */
extern char CampLevelSpells[8][11]; /* 6C9A per spell level: the count, then the spells (from 1) */
extern char CampLevelCount[8];      /* 6CF2 the spells memorized per spell level */
extern char CampSpellCount[36][2];  /* 6CFA per spell: memorized, ready */
extern int SpellsToRegain;          /* 6D42 spells were chosen: rest to get them */
extern char           HpBars;            /* E4C5 hit points shown as bars */
extern unsigned char  FrameFlat;         /* E606 */
extern char           Scratch[];         /* F16E */
extern byte far      *CurFont;           /* F090 */
extern byte far      *Font8;             /* F098 */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern byte far      *Var_CC58;          /* CC58 the shape greying a portrait */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern void (far     *Drv_6542)(int x, int y, int w, int h, byte far *buf);    /* 6542 restores a region */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A fills */
extern void (far     *Drv_65D2)(int x, int y, int w, int h, byte far *buf);    /* 65D2 saves a region */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void drop_character(int ch);                                     /* 1E24:043C */
void swap_mem(void far *a, void far *b, void far *buf, unsigned n);   /* 18D7:082F */
void select_char(int ch);                                        /* 1E24:0040 */
void gui_main(void);                                             /* 1DFE:002F */
void refresh_char_timers(void);                                  /* 27B7:0C75 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
void movmem(void far *src, void far *dest, unsigned n);
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
void farfree(void far *p);                                       /* 1000:7FF2 */
int  farheapcheck(void);
void exit(int status);                                           /* 1000:5A8D */
void flush_keys(void);                                           /* 3295:0516 */
int  key_pressed(void);                                          /* 3295:041E */
int  get_key(void);                                              /* 3295:05D0 */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
int  class_part_index(int base, int cls);                        /* 1B54:009C */
int  find_inventory_item(int ch, int type, int value);           /* 1E24:0148 */
int  has_ring(Character far *c, int kind);                       /* 1E24:01BE */
void flash_box(int n);                                           /* 1E24:054E */
void redraw_party(void);                                         /* 1E24:0686 */
void close_panels(void);                                         /* 1E24:0B5B */
void draw_frame(int x, int y, int w, int h, int pressed, int hollow);   /* 1E24:0AB2 */
char far *get_script_string(char far *text, int index);          /* 1E24:0869 */
void draw_portrait(int ch);                                      /* 2901:0AF9 */
int  in_box(int x, int y, int x1, int y1, int x2, int y2);       /* 28A8:00E5 */
int  cursor_in_rect(int x1, int y1, int x2, int y2);             /* 3269:0213 */
byte far *set_font(byte far *font);                              /* 3166:000A */
int  gui_process(void far *gui);                                 /* 2F86:004E */
Button far *gui_add_button(Button far *list, Button far *b);     /* 2D3D:0009 */
Button far *gui_find(Button far *list, unsigned index);          /* 2D3D:01D2 */
byte far *mem_alloc(unsigned long size, int flags);              /* 2D0E:000B */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void load_picture(char far *name, byte far *pal, int a, int page, int mode);   /* 18D7:05C8 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
int  select_window(int w);                                       /* 31B0:0133 */
int  select_window_clear(int w);                                 /* 31B0:01F2 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void clock_stop(int), clock_start(int);    /* 3187:003D / 0009 */
void sort_spells(int ch);                                        /* 20E2:0120 */
int  check_party_alive(int ask);                                 /* 1F55:09FE */
void system_shutdown(void);                                      /* 315F:0000 */
int  rest_party(void);                                           /* 51C5:0000 (overlay) */
int  load_game(int a, int b);                                    /* 559A:04A7 (overlay) */
int  save_game(int a, int b);                                    /* 559A:05A4 (overlay) */
void draw_play_screen(int scene);                                /* 53EC:1A19 (overlay) */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
static void far drop_npc(int ch);
static void far count_spells(int ch, char far *slots, unsigned long book, int cleric);
static void far apply_spells(int ch, int cleric);
static int far choose_char(int what);
static void far draw_spell_line(int spell, int line, int cleric, int direct, int selected);
static void far memorize_menu(int ch, int cleric);
static char far * far on_off(int on);
void far save_region(void);
void far restore_region(void);
Button far * far build_menu(int menu);
void far show_text_box(int text);
void far free_buttons(Button far *b);
void far draw_button(Button far *b, int pressed, int lit, int direct);
static void far scribe_scroll(void);
int far camp(Button far *b);
int far ask_party(int text);

/* Initialised data of the module (DS:0B98) */
CampMenu CampMenus[7] = {        /* 0B98 */
	{ 1, 10, 0, 7, 9 },          /* 0 the camp (title 0x38) */
	{ 1, 10, 7, 5, 9 },          /* 1 the game: load, save, quit (title 0x3A) */
	{ 1, 10, 12, 3, 9 },         /* 2 the options (title 0x39) */
	{ 0, 10, 15, 7, 15 },        /* 3 choose a character (title: what is asked) */
	{ 37, 10, 22, 9, 9 },       /* 4 memorize / pray */
	{ 0, 11, 32, 2, 15 },        /* 5 yes / no (ask_party) */
	{ 48, 10, 34, 2, 9 }         /* 6 scribe scrolls */
};
/* the level of each spell (copied to CampSpellLevel) */
char MageSpellLevels[] = {       /* 0BBB */
	1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5,
	6, 6, 6, 6, 7, 7, 8, 0
};
char ClericSpellLevels[] = {     /* 0BDC */
	1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
	6, 6, 8, 8, 0
};
static unsigned char PortraitX[6] = { 23, 32, 23, 32, 23, 32 };   /* 0BF9 in bytes */
static unsigned char PortraitY[6] = { 2, 2, 54, 54, 106, 106 };   /* 0BFF */
char ClericSlots[17][8] = {   /* 0C05 cleric spells per spell level, by cleric level */
	{ 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 3, 2, 0, 0, 0, 0, 0, 0 },
	{ 3, 3, 1, 0, 0, 0, 0, 0 },
	{ 3, 3, 2, 0, 0, 0, 0, 0 },
	{ 3, 3, 2, 1, 0, 0, 0, 0 },
	{ 3, 3, 3, 2, 0, 0, 0, 0 },
	{ 4, 4, 3, 2, 1, 0, 0, 0 },
	{ 4, 4, 3, 2, 2, 0, 0, 0 },
	{ 5, 4, 4, 3, 2, 1, 0, 0 },
	{ 6, 5, 5, 3, 2, 2, 0, 0 },
	{ 6, 6, 6, 4, 2, 2, 0, 0 },
	{ 6, 6, 6, 5, 3, 2, 1, 0 },
	{ 6, 6, 6, 6, 4, 2, 1, 0 },
	{ 7, 7, 7, 6, 4, 3, 1, 0 },
	{ 7, 7, 7, 7, 5, 3, 2, 0 },
};
char WisdomBonus[8][8] = {   /* 0C8D bonus cleric spells, by wisdom - 12 */
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 1, 0, 0, 0, 0, 0 },
	{ 2, 2, 1, 1, 0, 0, 0, 0 },
	{ 3, 2, 1, 2, 0, 0, 0, 0 },
};
char PaladinSlots[17][8] = {   /* 0CCD by paladin level */
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 1, 0, 0, 0, 0, 0 },
	{ 3, 2, 1, 0, 0, 0, 0, 0 },
	{ 3, 2, 1, 1, 0, 0, 0, 0 },
	{ 3, 3, 2, 1, 0, 0, 0, 0 },
	{ 3, 3, 3, 1, 0, 0, 0, 0 },
};
char MageSlots[17][8] = {   /* 0D55 by mage level */
	{ 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 3, 2, 0, 0, 0, 0, 0, 0 },
	{ 4, 2, 1, 0, 0, 0, 0, 0 },
	{ 4, 2, 2, 0, 0, 0, 0, 0 },
	{ 4, 3, 2, 1, 0, 0, 0, 0 },
	{ 4, 3, 3, 2, 0, 0, 0, 0 },
	{ 4, 3, 3, 2, 1, 0, 0, 0 },
	{ 4, 4, 3, 2, 2, 0, 0, 0 },
	{ 4, 4, 4, 3, 3, 0, 0, 0 },
	{ 4, 4, 4, 4, 4, 1, 0, 0 },
	{ 5, 5, 5, 4, 4, 2, 0, 0 },
	{ 5, 5, 5, 4, 4, 2, 1, 0 },
	{ 5, 5, 5, 5, 5, 2, 1, 0 },
	{ 5, 5, 5, 5, 5, 3, 2, 1 },
	{ 5, 5, 5, 5, 5, 3, 3, 2 },
};

/* the buttons of the camp menus (far data) */
MenuButton far CampButtons[36] = {   /* 343E:0000 */
	{ 0x02, 12, 20, 158, 14, 0x14, 3 },   /* 0 */
	{ 0x03, 12, 37, 158, 14, 0x34, 3 },   /* 1 */
	{ 0x04, 12, 54, 158, 14, 0x1A, 3 },   /* 2 */
	{ 0x05, 12, 71, 158, 14, 0x20, 3 },   /* 3 */
	{ 0x06, 12, 88, 158, 14, 0x00, 3 },   /* 4 */
	{ 0x07, 12, 105, 158, 14, 0x23, 3 },   /* 5 */
	{ 0x08, 128, 122, 40, 14, 0x13, 7 },   /* 6 */
	{ 0x09, 12, 20, 158, 14, 0x27, 3 },   /* 7 */
	{ 0x0A, 12, 37, 158, 14, 0x20, 3 },   /* 8 */
	{ 0x0B, 12, 54, 158, 14, 0x21, 3 },   /* 9 */
	{ 0x0C, 12, 71, 158, 14, 0x11, 3 },   /* 10 */
	{ 0x08, 128, 122, 40, 14, 0x13, 7 },   /* 11 */
	{ 0x12, 12, 20, 158, 14, 0x20, 3 },   /* 12 */
	{ 0x13, 12, 37, 158, 14, 0x32, 3 },   /* 13 */
	{ 0x08, 128, 122, 40, 14, 0x13, 7 },   /* 14 */
	{ 0x08, 128, 122, 40, 14, 0x13, 5 },   /* 15 */
	{ 0x00, 184, 0, 64, 48, 0x70, 0 },   /* 16 */
	{ 0x00, 256, 0, 64, 48, 0x71, 0 },   /* 17 */
	{ 0x00, 184, 56, 64, 48, 0x72, 0 },   /* 18 */
	{ 0x00, 256, 56, 64, 48, 0x73, 0 },   /* 19 */
	{ 0x00, 184, 112, 64, 48, 0x74, 0 },   /* 20 */
	{ 0x00, 256, 112, 64, 48, 0x75, 0 },   /* 21 */
	{ 0x24, 8, 126, 48, 14, 0x30, 5 },   /* 22 */
	{ 0x08, 128, 126, 40, 14, 0x13, 5 },   /* 23 */
	{ 0x00, 0, 50, 168, 72, 0x3D, 0 },   /* 24 */
	{ 0x1F, 11, 16, 20, 18, 0x02, 5 },   /* 25 */
	{ 0x20, 38, 16, 20, 18, 0x03, 5 },   /* 26 */
	{ 0x21, 65, 16, 20, 18, 0x04, 5 },   /* 27 */
	{ 0x22, 92, 16, 20, 18, 0x05, 5 },   /* 28 */
	{ 0x23, 119, 16, 20, 18, 0x06, 5 },   /* 29 */
	{ 0x3C, 146, 16, 20, 18, 0x07, 5 },   /* 30 */
	{ 0x3D, 150, 16, 20, 18, 0x08, 5 },   /* 31 */
	{ 0x26, 16, 57, 32, 14, 0x16, 7 },   /* 32 */
	{ 0x27, 128, 57, 32, 14, 0x33, 7 },   /* 33 */
	{ 0x08, 128, 126, 40, 14, 0x13, 7 },   /* 34 */
	{ 0x00, 0, 50, 168, 72, 0x3D, 0 },   /* 35 */
};

/* 4F5F:0000 Drops an NPC (a character chosen by choose_char(0x35)): if it
 * was one of the first 4, the 5th or 6th character takes its place */
static void far drop_npc(int ch)
{
	int to;

	if (ch >= 0 && char_is_active(ch, 1)) {
		drop_character(ch);
		to = ch;
		if (ch < 4) {
			if (char_is_active(5, 1))
				to = 5;
			else
				to = 4;
			swap_mem(&Party[to], &Party[ch], ShrinkBuffer, sizeof(Character));
		}
		select_char(0);
		gui_main();
		refresh_char_timers();
	}
}

/* 4F5F:008C Counts the spells of a character for the memorize menu: per
 * spell (memorized, ready), per spell level, and lists the spells of each
 * level it may choose (in its book; for a cleric, the levels it has slots
 * for) */
static void far count_spells(int ch, char far *slots, unsigned long book, int cleric)
{
	int i;
	Character far *c;
	signed char far *p;
	register int s, lv;

	if (ch < 0)
		return;
	c = &Party[ch];
	memset(CampLevelSpells, 0, 80);    /* sic: 88 bytes; the last 8 are not cleared (harmless: after the count) */
	memset(CampLevelCount, 0, 8);
	memset(CampSpellCount, 0, 72);
	memset(CampSpellLevel, 0, 32);
	if (cleric)
		strcpy(CampSpellLevel, ClericSpellLevels);
	else
		strcpy(CampSpellLevel, MageSpellLevels);
	p = c->mageSpells[0];
	if (cleric)
		p = c->clericSpells[0];
	for (i = 0; i < 80; i++) {
		s = *p++;
		if (s == 0 || s == 29)
			continue;
		if (s > 0)
			CampSpellCount[s - 1][1]++;
		else
			s = -s;
		s--;
		CampSpellCount[s][0]++;
		CampLevelCount[CampSpellLevel[s] - 1]++;
	}
	for (i = 0; i < 32; i++) {
		if (book & (1L << i)) {
			lv = CampSpellLevel[i] - 1;
			if (((cleric && slots[lv]) || !cleric) && lv >= 0) {
				CampLevelSpells[lv][0]++;
				CampLevelSpells[lv][CampLevelSpells[lv][0]] = i + 1;
			}
		}
	}
}

/* 4F5F:0212 Writes the spells chosen back into the character: the ready
 * ones first (> 0), the others to be regained by resting (< 0) */
static void far apply_spells(int ch, int cleric)
{
	int i;
	Character far *c;
	register int j, lv;

	if (ch < 0)
		return;
	c = &Party[ch];
	if (cleric)
		memset(c->clericSpells, 0, 80);
	else
		memset(c->mageSpells, 0, 80);
	if (cleric)
		Party[ch].clericSpells[0][0] = 29;
	for (i = 0; i < 32; i++) {
		if (CampSpellCount[i][0] < CampSpellCount[i][1])
			CampSpellCount[i][1] = CampSpellCount[i][0];
		if (CampSpellCount[i][1]) {
			CampSpellCount[i][1]--;
			CampSpellCount[i][0]--;
			lv = CampSpellLevel[i] - 1;
			for (j = 0; j < 10; j++) {
				if (cleric) {
					if (c->clericSpells[lv][j] == 0) {
						c->clericSpells[lv][j] = i + 1;
						break;
					}
				} else if (c->mageSpells[lv][j] == 0) {
					c->mageSpells[lv][j] = i + 1;
					break;
				}
			}
		} else if (CampSpellCount[i][0]) {
			CampSpellCount[i][0]--;
			lv = CampSpellLevel[i] - 1;
			SpellsToRegain = 1;
			for (j = 0; j < 10; j++) {
				if (cleric) {
					if (c->clericSpells[lv][j] == 0) {
						c->clericSpells[lv][j] = -(i + 1);
						break;
					}
				} else if (c->mageSpells[lv][j] == 0) {
					c->mageSpells[lv][j] = -(i + 1);
					break;
				}
			}
		} else
			continue;
		i--;                 /* the same spell again */
	}
}

/* 4F5F:03E4 Chooses a character for: 0x17 memorize (mages), 0x1A pray
 * (clerics, paladins), 0x31 scribe scrolls, 0x35 drop an NPC (anyone).
 * The others are greyed; with a single one it is taken at once. Returns
 * the character or -1 */
static int far choose_char(int what)
{
	Button far *list;
	unsigned key;
	int sel;
	char able[6];
	int n, mask, cannot, idx;
	register int cur, i;

	mask = what == 0x1A ? 0x14 : 2;
	flush_keys();
	CampPortraits = 0;
	n = 0;
	cannot = 0;
	sel = -1;
	for (i = 0; i < 6; i++) {
		able[i] = -1;
		if ((char_is_active(i, 1) && (ClassFlags[Party[i].cls] & mask))
		    || (what == 0x35 && char_is_active(i, 1))) {
			if (what == 0x35 || (Party[i].food && char_is_active(i, 4))) {
				able[i] = 0;
				sel = i;
				n++;
			} else
				cannot = 1;
		}
	}
	if (n == 0) {
		switch (what) {
		case 0x1A:
			if (cannot)
				n = 0x1B;
			else
				n = 0x49;
			break;
		case 0x17:
			if (cannot)
				n = 0x1C;
			else
				n = 0x48;
			break;
		case 0x31:
			n = 0x34;
			break;
		}
		show_text_box(n);
		sel = -1;
	} else {
		for (i = 0; i < 6; i++) {
			if (able[i] == -1 && char_is_active(i, 1)) {
				put_shape(0, Var_CC58, PortraitX[i] * 8, PortraitY[i], 0);
				put_shape(0, Var_CC58, PortraitX[i] * 8 + 16, PortraitY[i], 0);
				put_shape(0, Var_CC58, PortraitX[i] * 8 + 32, PortraitY[i], 0);
				put_shape(0, Var_CC58, PortraitX[i] * 8 + 48, PortraitY[i], 0);
				CampPortraits = 1;
			}
		}
		if (n == 1) {
			idx = class_part_index(4, Party[sel].cls);
			if (idx != -1 && Party[sel].level[idx] <= 8) {
				show_text_box(0x18);
				sel = -1;
				return sel;     /* the two returns: the JMPs of 4F5F:062D and 0630 */
			}
			return sel;
		} else {
			CampMenus[3].title = what;
			list = build_menu(3);
			sel = -2;
			for (cur = 0; cur < 6; cur++)
				if (!able[cur])
					break;
			if (!InventoryMode)
				cur = -1;
			set_font(GameFont);
			while (sel == -2) {
				key = gui_process(list);
				flash_box(cur);
				switch (key) {
				case 0x12:
				case 0x1F:
				case 0x5C:
				case 0x60:
					flash_box(-1);
					draw_portrait(cur);
					cur--;
					if (cur < 0)
						cur = 5;
					while (able[cur]) {
						cur--;
						if (cur < 0)
							cur = 5;
					}
					break;
				case 0x20:
				case 0x2E:
				case 0x62:
				case 0x66:
					flash_box(-1);
					draw_portrait(cur);
					cur++;
					if (cur >= 6)
						cur = 0;
					while (able[cur]) {
						cur++;
						if (cur >= 6)
							cur = 0;
					}
					break;
				case 0x6E:
				case 0x8010:
					set_font(Font8);
					draw_button(list, 1, 1, 1);
					wait_ticks(5);
					draw_button(list, 0, 0, 1);
					sel = -1;
					set_font(GameFont);
					break;
				case 0x8011:
				case 0x8012:
				case 0x8013:
				case 0x8014:
				case 0x8015:
				case 0x8016:
					sel = key - 0x8011;
					if (able[sel])
						sel = -2;
					break;
				case 0x2B:
				case 0x61:
					if (cur >= 0)
						sel = cur;
					break;
				}
			}
			flash_box(-1);
			if (cur)        /* ORIGINAL BUG: cur >= 0; draw_portrait(-1) with the mouse, and the 1st portrait is not redrawn */
				draw_portrait(cur);
			set_font(Font8);
			if (sel != -1 && what != 0x35) {
				if (mask == 0x14 && (ClassFlags[Party[sel].cls] & 0x10)
				    && Party[sel].level[0] <= 8) {
					show_text_box(0x18);
					sel = -1;
				}
				if (mask == 2 && find_inventory_item(sel, 0x1D, -1) == -1) {
					show_text_box(0x19);
					sel = -1;
				}
			}
			free_buttons(list);
		}
	}
	return sel;
}

/* 4F5F:08DE Draws a line of the memorize menu: the spell and how many are
 * memorized; direct: on the screen (else on the page drawn) */
static void far draw_spell_line(int spell, int line, int cleric, int direct, int selected)
{
	int colour;
	int y;

	colour = VideoMode == 0 ? 1 : 15;
	if (line < 0)
		return;
	y = line * 9 + 50;
	hide_mouse();
	if (spell) {
		sprintf(Scratch, "%-18s %1d", cleric ? GuiClericBook[spell] : GuiMageBook[spell],
		        CampSpellCount[spell - 1][0]);
		if (direct)
			Drv_65F2(Scratch, 8, y, selected ? 6 : colour, 0);
		else
			print_text(Scratch, 8, y, selected ? 6 : colour, FrameFlat);
	} else if (!direct)
		Drv_657A(6, y, 168, y + 8, FrameFlat);
	show_mouse();
}

/* 4F5F:09DF The memorize (mages) / pray (clerics) menu of a character */
static void far memorize_menu(int ch, int cleric)
{
	char slots[8];
	int idx, lv, wis;
	unsigned long book;
	int cl;
	Button far *list, far *b;
	int newLevel, shown, line;
	unsigned key;
	int running, count, redraw;
	register int level, i;

	if (ch == -1)
		return;
	wis = Party[ch].wis - 12;
	wis = wis < 0 ? 0 : wis;
	if (cleric) {
		idx = class_part_index(2, Party[ch].cls);
		if (idx != -1) {
			lv = Party[ch].level[idx] - 1;
			for (i = 0; i < 8; i++)
				if ((slots[i] = ClericSlots[lv][i]) != 0)
					slots[i] += WisdomBonus[wis][i];
			book = 0xF7FFFFFFL;
		} else {
			idx = class_part_index(4, Party[ch].cls);
			if (idx != -1) {
				lv = Party[ch].level[idx] - 1;
				lv = lv < 0 ? 0 : lv;
				for (i = 0; i < 8; i++)
					slots[i] = PaladinSlots[lv][i];
				book = 0x0A9BBD1DL;
			}
		}
		cl = 1;
	} else {
		idx = class_part_index(1, Party[ch].cls);
		if (idx != -1) {
			lv = Party[ch].level[idx] - 1;
			for (i = 0; i < 8; i++)
				slots[i] = MageSlots[lv][i];
			book = Party[ch].mageBook;
		}
		if (has_ring(&Party[ch], 1)) {
			slots[3] <<= 1;
			slots[4] <<= 1;
		}
		cl = 0;
	}
	count_spells(ch, slots, book, cl);
	list = 0;
	count = 1;
	level = shown = -1;
	newLevel = line = 0;
	list = build_menu(4);
	running = 1;
	while (running) {
		flash_box(ch);
		if (newLevel < 0)
			newLevel = 7;
		if (newLevel > 7)
			newLevel = 0;
		if (level != newLevel) {
			if (level >= 0) {
				b = gui_find(list, level + 0x1A);
				draw_button(b, 0, 0, 1);
			}
			b = gui_find(list, newLevel + 0x1A);
			draw_button(b, 0, 1, 1);
			line = 0;
			shown = -1;
			level = newLevel;
			count = 1;
			redraw = 1;
		}
		if (redraw) {
			redraw = 0;
			Var_62A5 = 2;
			for (i = 1; i < 9; i++)
				draw_spell_line(CampLevelSpells[level][i], i - 1, cl, 0, 0);
			Var_62A5 = 0;
			hide_mouse();
			copy_page(0, 50, 0, 50, 22, 72, 2, 0);
			show_mouse();
			shown = -1;
		}
		if (count) {
			count = 0;
			sprintf(Scratch, "%d of %d Remaining.  ", slots[level] - CampLevelCount[level], slots[level]);
			hide_mouse();
			print_text(Scratch, 8, 38, 9, FrameFlat);
			show_mouse();
		}
		if (line < 0)
			line = CampLevelSpells[level][0] - 1;
		if (line >= CampLevelSpells[level][0])
			line = 0;
		if (line != shown) {
			draw_spell_line(CampLevelSpells[level][shown + 1], shown, cl, 1, 0);
			draw_spell_line(CampLevelSpells[level][line + 1], line, cl, 1, 1);
			shown = line;
		}
		key = gui_process(list);
		switch (key) {
		case 0x6E:
			key = 0x8018;
			break;
		case 0x66:
			key = (level + 1) % 8 + 0x801A;
			break;
		case 0x5C:
			/* ORIGINAL BUG: 8 levels for 6 buttons (0x801A-0x801F): "previous" from
			 * the 1st level and "next" from the 6th go to the 7th (0x8020, no
			 * button); from it, "next" gives 0x8021, which nothing handles */
			if (level == 0)
				key = 0x8020;
			else
				key = level + 0x8019;
			break;
		default:
			if (!InventoryMode && in_box(ClickDown[0], ClickDown[1], 8, 50, 168, 122)) {
				line = (ClickDown[1] - 50) / 9;
				if (line > CampLevelSpells[level][0] - 1)
					line = CampLevelSpells[level][0] - 1;
			}
		}
		if (key & 0x8000) {
			b = gui_find(list, key & 0x7FFF);
			draw_button(b, 1, 1, 1);
			wait_ticks(5);
			draw_button(b, 0, 0, 1);
		}
		switch (key) {
		case 0x801A:
		case 0x801B:
		case 0x801C:
		case 0x801D:
		case 0x801E:
		case 0x801F:
		case 0x8020:
			newLevel = key - 0x801A;
			if (newLevel == level) {
				b = gui_find(list, key & 0x7FFF);
				draw_button(b, 0, 1, 1);
			}
			break;
		case 0x8017:                     /* clear the level */
			if (CampLevelCount[level]) {
				for (i = 1; i <= CampLevelSpells[level][0]; i++) {
					CampLevelCount[level] -= CampSpellCount[CampLevelSpells[level][i] - 1][0];
					CampSpellCount[CampLevelSpells[level][i] - 1][0] = 0;
				}
				count = 1;
				redraw = 1;
			}
			break;
		case 0x8018:                     /* done */
			redraw_party();
			running = 0;
			break;
		case 0x62:
			line = shown + 1;
			break;
		case 0x5B:
			line = 0;
			break;
		case 0x60:
			line = shown - 1;
			break;
		case 0x5D:
			line = CampLevelSpells[level][0] - 1;
			break;
		case 0x0D:
		case 0x2B:
		case 0x3D:
		case 0x61:
		case 0x6A:
		case 0x8019:                     /* add one */
			if (slots[level] - CampLevelCount[level] > 0 && shown > -1) {
				CampSpellCount[CampLevelSpells[level][shown + 1] - 1][0]++;
				CampLevelCount[level]++;
				draw_spell_line(CampLevelSpells[newLevel][shown + 1], shown, cl, 0, 1);
				count = 1;
			}
			break;
		case 0x0C:
		case 0x42:
		case 0x69:                       /* remove one */
			if (slots[level] && CampSpellCount[CampLevelSpells[level][shown + 1] - 1][0]) {
				CampSpellCount[CampLevelSpells[level][shown + 1] - 1][0]--;
				CampLevelCount[level]--;
				draw_spell_line(CampLevelSpells[newLevel][shown + 1], shown, cl, 0, 1);
				count = 1;
			}
			break;
		}
	}
	free_buttons(list);
	flash_box(-1);
	set_font(GameFont);
	draw_portrait(ch);
	set_font(Font8);
	apply_spells(ch, cl);
}

/* 4F5F:119F "on" or "off" */
static char far * far on_off(int on)
{
	on = on ? 0x15 : 0x16;
	return get_script_string(MenuStrings, on);
}

/* 4F5F:11C8 Saves the view and the window of the menus (into page 5) */
void far save_region(void)
{
	hide_mouse();
	Drv_65D2(0, 120, 22, 24, LoadBuffer + 20000);
	show_mouse();
}

/* 4F5F:11F8 Puts it back */
void far restore_region(void)
{
	hide_mouse();
	Drv_6542(0, 120, 22, 24, LoadBuffer + 20000);
	show_mouse();
}

/* 4F5F:1228 Draws a menu of the camp and makes the list of its buttons */
Button far * far build_menu(int menu)
{
	Button far *list;
	Button far *b;
	register int i, n;

	Var_62A5 = 2;
	if (CampMenus[menu].window) {
		Dims[CampMenus[menu].window].b = FrameFlat;
		select_window_clear(CampMenus[menu].window);
		draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	}
	print_text(get_script_string(MenuStrings, CampMenus[menu].title), 5, 5, CampMenus[menu].colour, 0);
	list = 0;
	for (i = 0; i < CampMenus[menu].count; i++) {
		n = i + CampMenus[menu].first;
		b = (Button far *)mem_alloc(60L, 0);
		movmem(&Buttons[0], b, 60);
		b->index = n + 1;
		b->key = CampButtons[n].key;
		b->key2 = CampButtons[n].key + 0x100;
		b->x = CampButtons[n].x;
		b->y = CampButtons[n].y;
		b->w = CampButtons[n].w;
		b->h = CampButtons[n].h;
		TEMPLATE(b) = &CampButtons[n];
		b->flags = 0x1100;
		b->arg = CampButtons[n].flags;
		draw_button(b, 0, 0, 0);
		if (list)
			list = gui_add_button(list, b);
		else
			list = b;
	}
	hide_mouse();
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	show_mouse();
	GuiDirty = 1;
	flush_keys();
	Var_62A5 = 0;
	return list;
}

/* 4F5F:14C2 Shows a message in window 11, until a key or 120 ticks */
void far show_text_box(int text)
{
	int style, page;
	byte far *font;
	register int i;

	page = Var_62A5;
	font = CurFont;
	style = GuiStyle;
	set_font(Font8);
	hide_mouse();
	Dims[11].b = FrameFlat;
	select_window_clear(11);
	Var_62A5 = 2;
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	print_text(get_script_string(MenuStrings, text), WinX * 8 + 5, WinY + 5, 15, 0);
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	show_mouse();
	for (i = 0; i < 120; i++) {
		if (key_pressed()) {
			flush_keys();
			break;
		}
		wait_ticks(1);
	}
	Var_62A5 = page;
	set_font(font);
	select_window(style);
	flush_keys();
}

/* 4F5F:15E9 Frees a list of buttons */
void far free_buttons(Button far *b)
{
	Button far *next;

	while (b) {
		next = b->next;
		farfree(b);
		b = next;
	}
}

/* 4F5F:1627 Draws a button of the camp menus; direct: on the screen */
void far draw_button(Button far *b, int pressed, int lit, int direct)
{
	MenuButton far *t;
	char far *text;
	int colour;
	register int x, y;

	if (!b)
		return;
	colour = VideoMode == 0 ? 1 : 15;
	t = TEMPLATE(b);
	hide_mouse();
	if (t->flags & 1)
		draw_frame(b->x, b->y, b->w, b->h, pressed, direct);
	if (t->text) {
		text = get_script_string(MenuStrings, t->text);
		if (t->flags & 4) {
			x = ((b->w - strlen(text) * 8) >> 1) + 1;
			y = (b->h - 7) >> 1;
		} else {
			x = 4;
			y = 3;
		}
		if (direct || pressed)
			Drv_65F2(text, b->x + x, b->y + y, lit ? 6 : colour, 0);
		else
			print_text(text, b->x + x, b->y + y, lit ? 6 : colour, 0);
	}
	show_mouse();
}

/* 4F5F:176C Scribe scrolls: a mage writes the scrolls carried by the party
 * into its spell book */
static void far scribe_scroll(void)
{
	Button far *list;
	int i, count = 0, cur, sel;
	unsigned key;
	int rebuild, item;
	int slot[32] = { 0 };            /* per spell: the inventory slot + 1 of a scroll */
	int owner[32] = { 0 };           /* who carries it */
	int shown[6] = { 0 };            /* the spells (from 1) of the lines */
	register int n, ch;

	count = 0;
	for (i = 0; i < 32; i++)
		for (ch = 0; ch < 6; ch++)
			if ((slot[i] = find_inventory_item(ch, 0x22, i + 1) + 1) > 0) {
				owner[i] = ch;
				count++;
				break;
			}
	if (!count) {
		show_text_box(0x32);
		return;
	}
	ch = choose_char(0x31);
	if (ch == -1)
		return;
	n = 0;
	for (i = 0; i < 32; i++) {
		if (slot[i]) {
			if (Party[ch].mageBook & (1L << i))
				slot[i] = 0;
			else
				shown[n++] = i + 1;
		}
		if (n >= 6)
			break;
	}
	if (!n) {
		show_text_box(0x33);
		return;
	}
	list = 0;
	rebuild = 1;
	while (n) {
		if (rebuild) {
			n = 0;
			for (i = 0; i < 32; i++) {
				if (slot[i])
					shown[n++] = i + 1;
				if (n >= 6)
					break;
			}
			if (!n)
				break;
			free_buttons(list);
			list = build_menu(6);
			hide_mouse();
			for (i = 0; i < n; i++)
				print_text(GuiMageBook[shown[i]], 8, i * 9 + 50, 15, 0);
			show_mouse();
			cur = -1;
			sel = 0;
			rebuild = 0;
		}
		if (cur != sel) {
			hide_mouse();
			if (cur >= 0)
				Drv_65F2(GuiMageBook[shown[cur]], 8, cur * 9 + 50, 15, 0);
			cur = sel;
			Drv_65F2(GuiMageBook[shown[cur]], 8, cur * 9 + 50, 6, 0);
			show_mouse();
		}
		key = gui_process(list);
		if (!key && cursor_in_rect(8, 50, 176, n * 9 + 49))
			sel = (ClickDown[1] - 50) / 9;
		switch (key) {
		case 0x6E:
		case 0x8023:
			n = 0;
			break;
		case 0x8024:
			sel = (ClickUp[1] - 50) / 9;
			if (sel >= n) {
				sel = cur;
				break;
			}
		case 0x2B:
		case 0x3D:
		case 0x61:
			i = shown[sel] - 1;
			item = Party[owner[i]].inventory[slot[i] - 1];
			Party[ch].mageBook |= 1L << i;
			Party[owner[i]].inventory[slot[i] - 1] = 0;
			slot[i] = 0;
			Items[item].block = -1;
			rebuild = 1;
			draw_portrait(owner[i]);
			n--;
			break;
		case 0x60:
			sel = (sel + n - 1) % n;
			break;
		case 0x62:
			sel = (sel + 1) % n;
			break;
		}
	}
	free_buttons(list);
}

/* 4F5F:1BBC The camp (the callback of its button). Returns 1 */
int far camp(Button far *b)
{
	static char keys[2] = { 0x6E, 0 };       /* 0E69 */
	static char keyButton[2] = { 6, 0 };     /* 0E6B */
	byte far *font;
	int style, running;
	unsigned key;
	Button far *list, far *lit, far *target;
	int menu, picture, portraits, step, first, last;
	register int i, next;

	close_panels();
	clock_stop(2);
	font = CurFont;
	style = GuiStyle;
	for (i = 0; i < 6; i++) {
		if (char_is_active(i, 1)) {
			Party[i].lastDamage = 0;
			Party[i].handResult[0] = 0;
			Party[i].handResult[1] = 0;
			draw_portrait(i);
		}
	}
	hide_mouse();
	save_region();
	show_mouse();
	set_font(Font8);
	picture = 0;
	CampPortraits = 0;
	portraits = 0;
	menu = -1;
	next = 0;
	list = 0;
	lit = 0;
	running = 1;
	do {
		if (farheapcheck() != 2) {
			Drv_65F2("Far heap corrupt!", 0, 0, 15, 0);
			flush_keys();
			get_key();
		}
		if (next == 2) {
			sprintf(get_script_string(MenuStrings, 0x11), "Tunes are %-3s", on_off(Var_65FC));
			sprintf(get_script_string(MenuStrings, 0x12), "Sounds are %-3s", on_off(SoundEnabled));
			sprintf(get_script_string(MenuStrings, 0x13), "Bar Graphs are %-3s", on_off(HpBars));
			sprintf(get_script_string(MenuStrings, 0x14), "Mouse is %-3s", on_off(!InventoryMode));
		}
		if (next != -1) {
			free_buttons(list);
			CampMenus[0].title = 1;
			if (!next)
				CampMenus[0].title = 0x38;
			if (next == 2)
				CampMenus[2].title = 0x39;
			if (next == 1)
				CampMenus[1].title = 0x3A;
			list = build_menu(next);
			if (next != menu) {
				target = list;
				lit = 0;
			}
			menu = next;
			next = -1;
		}
		key = gui_process(list);
		key &= 0x80FF;
		for (i = 0; keys[i]; i++)
			if (keys[i] == key) {
				key = keyButton[i] + 0x8001;
				break;
			}
		if (key == 0x2B || key == 0x3D || key == 0x61)
			key = lit->index + 0x8000;
		b = gui_find(list, key & 0x7FFF);
		if (b) {
			draw_button(lit, 0, 0, 1);
			draw_button(b, 1, 1, 1);
			wait_ticks(5);
			draw_button(b, 0, 1, 1);
			lit = 0;
			target = b;
		}
		switch (key) {
		case 0x8004:                     /* scribe scrolls */
			scribe_scroll();
			next = 0;
			break;
		case 0x800A:                     /* drop an NPC */
			step = 0;
			for (i = 0; i < 6; i++)
				if (char_is_active(i, 1))
					step++;
			if (step < 5)
				show_text_box(0x2D);
			else {
				drop_npc(choose_char(0x35));
				hide_mouse();
				draw_play_screen(0);
				picture = 1;
				Drv_65D2(0, 120, 22, 24, LoadBuffer + 20000);
				set_font(GameFont);
				redraw_party();
				set_font(Font8);
				show_mouse();
			}
			next = 0;
			break;
		case 0x8001:                     /* rest */
			if (rest_party())
				running = 0;
			else
				SpellsToRegain = 0;
			portraits = 1;
			next = 0;
			break;
		case 0x8002:                     /* memorize */
			memorize_menu(choose_char(0x17), 0);
			next = 0;
			break;
		case 0x8003:                     /* pray */
			memorize_menu(choose_char(0x1A), 1);
			next = 0;
			break;
		case 0x8005:                     /* the options */
			next = 2;
			break;
		case 0x800D:
			SoundEnabled = SoundEnabled == 0;
			next = 2;
			break;
		case 0x800E:
			HpBars = HpBars == 0;
			next = 2;
			portraits = 1;
			break;
		case 0x8008:                     /* load */
			picture = 1;
			if (!load_game(0, 0))
				next = 1;
			else {
				save_region();
				running = 0;
			}
			show_mouse();
			break;
		case 0x8009:                     /* save */
			if (save_game(0, 0))
				show_text_box(0x0E);
			next = 1;
			break;
		case 0x8007:                     /* exit */
			if (SpellsToRegain)
				show_text_box(0x2C);
		case 0x800C:
		case 0x800F:                     /* back */
			switch (menu) {
			case 0:
				if (key != 0x6E)
					running = 0;
				break;
			case 1:
			case 2:
				next = 0;
				break;
			}
			break;
		case 0x8006:                     /* the game menu */
			next = 1;
			break;
		case 0x800B:                     /* quit */
			if (ask_party(0x2E)) {
				system_shutdown();
				exit(0);
			}
			next = 0;
			break;
		case 0x5B:
		case 0x65:
			target = gui_find(list, CampMenus[menu].first + 1);
			break;
		case 0x5D:
		case 0x67:
			target = gui_find(list, CampMenus[menu].first + CampMenus[menu].count);
			key = 0x60;
		case 0x60:
		case 0x62:
			if (!lit)
				break;
			step = 1;
			if (key == 0x60)
				step = -1;
			i = lit->index + step;
			first = CampMenus[menu].first + 1;
			last = first + CampMenus[menu].count - 1;
			for (;;) {
				if (i < first)
					i = last;
				if (i > last)
					i = first;
				if (CampButtons[i - 1].flags & 2)
					break;
				i += step;
			}
			target = gui_find(list, i);
			break;
		default:
			if (!InventoryMode) {
				for (b = list; b; b = b->next)
					if ((b->arg & 2)
					    && in_box(ClickDown[0], ClickDown[1], b->x, b->y, b->x + b->w, b->y + b->h)) {
						target = b;
						break;
					}
			}
		}
		if (CampPortraits || portraits) {
			for (i = 0; i < 6; i++) {
				draw_portrait(i);
				sort_spells(i);
			}
			CampPortraits = 0;
			portraits = 0;
		}
		if (lit != target && next == -1 && running) {
			draw_button(lit, 0, 0, 1);
			draw_button(target, 0, 1, 1);
			lit = target;
		}
	} while (running);
	free_buttons(list);
	set_font(font);
	Var_62A5 = 2;
	restore_region();
	select_window(style);
	draw_scene(0);
	for (i = 0; i < 6; i++)
		sort_spells(i);
	Var_62A5 = 0;
	hide_mouse();
	select_window(10);
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	if (picture)
		load_picture("INVENT", 0L, 5, 3, 2);
	select_window(style);
	show_mouse();
	Var_62A5 = 0;
	clock_start(2);
	check_party_alive(1);
	return 1;
}

/* 4F5F:23F9 Asks a yes / no question (text: a string of MenuStrings).
 * Returns 1 for yes */
int far ask_party(int text)
{
	Button far *list, far *b;
	int style;
	byte far *font;
	unsigned key;
	int running, yes;
	register int sel, shown;

	style = GuiStyle;
	font = CurFont;
	Var_62A5 = 0;
	set_font(Font8);
	list = build_menu(5);
	hide_mouse();
	print_text(get_script_string(MenuStrings, text), (WinX + 1) * 8, WinY + 4, 15, 0);
	show_mouse();
	shown = -1;
	sel = 0;
	running = 1;
	while (running) {
		if (sel != shown) {
			if (shown != -1)
				draw_button(gui_find(list, shown + 0x21), 0, 0, 1);
			draw_button(gui_find(list, sel + 0x21), 0, 1, 1);
			shown = sel;
		}
		key = gui_process(list);
		switch (key) {
		case 0x5C:
		case 0x66:
			sel ^= 1;
			break;
		case 0x2B:
		case 0x3D:
		case 0x61:
			yes = shown == 0 ? 1 : 0;
			key = shown + 0x8021;
			running = 0;
			break;
		case 0x8021:
			yes = 1;
			running = 0;
			break;
		case 0x8022:
			yes = 0;
			running = 0;
			break;
		default:
			if (!InventoryMode) {
				for (b = list; b; b = b->next)
					if ((b->arg & 2)
					    && in_box(ClickDown[0], ClickDown[1], b->x, b->y, b->x + b->w, b->y + b->h)) {
						sel = b->index - 0x21;
						break;
					}
			}
		}
		if (!running) {
			b = gui_find(list, shown + 0x21);
			draw_button(b, 1, 1, 1);
			wait_ticks(5);
			draw_button(b, 0, 1, 1);
			sel = shown;
		}
	}
	free_buttons(list);
	set_font(font);
	select_window(style);
	return yes;
}
