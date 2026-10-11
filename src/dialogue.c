/*
 * Eye of the Beholder II - START.EXE
 * Dialogues and scenes: the texts of TEXT.DAT, the buttons under them, the
 * pictures of the dialogues, the NPCs who join the party; overlay module
 * 46DD (Ghidra 56DD), stub segment 3AF4.
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y dialogue.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* A piece of a scene picture, cut from page 3 (8 bytes) */
typedef struct SceneCut {
	int           x;             /* +0 in bytes */
	byte          y, w, h;       /* +2 */
	signed char   dx, dy;        /* +5 its offset on the screen */
	signed char   next;          /* +7 the next piece, -1 none */
} SceneCut;

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 */
extern int            CursorX, CursorY;  /* F094, F096 the text cursor in the window */
extern char           LineHeight;        /* 6289 */
extern int            WinY;              /* F106 */
extern int            ClickDown[2], ClickUp[2];   /* 661A, 6622 */
extern int            MouseHidden;       /* 6628 */
extern unsigned char  InventoryMode;     /* 6655 the keyboard moves the selection */
extern int            DialogueStyle;     /* DF4E */
extern void (far     *MoreFunc)();       /* 6367 */
extern char far *(far *Drv_658E)(int x, int y, int w, int h, void far *buf);   /* 658E cuts a shape */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A fills */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

extern Character      Party[6];          /* E608 */
extern char           Scratch[];         /* F16E */
extern char far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);   /* 6592 cuts a shape */
int  take_party_items(int type, int value);                      /* 1E24:0212 */
void drop_character(int ch);                                     /* 1E24:043C */
void recompute_ac(int ch);                                       /* 1F55:0CF7 */
int  create_item(int src);                                       /* 1CBE:092B */
void movmem(void far *src, void far *dest, unsigned n);
void load_cps(char far *name, int a, int page);                  /* 18D7:0657 */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern int            PartyBlock, PartyDirection;   /* 6DD2, 6DD4 */
extern signed char    FrontPos[4][4];    /* 2342 the places in the square ahead, per direction */
extern unsigned long  GlobalFlags;       /* 6E26 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void put_item(unsigned far *list, int block, int item, int pos); /* 1CBE:01D4 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
void more_off(void);                                             /* 31B0:0129 */
void draw_game_screen(void);                                     /* 1E24:0B88 */
#define ABS(x)  ((x) < 0 ? -(x) : (x))
extern int            Var_DF3E;          /* DF3E */
extern char           InvSlot;           /* E5A4 the selected inventory slot */
extern char           ScriptResult;      /* 6D7C result of the last dialogue */
extern char           SceneDirty;        /* D0AA the 3D view must be redrawn */
extern int            InDialogue;        /* EE38 a dialogue is open */
/* Uninitialised data of the module (DS:6D60-6D77, _BSS), in address order */
char MoreButtonText[20]; /* 6D60 the text of the button of dialogue_more */
char far *CurPicture;    /* 6D74 the picture shown in the dialogue frame */
extern Item           Items[];           /* 71CA */
extern int            HandItem;          /* 6DD6 the item in the mouse "hand" */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern void (far     *Drv_658A)(int x, int y, int w, int h, byte far *buf);    /* 658A saves a region */
extern void (far     *Drv_6542)(int x, int y, int w, int h, byte far *buf);    /* 6542 restores it */
void close_panels(void);                                         /* 1E24:0B5B */
void draw_scene(int full);                                       /* 4A5D:1FF4 (overlay) */
void manual_check(void);                                         /* 5315:0A21 (overlay) */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
int  strcmp(char far *a, char far *b);                           /* 1000:7540 */
void load_picture(char far *name, byte far *pal, int a, int page, int mode);   /* 18D7:05C8 */
void copy_pixels(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 56DD:1880 (overlay) */
void wait_key(int a);                                            /* 18D7:0B9A */
void free_decorations(void);                                     /* 53EC:0000 (overlay) */
void free_door_shapes(void);                                     /* 53EC:039B (overlay) */
void free_monster_shapes(int first, int count);                  /* 53EC:050C (overlay) */
byte far *alloc_page(int page);                                  /* 2F3E:0009 */
byte far *set_font(byte far *font);                              /* 3166:000A returns the old one */
void save_region(void);                                          /* 4F5F:11C8 (overlay) */
void restore_region(void);                                       /* 4F5F:11F8 (overlay) */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
void more_on(void);                                              /* 31B0:0119 */
int  select_window(int w);                                       /* 31B0:0133 */
void gui_clear(void);                                            /* 31B0:020F */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void put_shape(int page, char far *shape, int x, int y, int dim);   /* 2DA3:004A */
void farfree(void far *p);                                       /* 1000:7FF2 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void mouse_hide_rect(int x1, int y1, int x2, int y2);            /* 338B:0206 */
void mouse_show_rect(void);                                      /* 338B:02CA */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void draw_frame3d(int x1, int y1, int x2, int y2, int light, int dark, int fill);   /* 18D7:0A45 */
void flush_keys(void);                                           /* 3295:0516 */
int  read_key(void);                                             /* 3295:066D */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);      /* 18D7:067F */
unsigned strlen(char far *s);                                    /* 1000:7599 */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
void far load_text_dat(void);
void far dialogue_window(int win, int clear);
char far * far cut_scene_shape(int n);
void far draw_scene_shapes(int n, int pos);
void far show_scene(int which);
void far clear_dialogue(void);
void far dialogue_more(void);
void far dialogue_text(int n, char far *button);
void far draw_dialogue_button(int x, int y, int colour, char far *text);
int far dialogue_buttons(char first, char count, char far * far *labels);
int far make_room(int type, int value);
void far npc_joins(int npc);
int far npc_offer(int npc, int text, int join, int refuse);
int far drop_new_item(int item);
void far scene_insal(void);
void far scene_npc1(void);
void far scene_npc2(void);
void far play_npc_scene(int which);
void far play_sequence(int seq);
void far choose_character(void);
void far start_dialogue(void);
void far end_dialogue(void);
int far dialogue_choice(int text, char far *b1, char far *b2, char far *b3);
void far dialogue_picture(char far *name, int size, int x, int y, int flags);
void far read_note(int value);
void far choose_raise(void);
void far portal_sequence(void);
static void far dream_frame(int n);
void far rest_level3(void);
void far kheldran_laughs(void);

