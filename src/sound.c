/*
 * Eye of the Beholder II - START.EXE
 * The sounds of the game world: module 1A30 (Ghidra 2A30).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y sound.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A sound is not played at once: it waits in PendingSound and is played
 * by the next call, which game_loop() makes once a turn
 * (play_sound_here(0)). So one sound at most is heard per turn: the last
 * one asked for. Its volume falls with the distance to the party.
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern int            PartyBlock;        /* 6DD2 */
extern int            SoundEnabled;      /* 65FA */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void play_sound_volume(int sound, int volume);                   /* 2C9F:00BE */
int  distance(int from, int to);                                 /* 2A68:0FD8 */
void far play_sound_at(int sound, int block);

/* ------------------------------------------------------------------ */
/* Module 2A30, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:5E36) */
int PendingSound = 0;                    /* 5E36 */
int PendingVolume = 0;                   /* 5E38 0-240 */

/* 2A30:0000 [asm] - a sound where the party is */
void far play_sound_here(int sound)
{
	play_sound_at(sound, PartyBlock);
}

/* 2A30:0014 [asm] - play the waiting sound, then make this one wait;
 * block 0: full volume, farther than 15 blocks: not heard */
void far play_sound_at(int sound, register int block)
{
	int dist;

	if (SoundEnabled) {
		if (PendingSound > 0)
			play_sound_volume(PendingSound, PendingVolume);
		if (block && (dist = distance(PartyBlock, block)) > 15)
			PendingSound = 0;
		else
			PendingSound = sound;
		PendingVolume = (15 - (block ? dist : 0)) << 4;
	}
}
