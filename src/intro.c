/*
 * Eye of the Beholder II - START.EXE
 * The introduction (Khelben's message); overlay module 4989 (Ghidra 5989).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y intro.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * 13 pictures (STREET1.CPS... KHELBAN6.CPS) are loaded into page 2/3 and
 * cut into shapes (ShapeCuts); 44 small animation scripts (Anims) put the
 * shapes on the screen, copy rectangles, fade the palette, play sounds; the
 * texts are centred in window 17. A key stops it all (Skipped); with the
 * AdLib music, the pictures wait for the music to reach its next part.
 * The far data (texts, scripts, cuts) start at segment 3506.
 */
#include "eob2.h"

typedef unsigned char byte;

/* A step of an animation (11 bytes); op: 0 wait and fade, 1 shape then the
 * picture back, 2 shape on the page drawn, 3/4 shape with a shadow
 * (VGA), 5 copy a rectangle, 6 a sound, 7 wait then the picture back;
 * -1 ends */
typedef struct AnimStep {
	char          op;            /* +0 */
	char          shape;         /* +1 (a sound for 6) */
	int           x;             /* +2 */
	unsigned char y;             /* +4 */
	unsigned char delay;         /* +5 ticks */
	unsigned char pal;           /* +6 a palette, or a shade (3 and 4) */
	unsigned char sx, sy, w, h;  /* +7 the rectangle copied by 5 */
} AnimStep;

/* A shape cut from a picture: idx < 0 with the other cutting function
 * of the driver (no remapping); w = 0 ends */
typedef struct ShapeCut {
	int           idx;           /* +0 */
	unsigned char x, y, w, h;    /* +2 x, w in bytes */
} ShapeCut;

extern int            VideoMode;         /* 628A */
extern int            SoundType;         /* 638A */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            TextBg;            /* 636B */
extern int            TextColor;         /* 636D */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern volatile unsigned long TimerTicks; /* 667C */
extern byte far      *Palette;           /* F168 */
extern void (far     *Drv_653E)(int a, int b, int c, int d, int e, int f, int g, int h);   /* 653E copies (pixels) */
extern void (far     *Drv_654A)(int page, int mode, byte far *pal);   /* 654A */
extern void (far     *Drv_6552)(byte far *pal, byte far *table, int a, int b);   /* 6552 makes a fading table */
extern void (far     *Drv_655A)(void);   /* 655A */
extern void (far     *Drv_6576)(int shade);   /* 6576 SetShade */
extern byte far *(far *Drv_658E)(int x, int y, int w, int h, void far *buf);    /* 658E cuts a shape */
extern byte far *(far *Drv_6592)(int x, int y, int w, int h, void far *buf);    /* 6592 cuts a shape */
extern void (far     *Drv_659E)(int on); /* 659E */
extern void (far     *Drv_65DE)(byte far *table);   /* 65DE */
extern void (far     *Drv_65E2)(byte far *pal, int first, int last);   /* 65E2 sets colours */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */

int  load_lbm(char far *name, int page, int dest, byte far *pal, int mode);   /* 3095:0264 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
int  key_pressed(void);                                          /* 3295:041E */
int  get_key(void);                                              /* 3295:05D0 */
void flush_keys(void);                                           /* 3295:0516 */
void play_sound(int sound);                                      /* 2C9F:00AA */
void load_sounds(char far *name);                                /* 2C9F:0009 */
int  music_position(void);                                       /* 1000:0251 */
int  set_page(int page);                                         /* 310B:0001 */
void farfree(void far *p);                                       /* 1000:7FF2 */
int  file_exists(char far *name);                                /* 2DCE:11CF */
int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
long write_file(int fd, void far *buf, long n);                  /* 2DCE:05DE */
void wait_tick(void);                                            /* 2D9B:0021 */
void wait_retrace(int mode);                                      /* 3404:0019 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void put_shape(int page, byte far *shape, int x, int y, int dim);   /* 2DA3:004A */
int  select_window(int w);                                       /* 31B0:0133 */
int  select_window_clear(int w);                                 /* 31B0:01F2 */
void gui_clear(void);                                            /* 31B0:020F */
char far *strchr(char far *s, int c);                            /* 1000:7503 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
long load_file_into(char far *name, void far *buf, long n);      /* 3069:0000 */
void set_palette(byte far *pal);                                 /* 3111:000D */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
int far check_skip(void);
void far cut_shapes(int page, int pic);
void far fade_to(byte far *pal);
void far play_anim(int n);
void far fade_colour(unsigned char c, unsigned char step, byte far *pal);
void far intro_sound(int sound);

/* The far data: the texts, the animations, the cuts, each in its own segment
 * (made by tools/introdata.py) */
