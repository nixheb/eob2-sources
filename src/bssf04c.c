/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of guiproc.c (DS:F04C-F051, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after guiproc.c;
 * the empty code segment joins GUIPROC_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCGUIPROC_TEXT bssf04c.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:F04C-F051, _BSS), in address order */
int (far *ButtonHook)(Button far *b) = 0; /* F04C */
int Var_F050 = 0;                         /* F050 never used */
