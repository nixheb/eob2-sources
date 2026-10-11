/*
 * Eye of the Beholder II - START.EXE
 * The overlay interrupt of the game: START.EXE (where the overlays are) is
 * opened again before each load, so that the disks can be changed;
 * module 218E (Ghidra 318E, from 318E:0003).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -r- ovrint.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A C module (not assembler, as thought before): the relocations are in the
 * reverse order, as BCC writes them, and the prologue is the one of an
 * interrupt function. Compiled with -r- (no register variables: the three
 * locals stay on the stack); the retry loop is written with gotos (a while
 * loop moves its test). Its end is written with __emit__: the registers of
 * the caller are put back, the overlay manager's own handler is called
 * (system_init made it end with a retf), then the file is closed again.
 */
#include "eob2.h"

/* Initialised data of the module (DS:64EA): only the string "\\" below */

extern void interrupt (far *OldInt3F)(); /* F1C0 the overlay interrupt before the game */
extern char           ExePath[];         /* F108 the full name of the program */
extern char          *ExeName;           /* 1CC8 "START.EXE" */
extern int (far      *FileErrorHandler)(int err, char far *name);   /* 6294 */
extern int far        _OvrDosHandle;         /* 3A18:0128 the overlay manager's file */

int  close(int fd);                                              /* 1000:5A5F */
int  open(char far *path, int access, ...);                      /* 1000:71EA */
int  getdisk(void);                                              /* 1000:6B35 */
int  setdisk(int drive);                                         /* 1000:6B3B */
int  file_exists(char far *name);                                /* 2DCE:11CF */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);                                           /* 1000:5A8D */
char far *getcwd(char far *buf, int n);                          /* 1000:5E94 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
char far *strcat(char far *d, char far *s);                      /* 1000:74C4 */
void __emit__();                                                 /* Borland dos.h: bytes put in the code */

/* 318E:0003 INT 3Fh: START.EXE opened again (else, from the current
 * directory, the disk asked for by FileErrorHandler until it is there;
 * the game quits if it cannot be); then the old handler is called, and
 * the file closed */
void interrupt ovr_int3f(void)
{
	int ok, fd, drive;

	if (_OvrDosHandle != -1)
		close(_OvrDosHandle);
	if ((fd = open(ExePath, 0x8001)) == -1) {
		drive = getdisk();
		goto test;
	retry:
		if (!FileErrorHandler(2, ExeName))
			goto asked;
	test:
		ok = file_exists(ExeName);
		if (ok == 0)
			goto retry;
	asked:
		if (ok == 0) {
			system_shutdown();
			exit(5);
		}
		if (getcwd(ExePath, 80) == 0) {
			FileErrorHandler(12, ExeName);
			system_shutdown();
			exit(5);
		}
		if (ExePath[strlen(ExePath) - 1] != '\\')
			strcat(ExePath, "\\");
		strcat(ExePath, ExeName);
		if ((_OvrDosHandle = open(ExePath, 0x8001)) == -1) {
			FileErrorHandler(2, ExeName);
			system_shutdown();
			exit(5);
		}
		setdisk(drive);
	} else
		_OvrDosHandle = fd;
	__emit__(0x89, 0xEC);                                    /* mov sp,bp */
	__emit__(0x5D, 0x5F, 0x5E, 0x1F, 0x07, 0x5A, 0x59, 0x5B, 0x58);   /* pop bp, di, si, ds, es, dx, cx, bx, ax */
	__emit__(0x55);                                          /* push bp */
	((void (far *)(void))OldInt3F)();
	__emit__(0x5D);                                          /* pop bp */
	__emit__(0x50, 0x53, 0x51, 0x52, 0x06, 0x1E, 0x56, 0x57, 0x55);   /* push ax, bx, cx, dx, es, ds, si, di, bp */
	__emit__(0xFB);                                          /* sti */
	close(_OvrDosHandle);
	_OvrDosHandle = -1;
	__emit__(0x5D, 0x5F, 0x5E, 0x1F, 0x07, 0x5A, 0x59, 0x5B, 0x58);   /* pop bp ... ax */
	__emit__(0xCF);                                          /* iret (the epilogue of the compiler is never reached) */
}