/* Initialised data of the module (DS:1456), made by tools/dialogdata.py;
 * the static tables of the functions follow in DS:14A7-1582 */
SceneCut SceneCuts[3] = {   /* 1456 the pieces of the scene pictures */
	{ 0, 0, 5, 57, 0, 0, -1 },
	{ 0, 100, 5, 79, 0, 0, -1 },
	{ 0, 57, 6, 43, 0, 0, -1 }
};
static char far *YesNo[2] = { "yes", "no" };   /* 146E */
int ScenePosX[3] = {   /* 1476 the places of the scene pieces */
	88, 40, 166
};
int MoreX[2] = {   /* 147C the MORE button, per DialogueStyle */
	221, 76
};
int MoreY[2] = {   /* 1480 */
	189, 162
};
static byte ButtonX[14] = {   /* 1484 the places of the buttons */
	59, 166, 4, 112, 220, 4, 112, 220, 4, 112, 220, 4, 112, 220
};
signed char ButtonDY[14] = {   /* 1492 */
	0, 0, 0, 0, 0, 12, 12, 12, 24, 24, 24, 36, 36, 36
};
char CancelText[] = "CANCEL";   /* 14A0 */

Character far NpcChars[6] = {   /* 347A:0000 the NPCs who can join */
	{ 0, 1, "Insal", 15, 15, 0, 0, 13, 13, 11, 11, 17, 17, 16, 16, 9, 9,
	  3, 39, 10, 0, 10, 5, 5, -1, 8, { 6, 0, 0 }, { 27354, 0, 0 }, 0,
	  { 0 },
	  { 0 },
	  0,
	  { 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } },
	{ 0, 1, "Calandra", 18, 18, 36, 36, 13, 13, 8, 8, 15, 15, 16, 16, 14, 14,
	  4, 76, 10, 0, 1, 0, 2, -2, 12, { 9, 0, 0 }, { 253749, 0, 0 }, 0,
	  { 0 },
	  { 0 },
	  0,
	  { 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } },
	{ 0, 1, "Shorn", 15, 15, 0, 0, 14, 14, 13, 13, 14, 14, 13, 13, 16, 16,
	  40, 40, 10, 0, 6, 4, 3, -3, 100, { 8, 0, 0 }, { 137008, 0, 0 }, 0,
	  { 0 },
	  { 0 },
	  0,
	  { 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } },
	{ 0, 1, "San-raal", 11, 11, 0, 0, 18, 18, 13, 13, 14, 14, 16, 16, 9, 9,
	  28, 28, 10, 0, 2, 3, 5, -4, 100, { 8, 0, 0 }, { 97008, 0, 0 }, 0,
	  { 0 },
	  { 0 },
	  158025062L,
	  { 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } },
	{ 0, 1, "Tanglor", 16, 16, 0, 0, 13, 13, 16, 16, 15, 15, 11, 11, 12, 12,
	  53, 53, 9, 0, 4, 6, 1, -5, 100, { 7, 7, 0 }, { 69570, 69570, 0 }, 0,
	  { 0 },
	  { 0 },
	  0xFFFFFFFFL,
	  { 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } },
	{ 0, 1, "Amber", 14, 14, 0, 0, 17, 17, 12, 12, 18, 18, 9, 9, 17, 17,
	  36, 36, 10, 0, 3, 10, 1, -6, 100, { 7, 7, 0 }, { 64230, 64230, 0 }, 0,
	  { 0 },
	  { 0 },
	  2704742L,
	  { 0 }, { 0 }, { 0 }, 0, { 0 }, 0, 0, 0, { 0 }, 0, 0, 0, { 0 } }
};
signed char far DreamX[4] = { 0, 20, 0, 20 };   /* 34FC:0000 the dream pictures: part of DREAM.CPS */
signed char far DreamY[4] = { 0, 0, 96, 96 };   /* 34FD:0000 */
signed char far DreamDelay[4] = { 12, 7, 7, 12 };   /* 34FE:0000 the ticks each is shown */
signed char far DreamSeq[14] = { 1, 2, 3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, -1 };   /* 34FF:0000 the frames */

