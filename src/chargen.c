/*
 * Eye of the Beholder II - START.EXE
 * Creating the party (the character generation screen): overlay module
 * 3C72 (Ghidra 4C72).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y chargen.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The screen is CHARGENA.CPS (portraits), CHARGEN.CPS and CHARGENB.CPS
 * (the buttons). Its far data: the keys and rectangles of its buttons
 * (CgKeys, 3413), the backgrounds of the buttons (CgRects, 3428) and the
 * 17 buttons drawn (CgButtons, 342C).
 */
#include "eob2.h"

typedef unsigned char byte;

/* A button of the screen (9 bytes) */
typedef struct CgButton {
	byte          x, y, w, h;    /* +0 its shape in page 2 (x in bytes), w = 0: none */
	byte          sx, sy;        /* +4 where the shape goes on the background */
	byte          bg;            /* +6 its background (CgRects) */
	byte          px, py;        /* +7 its place on the screen (x in bytes) */
} CgButton;

/* A rectangle of page 2 (4 bytes, x and w in bytes) */
typedef struct CgRect {
	byte          x, y, w, h;
} CgRect;

/* The place and key of a button of the GUI (8 bytes) */
typedef struct CgKey {
	byte          x, y, w, h;    /* +0 x in bytes */
	byte          key;           /* +4 */
	byte          unk[3];        /* +5 */
} CgKey;

extern Character      Party[6];          /* E608 */
extern Button         Buttons[];         /* 39B8 Buttons[0]: the model of the buttons */
extern char far      *RaceNames[];       /* 1F16 */
extern char far      *ClassNames[];      /* 1F46 */
extern char far      *StatLabels[6];     /* 1EE6 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern char           GuiDirty;          /* 6389 redraw the list */
extern char           CharWidth;         /* 6288 */
extern char           Scratch[];         /* F16E */
extern byte far      *CurFont;           /* F090 */
extern byte far      *GameFont;          /* 9342 */
extern Button far    *CgButtonList[19];  /* 6D82 */
extern int NameX[4], NameY[4];   /* 0732, 073A the names under the portraits */
extern char far *CgShapes[17];   /* 0752 */
extern CgKey far CgKeys[41];   /* 3413:0000 */
extern CgRect far CgRects[15];   /* 3428:0000 */
extern CgButton far CgButtons[17];   /* 342C:0000 */
extern char far       MusicDone;         /* 1000:0250 set by the music driver at the end */
extern char far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);   /* 6592 cuts a shape */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

extern int            CgSlot;            /* CB78 the box (character) selected */
extern char far      *PortraitShapes[];  /* D0B2 */
extern char far      *CursorShapes[10];  /* D17A the animated frame of the box */
extern unsigned char  InventoryMode;     /* 6655 the keyboard moves the selection */
/* Uninitialised data: in bss6c6c.c (DS:6C6C-6C73) */
extern char AnimFrame;         /* 6C6C */
extern char AnimDrawn;         /* 6C6D */
extern unsigned long AnimNext; /* 6C6E */
extern int CgFemale;           /* 6C72 the character being made is a woman */
extern volatile unsigned long TimerTicks; /* 667C */
extern int PortX[4], PortY[4];   /* 0742, 074A the 4 boxes */
extern char PortraitOf[4];   /* 079A the portrait of each character, -1 none */
extern char AnimSlot;   /* 079E the box framed */
void flush_keys(void);                                           /* 3295:0516 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
extern int            StatMin[7];        /* CC18 the limits of the abilities while editing */
extern int            StatMax[6];        /* CB64 (strength: the exceptional limit in the high byte) */
Button far *gui_find(Button far *list, unsigned index);          /* 2D3D:01D2 */
int  max_hp(char cls, char con, char l0, char l1, char l2);      /* 1B54:034E */
int  min_hp(char cls, char con, char l0, char l1, char l2);      /* 1B54:040A */
char dex_ac_bonus(char dex);                                     /* 1B54:01BD */
extern Menu           Menus[];           /* 1E2C */
extern int            MenuKey;           /* EF4E the key not used by menu_choose */
extern int            ClickUp[2];        /* 6622 */
extern signed char    ClassThac0Group[]; /* 1ED7 0 warrior 1 wizard 2 priest 3 rogue */
extern byte ClassStart[];   /* 07E7 per class: the 3 levels, the number of classes */
extern int RaceMods[][6];   /* 079F [race][ability] */
extern int RaceMin[][6], RaceMax[][6], ClassMin[][6];   /* 0823, 086B, 08B3 [race or class][ability] */
extern byte BoxX[4], BoxY[4];   /* 0967, 096B */
extern unsigned long ClassMasks[];   /* 096F per race: the classes allowed */
int  roll_level_hp(char ch, char parts);                         /* 1B54:01FB */
void draw_rect(int x1, int y1, int x2, int y2, int colour);      /* 18D7:0571 */
void draw_menu(int menu, char far * far *items, unsigned long mask, int shift, int spacing);   /* 28A8:015D */
int  menu_choose(int menu, char far * far *items, char far *unused, unsigned long mask, int shift);   /* 28A8:027E */
int  menu_count(int n, unsigned long mask, char shift);          /* 28A8:010F */
int  cursor_in_rect(int x1, int y1, int x2, int y2);             /* 3269:0213 */
int  key_pressed(void);                                          /* 3295:041E */
int  read_key(void);                                             /* 3295:066D */
extern char far      *AlignNames[];      /* 1FA9 */
extern unsigned long AlignMasks[];   /* 0987 per class: the alignments allowed */
extern int            VideoMode;         /* 628A */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
int  in_box(int x, int y, int x1, int y1, int x2, int y2);       /* 28A8:00E5 */
int  edit_line(char far *text, char x, char y, char max, char fg, char bg, char cursorBg);   /* 18D7:0004 */
int  select_window(int w);                                       /* 31B0:0133 */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
void load_picture(char far *name, byte far *pal, int a, int page, int mode);   /* 18D7:05C8 */
void start_new_party(void);                                      /* 4EF4:057A (overlay) */
void play_sound(int sound);                                      /* 2C9F:00AA */
int  gui_process(void far *gui);                                 /* 2F86:004E */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void put_shape(int page, char far *shape, int x, int y, int dim);   /* 2DA3:004A */
void mouse_hide_rect(int x1, int y1, int x2, int y2);            /* 338B:0206 */
void mouse_show_rect(void);                                      /* 338B:02CA */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
void movmem(void far *src, void far *dest, unsigned n);
Button far *gui_add_button(Button far *list, Button far *b);     /* 2D3D:0009 */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
byte far *set_font(byte far *font);                              /* 3166:000A */
unsigned strlen(char far *s);                                    /* 1000:7599 */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
char far *format_strength(char str, char ext);                   /* 1B54:04C6 */
void flash_box(int n);                                           /* 1E24:054E */
static void far cut_cg_buttons(void);
static int far cg_gui_process(void far *list);
static void far draw_cg_button(unsigned i, int dx, int mode);
static void far press_cg_button(int i);
Button far * far make_cg_button(int n, int x, int y, int w, int h, int key);
Button far * far make_cg_buttons(int first, int count);
void far show_party_complete(void);
void far print_cg_name(int ch, int len, int colour);
void far show_char_sheet(int ch, char mode);
void far cg_flash(void);
void far choose_portrait(void);
int far next_portrait(int p, int female, int step, char far *used);
int far cg_char_menu(void);
int far cg_select_slot(void);
void far cg_show_prompt(void);
char far roll_stat(void);
void far cg_draw_box(int n);
void far cg_animate(void);
void far modify_stats(void);
void far roll_character(int ch);
int far edit_stat(int w, char far *val, char far *ext);
void far set_stat_limits(void);
int far choose_race(void);
int far nth_bit(int mask, int n, int max);
int far choose_class(int race);
int far choose_alignment(int cls);
void far create_character(void);
void far init_chargen(void);
void far create_party(void);


