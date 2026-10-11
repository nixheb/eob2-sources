/*
 * Eye of the Beholder II - The Legend of Darkmoon (Westwood / SSI, 1991)
 * START.EXE, Borland C++ 2.0 (bcc -ml -G -O -Z -y), large memory model.
 *
 * main() - hand reconstruction of code segment 0889 (Ghidra 1889:000A).
 * Addresses in comments are Ghidra addresses (Borland segment + 0x1000).
 * Every statement maps 1:1 onto the original machine code; the original
 * offset is given in the comments. Names are ours, the logic is not.
 */
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ovlnames.h"         /* the link names of overlay functions (no eob2.h here) */

/* ---- DGROUP globals used here (DS = 2B16 in the file, 3B16 in Ghidra) ---- */

/* The game keeps its messages behind far "char *" variables rather than
 * using literals directly - main() loads the pointer, not the string.     */
/* Initialised data of the module (DS:1CC2...) */
unsigned _ovrbuffer  = 0x2A8;   /* DS:1CC2 the overlay buffer: 680 paragraphs (OVERLAY.LIB's default: OVRBUFF) */
char *FontName      = "FONT8.FNT";                                         /* DS:1CC4 */
char *ExeName       = "START.EXE";                                         /* DS:1CC8 */
/* the two messages belong to the data module linked after this one (DS:1CFC-2FD5) */
extern char *Text_NeedRam;       /* DS:262E "\nEye of the Beholder II requires %lu bytes of RAM free.\n" */
extern char *Text_RunSetup;      /* DS:2632 "Please run SETUP before running The Legend of Darkmoon.\n" */

extern int   VideoMode;          /* DS:628A  1st word of EOSPREFS.DAT (3 on this install) */
extern int   SoundType;        /* DS:638A  2nd word                                      */
extern int   SoundEnabled;       /* DS:65FA                                                */
extern unsigned char MousePresent; /* DS:6654  (set during system_init - name is a guess)    */
extern char  InventoryMode;           /* DS:6655  meaning still unknown                         */
extern unsigned char far *Palette;   /* DS:F168  768-byte VGA palette buffer               */

/* Westwood library: sizes of the 8 graphic pages (offscreen buffers), filled
 * in here and used by video_init() to allocate them.                        */
extern long  PageSizes[8];        /* DS:62E7 */

/* Error hooks the library calls back (far function pointers). */
extern int (far *FileErrorHandler)();    /* DS:6294 */
extern int (far *FileErrorPrompt)();     /* DS:62A1 */
extern int (far *MemHandler)();     /* DS:629A */

/* _argc, _argv: DS:6A70, 6A72, the runtime's (SETARGV, dos.h), only "touched" by main() */

int  system_init(char *exename, long mem_needed, int check);          /* 316D:0002 */
int  video_init(int mode, char *fontname);                            /* 312D:000B */
void sound_init(int driver);                                          /* 2C9F:020B */
void sound_shutdown(void);                                            /* 2C9F:024E */
void system_shutdown(void);                                           /* 315F:0000 */
long load_file_into(char *name, void far *buf, long size);            /* 3069:0000 */
void set_palette(unsigned char far *pal);                             /* 3111:000D */
void dos_puts(char *s);                                               /* 3246:0006 */
void game_main(int argc, char *argv[]);                               /* 18AE:0005 */
int  file_error_handler();                                            /* 2A37:0286 */
int  file_error_prompt();                                             /* 2A37:0066 */
int  out_of_memory_handler();                                         /* 2F39:0001 */

int main(int argc, char *argv[])
{
	int  fd;
	int  check;                  /* [bp-2] */
	int  prefs3;                 /* [bp-4] 3rd word of EOSPREFS.DAT */
	unsigned far *mcb_size;      /* [bp-8] */
	long needed_free;            /* [bp-0C] */
	long needed;                 /* [bp-10] */

	/* 0015: "x = x;" on two globals - generated code, no visible effect */
	_argc  = _argc;
	*_argv = *_argv;
	check   = 1;

	/* 0032: an undocumented "-e" switch turns off the check passed to system_init() */
	if (argc > 1 && strcmp(argv[1], "-e") == 0)
		check = 0;

	/* 0058: preferences written by SETUP.EXE: video mode, sound driver, one more word */
	fd = open("EOSPREFS.DAT", O_RDONLY | O_BINARY);
	if (fd != -1) {
		read(fd, &VideoMode, 2);
		read(fd, &SoundType, 2);
		SoundEnabled = (SoundType != 0);
		read(fd, &prefs3, 2);
		close(fd);
	} else {
		dos_puts(Text_RunSetup);
		exit(0);
	}

	/* 00D9: the DOS memory control block sits one paragraph below the PSP;
	 * the word at offset 3 is the size (in paragraphs) of our own block.   */
	mcb_size = MK_FP(_psp - 1, 3);

	if (VideoMode == 3)
		needed = 565000L;        /* 0x89F08 */
	else
		needed = 570000L;        /* 0x8B290 */

	/* 0102: memory still required = total - what DOS already gave us */
	needed_free = needed - ((long)*mcb_size << 4);

	if (system_init(ExeName, needed_free, check)) {
		printf(Text_NeedRam, needed);
		return 1;
	}

	/* 015A: offscreen page sizes: 64000-byte 320x200 screens plus a small header */
	PageSizes[0] = PageSizes[1] = 64500L;
	PageSizes[2] = PageSizes[3] = 64500L;
	PageSizes[4] = PageSizes[5] = 32000L;
	PageSizes[6] = PageSizes[7] = 64600L;

	/* 01A6: install the library's error callbacks */
	FileErrorHandler = file_error_handler;
	FileErrorPrompt  = file_error_prompt;
	MemHandler  = out_of_memory_handler;

	if (video_init(VideoMode, FontName)) {
		sound_init(SoundType);
		load_file_into("palette.col", Palette, 0x300L);
		set_palette(Palette);

		if (!MousePresent)
			InventoryMode = 1;
		else
			InventoryMode = !prefs3;

		game_main(argc, argv);
		sound_shutdown();
	}
	system_shutdown();
	return 0;
}