/* 56DD:0000 TEXT.DAT (the texts of the dialogues) into page 5 */
void far load_text_dat(void)
{
	register int fd;

	fd = open_file("TEXT.DAT", 1);
	read_file(fd, LoadBuffer, 32000L);
	close_file(fd);
}

/* 56DD:003A The text window win (cleared, or the cursor back at the top);
 * "more" goes through dialogue_more */
void far dialogue_window(register int win, int clear)
{
	more_on();
	Dims[win].b = FrameFlat;
	select_window(win);
	if (clear)
		gui_clear();
	else
		CursorX = CursorY = 0;
	MoreFunc = dialogue_more;
}

/* 56DD:0081 Piece n of a scene picture, cut from page 3 */
char far * far cut_scene_shape(int n)
{
	SceneCut far *c;
	char far *s;

	c = &SceneCuts[n];
	Var_62A5 = 2;
	s = Drv_658E(c->x, c->y, c->w, c->h, 0L);
	Var_62A5 = 0;
	return s;
}

/* 56DD:00DA The pieces from n (a chain) at place pos of the view */
void far draw_scene_shapes(register int n, int pos)
{
	SceneCut far *c;
	char far *s;
	int y;
	register int x;

	while (n != -1) {
		c = &SceneCuts[n];
		s = cut_scene_shape(n);
		x = ScenePosX[pos] - (s[2] * 8 >> 1) + c->dx;
		y = 104 - s[1] + c->dy;
		put_shape(0, s, x, y, 5);
		farfree(s);
		n = SceneCuts[n].next;
	}
}

/* 56DD:018D The view back (page 6), and scene which drawn over it */
void far show_scene(int which)
{
	copy_page(0, 0, 0, 0, 22, 120, 6, 0);
	switch (which) {
	case 0:
		draw_scene_shapes(0, 0);
		break;
	case 1:
		draw_scene_shapes(1, 0);
		break;
	case 2:
		draw_scene_shapes(2, 0);
		break;
	}
}

/* 56DD:01DD The text area under the view, cleared */
void far clear_dialogue(void)
{
	mouse_hide_rect(0, 121, 319, 200);
	draw_frame3d(0, 121, 319, 199, FrameLight, FrameDark, FrameFlat);
	gui_clear();
	mouse_show_rect();
}

