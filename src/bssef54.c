/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of audio.c (DS:EF54-EFCF, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after audio.c;
 * the empty code segment joins AUDIO_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCAUDIO_TEXT bssef54.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:EF54-EFCF, _BSS), in address order */
int SavedAttack = 0;                 /* EF54 register 3 */
int SavedLevel = 0;                  /* EF56 AdLib register 1 of the instrument, before the volume */
unsigned char SoundMap[120] = { 0 }; /* EF58 the game's sound -> the driver's */
