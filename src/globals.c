/*
 * Eye of the Beholder II - START.EXE
 * The variables of the Westwood library: a module of data only, DGROUP
 * DS:6282-638D, linked between rnd.c and picture.c (no code: its place in
 * the link shows only by its data and its two relocations).
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y globals.c
 * [asm] = identical to START.EXE byte for byte (tools/check.py).
 * Variables in address order (the order of the original definitions);
 * Var_XXXX: no code reads them.
 */
#include "eob2.h"

extern void far      *DriverTable[];     /* 6522 (mouse.asm) the table given to the video driver */
void far more_prompt(char far *s, int len, int c1, int c2);   /* 31B0:0006 (text.c) */

/* Initialised data of the module (DS:6282) */
void (far *ExitHook)(void) = 0;          /* 6282 called by shutdown() */
int Var_6286 = 1;                        /* 6286 */
char CharWidth = 8;                      /* 6288 of the font */
char LineHeight = 8;                     /* 6289 */
int VideoMode = -1;                      /* 628A */
int MouseStepX = 0;                      /* 628C the mouse moves by steps (0: free) */
int MouseStepY = 0;                      /* 628E */
int MouseOrgX = 0;                       /* 6290 */
int MouseOrgY = 0;                       /* 6292 */
int (far *FileErrorHandler)(int err, char far *name) = 0;   /* 6294 */
int CurDim = -1;                         /* 6298 the window selected */
int (far *MemHandler)(void) = 0;         /* 629A called when memory runs out */
int MemBlocks = 0;                       /* 629E the blocks allocated */
char MorePrompt = 1;                     /* 62A0 the "more" is on */
int (far *FileErrorPrompt)(int err, char far *name) = 0;    /* 62A1 */
int Var_62A5 = 0;                        /* 62A5 the page drawn on */
unsigned PageSegs[16] = { 0 };           /* 62A7 the segment of each page (0: the screen) */
unsigned Var_62C7[16] = { 0 };           /* 62C7 */
unsigned long PageSizes[8] = { 0 };      /* 62E7 */
char Var_6307[96] = { 0 };               /* 6307 */
void (far *MoreFunc)(char far *text, int len, int a, int b) = more_prompt;   /* 6367 */
int TextBg = 12;                         /* 636B */
int TextColor = 15;                      /* 636D */
int GuiStyle = 0;                        /* 636F the current window */
char far *DriverFile = 0;                /* 6371 */
void far *Var_6375 = DriverTable;        /* 6375 */
int GuiX = 0;                            /* 6379 the mouse for the buttons */
int GuiY = 0;                            /* 637B */
int Var_637D = -1;                       /* 637D */
int Var_637F = -1;                       /* 637F */
int Var_6381 = 0;                        /* 6381 */
int Var_6383 = 0;                        /* 6383 */
int MouseKeys = 0;                       /* 6385 the left button: 1, 3, 0x81, 0x83 */
int MouseKeys2 = 0;                      /* 6387 the right button */
char GuiDirty = 0;                       /* 6389 redraw the list */
int SoundType = 0;                       /* 638A */
char Var_638C = 0;                       /* 638C */