/* Initialised data of the module (DS:0732), made by tools/chargendata.py; the
 * rest of it after choose_portrait, whose local array comes in between */
int NameX[4] = {   /* 0732 the names under the portraits */
	2, 66, 2, 66
};
int NameY[4] = {   /* 073A  */
	107, 107, 171, 171
};
int PortX[4] = {   /* 0742 the 4 boxes */
	16, 80, 16, 80
};
int PortY[4] = {   /* 074A  */
	63, 63, 127, 127
};
char far *CgShapes[17] = { 0 };   /* 0752 the shapes of the buttons */

/* The far data of the module */
CgKey far CgKeys[41] = {   /* 3413:0000 */
	{ 1, 55, 49, 50, 112, { 0, 0, 0 } },
	{ 9, 55, 49, 50, 113, { 0, 0, 0 } },
	{ 1, 119, 49, 50, 114, { 0, 0, 0 } },
	{ 9, 119, 49, 50, 115, { 0, 0, 0 } },
	{ 3, 181, 83, 16, 26, { 0, 0, 0 } },
	{ 33, 172, 38, 16, 25, { 0, 0, 0 } },
	{ 28, 172, 38, 16, 33, { 0, 0, 0 } },
	{ 33, 172, 38, 16, 50, { 0, 0, 0 } },
	{ 19, 80, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 88, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 96, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 104, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 112, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 120, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 128, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 136, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 144, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 152, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 160, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 168, 154, 8, 0, { 0, 0, 0 } },
	{ 19, 176, 154, 8, 0, { 0, 0, 0 } },
	{ 18, 66, 32, 16, 0, { 0, 0, 0 } },
	{ 18, 82, 32, 16, 0, { 0, 0, 0 } },
	{ 22, 66, 32, 32, 0, { 0, 0, 0 } },
	{ 26, 66, 32, 32, 0, { 0, 0, 0 } },
	{ 30, 66, 32, 32, 0, { 0, 0, 0 } },
	{ 34, 66, 32, 32, 0, { 0, 0, 0 } },
	{ 28, 156, 38, 16, 20, { 0, 0, 0 } },
	{ 33, 156, 38, 16, 52, { 0, 0, 0 } },
	{ 28, 172, 38, 16, 34, { 0, 0, 0 } },
	{ 33, 172, 38, 16, 38, { 0, 0, 0 } },
	{ 18, 128, 53, 8, 0, { 0, 0, 0 } },
	{ 18, 136, 53, 8, 0, { 0, 0, 0 } },
	{ 18, 144, 53, 8, 0, { 0, 0, 0 } },
	{ 18, 152, 53, 8, 0, { 0, 0, 0 } },
	{ 18, 160, 53, 8, 0, { 0, 0, 0 } },
	{ 18, 168, 53, 8, 0, { 0, 0, 0 } },
	{ 29, 136, 53, 8, 0, { 0, 0, 0 } },
	{ 27, 172, 21, 16, 13, { 0, 0, 0 } },
	{ 30, 172, 21, 16, 12, { 0, 0, 0 } },
	{ 33, 172, 37, 16, 25, { 0, 0, 0 } }
};
CgRect far CgRects[15] = {   /* 3428:0000 */
	{ 0, 128, 4, 32 },
	{ 4, 128, 4, 32 },
	{ 8, 128, 4, 32 },
	{ 12, 128, 4, 32 },
	{ 14, 160, 3, 16 },
	{ 11, 160, 3, 16 },
	{ 16, 128, 4, 16 },
	{ 16, 144, 4, 16 },
	{ 17, 160, 5, 16 },
	{ 17, 176, 5, 16 },
	{ 22, 160, 5, 16 },
	{ 22, 176, 5, 16 },
	{ 0, 160, 11, 16 },
	{ 20, 128, 11, 16 },
	{ 20, 144, 11, 16 }
};
CgButton far CgButtons[17] = {   /* 342C:0000 */
	{ 0, 0, 0, 0, 0, 0, 0, 2, 64 },
	{ 0, 0, 0, 0, 0, 0, 1, 10, 64 },
	{ 0, 0, 0, 0, 0, 0, 2, 2, 128 },
	{ 0, 0, 0, 0, 0, 0, 3, 10, 128 },
	{ 0, 192, 4, 5, 7, 5, 8, 28, 156 },
	{ 4, 192, 3, 5, 10, 5, 8, 33, 172 },
	{ 7, 192, 3, 5, 11, 5, 8, 33, 172 },
	{ 10, 192, 4, 5, 6, 5, 8, 33, 156 },
	{ 24, 192, 3, 5, 9, 5, 8, 28, 172 },
	{ 14, 192, 2, 5, 15, 5, 8, 33, 172 },
	{ 16, 192, 1, 5, 9, 5, 4, 27, 172 },
	{ 17, 192, 1, 1, 9, 7, 4, 30, 172 },
	{ 18, 192, 3, 7, 7, 4, 6, 18, 66 },
	{ 21, 192, 3, 7, 7, 4, 6, 18, 82 },
	{ 0, 0, 0, 0, 0, 0, 12, 3, 181 },
	{ 0, 0, 0, 0, 0, 0, 13, 3, 181 },
	{ 0, 0, 0, 0, 0, 0, 10, 28, 172 }
};
/* 4C72:0000 Cuts the shapes of the buttons from page 2 */
static void far cut_cg_buttons(void)
{
	register int i;

	for (i = 0; i < 17; i++) {
		CgShapes[i] = 0;
		if (CgButtons[i].w)
			CgShapes[i] = Drv_6592(CgButtons[i].x, CgButtons[i].y, CgButtons[i].w, CgButtons[i].h, 0L);
	}
}