/* 56DD:022C The "MORE" button (MoreButtonText): waits for it, or a key */
void far dialogue_more(void)
{
	int waiting, hidden, key;
	register int x, y;

	x = MoreX[DialogueStyle];
	y = MoreY[DialogueStyle];
	mouse_hide_rect(x, y, x + 94, y + 8);
	draw_dialogue_button(x, y, 15, MoreButtonText);
	mouse_show_rect();
	flush_keys();
	hidden = MouseHidden;
	if (!InventoryMode)
		while (MouseHidden)
			show_mouse();
	waiting = 1;
	do {
		key = read_key() & 0x8FF;
		switch (key) {
		case 0x41:
		case 0x42:
			if (!in_rect(ClickUp[0], ClickUp[1], x, y, x + 94, y + 8))
				break;
		case 0x2B:
		case 0x34:
		case 0x3D:
			waiting = 0;
			break;
		}
	} while (waiting);
	if (!InventoryMode)
		while (hidden--)
			hide_mouse();
	hide_mouse();
	Drv_657A(x, y, x + 94, y + 8, FrameFlat);
	gui_clear();
	show_mouse();
}

/* 56DD:033E Text n of TEXT.DAT in the window; with a button (its text),
 * waits for it */
void far dialogue_text(register int n, char far *button)
{
	byte far *base;
	int off;

	base = LoadBuffer;
	n--;
	off = ((int far *)base)[n];
	gui_printf(LoadBuffer + off);
	if (button && strlen(button)) {
		strcpy(MoreButtonText, button);
		dialogue_more();
		strcpy(MoreButtonText, "MORE");
	}
}

/* 56DD:03C0 A button of the dialogues (95 x 9 pixels) */
void far draw_dialogue_button(register int x, register int y, int colour, char far *text)
{
	draw_frame3d(x, y, x + 94, y + 8, FrameLight, FrameDark, FrameFlat);
	Drv_65F2(text, x + 48 - strlen(text) * 3, y + 2, colour, 0);
}

/* 56DD:042A count buttons (their places from first) under the text, to
 * choose with the mouse or the arrows; returns the one chosen */
int far dialogue_buttons(char first, char count, char far * far *labels)
{
	int y, sel, chosen, key, shown;
	register int k, i;

	y = WinY + (CursorY + 1) * LineHeight + 4;
	shown = chosen = -1;
	sel = 0;
	do {
		hide_mouse();
		if (shown != sel) {
			for (i = 0; i < count; i++) {
				k = first + i;
				draw_dialogue_button(ButtonX[k], y + ButtonDY[k], sel == i ? 9 : 15, labels[i]);
			}
			shown = sel;
		}
		show_mouse();
		key = read_key() & 0x8FF;
		for (i = 0; i < count; i++) {
			k = first + i;
			if (in_rect(ClickDown[0], ClickDown[1], ButtonX[k], ButtonDY[k] + y, ButtonX[k] + 94, ButtonDY[k] + y + 8)) {
				sel = i;
				break;
			}
		}
		switch (key) {
		case 0x41:
		case 0x42:
			for (i = 0; i < count; i++) {
				k = first + i;
				if (in_rect(ClickUp[0], ClickUp[1], ButtonX[k], ButtonDY[k] + y, ButtonX[k] + 94, ButtonDY[k] + y + 8)) {
					chosen = i;
					break;
				}
			}
			break;
		case 0x5C:
			sel--;
			if (sel < 0)
				sel = count - 1 > 2 ? 2 : count - 1;
			break;
		case 0x66:
			sel++;
			if (sel > 2 || count - 1 < sel)
				sel = 0;
			break;
		case 0x62:
			if (sel + 3 < count)
				sel += 3;
			else if (sel - 3 > 0)
				sel -= 3;
			/* ORIGINAL BUG: no break, down goes up again */
		case 0x60:
			if (sel - 3 > 0)
				sel -= 3;
			else if (sel + 3 < count)
				sel += 3;
			break;
		case 0x2B:
		case 0x3D:
		case 0x61:
			chosen = sel;
			break;
		}
	} while (chosen == -1);
	clear_dialogue();
	return chosen;
}

