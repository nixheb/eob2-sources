/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of sysinit.c (DS:F09C-F1C3, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after sysinit.c;
 * the empty code segment joins SYSINIT_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCSYSINIT_TEXT bssf09c.c
 */
#include "eob2.h"

typedef unsigned char byte;

/* Uninitialised data of the module (DS:F09C-F1C3, _BSS), in address order */
char StartDir[96] = { 0 };            /* F09C the current directory at start */
int DiskError = 0;                    /* F0FC the last critical error */
int MouseButton = 0;                  /* F0FE the button of the last key: 1 left, 2 right */
int WinH = 0;                         /* F100 */
int WinW = 0;                         /* F102 */
int WinX = 0;                         /* F104 */
int WinY = 0;                         /* F106 */
char ExePath[96] = { 0 };             /* F108 the full name of the program */
byte far *Palette = 0;                /* F168 */
int WaitTimer = 0;                    /* F16C passed to wait_retrace() (3404:0019) */
char Scratch[82] = { 0 };             /* F16E */
void interrupt (far *OldInt3F)() = 0; /* F1C0 the overlay interrupt before the game */