/* 4C72:00B0 gui_process, the music started again when it ended */
static int far cg_gui_process(void far *list)
{
	if (MusicDone == 1) {
		MusicDone = 0;
		play_sound(13);
	}
	return gui_process(list);
}

/* 4C72:00E3 Draws button i (its background dx further in CgRects): mode 0
 * at rest, 1 pressed (both on the screen), 2 only in page 2 */
static void far draw_cg_button(unsigned i, int dx, int mode)
{
	int x, y;
	register int bg;

	if (i < 17) {
		bg = CgButtons[i].bg + dx;
		if (mode == 0) {
			x = 20;
			y = 0;
		} else {
			x = CgButtons[i].px + 2;
			y = CgButtons[i].py - 64;
		}
		copy_page(CgRects[bg].x, CgRects[bg].y, x, y, CgRects[bg].w, CgRects[bg].h, 2, 2);
		if (CgButtons[i].w)
			put_shape(2, CgShapes[i], x * 8 + CgButtons[i].sx, y + CgButtons[i].sy, 0);
		if (mode != 2) {
			mouse_hide_rect(CgButtons[i].px * 8, CgButtons[i].py, (CgButtons[i].px + CgRects[bg].w) * 8,
			                CgButtons[i].py + CgRects[bg].h);
			copy_page(20, 0, CgButtons[i].px, CgButtons[i].py, CgRects[bg].w, CgRects[bg].h, 2, 0);
			mouse_show_rect();
		}
	}
}

/* 4C72:0335 A button pressed and released */
static void far press_cg_button(int i)
{
	draw_cg_button(i, 1, 0);
	play_sound(76);
	wait_ticks(5);
	draw_cg_button(i, 0, 0);
}

/* 4C72:0370 Button n of the GUI */
Button far * far make_cg_button(int n, int x, int y, int w, int h, int key)
{
	Button far *b;

	b = CgButtonList[n];
	movmem(&Buttons[0], b, 60);
	b->index = n + 1;
	b->key = key;
	b->key2 = key + 0x100;
	b->x = x * 8;
	b->y = y;
	b->w = w;
	b->h = h;
	b->flags = 0x1100;
	return b;
}

/* 4C72:03F4 The GUI of count buttons (CgKeys from first) */
Button far * far make_cg_buttons(int first, int count)
{
	Button far *list;
	register int i, k;

	list = CgButtonList[0];
	for (i = 0; i < count; i++) {
		k = first + i;
		make_cg_button(i, CgKeys[k].x, CgKeys[k].y, CgKeys[k].w, CgKeys[k].h, CgKeys[k].key);
	}
	for (i = 1; i < count; i++)
		list = gui_add_button(list, CgButtonList[i]);
	GuiDirty = 1;
	return list;
}

/* 4C72:04C9 With 4 characters, "Your party is complete" and the PLAY button */
void far show_party_complete(void)
{
	char i, n;

	n = 0;
	for (i = 0; i < 4; i++)
		if (Party[i].name[0])
			n++;
	n = n == 4;
	if (n) {
		Var_62A5 = 2;
		print_text("Your party is\rcomplete. Select\rthe PLAY button\ror press 'P' to\rstart the game.", 168, 61, 15, 0);
		Var_62A5 = 0;
		hide_mouse();
		copy_page(21, 61, 19, 125, 17, 40, 2, 0);
		show_mouse();
		draw_cg_button(15, 0, 0);
	} else
		draw_cg_button(14, 0, 0);
}

/* 4C72:0581 The name of character ch under its portrait, centred */
void far print_cg_name(int ch, int len, int colour)
{
	int x;
	byte far *font;

	font = CurFont;
	set_font(GameFont);
	x = (60 - CharWidth * len) >> 1;
	hide_mouse();
	Drv_65F2("          ", NameX[ch], NameY[ch], 12, 12);
	Drv_65F2(Party[ch].name, NameX[ch] + x, NameY[ch], colour, 0);
	show_mouse();
	set_font(font);
}

/* 4C72:0624 The sheet of character ch on the right; mode: the buttons
 * under it (1 creating, 2 viewing, 3 ...) */