/* 56DD:0667 Makes room for an NPC: with 6 characters, one is chosen to
 * leave (or CANCEL: returns 0); the items given (type, value) are taken */
int far make_room(int type, int value)
{
	int ch;
	char far *labels[7];
	register int i, n;

	for (n = i = 0; i < 6; i++) {
		n += Party[i].flags & 1;
		labels[i] = Party[i].name;
	}
	labels[6] = CancelText;
	if (n > 5) {
		clear_dialogue();
		gui_printf("You may only have six characters in your party.  Select the one you wish to drop.");
		ch = dialogue_buttons(2, 7, labels);
		if (ch == 6)
			return 0;
		take_party_items(type, value);
		drop_character(ch);
	} else
		take_party_items(type, value);
	return 1;
}

/* 56DD:071B NPC npc joins the party, in the first free place: his sheet
 * (NpcChars), his items created, his portrait from OUTPORTS.CPS */
void far npc_joins(int npc)
{
	Character far *c;
	char far *face;
	register int i, slot;

	for (slot = i = 0; i < 6; i++)
		if ((Party[i].flags & 1) == 0) {
			slot = i;
			break;
		}
	c = &Party[slot];
	face = c->face;
	movmem(&NpcChars[npc], c, 0x159);
	c->face = face;
	recompute_ac(slot);
	for (i = 0; i < 25; i++)
		if (c->inventory[i])
			c->inventory[i] = create_item(c->inventory[i]);
	load_cps("OUTPORTS", 3, 3);
	Var_62A5 = 2;
	face = Drv_6592(npc * 4, 0, 4, 32, 0L);
	movmem(face, c->face, 0x578);
	farfree(face);
	Var_62A5 = 0;
}

/* 56DD:0854 An NPC asks to join (text): yes (join: its text, or "X joins
 * the party."), or no (refuse). Returns 1 if he joined */
int far npc_offer(register int npc, int text, register int join, int refuse)
{
	int ch;

	clear_dialogue();
	dialogue_text(text, 0L);
	ch = dialogue_buttons(0, 2, YesNo);
	switch (ch) {
	case 0:
		if (join != -1)
			dialogue_text(join, "OK");
		else {
			sprintf(Scratch, "%s joins the party.\r", NpcChars[npc].name);
			gui_printf(Scratch);
			dialogue_more();
		}
		if (make_room(0x21, npc + 1))
			npc_joins(npc);
		break;
	case 1:
		dialogue_text(refuse, "OK");
		break;
	}
	return ch ^ 1;
}

/* 56DD:0912 A new item of type item (create_item) on the floor in front of
 * the party, at a random place */
int far drop_new_item(register int item)
{
	int pos;

	item = create_item(item);
	pos = FrontPos[PartyDirection][roll_dice(1, 2, -1)];
	put_item(&LevelMap[PartyBlock].items, PartyBlock, item, pos);
	return item;
}

/* 56DD:096C Insal asks to join */
void far scene_insal(void)
{
	play_sound(0x39);
	if (npc_offer(0, 1, 3, 2))
		GlobalFlags |= 0x40;
}

/* 56DD:09A2 The second NPC: talk or leave */
void far scene_npc1(void)
{
	static char far *answers[2] = { "talk", "leave" };
	int ch;

	play_sound(0x35);
	clear_dialogue();
	dialogue_text(4, 0L);
	ch = dialogue_buttons(0, 2, answers);
	switch (ch) {
	case 0:
		play_sound(0);
		delay_ticks(3);
		play_sound(0x5B);
		npc_offer(1, 5, 6, 7);
		break;
	case 1:
		GlobalFlags |= 0x20;
		break;
	}
}

/* 56DD:0A30 The prisoner: release him (half the time he joins, else he
 * leaves) or leave */
void far scene_npc2(void)
{
	static char far *answers[2] = { "release him", "leave" };
	int ch;

	play_sound(0x37);
	clear_dialogue();
	dialogue_text(8, 0L);
	ch = dialogue_buttons(0, 2, answers);
	switch (ch) {
	case 0:
		if (roll_dice(1, 2, -1))
			dialogue_text(9, "OK");
		else
			npc_offer(2, 0x66, 0x67, 0x68);
		GlobalFlags |= 8;
		break;
	case 1:
		PartyDirection = 0;
		break;
	}
}

