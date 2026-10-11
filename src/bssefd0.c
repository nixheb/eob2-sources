/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of shape.c (DS:EFD0-EFD9, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after shape.c;
 * the empty code segment joins SHAPE_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCSHAPE_TEXT bssefd0.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:EFD0-EFD9, _BSS), in address order */
int ClipY2 = 0;  /* EFD0 */
int ClipY = 0;   /* EFD2 */
int ClipX8 = 0;  /* EFD4 in bytes */
int ClipX = 0;   /* EFD6 in pixels */
int ClipX82 = 0; /* EFD8 in bytes */
