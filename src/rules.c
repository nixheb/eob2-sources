/*
 * Eye of the Beholder II - START.EXE
 * The AD&D 2nd edition rules tables: saving throws, ability bonuses, hit
 * points, experience, THAC0, turning undead: module 0B54 (Ghidra 1B54).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y rules.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The base classes (ClassParts): 0 warrior, 1 wizard, 2 priest, 3 rogue,
 * 4 paladin, 5 ranger.
 */
#include "eob2.h"

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Character      Party[6];          /* E608 */
extern signed char    ClassFlags[];      /* 1F9A 1 warrior 2 wizard 4 priest 8 rogue 0x10 paladin 0x20 ranger */
extern signed char    ClassCount[];      /* 1EC8 number of classes of a (multi-)class */
extern signed char    ClassThac0Group[]; /* 1ED7 0 warrior 1 wizard 2 priest 3 rogue */
/* Uninitialised data of the module (DS:EF20-EF29, _BSS), in address order */
char StrengthText[10]; /* EF20 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
int  sprintf(char far *buf, char far *fmt, ...);

/* ------------------------------------------------------------------ */
/* Module 1B54, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:319A...) */
/* hit points per level, by base class: the die up to MaxHdLevel, then a fixed number */
signed char HitDie[6] = {
	10, 4, 8, 6, 10, 10
};                                           /* 319A */
signed char MaxHdLevel[6] = {
	9, 10, 9, 10, 9, 9
};                                      /* 31A0 */
signed char HpAfter[6] = {
	3, 1, 2, 2, 3, 3
};                                         /* 31A6 */
/* the saving throws: [level band][5 types] (paralysis/poison/death,
 * rod/staff/wand, petrification/polymorph, breath weapon, spell)      */
signed char SavePriest[35] = {
	10, 9, 7, 6, 5,
	4, 2, 14, 13, 11,
	10, 9, 8, 6, 13,
	12, 10, 9, 8, 7,
	5, 16, 15, 13, 12,
	11, 10, 8, 15, 14,
	12, 11, 10, 9, 7
};                                      /* 31AC */
signed char SaveRogue[30] = {
	13, 12, 11, 10, 9,
	8, 14, 12, 10, 8,
	6, 7, 12, 11, 10,
	9, 8, 4, 16, 15,
	14, 13, 12, 11, 15,
	13, 11, 9, 7, 5
};                                       /* 31CF */
signed char SaveWarrior[50] = {
	16, 14, 13, 11, 10,
	8, 7, 5, 4, 3,
	18, 16, 15, 13, 12,
	10, 9, 7, 6, 5,
	17, 15, 14, 12, 11,
	9, 8, 6, 5, 4,
	20, 17, 16, 13, 12,
	9, 8, 5, 4, 4,
	19, 17, 16, 14, 13,
	11, 10, 8, 7, 6
};                                    /* 31ED */
signed char SaveWizard[25] = {
	14, 13, 11, 10, 8,
	11, 9, 7, 5, 3,
	13, 11, 9, 7, 5,
	15, 13, 11, 9, 7,
	12, 10, 8, 6, 4
};                                     /* 321F */
/* per THAC0 group / base class: the table, the last level counted, the
 * levels per band, the bytes per type                                 */