/* 56DD:0ACB The scene of an NPC (which: 0-2): its picture (OUTTAKE.CPS)
 * over the view, the texts */
void far play_npc_scene(register int which)
{
	load_cps("OUTTAKE", 5, 3);
	copy_page(0, 0, 0, 0, 22, 120, 0, 6);
	show_scene(which);
	show_mouse();
	load_text_dat();
	mouse_hide_rect(0, 121, 319, 200);
	draw_frame3d(0, 121, 319, 199, FrameLight, FrameDark, FrameFlat);
	mouse_show_rect();
	dialogue_window(9, 1);
	strcpy(MoreButtonText, "MORE");
	switch (which) {
	case 0:
		scene_insal();
		break;
	case 1:
		scene_npc1();
		break;
	case 2:
		scene_npc2();
		break;
	}
	more_off();
	hide_mouse();
	draw_game_screen();
}

/* 56DD:0BAE A sequence of the scripts: -2 the portal, -1 the manual check
 * (copy protection), else the scene of NPC seq */
void far play_sequence(register int seq)
{
	int slot;

	Var_DF3E = -1;
	DialogueStyle = 0;
	slot = InvSlot;
	hide_mouse();
	close_panels();
	draw_scene(1);
	switch (seq) {
	case -2:
		portal_sequence();
		InvSlot = slot;
		break;
	case -1:
		manual_check();
		break;
	default:
		play_npc_scene(seq);
		break;
	}
	select_window(7);
	show_mouse();
}

/* 56DD:0C1D The scripts ask for a character: ScriptResult = his place,
 * 0x63 for ABORT */
void far choose_character(void)
{
	char far *labels[7];
	int i, n;

	for (i = n = 0; i < 6; i++)
		if (char_is_active(i, 3)) {
			labels[n] = Party[i].name;
			n++;
		}
	labels[n] = "ABORT";
	n++;
	if ((ScriptResult = dialogue_buttons(2, n, labels)) == n - 1)
		ScriptResult = 0x63;
	else {
		for (i = n = 0; i < 6; i++)
			if (char_is_active(i, 3) && --ScriptResult < 0)
				break;
		ScriptResult = i;
	}
}

/* 56DD:0CE1 A dialogue begins: the text window under the view */
void far start_dialogue(void)
{
	Var_DF3E = -1;
	DialogueStyle = 0;
	CurPicture = 0;
	InDialogue = 1;
	strcpy(MoreButtonText, "MORE");
	close_panels();
	play_sound(0);
	load_text_dat();
	dialogue_window(9, 0);
}

/* 56DD:0D37 The dialogue ends: the game screen back */
void far end_dialogue(void)
{
	more_off();
	InDialogue = 0;
	CurPicture = 0;
	draw_game_screen();
	select_window(7);
	play_sound(2);
	SceneDirty = 1;
}

/* 56DD:0D73 Text text with 2 or 3 buttons; returns the one chosen (1-3) */
int far dialogue_choice(int text, char far *b1, char far *b2, char far *b3)
{
	char far *labels[3];
	register int ch;

	labels[0] = b1;
	labels[1] = b2;
	labels[2] = b3;
	dialogue_text(text, 0L);
	if (labels[2] == 0)
		ch = dialogue_buttons(0, 2, labels);
	else
		ch = dialogue_buttons(2, 3, labels);
	return ch + 1;
}

/* 56DD:0DDE The picture name (a CPS of page 3) in the dialogue frame:
 * part (x, y) of it, piece size 0 (the whole) or 1; flags 1: the border
 * is not redrawn, 2: drawn pixel by pixel (copy_pixels) */
