/*
 * Eye of the Beholder II - START.EXE
 * The end of the game (Dran is dead): Khelben, the destruction of the
 * temple, the credits and the heroes; overlay module 4B64 (Ghidra 5B64).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y finale.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The same machinery as the introduction (intro.c), copied and changed:
 * 13 pictures cut into shapes (FinaleCuts), 21 animation scripts
 * (FinaleAnims), texts centred in window 17, 8 palettes FINALE_n.PAL (the
 * VGA fades slowly from one to the other in the background: start_fade,
 * fade_tick). A key does not stop it. The credits (Credits, also written
 * to CREDITS.TXT) scroll up in window 18. The far data (texts, palettes,
 * shapes, scripts, cuts, credits) start at segment 3643.
 */
#include "eob2.h"

typedef unsigned char byte;

/* A step of an animation (11 bytes, as in intro.c); op: 0 wait (and a
 * palette), 1 a shape, wait, the picture back (window 18), 2 a shape on the
 * page drawn, 5 copy a rectangle (from page 6 if shape), 6 a sound, 7 wait
 * then the picture back; -1 ends */
typedef struct AnimStep {
	char          op;            /* +0 */
	char          shape;         /* +1 (a sound for 6) */
	int           x;             /* +2 */
	unsigned char y;             /* +4 */
	unsigned char delay;         /* +5 ticks */
	unsigned char pal;           /* +6 a palette (Palettes) faded to, 0 = none */
	unsigned char sx, sy, w, h;  /* +7 the rectangle copied by 5 */
} AnimStep;

/* A shape cut from a picture: idx < 0 with the other cutting function
 * of the driver (no remapping); w = 0 ends */
typedef struct ShapeCut {
	int           idx;           /* +0 */
	unsigned char x, y, w, h;    /* +2 x, w in bytes */
} ShapeCut;

/* A line of the credits on the screen (11 bytes) */
typedef struct CreditLine {
	int           x;             /* +0 in the window */
	int           y;             /* +2 */
	char far     *text;          /* +4 the text, or the shape */
	char          save;          /* +8 the character replaced by the 0 ending the line */
	char          h;             /* +9 its height (6: the big font) */
	char          shape;         /* +A a shape, not a text */
} CreditLine;

extern Character      Party[6];          /* E608 */
extern int            VideoMode;         /* 628A */
extern char           CharWidth;         /* 6288 */
extern int            SoundType;         /* 638A */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            TextBg;            /* 636B */
extern int            TextColor;         /* 636D */
extern volatile unsigned long TimerTicks; /* 667C */
/* Uninitialised data of the module (DS:6D78-6D7B, _BSS), in address order */
unsigned long FadeNext; /* 6D78 the next step of the background fade */
extern byte far      *Palette;           /* F168 */
extern char           Scratch[];         /* F16E */
extern byte far      *Font8;             /* F098 */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern byte far      *FloorShapes[];     /* DE20 11 large items (ITEML1), 26 small ones from 15 (ITEMS1) */
extern byte far      *ThrownShapes[];    /* E404 */
extern byte far      *FlameShapes[3];    /* EE24 */
extern byte far      *ItemIcons[];       /* CEE6 */
extern byte far      *DamageShape;       /* DF26 */
extern byte far      *HandResultShape;   /* DF34 */
extern byte far      *Shapes[];          /* CC34 */
extern byte far      *Var_E54E[6];       /* E54E */
extern byte far      *DeadShape;         /* E400 the bones of a dead character */
extern byte far      *UnconsciousShape;  /* CB60 */
extern byte far      *BookShape;         /* 71C0 */
extern byte far      *Var_CC58;          /* CC58 the shape greying a portrait */
extern byte far      *Shape_E4CA;        /* E4CA */
extern byte far      *Var_92A8[12];      /* 92A8 3 x 4 */
extern byte far      *Var_DED4;          /* DED4 */
extern byte far      *Var_D1C4;          /* D1C4 */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
extern void (far     *Drv_655A)(void);   /* 655A */
extern byte far *(far *Drv_658E)(int x, int y, int w, int h, void far *buf);    /* 658E cuts a shape */
extern byte far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);    /* 6592 cuts a shape */
extern void (far     *Drv_65E2)(byte far *pal, int first, int last);   /* 65E2 sets colours */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

int  write_save(int slot);                                       /* 559A:07E5 (overlay) */
void farfree(void far *p);                                       /* 1000:7FF2 */
void free_door_shapes(void);                                     /* 53EC:039B (overlay) */
void free_monster_shapes(int first, int count);                  /* 53EC:050C (overlay) */
void free_decorations(void);                                     /* 53EC:0000 (overlay) */
byte far *set_font(byte far *font);                              /* 3166:000A */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
int  select_window(int w);                                       /* 31B0:0133 */
int  select_window_clear(int w);                                 /* 31B0:01F2 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void copy_pixels(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 56DD:1880 (overlay) */
char far *strchr(char far *s, int c);                            /* 1000:7503 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
int  set_page(int page);                                         /* 310B:0001 */
int  load_lbm(char far *name, int page, int dest, byte far *pal, int mode);   /* 3095:0264 */
void set_palette(byte far *pal);                                 /* 3111:000D */
void wait_tick(void);                                            /* 2D9B:0021 */
void wait_retrace(int mode);                                      /* 3404:0019 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void load_sounds(char far *name);                                /* 2C9F:0009 */
int  music_position(void);                                       /* 1000:0251 */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long write_file(int fd, void far *buf, long n);                  /* 2DCE:05DE */
long load_file_into(char far *name, void far *buf, long n);      /* 3069:0000 */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void flush_keys(void);                                           /* 3295:0516 */
int  get_key(void);                                              /* 3295:05D0 */
int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
void far finale_free_shapes(void);
static void far scroll_credits(char far *text, int win, int saved, int work, int delay);
static void far finale_picture(int pic, int page);
static void far finale_page(int page);
static void far finale_page_wide(int page);
static int far finale_wait(unsigned long t);
int far finale_check(void);
void far finale_cut_shapes(int page, int pic);
void far finale_fade_to(byte far *pal);
static void far finale_anim(int n);
static void far finale_text(int col, char far *text);
static void far finale_end_text(void);
void far finale_fade_colour(unsigned char c, unsigned char step, byte far *pal);
static void far fade_slow(byte far *pal, byte far *target, unsigned delay);
int far fade_step(byte far *target, byte far *pal, int max);
void far finale_sound(int sound);
void far start_fade(byte far *target, int max);
void far fade_tick(void);
void far play_finale(void);

