/*
 * Eye of the Beholder II - START.EXE
 * Starting the system: module 216D (Ghidra 316D).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y sysinit.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

extern int            StartMode;         /* 64EC the video mode found at start */
/* Uninitialised data: in bssf09c.c (DS:F09C-F1C3) */
extern char StartDir[96];                /* F09C the current directory at start */
extern int DiskError;                    /* F0FC the last critical error */
extern int MouseButton;                  /* F0FE the button of the last key: 1 left, 2 right */
extern int WinH;                         /* F100 */
extern int WinW;                         /* F102 */
extern int WinX;                         /* F104 */
extern int WinY;                         /* F106 */
extern char ExePath[96];                 /* F108 the full name of the program */
extern byte far *Palette;                /* F168 */
extern int WaitTimer;                    /* F16C passed to wait_retrace() (3404:0019) */
extern char Scratch[82];                 /* F16E */
extern void interrupt (far *OldInt3F)(); /* F1C0 the overlay interrupt before the game */
extern int far        _OvrDosHandle;         /* 3A18:0128 the overlay manager's file */

void interrupt ovr_int3f();              /* 318E:0003 (assembler) */
int  get_video_mode(void);                                       /* 31A9:000C */
void install_critical_error(void);                               /* 2F42:0016 */
void count_drives(void);                                         /* 3263:0002 (assembler) */
int  file_exists(char far *name);                                /* 2DCE:11CF */
void dos_puts(char far *s);                                      /* 3246:0006 (assembler) */
long mem_free_total(void);                                       /* 2D0E:0249 */
void keyboard_install(void);                                     /* 3295:07B2 (assembler) */
void tick_install(void);                                         /* 33E1:000E */
void interrupt (far *getvect(int n))();                          /* 1000:6D95 */
void setvect(int n, void interrupt (far *isr)());                /* 1000:6DA4 */
int  getdisk(void);                                              /* 1000:6B35 */
int  setdisk(int drive);                                         /* 1000:6B3B */
char far *getcwd(char far *buf, int n);                          /* 1000:5E94 */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
char far *strcat(char far *d, char far *s);                      /* 1000:74C4 */
int  close(int fd);                                              /* 1000:5A5F */
int  _OvrInitExt(unsigned long start, unsigned long length);     /* 2000:C256 (overlay manager) */
int  _OvrInitEms(unsigned handle, unsigned first, unsigned pages);   /* 2000:C08C (overlay manager) */
void far *memcpy(void far *d, void far *s, unsigned n);          /* 1000:70E3 */

/* Initialised data of the module (DS:648A...) */
unsigned char Int3FPatch[9] = { 0x55, 0x8B, 0xEC, 0x83, 0xC5, 0x06, 0x75, 0x06, 0x90 };   /* 648A */

/* 316D:0002 [asm] - exe: the name of the program (0: no overlays);
 * ext: let the overlays use the extended and the EMS memory.
 * 0 if all is well */
int far system_init(char far *exe, long need, int ext)
{
	int disk;

	StartMode = get_video_mode();
	if (exe) {
		OldInt3F = getvect(0x3F);
		setvect(0x3F, ovr_int3f);
		memcpy(OldInt3F, Int3FPatch, 9);
		((unsigned char far *)OldInt3F)[0x65] = 0xCB;   /* retf */
	}
	disk = getdisk();
	install_critical_error();
	count_drives();
	if (exe) {
		if (file_exists(exe)) {
			getcwd(ExePath, 80);
			strcpy(StartDir, ExePath);
			if (ExePath[strlen(ExePath) - 1] != '\\')
				strcat(ExePath, "\\");
			strcat(ExePath, exe);
			close(_OvrDosHandle);
			_OvrDosHandle = -1;
			setdisk(disk);
			if (ext) {
				_OvrInitExt(0L, 0L);
				_OvrInitEms(0, 0, 0);
			}
		} else {
			dos_puts("\r\nProgram must be run from the source directory.");
			setvect(0x3F, OldInt3F);
			return 1;
		}
	}
	if (mem_free_total() < need) {
		dos_puts("\r\nNot enough memory to run program.");
		setvect(0x3F, OldInt3F);
		return 1;
	}
	keyboard_install();
	tick_install();
	return 0;
}