void far show_char_sheet(register int ch, char mode)
{
	char far *s;
	register int i;

	hide_mouse();
	copy_page(0, 0, 20, 0, 20, 128, 2, 2);
	Var_62A5 = 2;
	if (mode != 4)
		put_shape(2, Party[ch].face, 224, 2, 0);
	s = Party[ch].name;
	print_text(s, (20 - strlen(s)) * 4 + 160, 35, 15, 0);
	s = RaceNames[Party[ch].raceSex];
	print_text(s, (20 - strlen(s)) * 4 + 160, 45, 15, 0);
	s = ClassNames[Party[ch].cls];
	print_text(s, (20 - strlen(s)) * 4 + 160, 54, 15, 0);
	for (i = 8; i < 14; i++)
		print_text(StatLabels[i - 8], 163, i * 8, 15, 0);
	print_text("AC\rHP\rLVL", 248, 64, 15, 0);
	sprintf(Scratch, "%s\r%d\r%d\r%d\r%d\r%d", format_strength(Party[ch].str, Party[ch].strExt),
	        Party[ch].intel, Party[ch].wis, Party[ch].dex, Party[ch].con, Party[ch].cha);
	print_text(Scratch, 192, 64, 15, 0);
	sprintf(Scratch, "%d\r%d", Party[ch].ac, Party[ch].hpMax);
	print_text(Scratch, 280, 64, 15, 0);
	s = "%d";
	if (Party[ch].level[1] > 0)
		s = "%d/%d";
	if (Party[ch].level[2] > 0)
		s = "%d/%d/%d";
	sprintf(Scratch, s, Party[ch].level[0], Party[ch].level[1], Party[ch].level[2]);
	print_text(Scratch, 280, 80, 15, 0);
	switch (mode) {
	case 1:
		draw_cg_button(4, 0, 2);
		draw_cg_button(7, 0, 2);
		draw_cg_button(8, 0, 2);
		draw_cg_button(6, 0, 2);
		break;
	case 2:
		draw_cg_button(16, 0, 2);
		draw_cg_button(9, 0, 2);
		break;
	case 3:
		draw_cg_button(10, 0, 2);
		draw_cg_button(11, 0, 2);
		draw_cg_button(9, 0, 2);
		break;
	}
	copy_page(20, 0, 18, 64, 20, 128, 2, 0);
	show_mouse();
	Var_62A5 = 0;
}

/* 4C72:0A1A */
void far cg_flash(void)
{
	flash_box(10);
}

/* 4C72:0A2A Choosing the portrait of the character: 4 at a time (not those
 * of the other characters, men 0-28, women 26-43), arrows to scroll */
void far choose_portrait(void)
{
	unsigned key;
	char p, i, first, sel, chosen;
	char shown[4] = { 0 };
	Button far *list;

	flush_keys();
	list = make_cg_buttons(21, 6);
	p = CgFemale ? 26 : 0;
	show_char_sheet(CgSlot, 4);
	draw_cg_button(12, 0, 0);
	draw_cg_button(13, 0, 0);
	flash_box(-1);
	p = next_portrait(p, CgFemale, 1, PortraitOf);
	chosen = -1;
	sel = 1;
	while (chosen == -1) {
		first = p;
		hide_mouse();
		for (i = 0; i < 4; i++) {
			shown[i] = p;
			put_shape(0, PortraitShapes[shown[i]], i * 32 + 176, 66, 0);
			p = next_portrait(p + 1, CgFemale, 1, PortraitOf);
		}
		show_mouse();
		p = first;
		do {
			cg_animate();
			key = cg_gui_process(list);
			if (InventoryMode)
				flash_box(sel + 10);
			switch (key) {
			case 0x8003:
			case 0x8004:
			case 0x8005:
			case 0x8006:
				sel = key - 0x8003;
			case 0x2B:
			case 0x61:
				key = 3;
				break;
			case 0x5C:
			case 0x8001:
				press_cg_button(12);
				key = 1;
				break;
			case 0x66:
			case 0x8002:
				press_cg_button(13);
				key = 2;
				break;
			default:
				if (key & 0x8000)
					key &= 0xFF;
				else
					key = 0;
			}
		} while (!key);
		if (InventoryMode)
			flash_box(-1);
		switch (key) {
		case 1:
			if (sel == 0 || !InventoryMode)
				p = next_portrait(p - 1, CgFemale, -1, PortraitOf);
			else
				sel--;
			break;
		case 2:
			if (sel == 3 || !InventoryMode)
				p = next_portrait(p + 1, CgFemale, 1, PortraitOf);
			else
				sel++;
			break;
		case 3:
			chosen = sel;
			break;
		}
	}
	flash_box(-1);
	cg_animate();
	PortraitOf[CgSlot] = shown[chosen];
	Party[CgSlot].portrait = shown[chosen];
	mem_copy(PortraitShapes[shown[chosen]], Party[CgSlot].face, 0x578L);
	show_char_sheet(CgSlot, 1);
}


/* The data of the module, continued (DS:079A) */
char PortraitOf[4] = {   /* 079A the portrait of each character, -1 none */
	-1, -1, -1, -1
};
char AnimSlot = 6;   /* 079E the box framed */
int RaceMods[6][6] = {   /* 079F per race: added to the abilities rolled */
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, -1, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, -1,
	0, 1, -1, 0, 0, 0,
	-1, 0, 0, 1, 0, 0
};
byte ClassStart[60] = {   /* 07E7 per class: the 3 levels, the number of classes */
	7, 0, 0, 1,
	6, 0, 0, 1,
	6, 0, 0, 1,
	7, 0, 0, 1,
	7, 0, 0, 1,
	7, 0, 0, 1,
	6, 6, 0, 2,
	6, 6, 0, 2,
	6, 5, 0, 2,
	5, 5, 6, 3,
	6, 5, 0, 2,
	6, 6, 0, 2,
	5, 5, 5, 3,
	5, 6, 0, 2,
	6, 5, 0, 2
};
int RaceMin[6][6] = {   /* 0823 per race: the lowest abilities */
	3, 3, 3, 3, 3, 3,
	3, 8, 3, 7, 6, 8,
	3, 4, 3, 6, 6, 3,
	8, 3, 3, 3, 12, 2,
	6, 7, 2, 3, 8, 3,
	7, 6, 3, 8, 10, 6
};
int RaceMax[6][6] = {   /* 086B the highest (strength: 18 + the exceptional limit * 256) */
	25618, 18, 18, 18, 18, 18,
	25618, 18, 18, 19, 17, 18,
	25618, 18, 18, 18, 18, 18,
	25618, 18, 18, 17, 19, 16,
	25618, 19, 17, 18, 18, 18,
	17, 18, 17, 19, 18, 18
};
int ClassMin[15][6] = {   /* 08B3 per class: the lowest abilities */
	9, 0, 0, 0, 0, 0,
	13, 0, 14, 13, 14, 0,
	12, 0, 13, 0, 9, 17,
	0, 9, 0, 0, 0, 0,
	0, 0, 9, 0, 0, 0,
	0, 0, 0, 9, 0, 0,
	9, 0, 9, 0, 0, 0,
	9, 0, 0, 9, 0, 0,
	9, 9, 0, 0, 0, 0,
	9, 9, 0, 9, 0, 0,
	0, 9, 0, 9, 0, 0,
	0, 0, 9, 9, 0, 0,
	9, 9, 9, 0, 0, 0,
	13, 0, 14, 13, 14, 0,
	0, 9, 9, 0, 0, 0
};
byte BoxX[4] = {   /* 0967 the frame of the box being created */
	16, 80, 16, 80
};
byte BoxY[4] = {   /* 096B  */
	63, 63, 127, 127
};
unsigned long ClassMasks[6] = {   /* 096F per race: the classes allowed */
	63, 1979, 30715, 241, 2289, 177
};
unsigned long AlignMasks[15] = {   /* 0987 per class: the alignments allowed */
	511, 7, 1, 511, 511,
	510, 511, 510, 511, 510,
	510, 510, 511, 7, 511
};
/* 4C72:0D43 The next portrait from p (by step) not taken (used) and of
 * the right sex */
