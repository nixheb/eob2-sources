/*
 * Eye of the Beholder II - START.EXE
 * The data-only module: the tables of the game (windows, menus, names of
 * the races, classes, alignments and spells, the wall types, the view, the
 * inventory slots...) in DGROUP DS:1CFC-2635, its strings DS:2636-2FD5,
 * and far data 37D8-3A17 (the files, the item names, the camp texts, the
 * spells); the game's uninitialised variables (DS:6D7C-EF1D, _BSS) are in
 * bss6d7c.c (tools/bssdef.py). It is linked right after main.
 *
 * Made by tools/datamod.py from START.EXE; compiled with
 *     bcc -c -ml -G -O -Z -y data.c
 * Variables in address order (the order of the original definitions);
 * Var_XXXX: no code reads them.
 */
#include "eob2.h"

typedef unsigned char byte;

struct Dim { int x, y, w, h, a, b, c, d; };

/* A file the game knows (files.c) */
typedef struct FileEntry {
	char far     *name;
	long          size;
	byte far     *data;
	int           link;
	long          pos;
	byte          flags;
} FileEntry;

/* the spell effects (spelleffects.c) */
void far spell_acid_arrow(void);
void far spell_aid(void);
void far spell_armor(void);
void far spell_burning_hands(void);
void far spell_cause_critical_wounds(void);
void far spell_cause_light_wounds(void);
void far spell_cause_serious_wounds(void);
void far spell_cone_of_cold(void);
void far spell_create_food(void);
void far spell_cure_critical_wounds(void);
void far spell_cure_light_wounds(void);
void far spell_cure_serious_wounds(void);
void far spell_detect_magic(void);
void far spell_disintegrate(void);
void far spell_dispel_magic(void);
void far spell_fear(void);
void far spell_finger_of_death(void);
void far spell_fireball(void);
void far spell_flame_blade(void);
void far spell_flame_strike(void);
void far spell_flesh_to_stone(void);
void far spell_harm(void);
void far spell_heal(void);
void far spell_hold_monster(void);
void far spell_hold_person(void);
void far spell_ice_storm(void);
void far spell_improved_identify(void);
void far spell_lay_on_hands(void);
void far spell_lightning_bolt(void);
void far spell_magic_missile(void);
void far spell_neutralize_poison(void);
void far spell_power_word_stun(void);
void far spell_raise_dead(void);
void far spell_remove_curse(void);
void far spell_remove_paralysis(void);
void far spell_shocking_grasp(void);
void far spell_slow_poison(void);
void far spell_stone_to_flesh(void);
void far spell_true_seeing(void);
void far spell_turn_undead(void);
void far spell_vampiric_touch(void);
void far spell_wall_of_force(void);
int far end_aid();
int far end_detect_magic();
int far end_slow_poison();
int far end_touch_spell();
int far end_true_seeing();
int far end_vampiric_touch();
int far hit_acid_arrow();
int far hit_breath();
int far hit_cause_serious();
int far hit_death_spell();
int far hit_disintegrate_party();
int far hit_eighteen();
int far hit_fireball();
int far hit_flame_strike();
int far hit_hold_monster();
int far hit_hold_person();
int far hit_ice_storm();
int far hit_lightning_bolt();
int far hit_magic_missile();
int far hit_petrify();

FileEntry far FileData[1] = { { "" } };   /* 37D8:0000 the files: filled at run time; "" ends the table */