void far dialogue_picture(char far *name, register int size, int x, int y, int flags)
{
	static int w[2] = { 20, 40 };
	static int h[2] = { 96, 121 };
	static int dx[2] = { 1, 0 };
	static int dy[2] = { 8, 0 };
	register int page;

	if ((flags & 2) || size)
		page = 0;
	else
		page = 6;
	if (!CurPicture || strcmp(name, CurPicture) != 0) {
		if (size == 0) {
			if (!(flags & 1)) {
				load_picture("BORDER", 0L, 3, 3, 2);
				hide_mouse();
				copy_page(0, 0, 0, 0, 23, 121, 2, page);
				if (page == 0)
					copy_page(0, 0, 0, 0, 23, 121, 2, 6);
				show_mouse();
			} else if (page) {
				hide_mouse();
				copy_page(0, 0, 0, 0, 23, 121, 0, page);
				show_mouse();
			}
		}
		load_picture(name, 0L, 3, 3, 2);
		CurPicture = name;
	}
	hide_mouse();
	if (flags & 2)
		copy_pixels(x * 8, y, dx[size] * 8, dy[size], w[size] * 8, h[size], 2, 0);
	else
		copy_page(x, y, dx[size], dy[size], w[size], h[size], 2, page);
	if (page == 6)
		copy_page(0, 0, 0, 0, 23, 121, 6, 0);
	show_mouse();
}

/* 56DD:0F79 An item of type 11 used: value >= 0 a text (value + 1), else
 * a map (-value - 1) until a key */
void far read_note(register int value)
{
	static char far *pics[3] = { "MAP", "MAP", "MAP" };
	static byte xs[3] = { 0, 20, 0 };
	static byte ys[3] = { 0, 0, 96 };

	DialogueStyle = 1;
	close_panels();
	strcpy(MoreButtonText, "MORE");
	if (value >= 0) {
		load_text_dat();
		draw_frame3d(0, 0, 175, 174, FrameLight, FrameDark, FrameFlat);
		dialogue_window(12, 1);
		dialogue_text(value + 1, "OK");
	} else {
		value = ABS(value) - 1;
		dialogue_picture(pics[value], 0, xs[value], ys[value], 0);
		wait_key(0);
	}
	end_dialogue();
}

/* 56DD:1035 Raise dead (script): a dead character, or the bones (items of
 * type 0x21, value = the NPC) of an NPC, who joins */
void far choose_raise(void)
{
	char far *labels[20];
	char codes[20];
	int i, n, ch;
	register int item, j;

	for (i = n = 0; i < 6; i++)
		if (char_is_active(i, 1) && Party[i].hp == -10) {
			labels[n] = Party[i].name;
			codes[n] = i;
			n++;
		}
	for (i = 0; i < 6; i++)
		if (char_is_active(i, 1))
			for (j = 0; j < 27; j++) {
				item = Party[i].inventory[j];
				if (item && Items[item].type == 0x21) {
					labels[n] = NpcChars[Items[item].value - 1].name;
					codes[n] = -Items[item].value;
					n++;
				}
			}
	if (!InventoryMode) {
		item = HandItem;
		if (item && Items[item].type == 0x21) {
			labels[n] = NpcChars[Items[item].value - 1].name;
			codes[n] = -Items[item].value;
			n++;
		}
	}
	codes[n] = 0x63;
	labels[n] = "ABORT";
	n++;
	ScriptResult = 1;
	ch = codes[dialogue_buttons(2, n, labels)];
	if (ch == 0x63)
		ScriptResult = 0;
	else if (ch < 0) {
		if (make_room(0x21, ABS(ch)))
			npc_joins((ch = ABS(ch)) - 1);
	} else
		Party[ch].hp = 1;
}

/* 56DD:125A The portal (PORTALA.CPS, PORTALB.CPS): the 3 parts of the
 * portal turn to the code of the stones */
