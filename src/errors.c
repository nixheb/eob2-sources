/*
 * Eye of the Beholder II - START.EXE
 * The file errors of the Westwood library ("Insert the disk..."):
 * module 1A37 (Ghidra 2A37).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y errors.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern int            GuiStyle;          /* 636F the current window */
extern int            Var_6286;          /* 6286 */
extern int            CursorX, CursorY;  /* F094, F096 the text cursor in the window */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern byte far      *Font8;             /* F098 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
/* Uninitialised data of the module (DS:EF52-EF53, _BSS), in address order */
int ErrorDrive; /* EF52 the drive of the file */
extern char           VideoReady;        /* 638E set by video_init */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  select_window(int w);                                       /* 31B0:0133 returns the old one */
int  set_page(int a);                                        /* 310B:0001 returns the old value */
byte far *set_font(byte far *font);                              /* 3166:000A returns the old one */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
long mem_free_total(void);                                       /* 2D0E:0249 */
byte far *mem_alloc(unsigned long size, int flags);              /* 2D0E:000B */
void gui_clear(void);                                            /* 31B0:020F */
void draw_rect2(int x1, int y1, int x2, int y2, int colour);   /* 2D9E:0000 a frame */
int  file_disk(char far *name);                                 /* 2DCE:153E */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void flush_keys(void);                                           /* 3295:0516 */
int  get_key(void);                                              /* 3295:05D0 */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);
void farfree(void far *p);
int  printf(char far *fmt, ...);
extern int (far      *Drv_65EE)(int w, int h);                   /* 65EE the size to save a region */
extern void (far     *Drv_65D2)(int x, int y, int w, int h, byte far *buf);    /* 65D2 saves a region */
extern void (far     *Drv_6546)(int x, int y, int w, int h, byte far *buf); /* 6546 restores it */
void far grow_window(int n);

/* ------------------------------------------------------------------ */
/* Module 2A37, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5E3A...) */
char far *FileErrors[20] = {                                             /* 5E3A */
	"Could not create file '%s' on disk.",
	"Illegal MODE in Open_File.",
	"Could not find file '%s'.",
	"Too many files open.",
	"Trying to close unopened file.",
	"Trying to read unopened file.",
	"Trying to write unopened file.",
	"Trying to seek unopened file.",
	"Illegal starting position in Seek_File.",
	"Trying to write resident file '%s'.",
	"Trying to Get_Resident_Index on unknown file.",
	"You are exiting with open files.",
	"A fatal disk error has occurred. Now returning to DOS.",
	"File '%s' not listed in FileData[].",
	"File length does not match size listed in FileData[].",
	"Internal Error in File_Exists().",
	"Trying to Alloc a 0 sized file entry.",
	"Failure in sort code for resident files.",
	"\r ",
	"\r\n"
};
byte far *ErrorSave = 0;                 /* 5E8A what the error window covers */

/* 2A37:000F [asm] - widen the current window by n characters on each side */
void far grow_window(register int n)
{
	Dims[GuiStyle].x -= n;
	Dims[GuiStyle].y -= n * 8;
	Dims[GuiStyle].w += n * 2;
	Dims[GuiStyle].h += n << 4;
	select_window(GuiStyle);
	CursorX = CursorY = 0;
}

/* 2A37:0066 [asm] - show a file error in a window, wait for a key;
 * Escape quits to DOS */
int far file_error_prompt(int err, char far *name)
{
	int oldWin, key, x, y, w, x2, y2;
	char far *msg;
	byte far *font;
	char buf[4];
	int old;
	register unsigned size;
	register int h;

	size = 0;
	hide_mouse();
	msg = FileErrors[err];
	old = set_page(0);
	oldWin = select_window(Var_6286);
	font = set_font(Font8);
	x = WinX * 8;
	y = WinY;
	w = WinW * 8;
	h = WinH;
	x2 = x + w - 1;
	y2 = y + h - 1;
	if (!ErrorSave) {
		size = Drv_65EE(WinW, h);
		if (mem_free_total() >= size)
			ErrorSave = mem_alloc(size, 0);
		else
			size = 0;
	}
	if (ErrorSave)
		Drv_65D2(WinX, WinY, WinW, h, ErrorSave);
	gui_clear();
	draw_rect2(x, y, x2, y2, 15);
	grow_window(-1);
	ErrorDrive = file_disk(name);
	gui_printf(msg, name);
	if (err == 2 || err == 0) {
		buf[0] = ErrorDrive + '1';                /* sic: never used */
		buf[1] = 0;
		gui_printf("  Insert the disk containing '%s' in any drive.", name);
	}
	flush_keys();
	key = get_key();
	if (key == 0x1B) {
		gui_clear();
		gui_printf(FileErrors[12]);
		get_key();
		system_shutdown();
		exit(5);
	}
	grow_window(1);
	if (ErrorSave)
		Drv_6546(WinX, WinY, WinW, h, ErrorSave);
	select_window(oldWin);
	set_font(font);
	set_page(old);
	if (size) {
		farfree(ErrorSave);
		ErrorSave = 0;
	}
	show_mouse();
	return 1;
}

/* 2A37:0286 [asm] - the handler installed by main() */
int far file_error_handler(int err, char far *name)
{
	int page;

	page = Var_62A5;
	Var_62A5 = 0;
	file_error_prompt(err, name);
	Var_62A5 = page;
	return 1;
}

/* 2A37:02B4 [asm] */
int far open_error(int err, char far *name)
{
	printf("Cannot open %s - exiting\r\n", name);
	VideoReady = 0;
	system_shutdown();
	exit(5);
	return 0;
}

/* 2A37:02E3 [asm] */
int far io_error(int err, char far *name)
{
	printf("IO error with %s - exiting\r\n", name);
	VideoReady = 0;
	system_shutdown();
	exit(5);
	return 0;
}