char far Text_3506[] = "You were settling in for an evening \rby the fire.";   /* 3506:0000 */
char far Text_350A[] = "Then a note was slipped to you.";
char far Text_350C[] = "It was from your friend Khelben \rBlackstaff, Archmage of the city.";
char far Text_3511[] = "The message was urgent.";
char far Text_3513[] = "What could Khelben want?";
char far Text_3515[] = "Welcome, please come in.";
char far Text_3517[] = "Khelben awaits you in his study.";
char far Text_351A[] = "Thank you for coming so quickly.";
char far Text_351D[] = "I am troubled my friend.";
char far Text_351F[] = "Ancient Evil stirs in the \rTemple Darkmoon.";
char far Text_3522[] = "I fear for the safety of our city.";
char far Text_3525[] = "I need your help.";
char far Text_3527[] = "Three nights ago \rI sent forth a scout.";
char far Text_352A[] = "She has not yet returned.";
char far Text_352C[] = "I fear for her safety.";
char far Text_352E[] = "Take this coin.";
char far Text_352F[] = "I will use it to contact you.";
char far Text_3531[] = "You must act quickly.";
char far Text_3533[] = "I will teleport you near Darkmoon.";
char far Text_3536[] = "May luck be with you, my friend.";
AnimStep far Anim_3539[] = {
	{ 1, 0, 208, 16, 1, 1, 0, 0, 0, 0 },
	{ 1, 0, 208, 16, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 216, 51, 8, 0, 0, 0, 0, 0 },
	{ 6, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_353D[] = {
	{ 1, 1, 184, 8, 1, 1, 0, 0, 0, 0 },
	{ 1, 1, 184, 8, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 216, 51, 7, 0, 0, 0, 0, 0 },
	{ 6, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3541[] = {
	{ 1, 2, 216, 51, 1, 0, 0, 0, 0, 0 },
	{ 1, 2, 216, 51, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 216, 51, 18, 0, 0, 0, 0, 0 },
	{ 6, 6, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3545[] = {
	{ 0, 0, 0, 0, 1, 2, 0, 0, 0, 0 },
	{ 6, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 2, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3548[] = {
	{ 6, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 2, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_354B[] = {
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 6, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_354E[] = {
	{ 1, 0, 282, 8, 1, 0, 0, 0, 0, 0 },
	{ 1, 0, 282, 8, 1, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3551[] = {
	{ 1, 12, 54, 17, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 80, 53, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 104, 87, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 130, 124, 2, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3555[] = {
	{ 1, 12, 112, 8, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 126, 34, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 142, 60, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 170, 109, 2, 0, 0, 0, 0, 0 },
	{ 6, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_355A[] = {
	{ 1, 12, 170, 11, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 188, 38, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 203, 63, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 218, 85, 2, 0, 0, 0, 0, 0 },
	{ 1, 12, 232, 107, 2, 0, 0, 0, 0, 0 },
	{ 6, 9, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 3, 242, 125, 2, 0, 0, 0, 0, 0 },
	{ 1, 4, 240, 125, 3, 0, 0, 0, 0, 0 },
	{ 1, 5, 238, 124, 3, 0, 0, 0, 0, 0 },
	{ 1, 6, 236, 123, 3, 0, 0, 0, 0, 0 },
	{ 1, 7, 232, 121, 3, 0, 0, 0, 0, 0 },
	{ 1, 8, 226, 120, 4, 0, 0, 0, 0, 0 },
	{ 1, 9, 220, 119, 4, 0, 0, 0, 0, 0 },
	{ 1, 10, 241, 115, 4, 0, 0, 0, 0, 0 },
	{ 1, 11, 241, 115, 4, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3565[] = {
	{ 6, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 168, 24, 3, 0, 0, 128, 2, 32 },
	{ 5, 0, 152, 16, 3, 0, 0, 88, 5, 40 },
	{ 5, 0, 128, 0, 3, 0, 0, 0, 10, 88 },
	{ 5, 0, 104, 0, 3, 0, 13, 0, 14, 88 },
	{ -1 }
};
AnimStep far Anim_356A[] = {
	{ 5, 0, 184, 64, 6, 0, 30, 8, 3, 16 },
	{ 5, 0, 184, 64, 6, 0, 33, 8, 3, 16 },
	{ 5, 0, 184, 64, 6, 0, 36, 8, 3, 16 },
	{ -1 }
};
AnimStep far Anim_356D[] = {
	{ 5, 0, 184, 64, 0, 0, 30, 8, 3, 16 },
	{ 5, 0, 128, 88, 3, 0, 30, 32, 4, 24 },
	{ 5, 0, 128, 88, 3, 0, 34, 32, 4, 24 },
	{ 5, 0, 184, 64, 0, 0, 33, 8, 3, 16 },
	{ 5, 0, 128, 88, 3, 0, 30, 32, 4, 24 },
	{ 5, 0, 128, 88, 3, 0, 34, 32, 4, 24 },
	{ 5, 0, 184, 64, 0, 0, 36, 8, 3, 16 },
	{ 5, 0, 128, 88, 3, 0, 30, 32, 4, 24 },
	{ 5, 0, 128, 88, 3, 0, 34, 32, 4, 24 },
	{ -1 }
};
AnimStep far Anim_3574[] = {
	{ 3, 15, 36, 48, 1, 1, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 2, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 3, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 4, 0, 0, 0, 0 },
	{ 6, 11, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 5, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 6, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 90, 7, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 6, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 5, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 4, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 3, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 2, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 1, 0, 0, 0, 0 },
	{ 3, 15, 36, 48, 1, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_357F[] = {
	{ 2, 15, 36, 48, 90, 0, 0, 0, 0, 0 },
	{ 7, 15, 36, 48, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3582[] = {
	{ 3, 17, 36, 48, 0, 1, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 1, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 1, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 2, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 2, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 2, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 3, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 3, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 3, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 4, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 4, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 4, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 5, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 5, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 5, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 6, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 6, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 6, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 7, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 7, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 54, 7, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 6, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 6, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 6, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 5, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 5, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 5, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 4, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 4, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 4, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 3, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 3, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 3, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 2, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 2, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 2, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 1, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 1, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 1, 0, 0, 0, 0 },
	{ 3, 17, 36, 48, 0, 0, 0, 0, 0, 0 },
	{ 3, 16, 142, 80, 0, 0, 0, 0, 0, 0 },
	{ 3, 19, 151, 28, 1, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35A0[] = {
	{ 2, 17, 36, 48, 0, 0, 0, 0, 0, 0 },
	{ 2, 16, 142, 80, 0, 0, 0, 0, 0, 0 },
	{ 2, 19, 151, 28, 54, 0, 0, 0, 0, 0 },
	{ 7, 17, 36, 48, 0, 0, 0, 0, 0, 0 },
	{ 7, 16, 142, 80, 0, 0, 0, 0, 0, 0 },
	{ 7, 19, 151, 28, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35A5[] = {
	{ 3, 22, 21, 32, 0, 1, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 1, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 2, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 1, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 3, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 2, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 4, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 3, 0, 0, 0, 0 },
	{ 6, 11, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 5, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 4, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 6, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 5, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 7, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 18, 6, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 7, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 90, 6, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 6, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 5, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 5, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 4, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 4, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 3, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 3, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 2, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 2, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 1, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 1, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 1, 0, 0, 0, 0 },
	{ 3, 22, 21, 32, 0, 0, 0, 0, 0, 0 },
	{ 3, 23, 67, 80, 1, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35BB[] = {
	{ 2, 22, 21, 32, 0, 0, 0, 0, 0, 0 },
	{ 2, 23, 67, 80, 90, 0, 0, 0, 0, 0 },
	{ 7, 22, 21, 32, 0, 0, 0, 0, 0, 0 },
	{ 7, 23, 67, 80, 0, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35BF[] = {
	{ 6, 10, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 104, 24, 5, 0, 0, 8, 15, 112 },
	{ 5, 0, 104, 24, 5, 0, 15, 8, 15, 112 },
	{ -1 }
};
AnimStep far Anim_35C2[] = {
	{ 5, 0, 184, 64, 0, 0, 30, 8, 3, 16 },
	{ 5, 0, 104, 72, 5, 0, 10, 120, 10, 64 },
	{ 5, 0, 184, 64, 0, 0, 33, 8, 3, 16 },
	{ 5, 0, 104, 72, 5, 0, 20, 120, 10, 64 },
	{ 5, 0, 184, 64, 0, 0, 36, 8, 3, 16 },
	{ -1 }
};
AnimStep far Anim_35C7[] = {
	{ 5, 0, 104, 0, 8, 0, 0, 0, 13, 200 },
	{ 5, 0, 104, 0, 8, 0, 13, 0, 13, 200 },
	{ 5, 0, 104, 0, 8, 0, 26, 0, 13, 200 },
	{ -1 }
};
AnimStep far Anim_35CA[] = {
	{ 5, 0, 104, 0, 0, 0, 13, 0, 13, 200 },
	{ -1 }
};
AnimStep far Anim_35CC[] = {
	{ 5, 0, 104, 32, 0, 0, 10, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35CE[] = {
	{ 5, 0, 104, 32, 3, 0, 20, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35D0[] = {
	{ 5, 0, 104, 32, 0, 0, 30, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35D2[] = {
	{ 5, 0, 104, 32, 0, 0, 0, 96, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35D4[] = {
	{ 5, 0, 104, 32, 3, 0, 10, 96, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35D6[] = {
	{ 5, 0, 104, 32, 0, 0, 20, 96, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35D8[] = {
	{ 5, 0, 104, 32, 0, 0, 0, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35DA[] = {
	{ 5, 0, 104, 32, 3, 0, 0, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35DC[] = {
	{ 5, 0, 104, 32, 4, 0, 10, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35DE[] = {
	{ 5, 0, 104, 32, 8, 0, 20, 0, 10, 96 },
	{ -1 }
};
AnimStep far Anim_35E0[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 6, 112, 72, 6, 0, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 7, 112, 72, 6, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 8, 112, 72, 6, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35E8[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ 6, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35F1[] = {
	{ 6, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 3, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35F5[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 6, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_35FB[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 10, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 11, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 12, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 13, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 10, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 13, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3605[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 11, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 10, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 13, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 12, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 11, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 13, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_360F[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 2, 14, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 9, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 3, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 15, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 16, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 1, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 4, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 17, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 14, 136, 40, 0, 0, 0, 0, 0, 0 },
	{ 2, 2, 224, 72, 0, 0, 0, 0, 0, 0 },
	{ 2, 5, 240, 40, 3, 0, 0, 0, 0, 0 },
	{ 2, 17, 136, 40, 3, 0, 0, 0, 0, 0 },
	{ -1 }
};
AnimStep far Anim_3619[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 176, 56, 3, 0, 19, 136, 2, 8 },
	{ 5, 0, 176, 56, 3, 0, 21, 136, 2, 8 },
	{ 5, 0, 176, 56, 3, 0, 23, 136, 2, 8 },
	{ -1 }
};
AnimStep far Anim_361D[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 176, 56, 0, 0, 19, 136, 2, 8 },
	{ 5, 0, 144, 88, 5, 0, 1, 136, 6, 40 },
	{ 5, 0, 176, 56, 0, 0, 21, 136, 2, 8 },
	{ 5, 0, 144, 88, 3, 0, 7, 136, 6, 40 },
	{ 5, 0, 176, 56, 2, 0, 23, 136, 2, 8 },
	{ 5, 0, 144, 88, 3, 0, 13, 136, 6, 40 },
	{ 5, 0, 176, 56, 0, 0, 19, 136, 2, 8 },
	{ 5, 0, 144, 88, 6, 0, 1, 136, 6, 40 },
	{ 5, 0, 176, 56, 0, 0, 21, 136, 2, 8 },
	{ 5, 0, 144, 88, 3, 0, 7, 136, 6, 40 },
	{ 5, 0, 176, 56, 0, 0, 23, 136, 2, 8 },
	{ 5, 0, 144, 88, 3, 0, 1, 136, 6, 40 },
	{ -1 }
};
AnimStep far Anim_3627[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 80, 64, 0, 0, 1, 136, 2, 16 },
	{ 5, 0, 248, 64, 3, 0, 7, 136, 2, 16 },
	{ 5, 0, 80, 64, 0, 0, 3, 136, 2, 16 },
	{ 5, 0, 248, 64, 3, 0, 9, 136, 2, 16 },
	{ 5, 0, 80, 64, 0, 0, 5, 136, 2, 16 },
	{ 5, 0, 248, 64, 3, 0, 11, 136, 2, 16 },
	{ -1 }
};
AnimStep far Anim_362D[] = {
	{ 6, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 5, 0, 16, 48, 0, 0, 0, 152, 14, 48 },
	{ 5, 0, 192, 48, 4, 0, 14, 128, 13, 48 },
	{ 5, 0, 16, 48, 0, 0, 1, 40, 35, 48 },
	{ -1 }
};
ShapeCut far Cuts_3631[] = {
	{ 0, 1, 137, 4, 43 },
	{ 1, 5, 137, 4, 54 },
	{ 2, 9, 140, 4, 8 },
	{ 3, 1, 181, 1, 3 },
	{ 4, 9, 154, 1, 3 },
	{ 5, 9, 158, 2, 5 },
	{ 6, 9, 164, 3, 7 },
	{ 7, 9, 172, 3, 11 },
	{ 8, 9, 184, 5, 13 },
	{ 9, 15, 137, 6, 15 },
	{ 10, 15, 153, 2, 21 },
	{ 11, 17, 153, 2, 21 },
	{ 12, 1, 185, 1, 4 },
	{ 13, 1, 190, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_3637[] = {
	{ 0, 1, 137, 1, 16 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_3638[] = {
	{ 15, 2, 0, 32, 14 },
	{ 16, 5, 16, 5, 8 },
	{ 17, 2, 27, 32, 18 },
	{ 18, 0, 140, 35, 10 },
	{ 19, 21, 16, 3, 8 },
	{ 20, 0, 47, 22, 10 },
	{ 21, 0, 59, 31, 10 },
	{ 22, 0, 71, 35, 39 },
	{ 23, 0, 119, 24, 15 },
	{ 0, 0, 0, 0, 0 }
};
ShapeCut far Cuts_363C[] = {
	{ -9, 1, 136, 6, 40 },
	{ -1, 7, 136, 6, 40 },
	{ -2, 13, 136, 6, 40 },
	{ 3, 19, 136, 2, 16 },
	{ 4, 21, 136, 2, 16 },
	{ 5, 23, 136, 2, 16 },
	{ 6, 19, 152, 1, 8 },
	{ 7, 20, 152, 1, 8 },
	{ 8, 21, 152, 1, 8 },
	{ -10, 25, 136, 2, 16 },
	{ -11, 27, 136, 2, 16 },
	{ -12, 29, 136, 2, 16 },
	{ -13, 31, 136, 2, 16 },
	{ -14, 25, 152, 2, 16 },
	{ -15, 27, 152, 2, 16 },
	{ -16, 29, 152, 2, 16 },
	{ -17, 31, 152, 2, 16 },
	{ 0, 0, 0, 0, 0 }
};

/* Initialised data of the module (DS:185E...) */
byte far *PalBlack = 0;                  /* 185E */
byte far *PalKhelben = 0;                /* 1862 PALETTE3.PAL */
byte far *PalStudy = 0;                  /* 1866 PALETTE2.PAL */
byte far *PalStreet = 0;                 /* 186A PALETTE4.PAL */
byte far *PalNow = 0;                    /* 186E the colours set on the screen */
byte far *Fading = 0;                    /* 1872 7 fading tables (FADING.DAT) */
char far *IntroShapes[30] = { 0 };       /* 1876 */
int Skipped = 0;                         /* 18EE a key stopped the intro */
int CanSkip = 0;                         /* 18F0 */
AnimStep far *Anims[44] = {              /* 18F2 */
	Anim_3539, Anim_353D, Anim_3541, Anim_3545, Anim_354E, Anim_354B, Anim_355A, Anim_3551, Anim_3555,
	Anim_356A, Anim_356D, Anim_3574, Anim_357F, Anim_35BF, Anim_35C2, Anim_35C7, Anim_35E0, Anim_35E8,
	Anim_35F5, Anim_35FB, Anim_3605, Anim_360F, Anim_35CC, Anim_35CE, Anim_35D0, Anim_35D2, Anim_35D4,
	Anim_35D6, Anim_35D8, Anim_35DA, Anim_35DC, Anim_35DE, Anim_3619, Anim_361D, Anim_3627, Anim_362D,
	Anim_3565, Anim_35CA, Anim_3582, Anim_35A0, Anim_35A5, Anim_35BB, Anim_3548, Anim_35F1
};
ShapeCut far *ShapeCuts[13] = {          /* 19A2 one per picture */
	Cuts_3631, Cuts_3637, 0, 0, Cuts_3638, 0, 0, Cuts_363C, 0, 0, 0, 0, 0
};
char far *Pictures[13] = {               /* 19D6 */
	"STREET1.CPS", "STREET2.CPS", "DOORWAY1.CPS", "DOORWAY2.CPS", "WESTWOOD.CPS", "WINDING.CPS",
	"KHELBAN2.CPS", "KHELBAN1.CPS", "KHELBAN3.CPS", "KHELBAN4.CPS", "COIN.CPS", "KHELBAN5.CPS",
	"KHELBAN6.CPS"
};

/* 5989:0000 [asm] - a picture into page | 1, its shapes cut, then page */
void far show_picture(int pic, int page)
{
	if (!Skipped) {
		load_lbm(Pictures[pic], page | 1, page | 1, 0L, 1);
		cut_shapes(page, pic);
		Drv_654A(page | 1, page, 0L);
	}
}

/* 5989:0060 [asm] - the picture of page on the screen */
void far show_page(int page)
{
	if (!Skipped) {
		if (VideoMode == 3)
			fade_to(Palette);
		copy_page(0, 0, 1, 8, 38, 128, page, 0);
	}
}

/* 5989:00A5 [asm] - wait until TimerTicks reaches t (or a key) */
int far intro_wait(unsigned long t)
{
	if (Skipped)
		return 1;
	while (t > TimerTicks)
		if (check_skip())
			break;
	return check_skip();
}

/* 5989:00D8 [asm] */
int far check_skip(void)
{
	register int k;

	if (!Skipped && CanSkip) {
		k = key_pressed();
		if (k) {
			k = get_key();
			Skipped = 1;
		}
	}
	return Skipped;
}

/* 5989:0108 [asm] - the picture of page slides in from the right */
void far slide_in(register int page, int speed)
{
	unsigned long t;
	register int i;

	if (Skipped)
		return;
	if (VideoMode == 3 || VideoMode == 6)
		for (i = 0; i < 280; i += 3) {
			t = TimerTicks + speed;
			Drv_653E(11, 8, 8, 8, 0x12D, 0x80, 0, 0);
			Drv_653E(i, 0, 0x135, 8, 3, 0x80, page, 0);
			if (i == 0x60)
				play_anim(0x2A);
			if (intro_wait(t))
				break;
		}
	else
		for (i = 0; i < 35; i++) {
			t = TimerTicks + speed * 2;
			copy_page(2, 8, 1, 8, 37, 128, 0, 0);
			copy_page(i, 0, 38, 8, 1, 128, page, 0);
			if (i == 12)
				play_anim(0x2A);
			if (i == 25)
				play_sound(11);
			if (intro_wait(t))
				break;
		}
}

/* 5989:0255 [asm] - the shapes of a picture (page) */
void far cut_shapes(int page, int pic)
{
	int old;
	ShapeCut far *c;

	c = ShapeCuts[pic];
	if (c) {
		old = set_page(page);
		for (;;) {
			if (!c->w)
				break;
			if (c->idx < 0) {
				if (IntroShapes[-c->idx])
					farfree(IntroShapes[-c->idx]);
				IntroShapes[-c->idx] = Drv_6592(c->x, c->y, c->w, c->h, 0L);
			} else {
				if (IntroShapes[c->idx])
					farfree(IntroShapes[c->idx]);
				IntroShapes[c->idx] = Drv_658E(c->x, c->y, c->w, c->h, 0L);
			}
			c++;
		}
		set_page(old);
	}
}

/* 5989:0387 [asm] - the 7 fading tables of the VGA (made once, kept in
 * FADING.DAT) */
void far load_fading(void)
{
	register int i, fd;

	if (VideoMode == 3) {
		if (file_exists("FADING.DAT")) {
			fd = open_file("FADING.DAT", 1);
			read_file(fd, Fading, 0x700L);
		} else {
			for (i = 0; i < 7; i++)
				Drv_6552(Palette, Fading + (i << 8), 18, (i + 1) * 36);
			fd = open_file("FADING.DAT", 2);
			write_file(fd, Fading, 0x700L);
		}
		close_file(fd);
	}
}

/* 5989:043F [asm] - set the colours of pal that changed, 80 at a time */
void far fade_to(byte far *pal)
{
	int changed;
	byte far *p;
	byte far *q;
	register int i, j;

	if (VideoMode == 3) {
		p = pal;
		q = PalNow;
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
		mem_copy(pal, PalNow, 0x300L);
	}
}

/* 5989:0507 [asm] */
void far play_anim(int n)
{
	int y, w, h, sh, delay, op;
	unsigned long t;
	AnimStep far *s;
	register int x, pal;

	if (Skipped)
		return;
	for (s = Anims[n]; s->op != -1; s++) {
		sh = s->shape;
		op = s->op;
		x = s->x;
		y = s->y;
		delay = s->delay;
		pal = s->pal;
		switch (op) {
		case 6:
			intro_sound(sh);
			break;
		case 3:
		case 4:
			w = IntroShapes[sh][2];
			h = IntroShapes[sh][1];
			if (VideoMode == 3) {
				Drv_659E(1);
				Drv_6576(1);
				t = TimerTicks + delay;
				if (pal) {
					Drv_65DE((byte far *)((long)Fading + ((pal - 1) << 8)));
					copy_page((x >> 3) - 1, y - 8, 0, 0, w + 1, h, 2, 4);
					put_shape(4, IntroShapes[sh], x & 7, 0, 0);
					copy_page(0, 0, x >> 3, y, w + 1, h, 4, 0);
				} else
					copy_page((x >> 3) - 1, y - 8, x >> 3, y, w + 1, h, 2, 0);
				intro_wait(t);
				Drv_6576(0);
				Drv_659E(0);
			} else {
				if (pal)
					put_shape(0, IntroShapes[sh], x, y, 0);
				else
					copy_page((x >> 3) - 1, y - 8, x >> 3, y, w + 1, h, 2, 0);
				t = TimerTicks + delay * 7;
				while (t > TimerTicks)
					;
			}
			break;
		case 0:
			if (VideoMode == 3) {
				if (pal == 3)
					fade_to(PalStreet);
				if (pal == 2)
					fade_to(PalStudy);
				if (pal == 1)
					fade_to(PalKhelben);
			}
			intro_wait(TimerTicks + delay);
			if (pal && VideoMode == 3)
				fade_to(Palette);
			break;
		case 1:
			w = IntroShapes[sh][2];
			h = IntroShapes[sh][1];
			put_shape(0, IntroShapes[sh], x, y, 0);
			if (VideoMode == 3) {
				if (pal == 3)
					fade_to(PalStreet);
				if (pal == 2)
					fade_to(PalStudy);
				if (pal == 1)
					fade_to(PalKhelben);
			}
			intro_wait(TimerTicks + delay);
			if (pal && VideoMode == 3)
				fade_to(Palette);
			copy_page((x >> 3) - 1, y - 8, x >> 3, y, w + 1, h, 2, 0);
			break;
		case 7:
			w = IntroShapes[sh][2];
			h = IntroShapes[sh][1];
			intro_wait(TimerTicks + delay);
			copy_page((x >> 3) - 1, y - 8, x >> 3, y, w + 1, h, 2, 0);
			break;
		case 2:
			w = IntroShapes[sh][2];
			h = IntroShapes[sh][1];
			put_shape(Var_62A5, IntroShapes[sh], x, y, 0);
			if (VideoMode == 3) {
				if (pal == 3)
					fade_to(PalStreet);
				if (pal == 2)
					fade_to(PalStudy);
				if (pal == 1)
					fade_to(PalKhelben);
			}
			intro_wait(TimerTicks + delay);
			if (pal && VideoMode == 3)
				fade_to(Palette);
			break;
		case 5:
			w = s->w;
			h = s->h;
			copy_page(s->sx, s->sy, x >> 3, y, w, h, 2, 0);
			intro_wait(TimerTicks + delay);
			break;
		}
	}
}

/* 5989:0A7E [asm] - a text centred in window 17, line by line (\r); in
 * VGA its colour is copied into colour 255 */
void far intro_text(int col, char far *text)
{
	char far *p;
	char far *q;
	char save;
	register int len, line;

	if (Skipped)
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

/* 5989:0BE1 [asm] - the text fades out */
void far end_text(void)
{
	if (!Skipped) {
		if (VideoMode == 3)
			fade_colour(255, 2, Palette);
		gui_clear();
	}
}

/* 5989:0C0F [asm] - colour c of pal goes down to black, step by step.
 * (sic: it compares pal and changes Palette, the same palette here) */
void far fade_colour(unsigned char c, unsigned char step, byte far *pal)
{
	int changed;
	register int k, base;

	if (VideoMode == 3) {
		base = c * 3;
		for (;;) {
			changed = 0;
			for (k = 0; k < 3; k++)
				if (pal[base + k] > step) {
					Palette[base + k] -= step;
					changed = 1;
				} else if (Palette[base + k] > 0) {
					pal[base + k] = 0;
					changed = 1;
				}
			if (!changed)
				break;
			wait_tick();
			Drv_65E2(pal, c, c);
		}
	}
}

/* 5989:0CB2 [asm] - fade_palette of palette.c, 5 steps at a time */
void far fade_fast(char far *pal, char far *target, int delay)
{
	int changed, b;
	unsigned long end;
	int step, frac, d, t;
	register int a, i;

	if (target && pal) {
		if (VideoMode != 3)
			set_palette(target);
		else {
			step = delay << 8;
			for (i = 0; i < 0x300; i++) {
				a = target[i];
				b = pal[i];
				d = (a - b) / 2;
				d = d < 0 ? -d : d;
				t = delay << 8;
				if (d)
					t = t / d;
				step = step < t ? step : t;
			}
			frac = 0;
			end = TimerTicks;
			do {
				changed = 0;
				frac += step;
				end += frac >> 8;
				frac &= 0xFF;
				for (i = 0; i < 0x300; i++) {
					a = target[i];
					b = pal[i];
					if (a != b) {
						if (b < a) {
							b++;
							if (b < a)
								b++;
							if (b < a)
								b++;
							if (b < a)
								b++;
							if (b < a)
								b++;
							changed = 1;
						}
						if (b > a) {
							b--;
							if (b > a)
								b--;
							if (b > a)
								b--;
							if (b > a)
								b--;
							if (b > a)
								b--;
							changed = 1;
						}
						pal[i] = b;
					}
				}
				if (changed) {
					wait_retrace(0);
					fade_to(pal);
					while (TimerTicks < end)
						;
				}
			} while (changed);
		}
	}
}

/* 5989:0E4D [asm] */
void far intro_sound(int sound)
{
	if (!Skipped)
		play_sound(sound);
}

/* 5989:0E63 [asm] */
void far play_intro(void)
{
	int i;
	register int alt;

	hide_mouse();
	set_page(0);
	Drv_655A();
	PalBlack = mem_alloc(0x300L, 0x10);
	PalKhelben = mem_alloc(0x300L, 0x10);
	PalStudy = mem_alloc(0x300L, 0x10);
	PalStreet = mem_alloc(0x300L, 0x10);
	PalNow = mem_alloc(0x300L, 0x10);
	Fading = mem_alloc(0x700L, 0);
	load_file_into("PALETTE4.PAL", PalStreet, 0x300L);
	load_file_into("PALETTE3.PAL", PalKhelben, 0x300L);
	load_file_into("PALETTE2.PAL", PalStudy, 0x300L);
	load_sounds("INTRO");
	Skipped = 0;
	if (VideoMode == 3)
		load_file_into("PALETTE1.PAL", Palette, 0x300L);
	else
		load_file_into("PALETTE0.PAL", Palette, 0x300L);
	set_palette(Palette);
	play_sound(0);
	show_picture(4, 2);
	show_picture(0, 2);
	intro_wait(TimerTicks + 1);
	play_sound(12);
	if (VideoMode == 3) {
		memset(PalBlack, 0, 0x300);
		set_palette(PalBlack);
		copy_page(0, 0, 1, 8, 38, 128, 2, 0);
		fade_fast(PalBlack, Palette, 3);
	} else
		show_page(2);
	load_fading();
	set_page(2);
	select_window_clear(17);
	set_page(0);
	CanSkip = 1;
	flush_keys();
	intro_wait(TimerTicks + 18);
	play_anim(3);
	intro_wait(TimerTicks + 18);
	play_anim(6);
	intro_wait(TimerTicks + 18);
	play_anim(0);
	if (SoundType == 1)
		while (music_position() < 1 && !check_skip())
			;
	play_anim(VideoMode == 3 ? 11 : 12);
	play_anim(7);
	intro_wait(TimerTicks + 6);
	play_anim(2);
	intro_wait(TimerTicks + 6);
	if (SoundType == 1)
		while (music_position() < 2 && !check_skip())
			;
	play_anim(VideoMode == 3 ? 0x26 : 0x27);
	play_anim(3);
	play_anim(8);
	play_anim(1);
	intro_wait(TimerTicks + 10);
	play_anim(0);
	intro_wait(TimerTicks + 6);
	play_anim(2);
	if (SoundType == 1)
		while (music_position() < 3 && !check_skip())
			;
	if (!Skipped) {
		select_window_clear(17);
		set_page(2);
		select_window_clear(17);
		set_page(0);
	}
	play_anim(VideoMode == 3 ? 0x28 : 0x29);
	play_anim(7);
	intro_wait(TimerTicks + 18);
	intro_text(16, Text_3506);
	play_anim(7);
	intro_wait(TimerTicks + 90);
	end_text();
	intro_text(16, Text_350A);
	play_anim(8);
	play_anim(2);
	intro_wait(TimerTicks + 72);
	end_text();
	intro_text(16, Text_350C);
	play_anim(2);
	play_anim(6);
	intro_wait(TimerTicks + 36);
	play_anim(3);
	end_text();
	intro_text(16, Text_3511);
	show_picture(1, 2);
	if (SoundType == 1)
		while (music_position() < 4 && !check_skip())
			;
	slide_in(2, 1);
	if (!Skipped)
		copy_page(1, 8, 0, 0, 38, 128, 0, 2);
	play_anim(4);
	end_text();
	intro_wait(TimerTicks + 10);
	show_picture(2, 2);
	show_page(2);
	intro_wait(TimerTicks + 10);
	intro_text(16, Text_3513);
	intro_wait(TimerTicks + 25);
	show_picture(3, 2);
	intro_wait(TimerTicks + 54);
	play_anim(13);
	if (!Skipped)
		copy_page(13, 16, 12, 8, 15, 100, 0, 2);
	end_text();
	intro_text(15, Text_3515);
	play_anim(10);
	play_anim(10);
	play_anim(9);
	play_anim(9);
	end_text();
	intro_text(15, Text_3517);
	play_anim(10);
	play_anim(10);
	play_anim(10);
	play_anim(9);
	play_anim(14);
	show_picture(5, 2);
	if (SoundType == 1)
		while (music_position() < 5 && !check_skip())
			;
	end_text();
	if (!Skipped)
		Drv_655A();
	play_anim(15);
	play_anim(15);
	play_anim(15);
	play_anim(15);
	play_anim(15);
	play_anim(15);
	if (VideoMode != 3) {
		load_file_into("INTRO.EGA", Palette, 0x300L);
		set_palette(Palette);
	}
	show_picture(6, 2);
	show_picture(7, 2);
	if (!Skipped)
		Drv_655A();
	show_page(2);
	play_anim(16);
	intro_text(15, Text_351A);
	play_anim(16);
	play_anim(17);
	play_anim(16);
	play_anim(16);
	play_anim(16);
	end_text();
	play_anim(16);
	show_picture(8, 2);
	show_page(2);
	play_anim(32);
	intro_text(15, Text_351D);
	play_anim(33);
	play_anim(33);
	play_anim(32);
	play_anim(32);
	play_anim(32);
	play_anim(32);
	end_text();
	intro_text(15, Text_351F);
	play_anim(33);
	play_anim(43);
	play_anim(33);
	play_anim(32);
	play_anim(32);
	play_anim(32);
	end_text();
	intro_text(15, Text_3522);
	play_anim(33);
	play_anim(33);
	play_anim(33);
	play_anim(33);
	play_anim(32);
	play_anim(32);
	show_picture(9, 2);
	end_text();
	if (SoundType == 1)
		while (music_position() < 6 && !check_skip())
			;
	show_page(2);
	play_anim(34);
	intro_text(15, Text_3525);
	play_anim(34);
	play_anim(34);
	play_anim(34);
	play_anim(35);
	play_anim(34);
	play_anim(34);
	play_anim(34);
	play_anim(34);
	end_text();
	show_picture(12, 2);
	show_page(2);
	show_picture(6, 2);
	play_anim(18);
	intro_text(15, Text_3527);
	play_anim(19);
	play_anim(20);
	play_anim(22);
	play_anim(19);
	play_anim(20);
	play_anim(18);
	end_text();
	intro_text(15, Text_352A);
	play_anim(20);
	play_anim(19);
	play_anim(23);
	play_anim(24);
	play_anim(20);
	play_anim(19);
	play_anim(17);
	play_anim(18);
	end_text();
	intro_text(15, Text_352C);
	play_anim(19);
	play_anim(20);
	play_anim(20);
	play_anim(18);
	play_anim(25);
	play_anim(18);
	play_anim(18);
	end_text();
	play_anim(18);
	play_anim(18);
	intro_text(15, Text_352E);
	play_anim(28);
	play_anim(19);
	play_anim(20);
	play_anim(18);
	play_anim(18);
	end_text();
	show_picture(10, 2);
	if (!Skipped)
		Drv_655A();
	play_anim(37);
	intro_wait(TimerTicks + 18);
	play_anim(36);
	intro_wait(TimerTicks + 36);
	show_picture(12, 2);
	if (!Skipped)
		Drv_655A();
	show_page(2);
	show_picture(11, 2);
	intro_text(15, Text_352F);
	play_anim(19);
	play_anim(20);
	play_anim(20);
	play_anim(18);
	play_anim(18);
	end_text();
	intro_text(15, Text_3531);
	play_anim(19);
	play_anim(20);
	play_anim(19);
	play_anim(18);
	play_anim(18);
	end_text();
	play_anim(18);
	intro_text(15, Text_3533);
	play_anim(20);
	play_anim(27);
	play_anim(20);
	play_anim(19);
	play_anim(18);
	play_anim(18);
	end_text();
	play_anim(18);
	intro_text(15, Text_3536);
	play_anim(19);
	play_anim(19);
	play_anim(20);
	play_anim(18);
	end_text();
	play_anim(29);
	if (SoundType == 1)
		while (music_position() < 7 && !check_skip())
			;
	play_anim(30);
	play_anim(31);
	if (SoundType == 1)
		for (alt = 0; music_position() < 8 && !check_skip(); alt = !alt ? 1 : 0)
			play_anim(alt ? 30 : 31);
	if (!Skipped) {
		select_window(17);
		TextBg = 0;
		gui_clear();
		play_sound(14);
		if (VideoMode == 3) {
			memset(PalBlack + 3, 0x3F, 0x2FD);
			fade_fast(Palette, PalBlack, 1);
			select_window_clear(18);
			intro_wait(TimerTicks + 6);
			memset(PalBlack + 3, 0, 0x2FD);
			fade_fast(Palette, PalBlack, 1);
			Drv_655A();
		} else {
			select_window_clear(18);
			intro_wait(TimerTicks + 6);
			Drv_655A();
		}
	} else
		play_sound(15);
	for (i = 0; i < 30; i++) {
		if (IntroShapes[i])
			farfree(IntroShapes[i]);
		IntroShapes[i] = 0;
	}
	farfree(Fading);
	farfree(PalNow);
	farfree(PalBlack);
	farfree(PalKhelben);
	farfree(PalStudy);
	farfree(PalStreet);
	Drv_655A();
	show_mouse();
}