int far next_portrait(int p, int female, int step, char far *used)
{
	register int i;
	int clash;

	if (p < 0)
		p = 43;
	p %= 44;
	do {
		clash = 0;
		for (i = 0; i < 4; i++)
			if (Party[i].name[0] && used[i] == p)
				clash = 1;
		if (female && p < 26)
			clash = 1;
		if (!female && p > 28)
			clash = 1;
		if (clash) {
			p += step;
			if (p < 0)
				p = 43;
			p %= 44;
		}
	} while (clash);
	return p;
}

/* 4C72:0DC4 The character just made: modify, its portrait, its name... */
int far cg_char_menu(void)
{
	Button far *list;
	register unsigned key;

	choose_portrait();
	show_char_sheet(CgSlot, 1);
	list = make_cg_buttons(27, 4);
	flush_keys();
	do {
		cg_animate();
		key = cg_gui_process(list);
		switch (key) {
		case 0x8001:
			press_cg_button(4);
			cg_animate();
			roll_character(CgSlot);
			key = -1;
			break;
		case 0x8002:
			press_cg_button(7);
			modify_stats();
			key = -1;
			break;
		case 0x8003:
			press_cg_button(8);
			choose_portrait();
			key = -1;
			break;
		case 0x61:
		case 0x8004:
			press_cg_button(6);
			key = 1;
			break;
		default:
			key = 0;
		}
		if (key & 0x8000) {
			show_char_sheet(CgSlot, 1);
			list = make_cg_buttons(27, 4);
			key = 0;
		}
	} while (!key);
	flash_box(CgSlot + 6);
	flash_box(-1);
	return 0;
}

/* 4C72:0ED3 Choosing a box (a character to create or view); returns 1 for
 * PLAY */
int far cg_select_slot(void)
{
	unsigned key;
	int result;
	Button far *list;
	register int s, done;

	list = make_cg_buttons(0, 7);
	flush_keys();
	flash_box(-1);
	result = 0;
	done = 0;
	show_char_sheet(CgSlot, 2);
	while (Party[CgSlot].name[0] && !done) {
		flash_box(CgSlot + 6);
		key = cg_gui_process(list);
		s = CgSlot;
		switch (key) {
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
			s = (key & 0xFF) - 1;
			break;
		case 0x8005:
			result = 1;
			done = 1;
			break;
		case 0x2B:
		case 0x3D:
		case 0x61:
		case 0x6E:
		case 0x8006:
			press_cg_button(9);
			result = 0;
			done = 1;
			break;
		case 0x8007:
			if (Party[CgSlot].name) {    /* sic: always (an array) */
				press_cg_button(16);
				Party[CgSlot].name[0] = 0;
				print_cg_name(CgSlot, 1, 12);
				cg_draw_box(CgSlot + 50);
			}
			break;
		case 0x5C:
		case 0x66:
			s ^= 1;
			break;
		case 0x60:
		case 0x62:
			s ^= 2;
			break;
		}
		if (done)
			flash_box(-1);
		if (Party[s].name[0] == 0)
			done = 1;
		if (s != CgSlot) {
			CgSlot = s;
			flash_box(-1);
			if (!done)
				show_char_sheet(CgSlot, 2);
		}
	}
	return result;
}

/* 4C72:1077 "Select the box of the character you wish to create or view." */
void far cg_show_prompt(void)
{
	int page;

	copy_page(0, 0, 20, 0, 20, 128, 2, 2);
	page = Var_62A5;
	Var_62A5 = 2;
	print_text("Select the box of\rthe character you\rwish to create or\rview.", 168, 16, 15, 0);
	Var_62A5 = page;
	hide_mouse();
	copy_page(20, 0, 18, 64, 20, 128, 2, 0);
	show_mouse();
	show_party_complete();
}

/* 4C72:1102 4d6, the lowest die dropped */
char far roll_stat(void)
{
	int sum, low;
	register int i, r;

	low = 10;
	sum = 0;
	for (i = 0; i < 4; i++) {
		sum += r = roll_dice(1, 6, 0);
		if (r < low)
			low = r;
	}
	sum -= low;
	return sum;
}

/* 4C72:114B Box n of the screen redrawn: n > 48 the empty box n - 50, else
 * the portrait of the current character */
void far cg_draw_box(int n)
{
	flash_box(-1);
	hide_mouse();
	if (n > 48) {
		n -= 50;
		draw_cg_button(n, 0, 0);
	} else
		put_shape(0, Party[CgSlot].face, PortX[CgSlot], PortY[CgSlot] + 1, 0);
	show_mouse();
}

/* 4C72:11B6 The animated frame around the box of the current character
 * (10 frames, every 2 ticks) */
