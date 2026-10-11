/*
 * Eye of the Beholder II - START.EXE
 * Start of the game: the items, the shapes, the main menu, the manual
 * check; overlay module 4315 (Ghidra 5315).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y mainmenu.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern int            VideoMode;         /* 628A */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            TextBg;            /* 636B */
extern int            TextColor;         /* 636D */
extern int            GuiStyle;          /* 636F the current window */
extern Item           Items[];           /* 71CA */
extern ItemType       ItemTypes[];       /* 946E */
extern Character      Party[];           /* E608 */
extern int            ItemCount;         /* D1C8 */
extern int            ItemNameCount;     /* E5A0 */
extern char far       ItemNames[][35];   /* 37DA:0000 */
extern byte far      *ItemIcons[];       /* CEE6 */
extern byte far      *FloorShapes[];     /* DE20 11 large items (ITEML1), 26 small ones from 15 (ITEMS1) */
extern byte far      *ThrownShapes[9];   /* E404 */
extern byte far      *Var_E428[4];       /* E428 */
extern byte far      *FlameShapes[3];    /* EE24 */
extern byte far      *DamageShape;       /* DF26 */
extern byte far      *HandResultShape;   /* DF34 */
extern byte far      *Cursor;            /* CC14 the mouse pointer */
extern byte far      *Shapes[];          /* CC34 */
extern byte far      *Var_E54E[6];       /* E54E */
extern byte far      *DeadShape;         /* E400 the bones of a dead character */
extern byte far      *UnconsciousShape;  /* CB60 */
extern byte far      *BookShape;         /* 71C0 */
extern byte far      *Var_CC58;          /* CC58 */
extern byte far      *Shape_E4CA;        /* E4CA */
extern byte far      *Var_DED4;          /* DED4 */
extern byte far      *Var_92A8[12];      /* 92A8 3 x 4 */
extern char           SelectedChar;      /* EE20 the portrait clicked first (to swap), -1 */
extern void far      *Var_DF40;          /* DF40 */
extern byte far      *MonsterGfx[36];      /* A2D0 */
extern struct { byte far *shape; byte x, y; } DoorButtons[6];   /* 931E */
extern byte far      *DoorShapes[6];     /* EF06 */
extern byte far      *DecoShapes[300];   /* DF50 */
extern byte far      *Palette;           /* F168 */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *Font8;             /* F098 */
extern byte far      *CurFont;           /* F090 */
extern char           Var_9316;          /* 9316 0: a new game, -1 loading, -2 loaded */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 */
extern unsigned char  Var_E5C3, Col_6D80, Col_6D81, Col_9364, Col_929A, Col_92D8;   /* E5C3, 6D80, 6D81, 9364, 929A, 92D8 */
extern unsigned char  Col_71AE, Col_71BF, Col_DF44, Col_E444, Col_DF32;   /* 71AE, 71BF, DF44, E444, DF32 */
extern byte far *(far *Drv_658E)(int x, int y, int w, int h, void far *buf);   /* 658E cuts a shape from the page */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_658A)(int x, int y, int w, int h, byte far *buf);    /* 658A saves a region */
extern void (far     *Drv_6542)(int x, int y, int w, int h, byte far *buf);    /* 6542 restores it */

