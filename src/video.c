/*
 * Eye of the Beholder II - START.EXE
 * Starting the video: module 212D (Ghidra 312D).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y video.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The video modes, by number (the first word of EOBPREFS.DAT): each one has
 * its driver, an overlay file loaded into the driver table (DS:6546...):
 *   0 cga.ovl   1 tga.ovl (Tandy)   2 ega.ovl   3 mga.ovl (MCGA/VGA 256)
 *   4 vga.ovl   5 xga.ovl   6 yga.ovl          8: no driver
 * The pages (PageSegs, DS:62A7): page 0 is the screen. In EGA the 8 pages
 * are in video memory (A000, A200... AE00); in modes 4 and 5, 4 of them
 * (A000, A400, A800, AC00). The others are allocated (PageSizes).
 */
#include "eob2.h"

#define FP_SEG( fp )( (unsigned )( void _seg * )( void far * )( fp ))

typedef unsigned char byte;

extern unsigned       PageSegs[];        /* 62A7 */
extern unsigned long  PageSizes[];       /* 62E7 */
extern char far      *DriverFile;        /* 6371 */
/* Uninitialised data of the module (DS:F052-F08F, _BSS), in address order */
int PageFlags[31]; /* F052 the flags of mem_alloc for each page */
extern byte far      *Font8;             /* F098 */
extern byte far      *Palette;           /* F168 */
extern char           Scratch[];         /* F16E */
extern void (far     *Drv_655A)(void);   /* 655A starts the driver */
extern void (far     *Drv_65E2)(byte far *pal, int first, int last);   /* 65E2 sets colours */

void set_text_mode(int mode);                                    /* 31A9:0033 */
void mouse_init(void);                                           /* 3269:0008 (assembler) */
int  file_exists(char far *name);                                /* 2DCE:11CF */
int  load_video_driver(char far * far *name);                    /* 33CD:005D */
void dos_puts(char far *s);                                      /* 3246:0006 (assembler) */
void flush_keys(void);                                           /* 3295:0516 */
int  get_key(void);                                              /* 3295:05D0 */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
void far *mem_alloc(long size, int flags);                       /* 2D0E:000B */
byte far *load_file_alloc(char far *name, int flags);            /* 308C:0005 */
byte far *set_font(byte far *font);                              /* 3166:000A */
void set_palette(byte far *pal);                                 /* 3111:000D */
void files_init(void);                                           /* 2DCE:127C */
void measure_machine(void);                                      /* 2F7A:0003 */
int  select_window(int w);                                       /* 31B0:0133 */
long time(long far *t);                                          /* 1000:6524 */
void srand(unsigned seed);                                       /* 1000:6292 */
void far *memset(void far *s, int c, unsigned n);                /* 1000:712B */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */

/* Initialised data of the module (DS:638E...) */
char VideoReady = 0;                     /* 638E cleared by the file errors */
byte EgaPalette[16] = { 0, 5, 3, 2, 10, 14, 12, 6, 4, 11, 9, 1, 0, 8, 7, 15 };   /* 638F */
byte Pal16[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };       /* 639F */
char far *DriverNames[7] = {             /* 63AF */
	"cga.ovl", "tga.ovl", "ega.ovl", "mga.ovl", "vga.ovl", "xga.ovl", "yga.ovl"
};

/* 312D:000B [asm] - 0 if the driver or the font cannot be loaded */
int far video_init(register int mode, char far *font)
{
	int ok;
	register int i;

	switch (mode) {
	case 3:
		memset((void far *)0xA0000000L, 0, 64000U);
	}
	if (mode != 8) {
		set_text_mode(mode);
		mouse_init();
		DriverFile = DriverNames[mode];
		ok = file_exists(DriverFile);
		if (ok)
			ok = load_video_driver(&DriverFile) != -1;
		if (!ok) {
			set_text_mode(9);
			sprintf(Scratch, "\r\nCould not load overlay \"%s\".  Press a key to return to DOS...\r\n", DriverFile);
			dos_puts(Scratch);
			flush_keys();
			get_key();
			return 0;
		}
	}
	for (i = 2; i < 16; i += 2)
		if (PageSizes[i + 1] && i)
			PageSegs[i + 1] = FP_SEG(mem_alloc(PageSizes[i + 1], PageFlags[i] | 0x20));
	switch (mode) {
	case 0:
	case 1:
		PageSegs[0] = PageSegs[1] = 0xB800;
		PageSegs[2] = PageSegs[3];
		PageSegs[4] = PageSegs[5];
		PageSegs[6] = PageSegs[7];
		break;
	case 2:
		PageSegs[0] = 0xA000;
		PageSegs[2] = 0xA200;
		PageSegs[4] = 0xA400;
		PageSegs[6] = 0xA600;
		PageSegs[8] = 0xA800;
		PageSegs[10] = 0xAA00;
		PageSegs[12] = 0xAC00;
		PageSegs[14] = 0xAE00;
		break;
	case 4:
	case 5:
		PageSegs[0] = 0xA000;
		PageSegs[2] = 0xA400;
		PageSegs[4] = 0xA800;
		PageSegs[6] = 0xAC00;
		break;
	case 3:
	case 6:
		PageSegs[0] = PageSegs[1] = 0xA000;
		PageSegs[2] = PageSegs[3];
		PageSegs[4] = PageSegs[5];
		PageSegs[6] = PageSegs[7];
	}
	if (mode != 8) {
		Drv_655A();
		if (font) {
			Font8 = load_file_alloc(font, 8);
			if (!Font8) {
				set_text_mode(9);
				sprintf(Scratch, "\r\nUnable to load font %s\r\nReinstall program.\r\n", font);
				dos_puts(Scratch);
				get_key();
				return 0;
			}
			set_font(Font8);
		}
	}
	Palette = mem_alloc(0x300L, 0x10);
	switch (mode) {
	case 5:
		memcpy(Palette, Pal16, 16);
		Drv_65E2(Palette, 0, 16);
		break;
	case 2:
		memcpy(Palette, EgaPalette, 16);
		Drv_65E2(Palette, 0, 16);
		break;
	case 3:
	case 4:
		memset(Palette + 45, 0x3F, 3);
		set_palette(Palette);
	}
	files_init();
	VideoReady = 1;
	measure_machine();
	srand(time(0L));
	select_window(0);
	return 1;
}