void far cg_animate(void)
{
	if (AnimSlot != CgSlot) {
		AnimNext = 0;
		AnimSlot = CgSlot;
	}
	if (AnimNext < TimerTicks) {
		AnimFrame++;
		if (AnimFrame > 9)
			AnimFrame = 0;
		AnimNext = TimerTicks + 2;
	}
	if (AnimDrawn != AnimFrame) {
		copy_page(CgSlot * 4, 128, 36, 128, 4, 32, 2, 2);
		put_shape(2, CursorShapes[AnimFrame], 288, 128, 0);
		mouse_hide_rect(PortX[CgSlot], PortY[CgSlot] + 1, PortX[CgSlot] + 32, PortY[CgSlot] + 32);
		copy_page(36, 128, PortX[CgSlot] >> 3, PortY[CgSlot] + 1, 4, 32, 2, 0);
		mouse_show_rect();
		AnimDrawn = AnimFrame;
	}
}

/* 4C72:12EF Modifying ability which (0 strength... 6 hit points) of the
 * current character with the arrows; val and ext: the value and the
 * exceptional strength. Returns the ability to edit next, -2 when done */
int far edit_stat(register int w, char far *val, char far *ext)
{
	int next, v, e, done;
	Button far *list;
	Button far *btn;
	int old;
	register unsigned key;

	list = make_cg_buttons(31, 10);
	btn = gui_find(list, w + 1);
	show_char_sheet(CgSlot, 3);
	flush_keys();
	if (w == 0)
		sprintf(Scratch, "%s", format_strength(*val, *ext));
	else
		sprintf(Scratch, "%d", *val);
	hide_mouse();
	copy_page(((btn->x + 32) >> 3) - 18, btn->y - 64, (btn->x + 32) >> 3, btn->y, 5, btn->h, 2, 0);
	print_text(Scratch, btn->x + 32, btn->y, 6, 0);
	show_mouse();
	done = 0;
	e = *ext;
	next = w;
	if (w == 6) {
		/* ORIGINAL BUG (sic): StatMax[6] is past the end of the array, on
		 * LastHitMonster (CB70, party.c) and the byte after it */
		StatMax[6] = max_hp(Party[CgSlot].cls, Party[CgSlot].con, Party[CgSlot].level[0], Party[CgSlot].level[1], Party[CgSlot].level[2]);
		StatMin[6] = min_hp(Party[CgSlot].cls, Party[CgSlot].con, Party[CgSlot].level[0], Party[CgSlot].level[1], Party[CgSlot].level[2]);
	}
	while (!done) {
		v = *val;
		cg_animate();
		key = cg_gui_process(list);
		switch (key) {
		case 0x60:
			next--;
			next %= 7;
			done = 1;
			break;
		case 0x62:
			next++;
			next %= 7;
			done = 1;
			break;
		case 0x0D:
		case 0x66:
		case 0x6A:
		case 0x8008:
			press_cg_button(10);
			v++;
			break;
		case 0x0C:
		case 0x5C:
		case 0x69:
		case 0x8009:
			press_cg_button(11);
			v--;
			break;
		case 0x61:
		case 0x6E:
		case 0x800A:
			press_cg_button(9);
			done = 1;
			key = -1;
			break;
		default:
			if (key & 0x8000) {
				key &= 0xFF;
				key--;
				if (key != w) {
					next = key;
					done = 1;
				}
			}
			break;
		}
		if (*val != v) {
			if (w == 0) {
				while (v > 18) {
					v--;
					e++;
				}
				while (e > 0 && v < 18) {
					v++;
					e--;
				}
			}
			if (w == 0) {
				v = StatMin[w] < v ? v : StatMin[w];
				v = (StatMax[w] & 0xFF) > v ? v : StatMax[w] & 0xFF;
				*val = v;
				if (v < 18 || StatMax[w] < 19)
					e = 0;
				if (e > 100)
					e = 100;
				*ext = e;
			} else {
				v = StatMin[w] < v ? v : StatMin[w];
				v = StatMax[w] > v ? v : StatMax[w];
				*val = v;
			}
			if (w == 6)
				Party[CgSlot].hpMax = v;
			if (w == 0)
				sprintf(Scratch, "%s", format_strength(*val, *ext));
			else
				sprintf(Scratch, "%d", *val);
			hide_mouse();
			copy_page(((btn->x + 32) >> 3) - 18, btn->y - 64, (btn->x + 32) >> 3, btn->y, 5, btn->h, 2, 0);
			print_text(Scratch, btn->x + 32, btn->y, 6, 0);
			if (w == 4) {
				old = Party[CgSlot].hp;
				/* ORIGINAL BUG (sic): StatMax[6] is past the end of the array, on
				 * LastHitMonster (CB70, party.c) and the byte after it */
				StatMax[6] = max_hp(Party[CgSlot].cls, Party[CgSlot].con, Party[CgSlot].level[0], Party[CgSlot].level[1], Party[CgSlot].level[2]);
				StatMin[6] = min_hp(Party[CgSlot].cls, Party[CgSlot].con, Party[CgSlot].level[0], Party[CgSlot].level[1], Party[CgSlot].level[2]);
				Party[CgSlot].hp = Party[CgSlot].hpMax = Party[CgSlot].hp < StatMin[6] ? StatMin[6] : Party[CgSlot].hp;
				Party[CgSlot].hp = Party[CgSlot].hpMax = Party[CgSlot].hp > StatMax[6] ? StatMax[6] : Party[CgSlot].hp;
				if (Party[CgSlot].hp != old) {
					sprintf(Scratch, "%d", Party[CgSlot].hp);
					copy_page(15, 72, 33, 136, 5, 8, 2, 0);
					print_text(Scratch, 264, 136, 15, 0);
				}
			}
			if (w == 3) {
				old = Party[CgSlot].ac;
				Party[CgSlot].ac = dex_ac_bonus(v) + 10;
				if (Party[CgSlot].ac != old) {
					sprintf(Scratch, "%d", Party[CgSlot].ac);
					copy_page(15, 64, 33, 128, 5, 8, 2, 0);
					print_text(Scratch, 264, 128, 15, 0);
				}
			}
			show_mouse();
			done = 0;
		} else if (done) {
			if (key == -1)
				next = -2;
			if (w == 0)
				sprintf(Scratch, "%s", format_strength(*val, *ext));
			else
				sprintf(Scratch, "%d", *val);
			Var_62A5 = 0;
			hide_mouse();
			Drv_65F2(Scratch, (btn->x + 32) * 8, btn->y, 15, 0);
			show_mouse();
		}
	}
	return next;
}