void far portal_sequence(void)
{
	char far *rings[5];
	char far *outer[5];
	char far *inner[5];
	char far *stones[11] = { 0 };
	static char seq[] = {   /* 1505 (shape of the rings, stones shown) per step */
		1, 0,  0, 0,  0, 0,  0, 0,  0, 0,  1, 0,  2, 0,  1, 0,
		0, 0,  1, 0,  2, 0,  3, 0,  2, 0,  1, 0,  2, 0,  3, 0,
		4, 0,  3, 0,  2, 0,  1, 0,  0, 0,  0, 0,  0, 1,  0, 0,
		0, 1,  0, 2,  0, 1,  0, 0,  0, 1,  0, 2,  0, 3,  0, 4,
		0, 1,  1, 0,  2, 1,  3, 2,  2, 3,  1, 4,  0, 2,  1, 3,
		2, 4,  3, 2,  4, 3,  3, 4,  2, 2,  1, 1,  0, 0,  0, 0,
		0, 0,  1, 1,  2, 2,  3, 3,  4, 4,  4, 5,  4, 6,  3, 7,
		2, 8,  2, 8,  1, 9,  1, 9,  1, 9,  0, 10,  -1, -1
	};
	char far *p;
	register int i;

	free_door_shapes();
	free_monster_shapes(0, 0x24);
	free_decorations();
	load_cps("PORTALA", 5, 3);
	for (i = 0; i < 5; i++) {
		rings[i] = Drv_658E(i * 3, 0, 3, 0x4B, 0L);
		outer[i] = Drv_658E(i * 3, 0x50, 3, 0x4B, 0L);
		inner[i] = Drv_658E(15, i * 18, 15, 18, 0L);
	}
	stones[0] = Drv_658E(30, 0, 8, 0x4D, 0L);
	load_picture("PORTALB", 0L, 5, 3, 2);
	play_sound(0x21);
	play_sound(0x13);
	Var_62A5 = 2;
	Drv_658A(3, 0, 18, 0x68, alloc_page(5));
	copy_page(3, 0, 3, 0, 18, 0x68, 0, 2);
	put_shape(2, inner[0], 0x1C, 9, 0);
	put_shape(2, rings[0], 0x22, 0x1C, 0);
	put_shape(2, outer[0], 0x78, 0x1C, 0);
	put_shape(2, stones[0], 0x38, 0x1B, 0);
	copy_pixels(0x18, 0, 0x18, 0, 0x90, 0x68, 2, 0);
	Drv_6542(3, 0, 18, 0x68, alloc_page(5));
	delay_ticks(30);
	for (p = seq; *p > -1; ) {
		i = *p++;
		put_shape(0, inner[i], 0x1C, 9, 0);
		put_shape(0, rings[i], 0x22, 0x1C, 0);
		put_shape(0, outer[i], 0x78, 0x1C, 0);
		if (i == 1 && p[-3] == 0) {
			play_sound(0x18);
			play_sound(0x56);
		}
		i = *p++;
		if (i) {
			i--;
			copy_page(i % 5 * 8, i / 5 * 0x4D, 7, 0x1B, 8, 0x4D, 2, 0);
		} else
			put_shape(0, stones[0], 0x38, 0x1B, 0);
		if (i == 1)
			play_sound(0x1F);
		if (i == 3 && p[-2] == 3)
			play_sound(0x5A);
		delay_ticks(2);
	}
	farfree(stones[0]);
	for (i = 4; i >= 0; i--) {
		farfree(inner[i]);
		farfree(outer[i]);
		farfree(rings[i]);
	}
}

/* 56DD:167B Frame n of the dream (DREAM.CPS) */
static void far dream_frame(register int n)
{
	dialogue_picture("DREAM", 0, DreamX[n], DreamY[n], 0);
	delay_ticks(DreamDelay[n]);
}

/* 56DD:16C2 The dream of the rest on level 3 */
void far rest_level3(void)
{
	byte far *font;
	register int i;

	font = set_font(GameFont);
	restore_region();
	start_dialogue();
	dialogue_picture("DREAM", 0, 0, 0, 0);
	play_sound(0x36);
	clear_dialogue();
	dialogue_text(0x63, "");
	delay_ticks(12);
	for (i = 0; DreamSeq[i] != -1; i++)
		dream_frame(DreamSeq[i]);
	dialogue_text(0x14, "ok");
	end_dialogue();
	save_region();
	set_font(font);
}

/* 56DD:1774 The party all dead (with flag 0x100000): Kheldran laughs */
void far kheldran_laughs(void)
{
	byte far *font;

	font = set_font(GameFont);
	start_dialogue();
	dialogue_picture("KHELDRAN", 0, 0, 0, 0);
	clear_dialogue();
	gui_printf("    Such trusting whelps!");
	dialogue_text(0x14, "MORE");
	play_sound(0x38);
	dialogue_picture("KHELDRAN", 0, 0x14, 0, 0);
	delay_ticks(10);
	dialogue_picture("KHELDRAN", 0, 0, 0x60, 0);
	delay_ticks(10);
	dialogue_picture("KHELDRAN", 0, 0x14, 0x60, 0);
	delay_ticks(7);
	dialogue_text(0x4C, "OK");
	end_dialogue();
	set_font(font);
}