void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
void set_cursor(int hx, int hy, byte far *shape);                /* 337D:0000 (assembler) */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
int  load_lbm(char far *name, int page, int dest, byte far *pal, int mode);   /* 3095:0264 */
long load_file_into(char far *name, void far *buf, long n);      /* 3069:0000 */
void set_palette(byte far *pal);                                 /* 3111:000D */
byte far *set_font(byte far *font);                              /* 3166:000A */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
int  select_window(int w);                                       /* 31B0:0133 */
void gui_clear(void);                                            /* 31B0:020F */
void draw_menu(int menu, char far * far *items, unsigned long mask, int shift, int spacing);   /* 28A8:015D */
int  menu_choose(int menu, char far * far *items, char far *unused, unsigned long mask, int shift);   /* 28A8:027E */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
int  load_game(int a, int b);                                    /* 559A:04A7 (overlay) */
void create_party(void);                                         /* 4C72:266C (overlay) */
int  transfer_party(void);                                       /* 5881:0EB4 (overlay) */
void play_intro(void);                                           /* 5989:0E63 (overlay) */
void make_test_party(void);                                      /* 4EF4:0598 (overlay) HACK: quick start */
void quit(int code);                                             /* 18D7:0AC3 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void draw_frame3d(int x1, int y1, int x2, int y2, int light, int dark, int fill);   /* 18D7:0A45 */
int  edit_line(char far *text, char x, char y, char max, char fg, char bg, char cursorBg);   /* 18D7:0004 */
char far *get_script_string(char far *text, int index);          /* 1E24:0869 */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
char far *strlwr(char far *s);                                   /* 1000:75B8 */
int  strcmp(char far *a, char far *b);                           /* 1000:7540 */

/* The manual check: 50 symbols (a large item shape, a line, a word) and
 * the words to type */
char far ManualSymbols[50][4] = {                                       /* 3455:0000 */
	{ 9, 3, 4 }, { 9, 13, 2 }, { 9, 7, 7 }, { 20, 6, 5 }, { 20, 16, 6 },
	{ 20, 28, 3 }, { 26, 4, 6 }, { 26, 10, 5 }, { 26, 12, 5 }, { 21, 3, 4 },
	{ 21, 5, 5 }, { 21, 15, 2 }, { 21, 20, 3 }, { 21, 27, 2 }, { 21, 29, 3 },
	{ 35, 1, 3 }, { 35, 3, 3 }, { 35, 8, 3 }, { 35, 25, 2 }, { 35, 31, 4 },
	{ 35, 38, 4 }, { 38, 3, 2 }, { 38, 5, 6 }, { 38, 18, 3 }, { 38, 24, 1 },
	{ 38, 30, 1 }, { 38, 33, 4 }, { 23, 1, 3 }, { 23, 3, 6 }, { 23, 6, 2 },
	{ 23, 15, 5 }, { 23, 27, 1 }, { 23, 33, 6 }, { 18, 3, 2 }, { 18, 5, 4 },
	{ 18, 9, 2 }, { 18, 11, 4 }, { 18, 13, 6 }, { 18, 15, 3 }, { 18, 17, 5 },
	{ 18, 18, 2 }, { 31, 2, 4 }, { 31, 2, 7 }, { 31, 4, 3 }, { 31, 6, 3 },
	{ 31, 9, 3 }, { 31, 10, 1 }, { 28, 3, 3 }, { 28, 4, 2 }, { 28, 5, 6 }
};
char far ManualWords[] =
	"cursor\0majority\0right\0unusable\0greyed\0thrown\0spellcasters\0button\0"
	"characters\0carefree\0practical\0inherit\0while\0combinations\0charm\0"
	"individuals\0gestures\0pummel\0paladins\0skill\0advancement\0also\0"
	"counters\0knowledge\0greater\0assume\0several\0laying\0reach\0swiftly\0"
	"allows\0limited\0jack\0thrown\0weapons\0note\0certain\0damage\0done\0"
	"bonus\0may\0over\0box\0put\0portrait\0backpack\0inside\0causes\0until\0"
	"outright\0";

/* Initialised data of the module (DS:0FB8...) */
byte DoorKnobH[3] = { 0x11, 0x0A, 0x0A };                               /* 0FB8 */
byte DoorKnobY[3] = { 0x78, 0x89, 0x93 };                               /* 0FBB */
byte Rects_0FBE[6][4] = {                                               /* 0FBE */
	{ 12, 0x58, 2, 14 }, { 12, 0x67, 1, 7 }, { 12, 0x6F, 1, 7 },
	{ 12, 0x77, 1, 5 }, { 12, 0x7D, 1, 5 }, { 12, 0x83, 1, 3 }
};
byte Rects_0FD6[6][6] = {                                               /* 0FD6 */
	{ 0, 0, 4, 8 }, { 0, 8, 4, 8 }, { 4, 0, 4, 8 }, { 4, 8, 4, 8 },
	{ 8, 0, 5, 16 }, { 12, 0, 5, 16 }
};