/* 4C72:1A8A MODIFY: editing the 6 abilities and the hit points in turn */
void far modify_stats(void)
{
	char hp;
	register int i;

	i = 0;
	flush_keys();
	show_char_sheet(CgSlot, 3);
	while (i >= 0 && i <= 6) {
		switch (i) {
		case 0:
			i = edit_stat(i, &Party[CgSlot].str, &Party[CgSlot].strExt);
			break;
		case 1:
			i = edit_stat(i, &Party[CgSlot].intel, 0L);
			break;
		case 2:
			i = edit_stat(i, &Party[CgSlot].wis, 0L);
			break;
		case 3:
			i = edit_stat(i, &Party[CgSlot].dex, 0L);
			break;
		case 4:
			i = edit_stat(i, &Party[CgSlot].con, 0L);
			break;
		case 5:
			i = edit_stat(i, &Party[CgSlot].cha, 0L);
			break;
		case 6:
			hp = Party[CgSlot].hp;
			i = edit_stat(i, &hp, 0L);
			Party[CgSlot].hp = hp;
			break;
		}
		show_char_sheet(CgSlot, 3);
		if (i == -2)
			break;
		if (i < 0)
			i = 6;
		i %= 7;
	}
	show_char_sheet(CgSlot, 1);
}

/* 4C72:1C06 REROLL: the levels and experience of the class, the abilities
 * (4d6 without the lowest, + the race, within the limits; 18 strength gets
 * an exceptional d100 for those who may have it), the AC and hit points */
void far roll_character(int ch)
{
	Character far *c;
	int st[6];
	register int i, j;

	c = &Party[ch];
	for (i = 0; i < 3; i++) {
		c->level[i] = ClassStart[c->cls * 4 + i];
		c->experience[i] = 69000L / ClassStart[c->cls * 4 + 3];
	}
	set_stat_limits();
	for (i = 0; i < 6; i++) {
		st[i] = roll_stat() + RaceMods[c->raceSex >> 1][i];
		if (st[i] < StatMin[i])
			st[i] = StatMin[i];
		if (i == 0 && st[0] == 18)
			st[0] |= roll_dice(1, 100, 0) << 8;
		if (st[i] > StatMax[i])
			st[i] = StatMax[i];
	}
	c->str = c->strMax = st[0] & 0xFF;
	c->strExt = c->strExtMax = (st[0] & 0xFF00) >> 8;
	c->intel = c->intelMax = st[1];
	c->wis = c->wisMax = st[2];
	c->dex = c->dexMax = st[3];
	c->con = c->conMax = st[4];
	c->cha = c->chaMax = st[5];
	c->ac = dex_ac_bonus(st[3]) + 10;
	c->hp = 0;
	for (j = 0; j < c->level[0]; j++)
		c->hp += roll_level_hp(ch, 1);
	for (j = 0; j < c->level[1]; j++)
		c->hp += roll_level_hp(ch, 2);
	for (j = 0; j < c->level[2]; j++)
		c->hp += roll_level_hp(ch, 4);
	c->hpMax = c->hp;
}

/* 4C72:1E30 The limits of the abilities of the current character: the
 * highest of the class and race minimums, the race maximums */
void far set_stat_limits(void)
{
	int race;
	register int i, cls;

	race = Party[CgSlot].raceSex >> 1;
	cls = Party[CgSlot].cls;
	for (i = 0; i < 6; i++) {
		StatMin[i] = ClassMin[cls][i] > RaceMin[race][i] ? ClassMin[cls][i] : RaceMin[race][i];
		StatMax[i] = RaceMax[race][i];
		if (ClassThac0Group[cls])
			StatMax[0] = 18;
	}
}

/* 4C72:1EED Choosing the race (and sex) */
int far choose_race(void)
{
	register int r;

	Var_62A5 = 0;
	hide_mouse();
	draw_rect(BoxX[CgSlot], BoxY[CgSlot], BoxX[CgSlot] + 32, BoxY[CgSlot] + 33, 12);
	copy_page(0, 0, 18, 64, 20, 128, 2, 0);
	print_text("SELECT RACE:", 147, 67, 9, 0);
	show_mouse();
	flush_keys();
	Menus[1].sel = 0;
	draw_menu(1, RaceNames, 0xFFFFFFFFL, 0, 0);
	while ((r = menu_choose(1, RaceNames, 0L, 0xFFFFFFFFL, 0)) == -1)
		cg_animate();
	CgFemale = r % 2;
	return r;
}

/* 4C72:1FD8 The place of the n-th bit set in mask (not called) */
int far nth_bit(int mask, int n, int max)
{
	int i;

	for (i = 0; i < max; i++)
		if (mask & (1 << i)) {
			n--;
			if (!n)
				break;
		}
	return i;
}

/* 4C72:2007 Choosing the class among those of the race (no paladin or
 * ranger... with an evil character in the party: mask -5); returns 0x6E
 * for back */
int far choose_class(int race)
{
	int key, mask;
	register int c, done;

	mask = -1;
	for (c = 0; c < 4; c++)
		if (Party[c].name[0] && Party[c].alignment >= 6)
			mask = -5;
	race /= 2;
	c = menu_count(15, ClassMasks[race] & mask, 0);
	Menus[2].count = c;
	Menus[2].sel = 0;
	flush_keys();
	cg_animate();
	hide_mouse();
	copy_page(0, 0, 18, 64, 20, 128, 2, 0);
	print_text("SELECT CLASS:", 147, 67, 9, 0);
	draw_cg_button(5, 0, 0);
	show_mouse();
	draw_menu(2, ClassNames, ClassMasks[race] & mask, 0, 0);
	done = ClickUp[0] = ClickUp[1] = 0;
	c = -1;
	while (!done && c == -1) {
		cg_animate();
		if (cursor_in_rect(264, 171, 303, 187)) {
			if ((key = key_pressed()) != 0 && (key == 0xC6 || key == 0xC7)) {
				read_key();
				done = 1;
			}
		} else {
			c = menu_choose(2, ClassNames, 0L, ClassMasks[race] & mask, 0);
			if (MenuKey == 0x6E || MenuKey == 0x32)
				done = 1;
		}
	}
	if (done) {
		press_cg_button(5);
		c = 0x6E;
	}
	return c;
}

