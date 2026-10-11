/*
 * Eye of the Beholder II - START.EXE
 * The start of the game (buffers, intro, main menu, then the main loop)
 * and the item taken in hand: module 08AE (Ghidra 18AE).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y game.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern Item           Items[600];        /* 71CA */
extern Character      Party[6];          /* E608 */
extern char           HpBars;            /* E4C5 hit points shown as bars */
extern char           FadeInPending;     /* EE3B */
extern unsigned       Var_65FE;          /* 65FE */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *VcnTiles;          /* CB72 */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern byte far      *WorkPage;          /* DF3A page 3 */
extern byte far      *ScriptCode;        /* 71C4 */
extern unsigned far  *WallLayouts;       /* E4CE */
extern byte far      *PoisonRemap;       /* EEE4 colour tables (256 bytes each) */
extern byte far      *StoneRemap;        /* EE30 */
extern byte far      *NormalPalette;     /* CD12 */
extern byte far      *InvisibleRemap;    /* EEE8 */
extern byte far      *MagicPalette;      /* CC26 */
extern byte far      *IconBuffer;        /* D0A6 an item icon, recoloured */
extern byte far      *Var_D1C4;          /* D1C4 */
extern byte far      *Var_9318;          /* 9318 */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern unsigned       PageSegs[];        /* 62A7 [7]: the segment of page 7 */
extern int            HandItem;          /* 6DD6 the item in the mouse "hand" */
extern unsigned char  InventoryMode;     /* 6655 the keyboard moves in the inventory */
extern int            GuiStyle;          /* 636F */
extern unsigned long  PartyEffects;      /* 6DDC */
extern byte far      *ItemIcons[];       /* CEE6 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
void more_off(void);                                             /* 31B0:0129 */
void gui_set_hook(int unused, void far *hook);                 /* 2F86:0D9C */
byte far *load_file_alloc(char far *name, int page);             /* 308C:0005 */
byte far *alloc_page(int page);                                  /* 2F3E:0009 */
void far *normalize(void far *p);                                /* 3374:0071 */
byte far *sound_buffer(void);                                    /* 1000:025A */
void load_sounds(char far *name);                                 /* 2C9F:0009 */
void fn_3069_0230(int a, unsigned b);                            /* 3069:0230 */
void load_game_shapes(void);                                     /* 3ADD:004D (overlay 5315:04C7) */
void main_menu(void);                                            /* 3ADD:002F (overlay 5315:0926) */
void set_font(byte far *font);                                   /* 3166:000A */
void game_loop(void);                                            /* 1BB3:0EAC */
void set_hand_item(int item);                                    /* 1E24:037C */
int  get_hand_item(void);                                        /* 1E24:0352 */
void print_item_name(int item);                                  /* 1CBE:05D1 */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void remap_shape(byte far *shape, byte far *table);              /* 2A68:10FE */
void set_cursor(int hx, int hy, byte far *shape);                /* 337D:0000 */

/* ------------------------------------------------------------------ */
/* Module 18AE, in address order                                       */
/* ------------------------------------------------------------------ */

/* 18AE:0005 [asm] - called by main() */
void far game_main(int argc, char far * far *argv)
{
	HpBars = 1;
	more_off();
	if (argc || argv[1][1])
		;                                     /* sic: empty (a removed option?) */
	FadeInPending = 0;
	gui_set_hook(0, 0L);
	Var_65FE |= 1;
	GameFont = load_file_alloc("FONT6.FNT", 6);
	VcnTiles = alloc_page(7);
	LoadBuffer = alloc_page(5);
	WorkPage = alloc_page(3);
	/* the rest of page 7 */
	ScriptCode = normalize((void far *)((unsigned long)VcnTiles + 0xB98C));
	WallLayouts = normalize(ScriptCode + 0x1D54);
	PoisonRemap = (byte far *)(WallLayouts + 3000);
	StoneRemap = PoisonRemap + 0x100;
	NormalPalette = StoneRemap + 0x100;
	InvisibleRemap = NormalPalette + 0x100;
	MagicPalette = InvisibleRemap + 0x100;
	IconBuffer = MagicPalette + 0x100;
	Party[0].face = IconBuffer + 500;
	Var_D1C4 = Party[0].face;
	Var_9318 = Party[0].face + 1400;
	ShrinkBuffer = sound_buffer() + 0x2AF8;
	load_sounds("INTRO");
	fn_3069_0230(PageSegs[7], 0xFC00);
	load_game_shapes();
	main_menu();
	set_font(GameFont);
	Var_65FE &= ~1;
	game_loop();
}

/* 18AE:0197 [asm] - an item taken in hand: the mouse cursor becomes its
 * icon (blue when the party sees the invisible, for an item flagged 0x80) */
void far take_item(register int item)
{
	char invisible;

	set_hand_item(item);
	if (item && get_hand_item() == item) {
		if (GuiStyle == 7) {
			print_item_name(item);
			gui_printf(" taken.\r");
		}
		item = Items[item].icon;
	}
	if (!InventoryMode) {
		invisible = (Items[HandItem].flags & 0x80) && (PartyEffects & 2L);
		if (item && invisible) {
			mem_copy(ItemIcons[item], IconBuffer, 300L);
			remap_shape(IconBuffer, MagicPalette);
			item = 0x70;
		}
		if (item > 0)
			set_cursor(8, 8, ItemIcons[item]);
		else
			set_cursor(0, 0, ItemIcons[item]);
	}
}