/* 5315:0000 [asm] - the mouse pointer: the first icon of ITEMICN */
void far init_cursor(void)
{
	load_cps("ITEMICN", 5, 3);
	ItemIcons[0] = Drv_658E(0, 0, 2, 16, 0L);
	Cursor = ItemIcons[0];
	set_cursor(0, 0, Cursor);
	Var_62A5 = 0;
}

/* 5315:0062 [asm] - the items of the game, and their names */
void far load_items_dat(void)
{
	register int i, fd;

	fd = open_file("ITEM.DAT", 1);
	read_file(fd, &ItemCount, 2L);
	for (i = 0; i < 600; i++)
		Items[i].block = -1;
	for (i = 0; i < ItemCount; i++)
		read_file(fd, &Items[i], 14L);
	read_file(fd, &ItemNameCount, 2L);
	for (i = 0; i < ItemNameCount; i++)
		read_file(fd, ItemNames[i], 35L);
	close_file(fd);
}

/* 5315:0115 [asm] */
void far load_itemtypes_dat(void)
{
	int n;
	register int i, fd;

	fd = open_file("ITEMTYPE.DAT", 1);
	read_file(fd, &n, 2L);
	for (i = 0; i < n; i++)
		read_file(fd, &ItemTypes[i], 16L);
	close_file(fd);
}

/* 5315:0177 [asm] - the items on the floor, thrown, and their icons */
void far load_item_shapes(void)
{
	register int i;

	load_cps("ITEML1", 5, 3);
	for (i = 0; i < 11; i++)
		FloorShapes[i] = Drv_658E(i / 8 * 8, i % 8 * 24, 8, 24, 0L);
	load_cps("ITEMS1", 5, 3);
	for (i = 0; i < 26; i++)
		FloorShapes[i + 15] = Drv_658E(i / 8 * 4, i % 8 * 24, 4, 24, 0L);
	load_cps("THROWN", 5, 3);
	for (i = 0; i < 9; i++)
		ThrownShapes[i] = Drv_658E((i >> 3) * 4, i % 8 * 24, 4, 24, 0L);
	for (i = 0; i < 4; i++)
		Var_E428[i] = Drv_658E(8, i * 32, 6, 32, 0L);
	FlameShapes[0] = Drv_658E(16, 0, 4, 24, 0L);
	FlameShapes[1] = Drv_658E(16, 24, 4, 24, 0L);
	FlameShapes[2] = Drv_658E(16, 48, 3, 24, 0L);
	DamageShape = Drv_658E(16, 72, 5, 24, 0L);
	HandResultShape = Drv_658E(16, 96, 5, 16, 0L);
	load_cps("ITEMICN", 5, 3);
	for (i = 1; i < 112; i++)
		ItemIcons[i] = Drv_658E(i % 20 * 2, i / 20 * 16, 2, 16, 0L);
	Var_62A5 = 0;
}

/* 5315:03E6 [asm] - no shapes yet; a portrait buffer for characters 2-6 */
void far init_shape_tables(void)
{
	int i;

	SelectedChar = -1;
	Var_DF40 = 0;
	for (i = 0; i < 36; i++)
		MonsterGfx[i] = 0;
	for (i = 0; i < 6; i++) {
		DoorButtons[i].shape = 0;
		DoorShapes[i] = 0;
	}
	for (i = 0; i < 300; i++)
		DecoShapes[i] = 0;
	for (i = 1; i < 6; i++) {
		Party[i].id = i;
		Party[i].face = mem_alloc(1400L, 0);
	}
}