/* 4C72:21DD Choosing the alignment among those of the class (only good
 * ones with a paladin in the party: mask 0xFE3F); returns 0x6E for back */
int far choose_alignment(int cls)
{
	int mask;
	register int a, done;

	mask = -1;
	for (a = 0; a < 4; a++)
		if (Party[a].name[0] && Party[a].cls == 2)
			mask = 0xFE3F;
	a = menu_count(9, AlignMasks[cls] & mask, 0);
	Menus[3].count = a;
	Menus[3].sel = 0;
	cg_animate();
	hide_mouse();
	copy_page(0, 0, 18, 64, 20, 128, 2, 0);
	print_text("SELECT ALIGNMENT:", 147, 67, 9, 0);
	draw_cg_button(5, 0, 0);
	show_mouse();
	draw_menu(3, AlignNames, AlignMasks[cls] & mask, 0, 0);
	flush_keys();
	done = 0;
	while (!done && (a = menu_choose(3, AlignNames, 0L, AlignMasks[cls] & mask, 0)) == -1) {
		cg_animate();
		if (MenuKey == 0x6E || MenuKey == 0x32)
			done = 1;
		if ((MenuKey == 0x41 || MenuKey == 0x42) && in_box(ClickUp[0], ClickUp[1], 265, 171, 303, 187))
			done = 1;
	}
	if (done) {
		a = 0x6E;
		press_cg_button(5);
	}
	return a;
}

/* 4C72:238F Creating the character of the current box: race, class,
 * alignment (back goes one step back), the abilities, the portrait, the
 * name */
void far create_character(void)
{
	int len;
	register int step, back;

	select_window(2);
	step = 0;
	do {
		back = 0;
		switch (step) {
		case 0:
			Party[CgSlot].raceSex = choose_race();
			break;
		case 1:
			back = (Party[CgSlot].cls = choose_class(Party[CgSlot].raceSex)) == 0x6E;
			break;
		case 2:
			back = (Party[CgSlot].alignment = choose_alignment(Party[CgSlot].cls)) == 0x6E;
			break;
		}
		step++;
		if (back)
			step -= 2;
	} while (step != 3);
	roll_character(CgSlot);
	cg_char_menu();
	Party[CgSlot].name[0] = 0;
	do {
		cg_draw_box(StatMin[6]);         /* sic: the minimum hit points (<= 48: the portrait) */
		show_char_sheet(CgSlot, 0);
		hide_mouse();
		print_text("Name:", 149, 100, 9, 0);
		len = edit_line(Party[CgSlot].name, 24, 100, 10, 15, 0, 8);
		show_mouse();
		print_cg_name(CgSlot, len, 2);
	} while (Party[CgSlot].name[0] == 0);
}

/* 4C72:24F3 The buttons, the portraits (CHARGENA), the screen (CHARGEN), the
 * frames and buttons (CHARGENB) */
void far init_chargen(void)
{
	register int i;

	for (i = 0; i < 19; i++)
		CgButtonList[i] = (Button far *)mem_alloc(60L, 0);
	load_cps("CHARGENA", 3, 3);
	for (i = 0; i < 44; i++)
		PortraitShapes[i] = Drv_6592(i % 10 * 4, i / 10 << 5, 4, 32, 0L);
	Var_62A5 = 0;
	load_picture("CHARGEN", 0L, 3, 3, 0);
	load_cps("CHARGENB", 3, 3);
	for (i = 0; i < 10; i++)
		CursorShapes[i] = Drv_6592(i * 4, 0, 4, 32, 0L);
	cut_cg_buttons();
	if (VideoMode == 0 || VideoMode == 1) {
		Drv_654A(3, 4, 0L);
		copy_page(0, 0, 0, 0, 40, 200, 4, 2);
	} else
		Drv_654A(3, 2, 0L);
	Var_62A5 = 0;
	copy_page(18, 64, 0, 0, 20, 128, 0, 2);
	show_mouse();
}

/* 4C72:266C The character generation: the 4 boxes, until PLAY with 4
 * characters */
void far create_party(void)
{
	int old;
	Button far *list;
	int play;
	register int i;
	register unsigned key;

	play_sound(0);
	delay_ticks(1);
	init_chargen();
	play = 0;
	select_window(2);
	cg_show_prompt();
	list = make_cg_buttons(0, 5);
	play_sound(13);
	do {
		flash_box(CgSlot + 6);
		key = cg_gui_process(list);
		if (key) {
			switch (key) {
			case 0x5C:
			case 0x66:
				CgSlot ^= 1;
				break;
			case 0x60:
			case 0x62:
				CgSlot ^= 2;
				break;
			}
			flash_box(-1);
		}
		if (key & 0x8000) {
			key = (key & 0xF) - 1;
			if (key == 4)
				play = 1;
			else {
				CgSlot = key;
				key = 0x2B;
			}
		}
		if (key == 0x61 || key == 0x2B || key == 0x3D) {
			flash_box(-1);
			if (Party[CgSlot].name[0]) {
				old = CgSlot;
				if (cg_select_slot())
					play = 1;
				if (Party[CgSlot].name[0] == 0 && old != CgSlot)
					create_character();
			} else
				create_character();
			list = make_cg_buttons(0, 5);
			cg_show_prompt();
		}
		if (play)
			for (i = 0; i < 4; i++)
				if (Party[i].name[0] == 0)
					play = 0;
	} while (!play);
	press_cg_button(15);
	start_new_party();
	play_sound(15);
}
