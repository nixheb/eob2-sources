/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of font.c (DS:F090-F09B, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after font.c;
 * the empty code segment joins FONT_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCFONT_TEXT bssf090.c
 */
#include "eob2.h"

typedef unsigned char byte;

/* Uninitialised data of the module (DS:F090-F09B, _BSS), in address order */
byte far *CurFont = 0; /* F090 */
int CursorX = 0;       /* F094 */
int CursorY = 0;       /* F096 */
byte far *Font8 = 0;   /* F098 */