signed char far *SaveTables[6] = {                                      /* 3238 */
	SaveWarrior, SaveWizard, SavePriest, SaveRogue, SaveWarrior, SaveWarrior
};
signed char SaveMaxLevel[6] = {
	17, 21, 19, 21, 17, 17
};                                    /* 3250 */
signed char SaveLevelDiv[6] = {
	2, 5, 3, 4, 2, 2
};                                    /* 3256 */
signed char SaveStride[6] = {
	10, 5, 7, 6, 10, 10
};                                      /* 325C */
/* the base classes of each class (-1: none)                            3262 */
signed char ClassParts[15][3] = {
	0, -1, -1,
	5, -1, -1,
	4, -1, -1,
	1, -1, -1,
	2, -1, -1,
	3, -1, -1,
	0, 2, -1,
	0, 3, -1,
	0, 1, -1,
	0, 1, 3,
	3, 1, -1,
	2, 3, -1,
	0, 2, 1,
	5, 2, -1,
	2, 1, -1
};
/* exceptional strength 18/01 18/51 18/76 18/91 18/00 and the bonuses */
signed char ExtBand[5] = {
	1, 51, 76, 91, 100
};                                          /* 328F */
signed char ExtToHit[5] = {
	1, 2, 2, 2, 3
};                                        /* 3294 */
signed char StrToHit[25] = {
	-5, -3, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 4, 4, 5, 6, 7
};                                       /* 3299 */
signed char ExtBandDmg[5] = {
	1, 51, 76, 91, 100
};                                      /* 32B2 */
signed char ExtDmg[5] = {
	3, 3, 4, 5, 6
};                                           /* 32B7 */
signed char StrDmg[25] = {
	-4, -2, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 7, 8, 9, 10, 11, 12, 14
};                                          /* 32BC */
signed char DexToHit[21] = {
	-6, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5
};                                       /* 32D5 */
int DexAc[24] = {
	5, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0,
	0, 0, -1, -2, -3, -4, -4, -5, -5, -5, -6, -6
};                                                  /* 32EA */
signed char ConHp[24] = {
	-3, -2, -2, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 6, 6, 7, 7
};                                           /* 331A */
/* the experience needed for each level (-1: no more)                 */
long XpWizard[14] = {
	0L, 2500L, 5000L, 10000L, 20000L, 40000L, 60000L,
	90000L, 135000L, 250000L, 375000L, 750000L, 1125000L, -1L
};                                              /* 3332 */
long XpWarrior[14] = {
	0L, 2000L, 4000L, 8000L, 16000L, 32000L, 64000L,
	125000L, 250000L, 500000L, 750000L, 1000000L, 1250000L, -1L
};                                             /* 336A */
long XpPriest[14] = {
	0L, 1500L, 3000L, 6000L, 13000L, 27500L, 55000L,
	110000L, 225000L, 450000L, 675000L, 900000L, 1125000L, -1L
};                                              /* 33A2 */
long XpRogue[14] = {
	0L, 1250L, 2500L, 5000L, 10000L, 20000L, 40000L,
	70000L, 110000L, 160000L, 220000L, 440000L, 660000L, -1L
};                                               /* 33DA */
long XpPaladin[14] = {
	0L, 2250L, 4500L, 9000L, 18000L, 36000L, 75000L,
	150000L, 300000L, 600000L, 900000L, 1200000L, 1500000L, -1L
};                                             /* 3412 */
long far *XpTables[6] = {                                               /* 344A */
	XpWarrior, XpWizard, XpPriest, XpRogue, XpPaladin, XpPaladin
};
/* THAC0: 20 - (level - 1) / Thac0Div * Thac0Mul, by THAC0 group      */
signed char Thac0Mul[4] = {
	1, 1, 2, 1
};                                         /* 3462 */
signed char Thac0Div[4] = {
	1, 3, 3, 2
};                                         /* 3466 */
/* turning undead: [undead type][cleric level] the roll needed, 0: turned,
 * -1: destroyed, 99: impossible                                       346A */
signed char TurnTable[10][14] = {
	10, 7, 4, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	13, 10, 7, 4, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1,
	16, 13, 10, 7, 4, 0, 0, -1, -1, -1, -1, -1, -1, -1,
	19, 16, 13, 10, 7, 4, 0, 0, -1, -1, -1, -1, -1, -1,
	20, 19, 16, 13, 10, 7, 4, 0, 0, -1, -1, -1, -1, -1,
	99, 20, 19, 16, 13, 10, 7, 4, 0, 0, 0, -1, -1, 0,
	99, 99, 20, 19, 16, 13, 10, 7, 4, 0, 0, 0, 0, -1,
	99, 99, 99, 20, 19, 16, 13, 10, 7, 4, 4, 0, 0, 0,
	99, 99, 99, 99, 20, 19, 16, 13, 10, 7, 7, 4, 4, 0,
	99, 99, 99, 99, 99, 20, 19, 16, 13, 10, 10, 7, 7, 4
};

/* 1B54:000D [asm] - the saving throw needed */
int far save_table(char group, char level, char type)
{
	signed char far *t;

	t = SaveTables[group];
	if (SaveMaxLevel[group] < level)
		level = SaveMaxLevel[group];
	level /= SaveLevelDiv[group];
	level += SaveStride[group] * type;
	return t[level];
}

/* 1B54:0082 [asm] - the k-th base class of a class, -1 none */
char far class_part(char cls, char k)
{
	return ClassParts[cls][k];
}

/* 1B54:009C [asm] - which part of a class is the base class "base", -1 */
int far class_part_index(int base, char cls)
{
	if (class_part(cls, 0) == base)
		return 0;
	if (class_part(cls, 1) == base)
		return 1;
	if (class_part(cls, 2) == base)
		return 2;
	return -1;
}

/* 1B54:00F0 [asm] */
int far str_to_hit_bonus(int ch)
{
	int r;
	int i;

	r = StrToHit[Party[ch].str - 1];
	if (Party[ch].strExt)
		for (i = 0; i < 5; i++)
			if (Party[ch].strExt >= ExtBand[i])
				r = ExtToHit[i];
	return r;
}

