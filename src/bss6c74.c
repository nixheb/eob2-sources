/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of camp.c (DS:6C74-6D43, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after camp.c;
 * the empty code segment joins CAMP_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCCAMP_TEXT bss6c74.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:6C74-6D43, _BSS), in address order */
int CampPortraits = 0;               /* 6C74 portraits were greyed (redraw them) */
char CampSpellLevel[32] = { 0 };     /* 6C76 the level of each spell */
char Var_6C96[4] = { 0 };            /* 6C96 never used */
char CampLevelSpells[8][11] = { 0 }; /* 6C9A per spell level: the count, then the spells (from 1) */
char CampLevelCount[8] = { 0 };      /* 6CF2 the spells memorized per spell level */
char CampSpellCount[36][2] = { 0 };  /* 6CFA per spell: memorized, ready */
int SpellsToRegain = 0;              /* 6D42 spells were chosen: rest to get them */