/* 5315:04C7 [asm] - the shapes of the game screen (DECORATE.CPS) */
void far load_game_shapes(void)
{
	char j, k, i;

	load_item_shapes();
	load_cps("DECORATE", 5, 3);
	for (i = 0; i < 6; i++)
		Shapes[i + 3] = Drv_658E(Rects_0FD6[i][0], Rects_0FD6[i][1], Rects_0FD6[i][2], Rects_0FD6[i][3], 0L);
	for (j = 0; j < 6; j++)
		Var_E54E[j] = Drv_658E(Rects_0FBE[j][0], Rects_0FBE[j][1], Rects_0FBE[j][2], Rects_0FBE[j][3], 0L);
	Shapes[0] = Drv_658E(0x1D, 0, 2, 16, 0L);
	Shapes[1] = Drv_658E(0x1F, 0, 2, 16, 0L);
	Shapes[2] = Drv_658E(0x21, 0, 2, 16, 0L);
	DeadShape = Drv_658E(0, 0x58, 4, 32, 0L);
	UnconsciousShape = Drv_658E(4, 0x58, 4, 32, 0L);
	BookShape = Drv_658E(8, 0x58, 4, 16, 0L);
	Var_CC58 = Drv_658E(9, 0x58, 2, 8, 0L);
	Shape_E4CA = Drv_658E(8, 0x68, 4, 8, 0L);
	Var_DED4 = Drv_658E(0x12, 0x58, 4, 64, 0L);
	for (j = 0; j < 3; j++)
		for (k = 0; k < 4; k++)
			Var_92A8[j * 4 + k] = Drv_658E(k * 3, DoorKnobY[j], 3, DoorKnobH[j], 0L);
	Var_62A5 = 0;
}

/* 5315:078F [asm] - the picture of the main menu, with the version */
void far show_menu_screen(void)
{
	byte far *font;
	int old;

	load_lbm("MENU.CPS", 3, 3, Palette, 1);
	if (VideoMode == 5)
		load_file_into("MENU.EGA", Palette, 0x300L);
	set_palette(Palette);
	Drv_654A(3, 2, 0L);
	old = Var_62A5;
	Var_62A5 = 2;
	font = set_font(GameFont);
	Drv_65F2("V 1.1", 0xEA, 0xA0, 13, 0);
	set_font(font);
	Var_62A5 = old;
	copy_page(0, 0, 0, 0, 40, 200, 2, 0);
}

char far *MainMenuItems[5] = {                                          /* 0FFA */
	"LOAD GAME\r",
	"START A NEW PARTY\r",
	"TRANSFER EOB I PARTY\r",
	"QUICK START\r",
	"EXIT\r"
};

/* 5315:086D [asm] - 1 load, 2 new party, 3 transfer, 4 intro, 5 exit */
int far main_menu_choice(void)
{
	register int r;

	select_window(6);
	do {
		draw_menu(6, MainMenuItems, -1L, 0, 0);
		show_mouse();
		while ((r = menu_choose(6, MainMenuItems, 0L, -1L, 0)) == -1)
			;
		hide_mouse();
	} while (r < 0 || r > 5);
	return r + 1;
}

/* 5315:08DB [asm] - the colours of the interface */
void far set_ui_colours(void)
{
	FrameFlat = 0xB7;
	FrameLight = 0xB5;
	FrameDark = 0xBA;
	Var_E5C3 = 0x85;
	Col_6D80 = 0xB8;
	Col_6D81 = 0xB7;
	Col_9364 = 0xB5;
	Col_929A = 0xBA;
	Col_92D8 = 0x11;
	Col_71AE = 0x17;
	Col_71BF = 0x14;
	Col_DF44 = 0xB4;
	Col_E444 = 0xB1;
	Col_DF32 = 0xB6;
}