/* Initialised data of the module (DS:1CFC) */
struct Dim Dims[19] = {   /* 1CFC the text windows (x and w in bytes of 8 pixels, y and h in lines) */
	{ 0, 0, 40, 200, 15, 12, 0, 0 },
	{ 8, 72, 24, 56, 14, 12, 0, 0 },
	{ 19, 64, 20, 128, 6, 12, 0, 0 },
	{ 29, 120, 8, 64, 15, 13, 0, 0 },
	{ 2, 24, 20, 120, 15, 2, 3, 0 },
	{ 0, 0, 22, 120, 15, 13, 0, 0 },
	{ 10, 108, 21, 40, 15, 0, 0, 0 },
	{ 1, 180, 34, 18, 15, 12, 0, 0 },
	{ 2, 24, 20, 0, 15, 2, 3, 0 },
	{ 1, 125, 38, 64, 15, 0, 3, 0 },
	{ 0, 0, 22, 144, 15, 2, 0, 0 },
	{ 1, 20, 20, 56, 15, 2, 0, 0 },
	{ 1, 4, 20, 156, 15, 2, 0, 0 },
	{ 1, 25, 38, 100, 15, 2, 0, 0 },
	{ 1, 20, 20, 88, 15, 2, 0, 0 },
	{ 2, 6, 35, 120, 15, 2, 0, 0 },
	{ 9, 20, 22, 56, 15, 2, 0, 0 },
	{ 1, 150, 38, 49, 15, 0, 0, 0 },
	{ 1, 8, 38, 128, 12, 15, 0, 0 }
};
Menu Menus[8] = {   /* 1E2C the menus (menu.c) */
	{ 1, 16, 38, 20, 0, 15, 6, 0 },
	{ 0, 16, 16, 12, 0, 15, 6, 0 },
	{ 0, 16, 23, 0, 0, 15, 6, 0 },
	{ 0, 16, 16, 0, 0, 15, 6, 0 },
	{ 0, 16, 7, 4, 0, 15, 6, 0 },
	{ 0, 0, 17, 5, 0, 15, 6, 0 },
	{ 0, 0, 21, 5, 0, 15, 6, 0 },
	{ 0, 0, 17, 8, 0, 15, 6, 0 }
};
int Var_1EAC[12] = {   /* 1EAC unused */
	0, 2, 5, 4, 1, 255, 3, 255,
	7, 255, 6, 255
};
int far *Var_1EC4 = Var_1EAC;   /* 1EC4 unused */
signed char ClassCount[15] = {   /* 1EC8 per class: how many classes */
	1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 2, 2, 3, 2, 2
};
signed char ClassThac0Group[15] = {   /* 1ED7 per class: 0 warrior, 1 mage, 2 cleric, 3 thief (the THAC0 group) */
	0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 3, 2, 0, 0, 2
};
char far *StatLabels[6] = {   /* 1EE6 */
	"STR",
	"INT",
	"WIS",
	"DEX",
	"CON",
	"CHA"
};
char far *StatNames[6] = {   /* 1EFE */
	"STRENGTH",
	"INTELLIGENCE",
	"WISDOM",
	"DEXTERITY",
	"CONSTITUTION",
	"CHARISMA"
};
char far *RaceNames[12] = {   /* 1F16 race and sex */
	"HUMAN MALE",
	"HUMAN FEMALE",
	"ELF MALE",
	"ELF FEMALE",
	"HALF-ELF MALE",
	"HALF-ELF FEMALE",
	"DWARF MALE",
	"DWARF FEMALE",
	"GNOME MALE",
	"GNOME FEMALE",
	"HALFLING MALE",
	"HALFLING FEMALE"
};
char far *ClassNames[15] = {   /* 1F46 */
	"FIGHTER",
	"RANGER",
	"PALADIN",
	"MAGE",
	"CLERIC",
	"THIEF",
	"FIGHTER/CLERIC",
	"FIGHTER/THIEF",
	"FIGHTER/MAGE",
	"FIGHTER/MAGE/THIEF",
	"THIEF/MAGE",
	"CLERIC/THIEF",
	"FIGHTER/CLERIC/MAGE",
	"RANGER/CLERIC",
	"CLERIC/MAGE"
};
char far *ClassShortNames[6] = {   /* 1F82 */
	"FIGHTER",
	"MAGE",
	"CLERIC",
	"THIEF",
	"PALADIN",
	"RANGER"
};
signed char ClassFlags[15] = {   /* 1F9A per class: its classes (1 fighter, 2 mage, 4 cleric, 8 thief, 16 paladin, 32 ranger) */
	1, 32, 16, 2, 4, 8, 5, 9, 3, 11, 10, 12, 7, 5, 6
};
char far *AlignNames[9] = {   /* 1FA9 */
	"LAWFUL GOOD",
	"NEUTRAL GOOD",
	"CHAOTIC GOOD",
	"LAWFUL NEUTRAL",
	"TRUE NEUTRAL",
	"CHAOTIC NEUTRAL",
	"LAWFUL EVIL",
	"NEUTRAL EVIL",
	"CHAOTIC EVIL"
};
char far *LevelOrdinals[5] = {   /* 1FCD unused */
	"First",
	"Second",
	"Third",
	"Fourth",
	"Fifth"
};
char far *GuiMageBook[33] = {   /* 1FE1 the names in the spell book, per spell */
	"",
	"Armor",
	"Burning Hands",
	"Detect Magic",
	"Magic Missile",
	"Shield",
	"Shocking Grasp",
	"Blur",
	"Detect Invisible",
	"Imp Identify",
	"Invisibility",
	"M's Acid Arrow",
	"Dispel Magic",
	"Fireball",
	"Haste",
	"Hold Person",
	"Invisibility 10'",
	"Lightning Bolt",
	"Vampiric Touch",
	"Fear",
	"Ice Storm",
	"Imp Invisibility",
	"Remove Curse",
	"Cone of Cold",
	"Hold Monster",
	"Wall of Force",
	"Disintegrate",
	"Flesh to Stone",
	"Stone to Flesh",
	"True Seeing",
	"Finger of Death",
	"Power Word Stun",
	"Bigby's Fist"
};
char far *GuiClericBook[30] = {   /* 2065 */
	"",
	"Bless",
	"Cause Light Wnds",
	"Cure Light Wnds",
	"Detect Magic",
	"Protect-Evil",
	"Aid",
	"Flame Blade",
	"Hold Person",
	"Slow Poison",
	"Create Food",
	"Dispel Magic",
	"Magical Vestment",
	"Prayer",
	"Remove Paralysis",
	"Cause Serious",
	"Cure Serious",
	"Neutral-Poison",
	"Protect-Evil 10'",
	"Cause Critical",
	"Cure Critical",
	"Flame Strike",
	"Raise Dead",
	"Slay Living",
	"True Seeing",
	"Harm",
	"Heal",
	"Ressurection",
	"Lay on Hands",
	"Turn undead"
};
char far *Var_20DD[3] = {   /* 20DD unused: texts of the character generation */
	"CHARACTER GENERATION\r\rUSE THE ARROW KEYS TO HIGHLIGHT YOUR SELECTION",
	"",
	"Do you really wish to delete this character?\r"
};
signed char BlockStep[4] = {   /* 20E9 per direction: the step in LevelMap */
	-32, 1, 32, -1
};
int Var_20ED[29] = {   /* 20ED unused */
	184, 256, 184, 256, 184, 256, 0, 0,
	56, 56, 112, 112, 247, 319, 247, 319,
	48, 48, 104, 104, 0, 32, 32, 8,
	8, 24, 32, 16, 16
};
int SlotX[27] = {   /* 2127 the inventory slots on the screen */
	230, 278, 181, 199, 181, 199, 181, 199,
	181, 199, 181, 199, 181, 199, 181, 199,
	225, 224, 225, 298, 287, 277, 300, 300,
	300, 228, 240
};
unsigned char SlotY[27] = {   /* 215D */
	116, 116, 40, 40, 58, 58, 76, 76, 94, 94, 112, 112, 130, 130, 148, 148,
	56, 76, 96, 55, 75, 137, 94, 112, 130, 136, 136
};
signed char WallSet[80] = {   /* 2178 per wall type: the wall set drawn ([46] the illusory wall: 0 under True Seeing) */
	0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
signed char WallDecoration[80] = {   /* 21C8 per wall type: its decoration + 1, -1 a door */
	0, 0, 0, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, -1, -1, -1,
	-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
signed char WallClickType[80] = {   /* 2218 per wall type: what a click does */
	0, 0, 0, 1, 1, 1, 1, 1, 6, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
unsigned char WallTypeFlags[80] = {   /* 2268 per wall type: its flags (MOTEUR.md section 10) */
	7, 0, 64, 168, 136, 136, 136, 159, 168, 136, 136, 136, 159, 168, 136, 136,
	136, 159, 168, 136, 136, 136, 159, 3, 3, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
signed char FaceFront[4] = {   /* 22B8 */
	2, 3, 0, 1
};
signed char FaceLeft[4] = {   /* 22BC */
	1, 2, 3, 0
};
signed char FaceRight[4] = {   /* 22C0 */
	3, 0, 1, 2
};
signed char ViewSquares[72] = {   /* 22C4 per direction: the 18 squares of the view (offsets in LevelMap) */
	-99, -98, -97, -96, -95, -94, -93, -66, -65, -64, -63, -62, -33, -32, -31, -1,
	0, 1, -93, -61, -29, 3, 35, 67, 99, -62, -30, 2, 34, 66, -31, 1,
	33, -32, 0, 32, 99, 98, 97, 96, 95, 94, 93, 66, 65, 64, 63, 62,
	33, 32, 31, 1, 0, -1, 93, 61, 29, -3, -35, -67, -99, 62, 30, -2,
	-34, -66, 31, -1, -33, 32, 0, -32
};
int InWallX[18] = {   /* 230C */
	-56, -8, 40, 88, 136, 184, 232, -72,
	8, 88, 168, 248, -40, 88, 216, -88,
	88, 264
};
signed char SquareRow[18] = {   /* 2330 */
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3,
	3, 3
};
signed char FrontPos[4][4] = {   /* 2342 per direction: the 4 places of a square, front first */
	{ 0, 1, 2, 3 }, { 1, 3, 0, 2 }, { 3, 2, 1, 0 }, { 2, 0, 3, 1 }
};
signed char PlaceSeen[4][4] = {   /* 2352 */
	{ 0, 1, 2, 3 }, { 2, 0, 3, 1 }, { 3, 2, 1, 0 }, { 1, 3, 0, 2 }
};
int SlotMask[27] = {   /* 2362 per inventory slot: the item kinds it takes */
	8, 8, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	1, 2, 4, 32, 64, 16, -1, 128,
	128, 256, 256
};
unsigned char Var_2398[16] = {   /* 2398 unused */
	0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
};
unsigned char Remap_23A8[16] = {   /* 23A8 colour remaps */
	0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};
unsigned char Remap_23B8[16] = {   /* 23B8 */
	0, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13
};
unsigned char Palette_23C8[4] = {   /* 23C8 */
	0, 255, 255, 255
};
unsigned char BoxColours[16] = {   /* 23CC */
	15, 176, 178, 180, 182, 184, 186, 188, 12, 188, 186, 184, 182, 180, 178, 176
};
unsigned char Var_23DC[16] = {   /* 23DC unused */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
int FloorObjectX[138] = {   /* 23EC */
	-111, -63, -95, -63, -139, -59, -117, -59,
	-120, -61, -76, -63, -60, -63, -95, -59,
	-74, -59, -80, -61, -43, -63, -27, -63,
	-53, -59, -31, -59, -40, -61, -8, -63,
	8, -63, -10, -59, 10, -59, 0, -61,
	27, -63, 43, -63, 31, -59, 53, -59,
	40, -61, 60, -63, 76, -63, 74, -59,
	95, -59, 80, -61, 95, -63, 111, -63,
	117, -59, 139, -59, 120, -61, -118, -53,
	-92, -53, -152, -45, -120, -45, -118, -50,
	-66, -53, -40, -53, -84, -45, -51, -45,
	-59, -50, -13, -53, 13, -53, -16, -45,
	16, -45, 0, -50, 40, -53, 66, -53,
	51, -45, 84, -45, 59, -50, 92, -53,
	118, -53, 120, -45, 152, -45, 118, -50,
	-110, -35, -67, -35, -140, -22, -83, -22,
	-98, -30, -22, -35, 22, -35, -27, -22,
	27, -22
};
int ViewX = 0;   /* 2500 x of the 3D view on the screen */
int Var_2502[41] = {   /* 2502 unused */
	-30, 67, -35, 110, -35, 83, -22, 140,
	-22, 98, -30, -128, -4, 128, -4, -128,
	-66, 128, -66, 128, 0, -38, -4, 38,
	-4, -38, -66, 38, -66, 0, 0, -128,
	-4, 128, -4, -128, -66, 128, -66, 128,
	0
};
char Keys_2554[4] = {   /* 2554 the keys of an OK box */
	61, 43, 25, 0
};
signed char SpellMissile[14][4] = {   /* 2558 */
	{ 4, -1, 65, 12 }, { 11, -1, 4, 6 }, { 13, -1, 122, 9 }, { 15, -1, 74, 8 },
	{ 17, 6, 56, 10 }, { 20, 6, 122, 11 }, { 24, -1, 74, 8 }, { 54, -1, 75, 9 },
	{ 63, -1, 75, 9 }, { 65, 6, 75, 9 }, { 66, 6, 75, 8 }, { 67, 6, 75, 8 },
	{ 68, 6, 75, 8 }, { 69, 6, 75, 8 }
};
char ItemSize[113] = {   /* 2590 per item shape: its size on the floor */
	0, 0, 0, 5, 1, 2, 3, 4, 3, 5, 6, 6, 18, 5, 15, 18,
	20, 8, 19, 17, 21, 24, 7, 7, 7, 31, 9, 35, 9, 9, 9, 9,
	8, 28, 28, 26, 27, 33, 29, 29, 34, 34, 34, 0, 22, 0, 0, 23,
	23, 23, 23, 23, 34, 33, 25, 35, 16, 30, 23, 37, 23, 38, 18, 33,
	23, 35, 28, 0, 32, 37, 18, 24, 31, 7, 7, 21, 21, 15, 3, 9,
	30, 30, 0, 0, 0, 0, 0, 27, 27, 23, 23, 22, 22, 33, 8, 37,
	37, 37, 37, 40, 3, 4, 33, 0, 23, 0, 32, 36, 39, 39, 28, 39,
	0
};
signed char ThrownOfSize[45] = {   /* 2601 */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2,
	-1, -1, 4, -1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0
};

char far ItemNames[130][35] = { 0 };   /* 37DA:0000 the item names, read from a file */
/* 38F7:0000 the texts of the camp menus (get_script_string: the n-th
 * string); the declared size leaves room after them (2 bytes free) */
char far MenuStrings[2509] =
	"\0"
	"Select Option:\0"
	"Rest Party\0"
	"Memorize Spells\0"
	"Pray for Spells\0"
	"Scribe Scrolls\0"
	"Preferences\0"
	"Game Options\0"
	"Exit\0"
	"Load Game\0"
	"Save Game\0"
	"Drop Character\0"
	"Quit Game\0"
	"Game Options:\0"
	"\r   Game saved.\0"
	"Attempts to save your\rgame have failed!\0"
	"Cannot load your\rsave game.  The\rfile may be corrupt!\0"
	"Tunes are ON \0"
	"Sounds are ON \0"
	"Bar Graphs are ON \0"
	"Mouse is ON \0"
	"ON\0"
	"OFF\0"
	"\r\r Select a character\r from your party who\r would like to learn\r spells.\0"
	"\r Your Paladin is \r too low a level\r for spells.\0"
	"\r\r The Mage has no\r Spell Book!\0"
	"\r\r\r Select a character\r from your party\r who would like to\r pray for spells.\0"
	"\r No party members\r are capable of\r praying for new\r spells.\0"
	"\r No party members\r are capable of\r memorizing new\r spells.\0"
	" An unconscious\r or dead Mage\r cannot memorize\r spells.\0"
	" An unconscious\r or dead Cleric\r cannot pray for\r spells.\0"
	"1\0"
	"2\0"
	"3\0"
	"4\0"
	"5\0"
	"Clear\0"
	"Spells Available:\0"
	"Yes\0"
	"No\0"
	"\rWill your healers\rheal the party?\0"
	" Someone is still\r injured. Rest\r until healed?\0"
	"Resting party.\0"
	"\r All characters\r are fully\r rested.\0"
	" Your party needs\r to rest to gain\r spells.\0"
	" You cannot have\r less than four\r characters.\0"
	" Are you sure you\r wish to exit the\r game?\0"
	" Your party is\r starving. Do you\r wish to continue\r resting?\0"
	"Select the scroll(s)\ryou wish to scribe.\0"
	"\r\r\r Select a Mage\r from your party\r who would like to\r scribe spells.\0"
	" You don't have\r any scolls to be\r scribed.\0"
	" You don't have\r any scrolls that\r this Mage needs.\0"
	"\r You don't have\r any Mage able to\r scribe scrolls.\0"
	" Select the\r character you\r wish to drop.\0"
	" You must specify\r a name for your\r save game!\0"
	" Are you sure you\r wish to replace\r your saved game?\0"
	"Camp:\0"
	"Preferences:\0"
	"Game Options:\0"
	"Poisoned party\rmembers may die!\rRest anyway?\0"
	"6\0"
	"7\0"
	"8\0"
	"As you awaken, you\rdiscover that Insal\rthe thief is gone!\0"
	"A quick inventory\rreveals equipment\rhas been stolen!\0"
	"There is no game\rsaved in that slot!\0"
	"There may not be\renough room on your\rdisk.  Or the disk\rmay be write protected\0"
	"\rYour entire party\rhas been defeated!\rWould you like to\rload a previously\rsaved game?\0"
	"\r We cannot find your save game\r file.  Please insert the disk\r with the file EOBDATA.SAV into\r any drive.  If it is located\r in another directory on your\r hard drive, please copy it\r into your Eye of the Beholder II\r directory.  Do you wish to\r continue?\0"
	"You must transfer\rfour party members\rbefore you may play!\0"
	"You may only transfer\rfour party members to\rEOB II!\0"
	"Items which are not\ruseful or unbalance\rthe game will be\rdeleted.\0"
	" \r You do not have\r a mage in your\r party.\0"
	" \r You do not have\r a cleric or\r paladin in your\r party.";
int far Var_3994 = 0;   /* 3994:0000 a far segment of its own (2 bytes) */
int far TimeDetectMagic[4] = { 0, 546, 2, 1 };   /* 3995:0000 */
int far TimeShield[4] = { 0, 546, 5, 1 };   /* 3996:0000 */
int far TimeShockingGrasp[4] = { 0, 546, 1, 1 };   /* 3997:0000 */
int far TimeBlur[4] = { 3, 546, 1, 1 };   /* 3998:0000 */
int far TimeHaste[4] = { 3, 546, 1, 1 };   /* 3999:0000 */
int far TimeDetectInvisibility[4] = { 0, 546, 5, 1 };   /* 399A:0000 */
int far TimeVampiricTouch[4] = { 0, 546, 1, 1 };   /* 399B:0000 */
int far TimeImprovedInvisibility[4] = { 5, 546, 1, 1 };   /* 399C:0000 */
int far TimeTrueSeeing[4] = { 0, 546, 1, 1 };   /* 399D:0000 */
int far TimeBless[4] = { 6, 546, 0, 1 };   /* 399E:0000 */
int far TimeProtectionFromEvil[4] = { 0, 546, 3, 1 };   /* 399F:0000 */
int far TimeAid[4] = { 1, 546, 1, 1 };   /* 39A0:0000 */
int far TimeFlameBlade[4] = { 4, 546, 1, 1 };   /* 39A1:0000 */
int far TimeSlowPoison[4] = { 0, 32760, 1, 1 };   /* 39A2:0000 */
int far TimeMagicalVestment[4] = { 0, 546, 5, 1 };   /* 39A3:0000 */
int far TimePrayer[4] = { 0, 546, 1, 1 };   /* 39A4:0000 */
int far TimeMysticDefense[4] = { 1, 546, 0, 1 };   /* 39A5:0000 */

/* 39A6:0000 the spells: 1-32 mage, 34-62 cleric, then the attacks of
 * monsters cast as spells */
Spell far Spells[70] = {
	{ 0, 0, 0x0, 0, 0, 0, 0x0L, 0x0 },   /* 0 */
	{ "armor", spell_armor, 0x33, 0, 0, 92, 0x1L, 0x0 },   /* 1 */
	{ "burning hands", spell_burning_hands, 0x100, 0, 0, 87, 0x0L, 0x21 },   /* 2 */
	{ "detect magic", spell_detect_magic, 0x4C, TimeDetectMagic, end_detect_magic, 95, 0x2L, 0x0 },   /* 3 */
	{ "magic missile", spell_magic_missile, 0x100, 0, hit_magic_missile, 85, 0x0L, 0x1 },   /* 4 */
	{ "shield", 0, 0x8B, TimeShield, 0, 92, 0x8L, 0x0 },   /* 5 */
	{ "shocking grasp", spell_shocking_grasp, 0x488, TimeShockingGrasp, end_touch_spell, 88, 0x0L, 0x3 },   /* 6 */
	{ "blur", 0, 0x89, TimeBlur, 0, 75, 0x10L, 0x0 },   /* 7 */
	{ "detect invisibility", 0, 0x4C, TimeDetectInvisibility, 0, 95, 0x20L, 0x0 },   /* 8 */
	{ "improved identify", spell_improved_identify, 0x80, 0, 0, 20, 0x40L, 0x0 },   /* 9 */
	{ "invisibility", 0, 0x21, 0, 0, 94, 0x40L, 0x0 },   /* 10 */
	{ "melf's acid arrow", spell_acid_arrow, 0x100, 0, hit_acid_arrow, 96, 0x0L, 0x11 },   /* 11 */
	{ "dispel magic", spell_dispel_magic, 0x1000, 0, 0, 97, 0x0L, 0x0 },   /* 12 */
	{ "fireball", spell_fireball, 0x100, 0, hit_fireball, 99, 0x0L, 0x21 },   /* 13 */
	{ "haste", 0, 0x248, TimeHaste, 0, 100, 0x10000L, 0x0 },   /* 14 */
	{ "Hold Person", spell_hold_person, 0x100, 0, hit_hold_person, 101, 0x0L, 0x0 },   /* 15 */
	{ "invisibility 10' radius", 0, 0x240, 0, 0, 94, 0x40L, 0x0 },   /* 16 */
	{ "lightning bolt", spell_lightning_bolt, 0x100, 0, hit_lightning_bolt, 71, 0x0L, 0x3 },   /* 17 */
	{ "vampiric touch", spell_vampiric_touch, 0x488, TimeVampiricTouch, end_vampiric_touch, 102, 0x0L, 0x1 },   /* 18 */
	{ "fear", spell_fear, 0x100, 0, 0, 103, 0x0L, 0x0 },   /* 19 */
	{ "ice storm", spell_ice_storm, 0x100, 0, hit_ice_storm, 89, 0x0L, 0x41 },   /* 20 */
	{ "improved invisibility", 0, 0x29, TimeImprovedInvisibility, 0, 8, 0x100L, 0x0 },   /* 21 */
	{ "remove curse", spell_remove_curse, 0x20, 0, 0, 83, 0x0L, 0x0 },   /* 22 */
	{ "cone of cold", spell_cone_of_cold, 0x100, 0, 0, 118, 0x0L, 0x41 },   /* 23 */
	{ "hold monster", spell_hold_monster, 0x100, 0, hit_hold_monster, 101, 0x0L, 0x0 },   /* 24 */
	{ "wall of force", spell_wall_of_force, 0x0, 0, 0, 74, 0x0L, 0x0 },   /* 25 */
	{ "disintegrate", spell_disintegrate, 0x1100, 0, 0, 119, 0x0L, 0x1 },   /* 26 */
	{ "flesh to stone", spell_flesh_to_stone, 0x100, 0, 0, 68, 0x0L, 0x1 },   /* 27 */
	{ "stone to flesh", spell_stone_to_flesh, 0x20, 0, 0, 69, 0x0L, 0x0 },   /* 28 */
	{ "true seeing", spell_true_seeing, 0x4C, TimeTrueSeeing, end_true_seeing, 73, 0x200L, 0x0 },   /* 29 */
	{ "finger of death", spell_finger_of_death, 0x0, 0, 0, 0, 0x0L, 0x1 },   /* 30 */
	{ "power word stun", spell_power_word_stun, 0x0, 0, 0, 0, 0x0L, 0x0 },   /* 31 */
	{ "bigby's clenched fist", 0, 0x0, 0, 0, 0, 0x0L, 0x0 },   /* 32 */
	{ 0, 0, 0x0, 0, 0, 0, 0x0L, 0x0 },   /* 33 */
	{ "bless", 0, 0x5C, TimeBless, 0, 91, 0x400L, 0x0 },   /* 34 */
	{ "cause light wounds", spell_cause_light_wounds, 0x100, 0, 0, 107, 0x0L, 0x1 },   /* 35 */
	{ "cure light wounds", spell_cure_light_wounds, 0x20, 0, 0, 104, 0x0L, 0x0 },   /* 36 */
	{ "detect magic", spell_detect_magic, 0x4C, TimeDetectMagic, end_detect_magic, 95, 0x2L, 0x0 },   /* 37 */
	{ "protection from evil", 0, 0x29, TimeProtectionFromEvil, 0, 110, 0x800L, 0x0 },   /* 38 */
	{ "aid", spell_aid, 0x39, TimeAid, end_aid, 91, 0x0L, 0x0 },   /* 39 */
	{ "flame blade", spell_flame_blade, 0x2408, TimeFlameBlade, end_touch_spell, 99, 0x0L, 0x21 },   /* 40 */
	{ "hold person", spell_hold_person, 0x100, 0, hit_hold_person, 101, 0x0L, 0x0 },   /* 41 */
	{ "slow poison", spell_slow_poison, 0x28, TimeSlowPoison, end_slow_poison, 111, 0x2000L, 0x0 },   /* 42 */
	{ "create food", spell_create_food, 0x40, 0, 0, 112, 0x0L, 0x0 },   /* 43 */
	{ "dispel magic", spell_dispel_magic, 0x1000, 0, 0, 97, 0x0L, 0x0 },   /* 44 */
	{ "magical vestment", 0, 0x99, TimeMagicalVestment, 0, 113, 0x4000L, 0x0 },   /* 45 */
	{ "prayer", 0, 0x4C, TimePrayer, 0, 91, 0x8000L, 0x0 },   /* 46 */
	{ "remove paralysis", spell_remove_paralysis, 0x40, 0, 0, 114, 0x0L, 0x0 },   /* 47 */
	{ "cause serious wounds", spell_cause_serious_wounds, 0x100, 0, 0, 108, 0x0L, 0x1 },   /* 48 */
	{ "cure serious wounds", spell_cure_serious_wounds, 0x20, 0, 0, 105, 0x0L, 0x0 },   /* 49 */
	{ "neutralize poison", spell_neutralize_poison, 0x20, 0, 0, 115, 0x0L, 0x0 },   /* 50 */
	{ "protection from evil 10' radius", 0, 0x248, TimeProtectionFromEvil, 0, 110, 0x800L, 0x0 },   /* 51 */
	{ "cause critical wounds", spell_cause_critical_wounds, 0x100, 0, 0, 109, 0x0L, 0x1 },   /* 52 */
	{ "cure critical wounds", spell_cure_critical_wounds, 0x20, 0, 0, 106, 0x0L, 0x0 },   /* 53 */
	{ "flame strike", spell_flame_strike, 0x100, 0, hit_flame_strike, 98, 0x0L, 0x21 },   /* 54 */
	{ "raise dead", spell_raise_dead, 0x20, 0, 0, 117, 0x0L, 0x0 },   /* 55 */
	{ "slay living", spell_finger_of_death, 0x100, 0, 0, 72, 0x0L, 0x1 },   /* 56 */
	{ "true seeing", spell_true_seeing, 0x4C, TimeTrueSeeing, end_true_seeing, 73, 0x200L, 0x0 },   /* 57 */
	{ "harm", spell_harm, 0x100, 0, 0, 70, 0x0L, 0x1 },   /* 58 */
	{ "heal", spell_heal, 0x20, 0, 0, 84, 0x0L, 0x0 },   /* 59 */
	{ "ressurection", 0, 0x0, 0, 0, 0, 0x0L, 0x0 },   /* 60 */
	{ "lay on hands", spell_lay_on_hands, 0x20, 0, 0, 91, 0x0L, 0x0 },   /* 61 */
	{ "turn undead", spell_turn_undead, 0x1000, 0, 0, 103, 0x0L, 0x0 },   /* 62 */
	{ "", 0, 0x0, 0, hit_breath, 98, 0x0L, 0x0 },   /* 63 */
	{ "mystic defense", 0, 0x4C, TimeMysticDefense, 0, 91, 0x20000L, 0x0 },   /* 64 */
	{ "", 0, 0x0, 0, hit_eighteen, 98, 0x0L, 0x0 },   /* 65 */
	{ "", 0, 0x0, 0, hit_death_spell, 101, 0x0L, 0x0 },   /* 66 */
	{ "", 0, 0x0, 0, hit_disintegrate_party, 101, 0x0L, 0x0 },   /* 67 */
	{ "", 0, 0x0, 0, hit_cause_serious, 101, 0x0L, 0x0 },   /* 68 */
	{ "", 0, 0x0, 0, hit_petrify, 101, 0x0L, 0x0 }    /* 69 */
};
signed char far SpellPlaces[4][8] = {   /* 3A14:0000 per direction */
	{ 2, 3, 4, 1, 2, 5, 0, 0 },
	{ 0, 2, 4, 1, 3, 5, 0, 0 },
	{ 0, 1, 4, 2, 3, 5, 0, 0 },
	{ 1, 3, 4, 0, 2, 5, 0, 0 }
};
signed char far PlaceSeen2[4][4] = {   /* 3A16:0000 */
	{ 0, 0, 1, 1 },
	{ 1, 0, 1, 0 },
	{ 1, 1, 0, 0 },
	{ 0, 1, 0, 1 }
};
signed char far HornSounds[4] = { 64, 65, 66, 67 };   /* 3A17:0000 */

char *Text_NeedRam = "\nEye of the Beholder II requires %lu bytes of RAM free.\n";   /* 262E */
char *Text_RunSetup = "Please run SETUP before running The Legend of Darkmoon.\n";   /* 2632 */