/* 1B54:0149 [asm] */
int far str_damage_bonus(int ch)
{
	int r;
	int i;

	r = StrDmg[Party[ch].str - 1];
	if (Party[ch].strExt)
		for (i = 0; i < 5; i++)
			if (Party[ch].strExt >= ExtBandDmg[i])
				r = ExtDmg[i];
	return r;
}

/* 1B54:01A2 [asm] - the missile to-hit bonus of dexterity */
int far dex_to_hit_bonus(int ch)
{
	return DexToHit[Party[ch].dex - 1];
}

/* 1B54:01BD [asm] */
int far dex_ac_bonus(char dex)
{
	return DexAc[dex - 1];
}

/* 1B54:01CE [asm] - more than +2 only for warriors (fighter, paladin, ranger) */
int far con_hp_bonus(char cls, char con)
{
	con = ConHp[con - 1];
	if (con > 2 && !(ClassFlags[cls] & 0x31))
		con = 2;
	return con;
}

/* 1B54:01FB [asm] - the hit points gained at a new level by the class
 * parts in "parts" (a bit each), shared by the number of classes */
int far roll_level_hp(char ch, char parts)
{
	int con, k;
	Character far *c;
	register int hp;
	int part;

	c = &Party[ch];
	hp = 0;
	con = con_hp_bonus(c->cls, c->con);
	for (k = 0; k < 3; k++)
		if ((1 << k) & parts) {
			part = class_part(c->cls, k);
			hp += c->level[k] > MaxHdLevel[part] ? HpAfter[part] : roll_dice(1, HitDie[part], 0);
			hp += con;
		}
	hp /= ClassCount[c->cls];
	if (hp < 1)
		hp = 1;
	return hp;
}

/* 1B54:02BD [asm] - the hit points of a class part at a level: the most
 * (mode 0) or the least (mode 1) */
int far hp_for_level(char part, char con, char level, char mode)
{
	int die;
	register int dice, hp;

	dice = MaxHdLevel[part] > level ? level : MaxHdLevel[part];
	die = HitDie[part];
	switch (mode) {
	case 0:
		hp = dice * die;
		break;
	case 1:
		hp = dice;
		break;
	}
	if (level > dice) {
		dice = level - dice;
		hp += HpAfter[part] * dice;
	}
	if (mode != 1 || con > 0)
		hp += level * con;
	return hp;
}

/* 1B54:034E [asm] */
int far max_hp(char cls, char con, char l0, char l1, char l2)
{
	int part;
	register int hp;

	con = con_hp_bonus(cls, con);
	if ((part = class_part(cls, 0)) != -1)
		hp = hp_for_level(part, con, l0, 0);
	if ((part = class_part(cls, 1)) != -1)
		hp += hp_for_level(part, con, l1, 0);
	if ((part = class_part(cls, 2)) != -1)
		hp += hp_for_level(part, con, l2, 0);
	return hp / ClassCount[cls];
}

/* 1B54:040A [asm] */
int far min_hp(char cls, char con, char l0, char l1, char l2)
{
	int part;
	register int hp;

	con = con_hp_bonus(cls, con);
	if ((part = class_part(cls, 0)) != -1)
		hp = hp_for_level(part, con, l0, 1);
	if ((part = class_part(cls, 1)) != -1)
		hp += hp_for_level(part, con, l1, 1);
	if ((part = class_part(cls, 2)) != -1)
		hp += hp_for_level(part, con, l2, 1);
	return hp / ClassCount[cls];
}

/* 1B54:04C6 [asm] - "18/76", "18/00", "17" */
char far *far format_strength(char str, char ext)
{
	if (ext > 0) {
		if (ext == 100)
			ext = 0;
		sprintf(StrengthText, "%d/%02d", str, ext);
	} else
		sprintf(StrengthText, "%d", str);
	return StrengthText;
}

/* 1B54:0515 [asm] - the experience for a level of a class part, -1 */
long far xp_for_level(char cls, char k, char level)
{
	long far *t;

	cls = class_part(cls, k);
	if (cls != -1) {
		t = XpTables[cls];
		return *(t + level - 1);
	}
	return -1L;
}

/* 1B54:0569 [asm] - the roll a character needs to hit armour class "ac" */
char far thac0_target(char ch, char ac)
{
	char lvl, g;
	register char r;

	lvl = Party[ch].level[0] - 1;
	g = ClassThac0Group[Party[ch].cls];
	r = 20 - lvl / Thac0Div[g] * Thac0Mul[g];
	r -= ac;
	return r;
}

/* 1B54:05D3 [asm] - the same for a monster: its type gives THAC0 */
int far to_hit_target(int thac0, int ac)
{
	return thac0 - ac;
}

/* 1B54:05DE [asm] - turning undead: the roll needed (TurnTable) */
int far turn_result(int level, int undead)
{
	if (level > 14)
		level = 14;
	return TurnTable[undead][level];
}
