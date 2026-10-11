/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of files.c (DS:EFDA-F04B, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after files.c;
 * the empty code segment joins FILES_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCFILES_TEXT bssefda.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:EFDA-F04B, _BSS), in address order */
long FileSize[10] = { 0 };        /* EFDA */
char CurDrive = 0;                /* F002 the drive at start (BSS) */
char Pad_F003 = 0;                /* F003 the alignment of the _BSS */
long FileStart[10] = { 0 };       /* F004 where the file starts in its library */
unsigned FileHandles[10] = { 0 }; /* F02C */
int StartDrive = 0;               /* F040 the drive at start */
char LastDrive = 0;               /* F042 */
char Pad_F043 = 0;                /* F043 the alignment of the _BSS */
long LibSize = 0;                 /* F044 of the file found by lib_find */
long LibStart = 0;                /* F048 */
