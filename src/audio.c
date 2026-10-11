/*
 * Eye of the Beholder II - START.EXE
 * Sound effects: AdLib (FM instruments) or digitized sound:
 * module 1C9F (Ghidra 2C9F).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y audio.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * SoundType (from EOSPREFS.DAT, set by SETUP): 0 none, 1 AdLib,
 * 2 and 3 digitized (3 with a second mode, Var_6C45).
 * NAME.ADL / NAME.SND: 120 bytes mapping the game's sounds to the
 * driver's (0xFF none), then 15 120 bytes for the driver.
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern int            SoundEnabled;      /* 65FA */
extern int            SoundType;         /* 638A */
extern char           Var_638C;          /* 638C */
extern char           Var_6C45;          /* 6C45 */
extern char           Scratch[];         /* F16E */
/* Uninitialised data: in bssef54.c (DS:EF54-EFCF) */
extern int SavedAttack;             /* EF54 register 3 */
extern int SavedLevel;              /* EF56 AdLib register 1 of the instrument, before the volume */
extern unsigned char SoundMap[120]; /* EF58 the game's sound -> the driver's */
/* in the sound driver (assembler, segment 1000) */
extern char far       SndBusy;           /* 1000:43B0 */
extern char far       SndReady;          /* 1000:43B9 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  sprintf(char far *buf, char far *fmt, ...);
int  open_file(char far *name, int mode);                        /* 2DDE:0007 */
void read_file(int fd, void far *buf, long n);                   /* 2E12:000E */
void close_file(int fd);                                         /* 2E0E:0006 */
void far *sound_buffer(void);                                    /* 1000:025A */
void sound_reset(void);                                          /* 1000:02AA */
void adlib_set(int ins, int reg, int value);                     /* 1000:0260 */
int  adlib_get(int ins, int reg);                                /* 1000:027A */
void adlib_play(int ins);                                        /* 1000:4886 */
void far *sound_data(int snd);                                   /* 1000:0293 */
void digi_play(void far *data);                                  /* 2CC7:0412 */
void digi_mode(void);                                            /* 2CC7:0452 */
void sound_timer_install(void);                                  /* 1000:47F1 */
void sound_timer_remove(void);                                   /* 1000:4844 */
void digi_init(void);                                            /* 2CC7:0372 */
void digi_mode_off(void);                                        /* 2CC7:0465 */
void digi_shutdown(void);                                        /* 2CC7:03D4 */
void far play_sound_volume(int s, int volume);

/* ------------------------------------------------------------------ */
/* Module 2C9F, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:61B4) */
int LastInstrument = 0xFF;               /* 61B4 the AdLib instrument whose volume was changed */

/* 2C9F:0009 [asm] - load the sounds NAME.ADL or NAME.SND */
void far load_sounds(char far *name)
{
	register int fd;

	SndReady = 0;
	switch (SoundType) {
	case 2:
	case 3:
		sprintf(Scratch, "%s.SND", name);
		break;
	case 1:
		sprintf(Scratch, "%s.ADL", name);
		break;
	default:
		return;
	}
	fd = open_file(Scratch, 1);
	read_file(fd, SoundMap, 120L);
	read_file(fd, sound_buffer(), 0x3B10L);
	close_file(fd);
	sound_reset();
	SndReady = 1;
}

/* 2C9F:00AA [asm] */
void far play_sound(int s)
{
	play_sound_volume(s, -1);
}

/* 2C9F:00BE [asm] - volume -1: the instrument as it is, else 0-240 */
void far play_sound_volume(register int s, int volume)
{
	unsigned v;
	register int snd;

	if (!SoundEnabled)
		return;
	if (!SoundType)
		return;
	if (s < 0)
		return;
	if (s >= 120)
		return;
	snd = SoundMap[s];
	if (snd == 0xFF)
		return;
	if (SoundType == 1) {
		while (SndBusy)
			;
		if (LastInstrument != 0xFF) {
			adlib_set(LastInstrument, 1, SavedLevel);
			adlib_set(LastInstrument, 3, SavedAttack);
			LastInstrument = 0xFF;
		}
		if (volume != -1 && snd && snd != 0xFF) {
			LastInstrument = snd;
			SavedLevel = adlib_get(snd, 1);
			SavedAttack = adlib_get(snd, 3);
			v = ((SavedAttack << 2) ^ 0xFF) * volume;
			adlib_set(snd, 3, (v >> 10) ^ 0x3F);
			adlib_set(snd, 1, v >> 11);
		}
		adlib_play(snd);
	} else {
		if (Var_638C == 1) {
			if (snd < 0 && Var_6C45) {
				Var_6C45 = 0;
				digi_mode();
			}
			if (snd > 0 && Var_6C45 == 0) {
				Var_6C45 = 1;
				digi_mode();
			}
		}
		snd &= 0x7F;
		digi_play(sound_data(snd));
	}
}

/* 2C9F:020B [asm] */
void far sound_init(int type)
{
	switch (SoundType = type) {
	case 1:
		sound_timer_install();
		break;
	case 2:
		digi_init();
		Var_638C = 0;
		break;
	case 3:
		digi_init();
		Var_6C45 = 1;
		Var_638C = 1;
		digi_mode();
	}
}

/* 2C9F:024E [asm] */
void far sound_shutdown(void)
{
	switch (SoundType) {
	case 1:
		sound_timer_remove();
		break;
	case 2:
		digi_shutdown();
		break;
	case 3:
		digi_mode_off();
		Var_6C45 = 0;
		digi_shutdown();
	}
	SoundType = 0;
}
