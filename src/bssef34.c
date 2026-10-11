/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of spells.c (DS:EF34-EF4D, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after spells.c;
 * the empty code segment joins SPELLS_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCSPELLS_TEXT bssef34.c
 */
#include "eob2.h"

/* Uninitialised data of the module (DS:EF34-EF4D, _BSS), in address order */
int SparkI = 0;      /* EF34 */
int SparkColour = 0; /* EF36 */
int SparkLive = 0;   /* EF38 */
int SparkCX = 0;     /* EF3A */
int SparkCY = 0;     /* EF3C */
int SparkX = 0;      /* EF3E */
int SparkY = 0;      /* EF40 */
int SparkPage = 0;   /* EF42 */
int SparkBack = 0;   /* EF44 */
int Var_EF46 = 0;    /* EF46 */
int CharTries = 0;   /* EF48 */
int Var_EF4A = 0;    /* EF4A */
int BackColour = 0;  /* EF4C */
