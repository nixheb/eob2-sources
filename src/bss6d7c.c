/*
 * Eye of the Beholder II - START.EXE
 * The uninitialised variables of data.c (DS:6D7C-EF1D, _BSS), in address order.
 *
 * BCC 2.0 lays the _BSS of a module out in the order of its symbol table (a
 * hash of the names); with names other than the original ones, lost, the
 * order would differ. So they are written here as zeros in a data segment
 * renamed _BSS of class BSS (-zR_BSS -zTBSS), with the bytes BCC leaves to
 * align the words (Pad_XXXX): they come in the order of this file. (-a would
 * pad the structures as well.) Linked right after data.c;
 * the empty code segment joins DATA_TEXT (-zC). Made by tools/bssdef.py.
 *
 * Compiled with
 *     bcc -c -ml -G -O -Z -y -zR_BSS -zTBSS -zCDATA_TEXT bss6d7c.c
 */
#include "eob2.h"

typedef unsigned char byte;
typedef struct MovingDoor { int block; signed char dir; } MovingDoor;
typedef struct DoorButton { char far *shape; byte x, y; } DoorButton;
typedef struct Accessory { char far *shape; char x, y; } Accessory;
typedef struct TimedEvent { int what; unsigned interval; unsigned long next; } TimedEvent;
typedef struct Decoration {
	byte          shape[10];     /* +0 the shape of each place, FF = none */
	char          next;          /* +A the next decoration of the chain */
	byte          flags;         /* +B */
	int           x[10], y[10];  /* +C */
} Decoration;

