/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of chargen.c (DS:6C6C-6C73, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after chargen.c;
 * the empty code segment joins CHARGEN_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCCHARGEN_TEXT bss6c6c.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:6C6C-6C73, _BSS), in address order */
char AnimFrame = 0;         /* 6C6C */
char AnimDrawn = 0;         /* 6C6D */
unsigned long AnimNext = 0; /* 6C6E */
int CgFemale = 0;           /* 6C72 the character being made is a woman */
