/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of text.c (DS:F1E4-F255, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after text.c;
 * the empty code segment joins TEXT_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCTEXT_TEXT bssf1e4.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:F1E4-F255, _BSS), in address order */
char Pend[2] = { 0 };   /* F1E4 the next two characters */
char PendSaved = 0;     /* F1E6 the one after the %s or %d */
char Pad_F1E7 = 0;      /* F1E7 the alignment of the _BSS */
char far *SubText = 0;  /* F1E8 the text of a %s or %d */
char Line[100] = { 0 }; /* F1EC the line being made */
char far *FmtText = 0;  /* F250 */
int LineLen = 0;        /* F254 */