/* Uninitialised data of the module (DS:6D7C-EF1D, _BSS), in address order */
char ScriptResult = 0;                    /* 6D7C result of the last dialogue */
char Pad_6D7D = 0;                        /* 6D7D the alignment of the _BSS */
int AnimPhase = 0;                        /* 6D7E toggled every 10 ticks: animated decorations */
unsigned char Col_6D80 = 0;               /* 6D80 */
unsigned char Col_6D81 = 0;               /* 6D81 */
Button far *CgButtonList[19] = { 0 };     /* 6D82 */
int CurrentLevel = 0;                     /* 6DCE */
int CurrentSubLevel = 0;                  /* 6DD0 */
int PartyBlock = 0;                       /* 6DD2 */
int PartyDirection = 0;                   /* 6DD4 */
int HandItem = 0;                         /* 6DD6 the item in the mouse "hand" */
unsigned long LevelsVisited = 0;          /* 6DD8 bit n-1: level n has a saved state */
unsigned long PartyEffects = 0;           /* 6DDC */
char Var_6DE0 = 0;                        /* 6DE0 never used */
char Var_6DE1 = 0;                        /* 6DE1 resting is not safe here */
unsigned long LevelFlags[17] = { 0 };     /* 6DE2 one set of 32 flags per level */
unsigned long GlobalFlags = 0;            /* 6E26 */
Monster Monsters[30] = { 0 };             /* 6E2A */
unsigned char Col_71AE = 0;               /* 71AE */
char PairFirst[16] = { 0 };               /* 71AF */
unsigned char Col_71BF = 0;               /* 71BF */
byte far *BookShape = 0;                  /* 71C0 */
byte far *ScriptCode = 0;                 /* 71C4 */
int Var_71C8 = 0;                         /* 71C8 casting from a scroll or a wand: hand + 1 */
Item Items[600] = { 0 };                  /* 71CA */
unsigned char Col_929A = 0;               /* 929A */
char Pad_929B = 0;                        /* 929B the alignment of the _BSS */
MovingDoor MovingDoors[3] = { 0 };        /* 929C */
char Var_92A5 = 0;                        /* 92A5 never used */
char Var_92A6 = 0;                        /* 92A6 */
char Pad_92A7 = 0;                        /* 92A7 the alignment of the _BSS */
byte far *Var_92A8[12] = { 0 };           /* 92A8 3 x 4 */
unsigned char Col_92D8 = 0;               /* 92D8 */
char Pad_92D9 = 0;                        /* 92D9 the alignment of the _BSS */
int EvalStack[30] = { 0 };                /* 92DA */
char Var_9316 = 0;                        /* 9316 0: a new game */
char InvCharacter = 0;                    /* 9317 */
byte far *Var_9318 = 0;                   /* 9318 */
char ScriptCharacter = 0;                 /* 931C party member selected by a condition, -1 = none */
char Pad_931D = 0;                        /* 931D the alignment of the _BSS */
DoorButton DoorButtons[6] = { 0 };        /* 931E the knobs of the doors */
byte far *GameFont = 0;                   /* 9342 */
ForceWall ForceWalls[5] = { 0 };          /* 9346 */
unsigned char Col_9364 = 0;               /* 9364 */
char Pad_9365 = 0;                        /* 9365 the alignment of the _BSS */
int DoorKind[2] = { 0 };                  /* 9366 */
signed char BookSlot = 0;                 /* 936A */
char Pad_936B = 0;                        /* 936B the alignment of the _BSS */
Accessory MonsterAccessories[36] = { 0 }; /* 936C */
char Front = 0;                           /* 9444 */
char Pad_9445 = 0;                        /* 9445 the alignment of the _BSS */
int ViewBlock[18] = { 0 };                /* 9446 [13]: the block in front of the party */
char ScriptEnd = 0;                       /* 946A set by op_end */
char Pad_946B = 0;                        /* 946B the alignment of the _BSS */
int EvalSp = 0;                           /* 946C */
ItemType ItemTypes[51] = { 0 };           /* 946E */
byte Var_979E[0x60] = { 0 };              /* 979E */
char Var_97FE[128] = { 0 };               /* 97FE never used */
unsigned ViewTiles[2][15][22] = { 0 };    /* 987E */
unsigned Var_9DA6[2][15][22] = { 0 };     /* 9DA6 never used: the size of ViewTiles */
int ScriptBlock = 0;                      /* A2CE square whose script is running */
char far *MonsterGfx[36] = { 0 };         /* A2D0 slot * 18 + picture - 1 */
LevelBlock LevelMap[1024] = { 0 };        /* A360 */
byte far *UnconsciousShape = 0;           /* CB60 */
int StatMax[6] = { 0 };                   /* CB64 */
char LastHitMonster = 0;                  /* CB70 */
char Pad_CB71 = 0;                        /* CB71 the alignment of the _BSS */
byte far *VcnTiles = 0;                   /* CB72 */
char Var_CB76 = 0;                        /* CB76 */
char Pad_CB77 = 0;                        /* CB77 the alignment of the _BSS */
int CgSlot = 0;                           /* CB78 the box (character) selected */
int ViewClip[36] = { 0 };                 /* CB7A per square: the columns not hidden (left, right) */
signed char SpellTarget = 0;              /* CBC2 */
char Pad_CBC3 = 0;                        /* CBC3 the alignment of the _BSS */
byte far *ShrinkBuffer = 0;               /* CBC4 */
TimedEvent TimedEvents[5] = { 0 };        /* CBC8 */
int ViewTop[18] = { 0 };                  /* CBF0 per square: the lines of a door frame */
byte far *Cursor = 0;                     /* CC14 the mouse pointer */
int StatMin[7] = { 0 };                   /* CC18 the limits of the abilities while editing */
byte far *MagicPalette = 0;               /* CC26 */
signed char far *BookSpells = 0;          /* CC2A the memorized spells of the book's owner */
int BurstX = 0;                           /* CC2E */
int BurstY = 0;                           /* CC30 */
int LastThrownItem = 0;                   /* CC32 */
byte far *Shapes[9] = { 0 };              /* CC34 */
byte far *Var_CC58 = 0;                   /* CC58 the shape greying a portrait */
Missile Thrown[10] = { 0 };               /* CC5C */
int Var_CD10 = 0;                         /* CD10 never used */
byte far *NormalPalette = 0;              /* CD12 */
signed char BookLevel = 0;                /* CD16 the spell level shown */
char Pad_CD17 = 0;                        /* CD17 the alignment of the _BSS */
int Var_CD18 = 0;                         /* CD18 the spell cast (for the level scripts) */
MonsterType MonsterTypes[10] = { 0 };     /* CD1A */
byte far *ItemIcons[85] = { 0 };          /* CEE6 */
byte far *HandShapes[2] = { 0 };          /* D03A the empty hands */
char Var_D042[64] = { 0 };                /* D042 never used */
byte far *HandShape2 = 0;                 /* D082 the left hand when GlobalFlags & 0x80000000 */
char Var_D086[32] = { 0 };                /* D086 never used */
byte far *IconBuffer = 0;                 /* D0A6 an item icon, recoloured */
char SceneDirty = 0;                      /* D0AA the 3D view must be redrawn */
signed char BookPage = 0;                 /* D0AB 0 or 6 */
byte far *LoadBuffer = 0;                 /* D0AC page 5 */
char BookCaster = 0;                      /* D0B0 the character whose spell book is open */
char Pad_D0B1 = 0;                        /* D0B1 the alignment of the _BSS */
char far *PortraitShapes[50] = { 0 };     /* D0B2 */
char far *CursorShapes[10] = { 0 };       /* D17A the animated frame of the box */
char Var_D1A2[30] = { 0 };                /* D1A2 never used */
int far *DecoRects = 0;                   /* D1C0 x, y, w, h of each decoration shape (.DEC) */
byte far *Var_D1C4 = 0;                   /* D1C4 */
int ItemCount = 0;                        /* D1C8 */
int Var_D1CA = 0;                         /* D1CA never used */
Decoration Decorations[60] = { 0 };       /* D1CC */
int ViewBottom[18] = { 0 };               /* DDFC */
byte far *FloorShapes[45] = { 0 };        /* DE20 11 large items (ITEML1), 26 small ones from 15 (ITEMS1) */
byte far *Var_DED4 = 0;                   /* DED4 */
char Var_DED8[4] = { 0 };                 /* DED8 never used */
char far *ViewWalls[18] = { 0 };          /* DEDC &LevelMap[ViewBlock[i]].walls; [13] = the square in front */
char Var_DF24 = 0;                        /* DF24 */
char Pad_DF25 = 0;                        /* DF25 the alignment of the _BSS */
byte far *DamageShape = 0;                /* DF26 */
char CurrentSpell = 0;                    /* DF2A the spell being cast (read as a byte here) */
char Left = 0;                            /* DF2B */
char Var_DF2C = 0;                        /* DF2C the party has moved */
char Pad_DF2D = 0;                        /* DF2D the alignment of the _BSS */
void (far *SpellStart)(void) = 0;         /* DF2E */
unsigned char Col_DF32 = 0;               /* DF32 */
char Pad_DF33 = 0;                        /* DF33 the alignment of the _BSS */
byte far *HandResultShape = 0;            /* DF34 */
char Var_DF38 = 0;                        /* DF38 set when the spell fails */
char Pad_DF39 = 0;                        /* DF39 the alignment of the _BSS */
byte far *WorkPage = 0;                   /* DF3A page 3 */
int Var_DF3E = 0;                         /* DF3E */
void far *Var_DF40 = 0;                   /* DF40 */
unsigned char Col_DF44 = 0;               /* DF44 */
char Pad_DF45 = 0;                        /* DF45 the alignment of the _BSS */
void far *BookGui = 0;                    /* DF46 */
char Right = 0;                           /* DF4A */
char Pad_DF4B = 0;                        /* DF4B the alignment of the _BSS */
int Var_DF4C = 0;                         /* DF4C */
int DialogueStyle = 0;                    /* DF4E */
char far *DecoShapes[300] = { 0 };        /* DF50 */
byte far *DeadShape = 0;                  /* E400 the bones of a dead character */
byte far *ThrownShapes[9] = { 0 };        /* E404 */
byte far *Var_E428[4] = { 0 };            /* E428 */
char Var_E438[8] = { 0 };                 /* E438 never used */
char Var_E440 = 0;                        /* E440 the place the caster aims from */
char BookOpen = 0;                        /* E441 the spell book is open */
int Var_E442 = 0;                         /* E442 the first item picked up with the keyboard */
unsigned char Col_E444 = 0;               /* E444 */
char PairNext[128] = { 0 };               /* E445 */
char HpBars = 0;                          /* E4C5 hit points shown as bars */
char Var_E4C6 = 0;                        /* E4C6 set by launch_spell (spell from a character?) */
char Pad_E4C7 = 0;                        /* E4C7 the alignment of the _BSS */
int TimedEventCount = 0;                  /* E4C8 */
byte far *Shape_E4CA = 0;                 /* E4CA */
unsigned far *WallLayouts = 0;            /* E4CE */
int NoViewCopy = 0;                       /* E4D2 */
byte far *CallStack[10] = { 0 };          /* E4D4 */
char Var_E4FC[80] = { 0 };                /* E4FC never used */
int FloorParity = 0;                      /* E54C the background is mirrored */
byte far *Var_E54E[6] = { 0 };            /* E54E */
byte Remap_E566[16] = { 0 };              /* E566 the colours of the wall tiles */
char Var_E576[40] = { 0 };                /* E576 never used */
int CallDepth = 0;                        /* E59E */
int ItemNameCount = 0;                    /* E5A0 */
int Var_E5A2 = 0;                         /* E5A2 */
char InvSlot = 0;                         /* E5A4 the selected inventory slot */
char Pad_E5A5 = 0;                        /* E5A5 the alignment of the _BSS */
int ScriptAbort = 0;                      /* E5A6 set when the level changes under our feet */
Character far *TargetChar = 0;            /* E5A8 */
char Var_E5AC[20] = { 0 };                /* E5AC never used */
int DecorationCount = 0;                  /* E5C0 */
unsigned char FrameDark = 0;              /* E5C2 */
unsigned char Var_E5C3 = 0;               /* E5C3 */
char Var_E5C4[4] = { 0 };                 /* E5C4 never used */
int StepCounter = 0;                      /* E5C8 */
char GuiState[60] = { 0 };                /* E5CA */
unsigned char FrameFlat = 0;              /* E606 */
char Pad_E607 = 0;                        /* E607 the alignment of the _BSS */
Character Party[6] = { 0 };               /* E608 */
int Var_EE1E = 0;                         /* EE1E */
char SelectedChar = 0;                    /* EE20 the portrait clicked first (to swap), -1 */
char Pad_EE21 = 0;                        /* EE21 the alignment of the _BSS */
unsigned ScriptEvents = 0;                /* EE22 events that triggered it */
byte far *FlameShapes[3] = { 0 };         /* EE24 */
byte far *StoneRemap = 0;                 /* EE30 */
int DoorNoKnob[2] = { 0 };                /* EE34 */
int InDialogue = 0;                       /* EE38 a dialogue is open */
char Var_EE3A = 0;                        /* EE3A the inventory screen: 0 none, 1 inventory, 2 statistics */
char FadeInPending = 0;                   /* EE3B */
int Var_EE3C = 0;                         /* EE3C never used */
char SaveMask = 0;                        /* EE3E bit n: EOBDATAn.SAV exists */
char SaveNames[6][20] = { 0 };            /* EE3F */
char Var_EEB7[40] = { 0 };                /* EEB7 never used */
char Pad_EEDF = 0;                        /* EEDF the alignment of the _BSS */
Button far *GameGui = 0;                  /* EEE0 the buttons of the screen */
byte far *PoisonRemap = 0;                /* EEE4 colour tables (256 bytes each) */
byte far *InvisibleRemap = 0;             /* EEE8 */
int MenuX = 0;                            /* EEEC */
int MenuY = 0;                            /* EEEE */
unsigned char FrameLight = 0;             /* EEF0 */
char Pad_EEF1 = 0;                        /* EEF1 the alignment of the _BSS */
unsigned long Var_EEF2 = 0;               /* EEF2 */
byte Remap_EEF6[16] = { 0 };              /* EEF6 the colours of the background tiles */
char far *DoorShapes[6] = { 0 };          /* EF06 slot * 3 + size */