/* The far data: the texts, the palettes, the shapes, the animations, the
 * cuts, the credits, each in its own segment (made by tools/finaledata.py) */
char far Text_3643[] = "Finally, Dran has been defeated.";   /* 3643:0000 */
char far Text_3646[] = "Suddenly, your friend Khelben appears.";
char far Text_3649[] = "Greetings, my victorious friends.";
char far Text_364C[] = "You have defeated Dran!";
char far Text_364E[] = "I did not know Dran was a dragon.";
char far Text_3651[] = "He must have been over 300 years old!";
char far Text_3654[] = "His power is gone.";
char far Text_3656[] = "But Darkmoon is still a source\rof great evil.";
char far Text_3659[] = "And many of his minions remain.";
char far Text_365B[] = "Now we must leave this place.";
char far Text_365D[] = "So my forces can destroy it\ronce and for all.";
char far Text_3660[] = "Follow me.";
char far Text_3661[] = "Powerful mages stand ready\rfor the final assault\ron Darkmoon.";
char far Text_3665[] = "The Temple's evil is very strong.";
char far Text_3668[] = "It must not be allowed to survive!";
char far Text_366B[] = "The Temple ceases to exist.";
char far Text_366D[] = "My friends, our work is done.";
char far Text_366F[] = "Thank you.";
char far Text_3670[] = "You have earned my deepest respect.";
char far Text_3673[] = "We will remember you always.";
byte far *far Palettes[8] = { 0 };         /* 3675 the 8 palettes FINALE_n.PAL */
byte far *far FinalePalNow = 0;            /* 3677 the colours set on the screen */
char far *far FinaleShapes[30] = { 0 };    /* 3678 */
AnimStep far Anim_3680[] = {
	{ 6, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 136, 8, 8, 0, 5, 136, 11, 48 },
	{ 2, 1, 136, 8, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 80, 8, 0, 0, 0, 136, 5, 40 },
	{ 6, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 232, 88, 4, 0, 0, 88, 8, 48 },
	{ 2, 3, 80, 8, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 232, 88, 4, 0, 8, 88, 8, 48 },
	{ 5, 0, 232, 88, 4, 0, 16, 88, 8, 48 },
	{ 2, 2, 232, 88, 4, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3688[] = {
	{ 6, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 136, 8, 12, 0, 5, 136, 11, 48 },
	{ 2, 1, 136, 8, 0, 0, 0, 0, 0, 0 },
	{ 6, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 232, 88, 6, 0, 0, 88, 8, 48 },
	{ 5, 0, 232, 88, 6, 0, 8, 88, 8, 48 },
	{ 5, 0, 232, 88, 6, 0, 16, 88, 8, 48 },
	{ 2, 2, 232, 88, 6, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_368F[] = {
	{ 5, 0, 232, 112, 6, 0, 24, 176, 3, 16 },
	{ 5, 0, 232, 112, 6, 0, 27, 176, 3, 16 },
	{ 5, 0, 232, 112, 6, 0, 30, 176, 3, 16 },
	{ -1 }
};
AnimStep far Anim_3692[] = {
	{ 5, 0, 80, 8, 0, 0, 0, 136, 5, 40 },
	{ 5, 0, 232, 112, 3, 0, 24, 176, 3, 16 },
	{ 2, 3, 80, 8, 3, 0, 0, 0, 0, 0 },
	{ 5, 0, 232, 112, 3, 0, 27, 176, 3, 16 },
	{ 5, 0, 232, 112, 3, 0, 30, 176, 3, 16 },
	{ -1 }
};
AnimStep far Anim_3697[] = {
	{ 6, 6, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 104, 40, 3, 0, 32, 88, 8, 88 },
	{ 5, 0, 104, 40, 3, 0, 32, 0, 8, 88 },
	{ 5, 0, 104, 40, 3, 0, 24, 0, 8, 88 },
	{ 5, 0, 104, 40, 3, 0, 16, 0, 8, 88 },
	{ 5, 0, 104, 40, 3, 0, 0, 0, 8, 88 },
	{ 5, 0, 104, 40, 3, 0, 24, 88, 8, 88 },
	{ -1 }
};
AnimStep far Anim_369D[] = {
	{ 5, 0, 232, 112, 0, 0, 24, 176, 3, 16 },
	{ 5, 0, 128, 40, 4, 0, 33, 176, 2, 16 },
	{ 5, 0, 232, 112, 0, 0, 27, 176, 3, 16 },
	{ 5, 0, 128, 40, 4, 0, 35, 176, 2, 16 },
	{ 5, 0, 232, 112, 0, 0, 30, 176, 3, 16 },
	{ 5, 0, 128, 40, 4, 0, 37, 176, 2, 16 },
	{ -1 }
};
AnimStep far Anim_36A2[] = {
	{ 5, 0, 104, 40, 0, 0, 16, 136, 8, 48 },
	{ -1 }
};
AnimStep far Anim_36A4[] = {
	{ 5, 0, 208, 80, 4, 0, 0, 128, 6, 56 },
	{ 5, 0, 208, 80, 4, 0, 6, 128, 6, 56 },
	{ 5, 0, 208, 80, 4, 0, 12, 128, 6, 56 },
	{ 5, 0, 208, 80, 4, 0, 18, 128, 6, 56 },
	{ 5, 0, 208, 80, 4, 0, 24, 128, 6, 56 },
	{ -1 }
};
AnimStep far Anim_36A9[] = {
	{ 5, 0, 224, 56, 3, 0, 0, 184, 4, 16 },
	{ 5, 0, 224, 56, 0, 0, 4, 184, 4, 16 },
	{ -1 }
};
AnimStep far Anim_36AC[] = {
	{ 5, 0, 80, 80, 4, 0, 0, 128, 6, 56 },
	{ 5, 0, 80, 80, 4, 0, 6, 128, 6, 56 },
	{ 5, 0, 80, 80, 4, 0, 12, 128, 6, 56 },
	{ 5, 0, 80, 80, 4, 0, 18, 128, 6, 56 },
	{ 5, 0, 80, 80, 4, 0, 24, 128, 6, 56 },
	{ 5, 0, 80, 80, 4, 0, 30, 128, 6, 56 },
	{ -1 }
};
AnimStep far Anim_36B1[] = {
	{ 5, 0, 96, 56, 3, 0, 0, 184, 4, 16 },
	{ 5, 0, 96, 56, 0, 0, 4, 184, 4, 16 },
	{ -1 }
};
AnimStep far Anim_36B4[] = {
	{ 1, 1, -10, 40, 2, 0, 0, 0, 0, 0 },
	{ 1, 1, 0, 40, 2, 0, 0, 0, 0, 0 },
	{ 1, 1, 10, 40, 2, 0, 0, 0, 0, 0 },
	{ 1, 1, 20, 40, 2, 0, 0, 0, 0, 0 },
	{ 1, 1, 30, 40, 2, 0, 0, 0, 0, 0 },
	{ 1, 1, 40, 40, 2, 0, 0, 0, 0, 0 },
	{ 2, 2, 48, 40, 2, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_36BA[] = {
	{ 5, 1, 8, 40, 0, 0, 32, 0, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 24, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 12, 80, 1, 16 },
	{ 5, 1, 8, 40, 0, 0, 32, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 28, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 13, 80, 1, 16 },
	{ 5, 1, 8, 40, 0, 0, 24, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 32, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 14, 80, 1, 16 },
	{ 6, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 12, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 128, 12, 24 },
	{ -1 }
};
AnimStep far Anim_36C4[] = {
	{ 5, 1, 40, 32, 0, 0, 16, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 18, 176, 6, 24 },
	{ 5, 1, 96, 80, 0, 0, 0, 152, 12, 24 },
	{ 5, 1, 8, 40, 0, 0, 24, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 32, 160, 4, 40 },
	{ 5, 1, 96, 96, 0, 0, 14, 80, 1, 16 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 5, 1, 8, 40, 0, 0, 24, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 32, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 14, 80, 1, 16 },
	{ 5, 1, 8, 40, 0, 0, 32, 0, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 24, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 12, 80, 1, 16 },
	{ 5, 1, 8, 40, 0, 0, 16, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 36, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 15, 80, 1, 16 },
	{ -1 }
};
AnimStep far Anim_36D1[] = {
	{ 5, 1, 40, 32, 0, 0, 16, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 18, 176, 6, 24 },
	{ 5, 1, 96, 80, 0, 0, 0, 152, 12, 24 },
	{ 5, 1, 8, 40, 0, 0, 24, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 32, 160, 4, 40 },
	{ 5, 1, 96, 96, 0, 0, 14, 80, 1, 16 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 8, 40, 0, 0, 32, 0, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 24, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 12, 80, 1, 16 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 8, 40, 0, 0, 16, 80, 8, 80 },
	{ 5, 1, 280, 96, 0, 0, 36, 160, 4, 40 },
	{ 5, 1, 96, 96, 3, 0, 15, 80, 1, 16 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ -1 }
};
AnimStep far Anim_36E1[] = {
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 6, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 0, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 80, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 16, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 6, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 104, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 12, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 128, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ -1 }
};
AnimStep far Anim_36F1[] = {
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 6, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 0, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 80, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 16, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 6, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 104, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 6, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 12, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 128, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ -1 }
};
AnimStep far Anim_3702[] = {
	{ 5, 1, 40, 32, 0, 0, 0, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 0, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 80, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 168, 16, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 16, 0, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 6, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 104, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 1, 40, 32, 0, 0, 0, 40, 16, 40 },
	{ 5, 1, 248, 88, 0, 0, 12, 176, 6, 24 },
	{ 5, 1, 96, 80, 3, 0, 0, 128, 12, 24 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ -1 }
};
AnimStep far Anim_3710[] = {
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 168, 16, 3, 0, 0, 0, 0, 0 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 168, 16, 3, 0, 0, 0, 0, 0 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ 6, 15, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 168, 16, 3, 0, 0, 0, 0, 0 },
	{ 5, 0, 168, 16, 0, 0, 32, 0, 8, 80 },
	{ 5, 0, 232, 16, 0, 0, 32, 80, 8, 80 },
	{ -1 }
};
AnimStep far Anim_3719[] = {
	{ 5, 0, 168, 16, 1, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 6, 10, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 168, 16, 3, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 0, 16, 80 },
	{ -1 }
};
AnimStep far Anim_3727[] = {
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 0, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 3, 0, 16, 0, 16, 80 },
	{ 5, 0, 168, 16, 1, 0, 0, 80, 16, 80 },
	{ 5, 0, 168, 16, 2, 0, 16, 80, 16, 80 },
	{ 5, 0, 168, 16, 0, 0, 16, 80, 16, 80 },
	{ -1 }
};
ShapeCut far Cuts_373B[] = {
	{ -1, 16, 0, 11, 48 },
	{ -2, 28, 80, 8, 48 },
	{ -3, 9, 0, 5, 40 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_373D[] = {
	{ -1, 30, 0, 8, 96 },
	{ -2, 30, 104, 10, 96 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_373F[] = {
	{ 1, 0, 0, 16, 72 },
	{ 2, 16, 0, 16, 72 },
	{ 3, 0, 72, 16, 72 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_3741[] = {
	{ 0, 0, 0, 32, 16 },
	{ 2, 0, 36, 35, 41 },
	{ 3, 0, 77, 24, 17 },
	{ 4, 0, 94, 15, 33 },
	{ 5, 24, 77, 10, 17 },
	{ 6, 16, 99, 23, 69 },
	{ -10, 0, 136, 8, 64 },
	{ -11, 8, 136, 8, 64 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_3745[] = {
	{ 1, 0, 0, 40, 30 },
	{ 15, 9, 37, 21, 48 },
	{ 16, 16, 88, 6, 56 },
	{ 0, 0, 0, 0, 0 }
};
/* the credits: 1, a line in the big font; 2 and n, the shape n */
char far Credits[] =
	"\002\005\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
	"\002\020\r\r\r\r\r\r\r\r\r\r\r\r\r"
	"\002\003\r"
	"\002\004\r\r\r\r\r\r\r\r\r\r"
	"\002\006\r\r\r"
	"\002\001\r\r\r\r"
	"          Director: Brett W. Sperry   \r\r\r\r"
	"  Original Concept: Philip W. Gorrow  \r\r"
	"                    Brett W. Sperry   \r\r\r\r"
	"       Game Design: Brett W. Sperry   \r\r"
	"                    Paul S. Mudra     \r\r"
	"                    Bill Stokes       \r\r\r\r"
	"   Lead Programmer: Philip W. Gorrow  \r\r\r\r"
	"  Level Programmer: Bill Stokes       \r\r\r\r"
	"  Amiga Programmer: Bill Stokes       \r\r\r\r"
	" Intro & Finale by: Joe Bostic        \r\r\r\r"
	"    Art & Graphics: Rick Parks        \r\r"
	"                    Aaron Powell      \r\r"
	"                    Joseph Hewitt IV  \r\r"
	"                    Ren Olsen         \r\r"
	"                    Louise Sandoval   \r\r"
	"                    Judith Peterson   \r\r\r\r"
	"             Music: Frank Klepacki    \r\r\r\r"
	"     Sound Effects: Paul S. Mudra     \r\r"
	"                    Dwight Okahara    \r\r\r\r"
	"            Writer: Marc Cram         \r\r\r\r"
	" Quality Assurance: Glenn Sperry      \r\r\r\r\r\r"
	"Special Thanks to\r\r\r"
	"Chuck Kroegel\r\r"
	"Xanathar the Beholder\r\r"
	"and\r\r"
	"Mike's cat, Sparky\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
	"Published by\r\r"
	"\002\002\r\r\r\r\r\r\r"
	"         Producers: Dan Cermak        \r\r"
	"                    George MacDonald  \r\r\r\r"
	"  Game Development: Nicholas Beliaeff \r\r"
	"                    David A. Lucca    \r\r\r\r"
	"      Play Testers: Phil Alne         \r\r"
	"                    John Boockholdt   \r\r"
	"                    Glen Cureton      \r\r"
	"                    Mark Hall         \r\r"
	"                    Brian Lowe        \r\r"
	"                    Robert Lupo       \r\r"
	"                    Alan Marenco      \r\r"
	"                    Jason Ray         \r\r"
	"                    Jeff Shotwell     \r\r"
	"                    Andre Vrignaud    \r\r"
	"                    Chris Warshauer   \r\r"
	"                    and TSCS, Inc     \r\r\r\r"
	"      Test Support: Kym Goyer         \r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
	"Cast of Characters\r\r\r"
	"The Forces of Darkness\r\r\r"
	"Dran Draggore (The Big Bad Dragon)\r\r"
	"Nadia & Joril (The Two-Faced Clerics)\r\r"
	"Dying Servant (Dran's Right Hand Man)\r\r"
	"Trapped Cleric (The Red Herring)\r\r"
	"King of the Frost Giants\r\r\r\r\r\r\r\r\r\r\r\r"
	"The Forces of Good\r\r\r"
	"Khelben Blackstaff\r\r"
	"Amber (Khelben's Scout)\r\r"
	"Shorn Diergar (Pesky Dwarf)\r\r"
	"Insal the Thief (The Nuisance)\r\r"
	"Calandra (Ira's Lost Sister)\r\r"
	"Alain (The Ancient One)\r\r"
	"San-Raal (Drow Mage)\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
	"\001Any characters in this game who bear\r"
	"\001resemblance to persons living or\r"
	"\001dead are purely coincidental.\r\r\r\r\r\r\r\r\r\r\r"
	"\002\021\r\r\r\r\r\r\0";

/* Initialised data of the module (DS:1B1E) */
int FinaleSkipped = 0;                   /* 1B1E always 0 here: nothing sets it */
int FinaleCanSkip = 1;                   /* 1B20 (written, never read) */
AnimStep far *FinaleAnims[21] = {                /* 1B22 */
	Anim_3680, Anim_3688, Anim_368F, Anim_3692, Anim_3697, Anim_369D, Anim_36A2, Anim_36A4, Anim_36A9, Anim_36B4, Anim_36BA, Anim_36C4, Anim_36D1, Anim_36E1, Anim_36F1, Anim_3702, Anim_3710, Anim_3719, Anim_3727, Anim_36AC, Anim_36B1
};
ShapeCut far *FinaleCuts[13] = {                  /* 1B76 */
	Cuts_373B, 0, 0, Cuts_373D, 0, 0, 0, Cuts_373F, 0, Cuts_3741, Cuts_3745, 0, 0
};
char far *FinalePictures[13] = {         /* 1BAA */
	"DRAGON1.CPS", "DRAGON2.CPS", "HURRY1.CPS", "HURRY2.CPS", "DESTROY0.CPS", "DESTROY1.CPS",
	"DESTROY2.CPS", "MAGIC.CPS", "DESTROY3.CPS", "CREDITS2.CPS", "CREDITS3.CPS", "HEROES.CPS",
	"THANKS.CPS"
};
byte far *FadeTarget = 0;                /* 1BDE the palette of the background fade */
int FadeMax = 0;                         /* 1BE2 its step, 0 = no fade */
int HeroPos[6][2] = {                    /* 1BE4 the portraits on HEROES.CPS */
	{ 80, 80 }, { 208, 80 }, { 80, 144 }, { 208, 144 }, { 144, 80 }, { 144, 144 }
};

/* 5B64:0000 Frees the shapes of the game (the finale needs the memory)
 * after writing FINAL.SAV; the face of the NPC, shared with Var_D1C4, is
 * copied */
void far finale_free_shapes(void)
{
	byte far *face;
	register int i;

	write_save(-1);
	for (i = 0; i < 11; i++)
		farfree(FloorShapes[i]);
	for (i = 0; i < 26; i++)
		farfree(FloorShapes[15 + i]);
	for (i = 0; i < 13; i++)
		farfree(ThrownShapes[i]);
	for (i = 0; i < 3; i++)
		farfree(FlameShapes[i]);
	for (i = 1; i < 112; i++)
		farfree(ItemIcons[i]);
	farfree(DamageShape);
	farfree(HandResultShape);
	for (i = 0; i < 6; i++) {
		farfree(Shapes[3 + i]);
		farfree(Var_E54E[i]);
	}
	for (i = 0; i < 3; i++)
		farfree(Shapes[i]);
	farfree(DeadShape);
	farfree(UnconsciousShape);
	farfree(BookShape);
	farfree(Var_CC58);
	farfree(Shape_E4CA);
	for (i = 0; i < 12; i++)
		farfree(Var_92A8[i]);
	farfree(Var_DED4);
	free_door_shapes();
	free_monster_shapes(0, 36);
	free_decorations();
	set_font(Font8);
	for (i = 0; i < 6; i++)
		if (Party[i].face == Var_D1C4) {
			face = mem_alloc(0x578L, 0);
			mem_copy(Party[i].face, face, 0x578L);
			Party[i].face = face;
			break;
		}
}

/* 5B64:0255 Scrolls a text up in window win, 2 lines a step (every delay
 * ticks): \r ends a line, 1 starts a line in the big font, 2 and n puts the
 * shape n. saved: the page keeping the background, work: the page drawn on */
static void far scroll_credits(char far *text, int win, int saved, int work, int delay)
{
	char far *s;
	char far *p;
	unsigned long t;
	int sh, x, y;
	CreditLine lines[35];
	register int n, i;

	if (!text)
		return;
	p = text;
	n = 0;
	select_window(win);
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 0, saved);
	t = TimerTicks;
	for (;;) {
		for (fade_tick(); t > TimerTicks; fade_tick())   /* (the JMPs of 02B4/02B6) */
			;
		t = TimerTicks + delay;
		for (; n < 35 && *p; n++) {
			if (n) {
				x = lines[n - 1].y;
				x += lines[n - 1].h + (lines[n - 1].h >> 2);
			} else
				x = WinH;
			s = p;
			p = strchr(p, '\r');
			if (!p)
				p = strchr(s, 0);
			lines[n].save = *p;
			*p = 0;
			if (!*p)                     /* sic: always (the 0 just written) */
				p++;
			if (*s == 2) {
				s++;
				sh = *s++;
				s = (char far *)FinaleShapes[sh - 1];
				lines[n].h = s[1];
				lines[n].x = (WinW - s[2]) * 4;
				lines[n].shape = 1;
			} else {
				if (*s == 1) {
					s++;
					lines[n].h = 6;
				} else
					lines[n].h = CharWidth;
				lines[n].shape = 0;
				lines[n].x = (WinW * 8 - strlen(s) * lines[n].h) / 2 + 1;
			}
			lines[n].y = x;
			lines[n].text = s;
		}
		copy_page(WinX, WinY, WinX, WinY, WinW, WinH, saved, work);
		for (i = 0; i < n; i++) {
			s = lines[i].text;
			x = lines[i].x;
			if ((y = lines[i].y) < WinH) {
				if (lines[i].shape)
					put_shape(work, (byte far *)s, x, y, win);
				else {
					set_page(work);
					if (lines[i].h == 6)
						set_font(GameFont);
					Drv_65F2(lines[i].text, x + WinX * 8 - 1, y + WinY + 1, 12, 0);
					Drv_65F2(lines[i].text, x + WinX * 8, y + WinY, 240, 0);
					if (lines[i].h == 6)
						set_font(Font8);
					set_page(0);
				}
			}
			lines[i].y -= 2;
		}
		copy_page(WinX, WinY, WinX, WinY, WinW, WinH, work, 0);
		if (lines[0].y < -lines[0].h) {
			lines[0].text += strlen(lines[0].text);
			*lines[0].text = lines[0].save;
			n--;
			memcpy(&lines[0], &lines[1], n * sizeof(CreditLine));
		}
		if (n < 35 && lines[n - 1].y + lines[n - 1].h < WinY + WinH)
			break;
		if (!n || finale_check())
			break;
	}
}

/* 5B64:0765 A picture into page | 1, its shapes cut, then page */
static void far finale_picture(int pic, int page)
{
	if (!FinaleSkipped) {
		load_lbm(FinalePictures[pic], page | 1, page | 1, Palette, 1);
		finale_cut_shapes(page, pic);
		if (VideoMode != 3)
			set_palette(Palette);
		Drv_654A(page | 1, page, 0L);
	}
}

/* 5B64:07DE The picture of page on the screen */
static void far finale_page(int page)
{
	if (!FinaleSkipped) {
		if (VideoMode == 3)
			finale_fade_to(Palette);
		copy_page(0, 0, 1, 8, 38, 128, page, 0);
	}
}

/* 5B64:0823 The same, wider (in pixels) */
static void far finale_page_wide(int page)
{
	if (!FinaleSkipped) {
		if (VideoMode == 3)
			finale_fade_to(Palette);
		copy_pixels(0, 0, 8, 8, 304, 128, page, 0);
	}
}

/* 5B64:0865 Waits until TimerTicks reaches t */
static int far finale_wait(unsigned long t)
{
	if (FinaleSkipped)
		return 1;
	while (t > TimerTicks)
		if (finale_check())
			break;
	return finale_check();
}

/* 5B64:0898 The background fade goes on (no key is read) */
int far finale_check(void)
{
	if (!FinaleSkipped)
		fade_tick();
	return FinaleSkipped;
}

/* 5B64:08AC The shapes of a picture (page) */
void far finale_cut_shapes(int page, int pic)
{
	int old;
	ShapeCut far *c;

	if (!FinaleSkipped) {
		c = FinaleCuts[pic];
		if (c) {
			old = set_page(page);
			for (;;) {
				if (!c->w)
					break;
				if (c->idx < 0) {
					if (FinaleShapes[-c->idx])
						farfree(FinaleShapes[-c->idx]);
					FinaleShapes[-c->idx] = Drv_6592(c->x, c->y, c->w, c->h, 0L);
				} else {
					if (FinaleShapes[c->idx])
						farfree(FinaleShapes[c->idx]);
					FinaleShapes[c->idx] = Drv_658E(c->x, c->y, c->w, c->h, 0L);
				}
				c++;
			}
			set_page(old);
		}
	}
}

/* 5B64:0A0B Sets the colours of pal that changed, 80 at a time */
void far finale_fade_to(byte far *pal)
{
	int changed;
	byte far *p;
	byte far *q;
	register int i, j;

	if (!FinaleSkipped && VideoMode == 3) {
		p = pal;
		q = FinalePalNow;
		for (i = 0; i < 255; i++) {
			changed = (*(unsigned long far *)p & 0xFFFFFFL) != (*(unsigned long far *)q & 0xFFFFFFL);
			p += 3;
			q += 3;
			if (changed) {
				j = i + 79;
				j = j > 254 ? 254 : j;
				wait_tick();
				Drv_65E2(pal, i, j);
				i = j;
			}
		}
		mem_copy(pal, FinalePalNow, 0x300L);
	}
}

/* 5B64:0AEB Plays an animation script */
static void far finale_anim(int n)
{
	int y, w, h, x0, delay, pal, op;
	AnimStep far *s;
	register int x, sh;

	if (FinaleSkipped)
		return;
	for (s = FinaleAnims[n]; s->op != -1; s++) {
		sh = s->shape;
		op = s->op;
		x = s->x;
		y = s->y;
		delay = s->delay;
		pal = s->pal;
		switch (op) {
		case 6:
			finale_sound(sh);
			break;
		case 0:
			if (VideoMode == 3 && pal)
				finale_fade_to(Palettes[pal]);
			finale_wait(TimerTicks + delay);
			break;
		case 1:
			w = FinaleShapes[sh][2];
			h = FinaleShapes[sh][1];
			select_window(18);
			x -= WinX * 8;
			y -= WinY;
			x0 = x;
			if (x < 0) {
				w -= (-x >> 3) + 1;
				x = 0;
			}
			put_shape(0, FinaleShapes[sh], x0, y, 18);
			if (VideoMode == 3 && pal)
				finale_fade_to(Palettes[pal]);
			finale_wait(TimerTicks + delay);
			copy_page(x >> 3, y, (x >> 3) + WinX, y + WinY, w + 1, h, 2, 0);
			break;
		case 7:
			w = FinaleShapes[sh][2];
			h = FinaleShapes[sh][1];
			finale_wait(TimerTicks + delay);
			copy_page((x >> 3) - 1, y - 8, x >> 3, y, w + 1, h, 2, 0);
			break;
		case 2:
			w = FinaleShapes[sh][2];
			h = FinaleShapes[sh][1];
			put_shape(Var_62A5, FinaleShapes[sh], x, y, 0);
			if (VideoMode == 3 && pal)
				finale_fade_to(Palettes[pal]);
			finale_wait(TimerTicks + delay);
			break;
		case 5:
			w = s->w;
			h = s->h;
			if (VideoMode == 3 && pal)
				finale_fade_to(Palettes[pal]);
			copy_page(s->sx, s->sy, x >> 3, y, w, h, sh ? 6 : 2, 0);
			finale_wait(TimerTicks + delay);
			break;
		}
	}
}

/* 5B64:0E59 A text centred in window 17, line by line (\r); in VGA its
 * colour is copied into colour 255 (intro_text) */
static void far finale_text(int col, char far *text)
{
	char far *p;
	char far *q;
	char save;
	register int len, line;

	if (FinaleSkipped)
		return;
	select_window_clear(17);
	if (VideoMode == 3) {
		memcpy(Palette + 0x2FD, Palette + col * 3, 3);
		Drv_65E2(Palette, 255, 255);
		TextColor = 255;
	} else
		TextColor = 15;
	line = 0;
	for (p = text; *p; line += 9) {
		q = strchr(p, '\r');
		if (q) {
			save = *q;
			*q = 0;
			len = strlen(p);
			Drv_65F2(p, (WinX + (WinW - len) / 2) * 8, WinY + line, TextColor, TextBg);
			*q = save;
			p = q;
			p++;
		} else {
			len = strlen(p);
			Drv_65F2(p, (WinX + (WinW - len) / 2) * 8, WinY + line, TextColor, TextBg);
			p += len;
		}
	}
}

/* 5B64:0FBC The text fades out */
static void far finale_end_text(void)
{
	if (!FinaleSkipped) {
		if (VideoMode == 3)
			finale_fade_colour(255, 2, Palette);
		select_window_clear(17);
	}
}

/* 5B64:0FF0 Colour c of pal goes down to black, step by step (fade_colour
 * of intro.c; sic: the second test reads Palette, not pal) */
void far finale_fade_colour(unsigned char c, unsigned char step, byte far *pal)
{
	int changed;
	register int k, base;

	if (VideoMode == 3) {
		base = c * 3;
		for (;;) {
			changed = 0;
			for (k = 0; k < 3; k++)
				if (pal[base + k] > step) {
					pal[base + k] -= step;
					changed = 1;
				} else if (Palette[base + k] > 0) {
					pal[base + k] = 0;
					changed = 1;
				}
			if (!changed)
				break;
			wait_retrace(0);
			Drv_65E2(pal, c, c);
		}
	}
}

/* 5B64:1093 Fades pal to target in about delay ticks */
static void far fade_slow(byte far *pal, byte far *target, unsigned delay)
{
	int max, step, per;
	unsigned long end;
	int frac;
	register int i, d;

	if (!pal || !target)
		return;
	if (VideoMode != 3 || !delay) {
		memcpy(pal, target, 0x300);
		set_palette(pal);
		return;
	}
	max = 0;
	for (i = 0; i < 0x300; i++) {
		d = (char)pal[i] - (char)target[i];
		d = d < 0 ? -d : d;
		max = max > d ? max : d;
	}
	if (!max)
		return;
	step = ((long)delay << 8) / max;
	per = max / delay;
	per = per < 1 ? 1 : per;
	frac = 0;
	end = TimerTicks;
	while (fade_step(target, pal, per)) {
		finale_fade_to(pal);
		frac += step;
		end += frac >> 8;
		frac &= 0xFF;
		while (TimerTicks < end)
			;
	}
}

/* 5B64:11D7 Moves each colour of pal by max at most towards target.
 * Returns 1 if something changed */
int far fade_step(byte far *target, byte far *pal, int max)
{
	int changed;
	register int i, d;
	int a, b;

	changed = 0;
	for (i = 0; i < 0x300; i++) {
		a = *target++;
		b = *pal++;
		d = a - b;
		if (!d)
			continue;
		changed = 1;
		d = d < 0 ? -d : d;
		d = max < d ? max : d;
		if (b < a)
			b += d;
		else
			b -= d;
		pal--;
		*pal = b;
		pal++;
	}
	return changed;
}

/* 5B64:1256 */
void far finale_sound(int sound)
{
	play_sound(sound);
}

/* 5B64:1265 Starts fading the screen to target in the background
 * (fade_tick, called while waiting), by max a step */
void far start_fade(byte far *target, int max)
{
	memcpy(Palette, FinalePalNow, 0x300);
	FadeTarget = target;
	FadeMax = max;
	FadeNext = TimerTicks + 2;
}

/* 5B64:12B4 A step of the background fade, every 3 ticks */
void far fade_tick(void)
{
	if (VideoMode == 3 && FadeMax && TimerTicks > FadeNext) {
		if (fade_step(FadeTarget, Palette, FadeMax)) {
			finale_fade_to(Palette);
			FadeNext = TimerTicks + 3;
		} else
			FadeMax = 0;
	}
}

/* 5B64:1326 The finale (after finale_free_shapes) */
void far play_finale(void)
{
	register int i, fd;

	fd = open_file("CREDITS.TXT", 2);
	write_file(fd, Credits, strlen(Credits) + 1);
	close_file(fd);
	ShrinkBuffer = mem_alloc(0x960L, 0);
	FinaleSkipped = 0;
	FinaleCanSkip = 0;
	for (i = 0; i < 8; i++) {
		sprintf(Scratch, "FINALE_%d.PAL", i);
		Palettes[i] = mem_alloc(0x300L, 0x10);
		load_file_into(Scratch, Palettes[i], 0x300L);
	}
	FinalePalNow = mem_alloc(0x300L, 0x10);
	hide_mouse();
	set_page(0);
	play_sound(0);
	load_sounds("FINALE1");
	finale_wait(TimerTicks + 3);
	Drv_655A();
	play_sound(0);
	memcpy(Palette, Palettes[0], 0x300);
	if (VideoMode != 3)
		mem_copy(Palettes[6], Palette, 0x300L);

	/* Dran is dead */
	finale_picture(0, 2);
	finale_wait(TimerTicks + 18);
	play_sound(1);
	finale_page(2);
	finale_picture(1, 2);
	finale_anim(0);
	finale_anim(0);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_anim(1);
	finale_anim(2);
	finale_anim(2);
	finale_text(10, Text_3643);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_anim(2);
	if (SoundType == 1)
		while (music_position() < 1)
			fade_tick();

	/* Khelben */
	finale_text(10, Text_3646);
	finale_anim(4);
	finale_anim(2);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_3649);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_anim(6);
	finale_text(15, Text_364C);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_364E);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_3651);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_3654);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_3656);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();
	finale_text(15, Text_3659);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(5);
	finale_anim(2);
	finale_anim(2);
	finale_end_text();

	/* "Follow me." */
	finale_picture(2, 2);
	finale_page(2);
	finale_picture(3, 2);
	if (!FinaleSkipped)
		copy_page(1, 8, 0, 0, 38, 128, 0, 2);
	finale_text(15, Text_365B);
	finale_anim(7);
	finale_anim(8);
	finale_anim(7);
	finale_anim(7);
	finale_wait(TimerTicks + 36);
	finale_end_text();
	finale_text(15, Text_365D);
	finale_anim(7);
	finale_anim(7);
	finale_anim(7);
	finale_anim(8);
	finale_anim(7);
	finale_anim(7);
	finale_wait(TimerTicks + 36);
	finale_anim(8);
	finale_wait(TimerTicks + 18);
	finale_end_text();
	finale_text(15, Text_3660);
	finale_anim(7);
	finale_wait(TimerTicks + 18);
	finale_anim(9);
	finale_wait(TimerTicks + 18);
	finale_anim(8);
	finale_wait(TimerTicks + 18);
	finale_end_text();

	/* the mages destroy the temple */
	if (VideoMode != 3)
		mem_copy(Palettes[7], Palette, 0x300L);
	finale_picture(7, 2);
	FinaleSkipped = 0;
	if (VideoMode == 3)
		mem_copy(Palettes[2], Palette, 0x300L);
	finale_picture(4, 2);
	if (SoundType == 1)
		while (music_position() < 2)
			fade_tick();
	Drv_655A();
	finale_page(2);
	finale_picture(8, 2);
	finale_picture(6, 6);
	finale_wait(TimerTicks + 10);
	finale_text(10, Text_3661);
	finale_wait(TimerTicks + 90);
	finale_end_text();
	if (SoundType == 1)
		while (music_position() < 3)
			fade_tick();
	play_sound(7);
	finale_wait(TimerTicks + 8);
	finale_anim(10);
	finale_anim(13);
	start_fade(Palettes[3], 1);
	finale_anim(14);
	finale_anim(13);
	finale_anim(14);
	finale_anim(14);
	finale_anim(13);
	start_fade(Palettes[1], 1);
	finale_anim(15);
	finale_anim(14);
	finale_anim(13);
	finale_anim(15);
	finale_anim(15);
	finale_anim(11);
	finale_text(10, Text_3665);
	finale_wait(TimerTicks + 72);
	finale_end_text();
	finale_text(10, Text_3668);
	finale_wait(TimerTicks + 72);
	finale_end_text();
	if (SoundType == 1)
		while (music_position() < 4)
			fade_tick();
	play_sound(7);
	finale_wait(TimerTicks + 8);
	finale_anim(10);
	start_fade(Palettes[4], 1);
	finale_anim(13);
	finale_anim(14);
	finale_anim(13);
	finale_anim(14);
	finale_anim(13);
	finale_anim(13);
	finale_anim(14);
	finale_anim(14);
	finale_anim(13);
	finale_anim(12);
	finale_anim(16);
	finale_anim(16);
	finale_anim(16);
	finale_anim(16);
	finale_anim(17);
	finale_anim(18);
	finale_text(10, Text_366B);
	start_fade(Palettes[5], 2);
	finale_wait(TimerTicks + 36);
	play_sound(11);
	finale_wait(TimerTicks + 54);
	finale_end_text();

	/* Khelben thanks the heroes */
	if (VideoMode != 3)
		mem_copy(Palettes[7], Palette, 0x300L);
	finale_picture(12, 2);
	if (SoundType == 1)
		while (music_position() < 5)
			fade_tick();
	play_sound(6);
	finale_page_wide(2);
	finale_wait(TimerTicks + 18);
	finale_text(15, Text_366D);
	finale_anim(20);
	finale_anim(19);
	finale_anim(19);
	finale_wait(TimerTicks + 36);
	play_sound(12);
	finale_end_text();
	finale_text(15, Text_366F);
	finale_anim(19);
	finale_anim(20);
	finale_wait(TimerTicks + 36);
	finale_end_text();
	finale_text(15, Text_3670);
	play_sound(11);
	finale_anim(19);
	finale_anim(20);
	finale_anim(19);
	play_sound(11);
	finale_wait(TimerTicks + 36);
	finale_end_text();
	finale_text(15, Text_3673);
	finale_anim(19);
	finale_anim(19);
	finale_wait(TimerTicks + 18);
	play_sound(11);
	finale_anim(20);
	finale_wait(TimerTicks + 18);
	finale_end_text();
	finale_wait(TimerTicks + 18);
	finale_wait(TimerTicks + 10);
	play_sound(12);
	finale_wait(TimerTicks + 3);
	FinaleSkipped = 0;
	finale_picture(5, 2);
	play_sound(6);
	finale_page_wide(2);
	play_sound(12);
	finale_wait(TimerTicks + 5);
	play_sound(11);
	finale_wait(TimerTicks + 11);
	play_sound(12);
	finale_wait(TimerTicks + 7);
	play_sound(11);
	finale_wait(TimerTicks + 12);
	play_sound(12);
	FinaleSkipped = 0;
	FinaleSkipped = 0;

	/* the credits */
	finale_picture(10, 2);
	finale_picture(9, 2);
	play_sound(0);
	finale_wait(TimerTicks + 3);
	load_sounds("FINALE2");
	if (!FinaleSkipped) {
		FinaleCanSkip = 1;
		finale_wait(TimerTicks + 18);
		play_sound(1);
		scroll_credits(Credits, 18, 2, 6, 2);
		FinaleSkipped = 0;
		finale_wait(TimerTicks + 90);
		Credits[0] = '\r';
		Credits[1] = '\r';
		Credits[2] = '\r';
	}

	/* the heroes */
	if (VideoMode == 3) {
		mem_copy(FinalePalNow, Palette, 0x300L);
		memset(Palettes[0], 0, 0x300);
		fade_slow(Palette, Palettes[0], 10);
	}
	Drv_655A();
	finale_picture(11, 2);
	for (i = 0; i < 6; i++)
		if (char_is_active(i, 1)) {
			if (i > 3)
				put_shape(2, FinaleShapes[6 + i], HeroPos[i][0] - 16, HeroPos[i][1] - 16, 0);
			put_shape(2, Party[i].face, HeroPos[i][0], HeroPos[i][1], 0);
		}
	copy_page(0, 0, 0, 0, 40, 200, 2, 0);
	fade_slow(Palettes[0], Palette, 18);
	flush_keys();
	while (get_key() != 27)
		;
	for (i = 0; i < 30; i++) {
		if (FinaleShapes[i])
			farfree(FinaleShapes[i]);
		FinaleShapes[i] = 0;
	}
	play_sound(0);
	farfree(FinalePalNow);
	for (i = 0; i < 8; i++)
		farfree(Palettes[i]);
	Drv_655A();
	show_mouse();
}
