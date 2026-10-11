/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of mainloop.c (DS:EF2A-EF2F, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after mainloop.c;
 * the empty code segment joins MAINLOOP_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCMAINLOOP_TEXT bssef2a.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:EF2A-EF2F, _BSS), in address order */
unsigned long NextAmbient = 0; /* EF2A the next ambient sound */
int PrevPartyBlock = 0;        /* EF2E */
