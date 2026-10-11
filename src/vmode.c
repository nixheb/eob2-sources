/*
 * Eye of the Beholder II - START.EXE
 * The video mode (BIOS): module 21A9 (Ghidra 31A9).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y vmode.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

/* dos.h */
struct WORDREGS { unsigned int ax, bx, cx, dx, si, di, cflag, flags; };
struct BYTEREGS { unsigned char al, ah, bl, bh, cl, ch, dl, dh; };
union REGS { struct WORDREGS x; struct BYTEREGS h; };

/* Uninitialised data of the module (DS:F1C4-F1E3, _BSS), in address order */
union REGS OutRegs; /* F1C4 */
union REGS InRegs;  /* F1D4 */

int int86(int n, union REGS far *in, union REGS far *out);

/* Initialised data of the module (DS:64EC) */
int StartMode = 0;                       /* 64EC the mode at start */
int VideoModes[9] = { 4, 9, 13, 19, 19, 14, 9, 7, 2 };                  /* 64EE */
int Var_6500 = 0;                        /* 6500 read as VideoModes[9] (sic) */

/* 31A9:000C [asm] */
int far get_video_mode(void)
{
	InRegs.x.ax = 0xF00;
	int86(0x10, &InRegs, &OutRegs);
	return OutRegs.x.ax & 0xFF;
}

/* 31A9:0033 [asm] - 9: the mode found at start (text) */
void far set_text_mode(register int mode)
{
	InRegs.x.ax = mode < 9 ? VideoModes[mode] : StartMode;
	if (VideoModes[mode] != StartMode)            /* sic: VideoModes[9] for mode 9 */
		int86(0x10, &InRegs, &OutRegs);
}