/* 5315:0926 [asm] - from the start of the game to a party ready to play */
void far main_menu(void)
{
	register int choice;

	init_cursor();
	init_shape_tables();
	load_items_dat();
	load_itemtypes_dat();
	set_ui_colours();
	choice = 4;
	do {
		switch (choice) {
		case 0:
			show_menu_screen();
			choice = main_menu_choice();
			if (choice == 4)		/* HACK: the 4th entry is now QUICK START (was the */
				choice = 7;		/* introduction, which still plays at start: case 4) */
			break;
		case 1:
			Var_9316 = -1;
			show_mouse();
			if (!load_game(0x48, 14)) {
				hide_mouse();
				show_menu_screen();
				show_mouse();
				Var_9316 = 0;
				choice = 0;
			} else {
				Var_9316 = -2;
				choice = 6;
			}
			hide_mouse();
			break;
		case 2:
			set_font(Font8);
			create_party();
			set_font(GameFont);
			Var_9316 = 0;
			choice = 6;
			break;
		case 3:
			if (!transfer_party()) {
				hide_mouse();
				show_menu_screen();
				show_mouse();
				choice = 0;
			} else {
				Var_9316 = 0;
				choice = 6;
			}
			break;
		case 4:
			play_intro();
			choice = 0;
			break;
		case 5:
			quit(0);
		case 7:					/* HACK: QUICK START, the test party of newparty.c */
			set_font(Font8);
			make_test_party();
			set_font(GameFont);
			Var_9316 = 0;
			choice = 6;
			break;
		}
	} while (choice != 6);
}

/* 5315:0A21 [asm] - the manual check: find a word of the manual.
 * 3 wrong words: quit. The shipped START.EXE has one byte patched after
 * the build (patches.txt): the jz after strcmp became a jmp, so any word
 * passes. */
void far manual_check(void)
{
	byte far *font;
	int old;
	char word[30];
	char buf[150];
	register int r, tries;

	old = GuiStyle;
	font = CurFont;
	set_font(Font8);
	Dims[13].b = FrameFlat;
	Var_62A5 = 0;
	select_window(13);
	Drv_658A(WinX, WinY, WinW, WinH, LoadBuffer);
	tries = 0;
	gui_clear();
	draw_frame3d(WinX * 8, WinY, (WinX + WinW) * 8 - 1, WinY + WinH - 1, FrameLight, FrameDark, -1);
	draw_frame3d(WinX * 8 + 1, WinY + 1, (WinX + WinW) * 8 - 2, WinY + WinH - 2, FrameLight, FrameDark, -1);
	Dims[GuiStyle].x++;
	Dims[GuiStyle].y += 2;
	Dims[GuiStyle].w -= 2;
	Dims[GuiStyle].h -= 16;
	select_window(13);
	for (;;) {
		r = roll_dice(1, 50, -1);
		gui_clear();
		put_shape(0, FloorShapes[ManualSymbols[r][0]], 100, 2, 13);
		sprintf(buf, "\r\r\r\rOn the page with this symbol...\r\rFind line %d\rEnter word %d\r",
			ManualSymbols[r][1], ManualSymbols[r][2]);
		print_text(buf, (WinX + 1) * 8, WinY, TextColor, TextBg);
		word[0] = 0;
		draw_frame3d(0x4C, 100, 0xD0, 0x71, FrameDark, FrameLight, -1);
		draw_frame3d(0x4D, 0x65, 0xCF, 0x70, FrameDark, FrameLight, -1);
		while (edit_line(word, 10, 0x67, 15, TextColor, TextBg, 8) < 1)
			;
		while (word[strlen(word) - 1] == ' ')
			word[strlen(word) - 1] = 0;
		strlwr(word);
		/* HACK: the shipped patch (patches.txt) in the source: any word passes */
		strcmp(get_script_string(ManualWords, r), word);
		break;
		tries++;
		if (tries == 3)
			quit(0);
	}
	Dims[GuiStyle].x--;
	Dims[GuiStyle].y -= 2;
	Dims[GuiStyle].w += 2;
	Dims[GuiStyle].h += 16;
	select_window(13);
	Drv_6542(WinX, WinY, WinW, WinH, LoadBuffer);
	select_window(old);
	set_font(font);
}
