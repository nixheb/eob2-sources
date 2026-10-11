/*
 * Eye of the Beholder II - START.EXE
 * Leaving the game: everything put back as it was: module 215F (Ghidra 315F).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y shutdown.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

extern char           MousePresent;          /* 6654 */
extern char           VideoReady;        /* 638E set by video_init */
extern void (far     *ExitHook)(void);   /* 6282 */
extern int            StartMode;         /* 64EC the video mode found at start */
extern void interrupt (far *OldInt3F)(); /* F1C0 the overlay interrupt before the game */

void sound_shutdown(void);                                       /* 2C9F:024E */
void mouse_remove(void);                                         /* 3269:01F2 */
void files_shutdown(void);                                         /* 2DCE:128D */
void tick_remove(void);                                          /* 33E1:0036 */
void keyboard_remove(void);                                      /* 3295:0A91 */
int  get_video_mode(void);                                       /* 31A9:000C */
void set_text_mode(int mode);                                    /* 31A9:0033 */
int  farheapcheck(void);
void dos_puts(char far *s);                                      /* 3246:0006 */
void setvect(int n, void interrupt (far *isr)());

/* 315F:0000 [asm] */
void far system_shutdown(void)
{
	sound_shutdown();
	if (MousePresent)
		mouse_remove();
	if (VideoReady)
		files_shutdown();
	tick_remove();
	keyboard_remove();
	if (ExitHook)
		ExitHook();
	if (get_video_mode() != StartMode)
		set_text_mode(9);
	if (farheapcheck() == -1)
		dos_puts("\r\nMemory Corrupt!!!\r\n");
	setvect(0x3F, OldInt3F);
}
