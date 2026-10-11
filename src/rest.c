/*
 * Eye of the Beholder II - START.EXE
 * Resting (camp): spells regained, healing, food; overlay module 41C5
 * (Ghidra 51C5).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y rest.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * The data of the module (DS:0EE8) are only its strings, and the 6 bytes
 * that initialise a local array.
 */
#include "eob2.h"

typedef unsigned char byte;

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern Character      Party[];           /* E608 */
extern Item           Items[];           /* 71CA */
extern Monster        Monsters[];        /* 6E2A */
extern Spell far      Spells[];          /* 39A6:0000 */
extern char far       MenuStrings[];     /* 38F7:0000 */
extern char           Var_6DE1;          /* 6DE1 resting is not safe here */
extern int            CurrentLevel;      /* 6DCE */
extern int            PartyBlock;        /* 6DD2 */
extern unsigned long  GlobalFlags;       /* 6E26 */
extern unsigned long  GameTicks;         /* 6684 */
extern volatile unsigned long TimerTicks; /* 667C */
extern int            SoundEnabled;      /* 65FA */
extern int            NoViewCopy;        /* E4D2 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern int            GuiStyle;          /* 636F the current window */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
extern char           Scratch[];         /* F16E */
extern byte far      *GameFont;          /* 9342 */
extern byte far      *Font8;             /* F098 */
extern byte far      *CurFont;           /* F090 */
extern byte far      *LoadBuffer;        /* D0AC page 5 */
extern byte far      *ShrinkBuffer;      /* CBC4 */
extern unsigned char  FrameFlat, FrameLight, FrameDark;   /* E606, EEF0, E5C2 */
extern char           Keys_2554[];       /* 2554 */
extern void (far     *Drv_65D2)(int x, int y, int w, int h, byte far *buf);    /* 65D2 saves a region */

int  char_is_active(int ch, unsigned what);                      /* 1E24:06E9 */
int  class_part_index(int base, int cls);                        /* 1B54:009C */
int  find_inventory_item(int ch, int type, int value);           /* 1E24:0148 */
void draw_frame(int x, int y, int w, int h, int pressed, int hollow);   /* 1E24:0AB2 */
int  select_window(int w);                                       /* 31B0:0133 */
int  select_window_clear(int w);                                 /* 31B0:01F2 */
void gui_clear(void);                                            /* 31B0:020F */
void gui_printf(char far *fmt, ...);                             /* 31B0:0284 */
byte far *set_font(byte far *font);                              /* 3166:000A */
void hide_mouse(void);                                           /* 338B:030F */
void show_mouse(void);                                           /* 338B:033A */
void wait_ticks(int ticks);                                      /* 2D9B:0003 */
void delay_ticks(int ticks);                                           /* 18D7:04D2 */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
void draw_portrait(int ch);                                      /* 2901:0AF9 */
int  has_ring(Character far *c, int kind);                       /* 1E24:01BE */
void damage_character(int ch, int damage);                       /* 1F55:14AC */
int  key_pressed(void);                                          /* 3295:041E */
int  read_key(void);                                             /* 3295:066D */
void flush_keys(void);                                           /* 3295:0516 */
int  check_party_alive(int ask);                                 /* 1F55:09FE */
void monsters_think(int group);                                  /* 1997:0F1C */
void timer_thrown(int arg);                                      /* 27B7:02C1 */
int  distance(int from, int to);                                 /* 2A68:0FD8 */
void drop_character(int ch);                                     /* 1E24:043C */
void swap_mem(void far *a, void far *b, void far *buf, unsigned n);   /* 18D7:082F */
int  remove_items(int type, int value, int mode, int count);     /* 1E24:0BF9 */
void redraw_party(void);                                         /* 1E24:0686 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void print_text(char far *s, int x, int y, int fg, int bg);      /* 18D7:0908 */
void draw_frame3d(int x1, int y1, int x2, int y2, int light, int dark, int fill);   /* 18D7:0A45 */
int  in_rect(int x, int y, int x1, int y1, int x2, int y2);      /* 18D7:067F */
char far *get_script_string(char far *text, int index);          /* 1E24:0869 */
void play_sound(int sound);                                      /* 2C9F:00AA */
int  ask_party(int text);                                        /* 4F5F:23F9 (overlay) */
void rest_level3(void);                                          /* 56DD:16C2 (overlay) */
int  drop_new_item(int type);                                     /* 56DD:0912 (overlay) */
void draw_play_screen(int scene);                                /* 53EC:1A19 (overlay) */
void start_dialogue(void);                                       /* 56DD:0CE1 (overlay) */
void dialogue_picture(char far *name, int a, int b, int c, int d);   /* 56DD:0DDE (overlay) */
void clear_dialogue(void);                                       /* 56DD:01DD (overlay) */
void dialogue_text(int text, char far *button);                  /* 56DD:033E (overlay) */
void end_dialogue(void);                                         /* 56DD:0D37 (overlay) */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
char far *strchr(char far *s, int c);
void far *memchr(void far *s, int c, unsigned n);
extern int            ClickUp[2];        /* 6622 */
int far most_hurt(void);
void far show_hours(int hours, int redraw);
void far show_message(char far *s);
int far monsters_near(void);
int far spells_to_regain(void);
void far rest_text_box(int text);
void far drop_extra_npc(void);
int far knows_healing(int ch);

/* 51C5:0000 [asm] - rest until all is regained (or a key, a monster...):
 * 0 if the rest did not start (poisoned party and no), else whether the
 * party was attacked or died */
int far rest_party(void)
{
	int askHeal, healing, result, poisoned, changed;
	signed char far *p;
	unsigned char cnt[6] = { 0, 0, 0, 0, 0, 0 };
	int heal, hurt, hours, step, sp;
	register int i, ch;

	hours = 0;
	if (Var_6DE1) {
		show_message("\x06\x06You do not feel it is safe to rest here.");
		return 1;
	}
	if (monsters_near())
		return 1;
	if (CurrentLevel == 3) {
		rest_level3();
		return 1;
	}
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	poisoned = 0;
	for (i = 0; i < 6; i++)
		if (char_is_active(i, 1))
			poisoned |= !char_is_active(i, 16);
	if (poisoned && !ask_party(0x3B))
		return 0;
	healing = 0;
	askHeal = 0;
	if ((hurt = most_hurt()) > 0)
		for (i = 0; i < 6; i++)
			if (char_is_active(i, 13) && (class_part_index(2, Party[i].cls) != -1 || class_part_index(4, Party[i].cls) != -1)
			    && find_inventory_item(i, 0x1E, -1) != -1 && knows_healing(i)) {
				askHeal = ask_party(0x28);
				break;
			}
	select_window_clear(7);
	set_font(GameFont);
	show_hours(hours, 1);
	step = 0;
	do {
		step++;
		for (ch = 0; ch < 6; ch++) {
			cnt[ch]++;
			if (Party[ch].food == 0)
				continue;
			if (!char_is_active(ch, 5))
				continue;
			if (find_inventory_item(ch, 0x1E, -1) != -1) {
				p = Party[ch].clericSpells[0];
				for (i = 0; i < 80; p++, i++) {
					if (i / 10 + 48 >= cnt[ch])
						break;
					if (*p < 0) {
						*p = *p * -1;
						cnt[ch] = 48;
						sprintf(Scratch, "%s gained %s.\r", Party[ch].name, Spells[*p + 33].name);
						hide_mouse();
						gui_printf(Scratch);
						show_mouse();
						wait_ticks(5);
						break;
					}
				}
			}
			if (find_inventory_item(ch, 0x1D, -1) == -1)
				continue;
			p = Party[ch].mageSpells[0];
			for (i = 0; i < 80; p++, i++) {
				if (i / 6 + 48 >= cnt[ch])
					break;
				if (*p < 0) {
					*p = *p * -1;
					cnt[ch] = 48;
					sprintf(Scratch, "%s memorized %s.\r", Party[ch].name, Spells[*p].name);
					hide_mouse();
					gui_printf(Scratch);
					show_mouse();
					wait_ticks(5);
					break;
				}
			}
		}
		if (askHeal && hurt)
			for (ch = 0; ch < 6; ch++) {
				if ((class_part_index(2, Party[ch].cls) != -1 || class_part_index(4, Party[ch].cls) != -1)
				    && find_inventory_item(ch, 0x1E, -1) != -1) {
					if (cnt[ch] >= 48 && (memchr(Party[ch].clericSpells, -3, 80) || memchr(Party[ch].clericSpells, -16, 80)
					    || memchr(Party[ch].clericSpells, -20, 80)))
						break;
					p = Party[ch].clericSpells[0];
					for (i = 0; i < 80; i++) {
						heal = 0;
						switch (*p) {
						case 3:
							heal = roll_dice(1, 8, 0);
							*p = *p * -1;
							break;
						case 16:
							heal = roll_dice(2, 8, 1);
							*p = *p * -1;
							break;
						case 20:
							heal = roll_dice(3, 8, 3);
							*p = *p * -1;
							break;
						}
						p++;
						if (heal) {
							cnt[ch] = 0;
							hurt--;
							sprintf(Scratch, "%s casts healing on %s.\r", Party[ch].name, Party[hurt].name);
							hide_mouse();
							gui_printf(Scratch);
							show_mouse();
							wait_ticks(5);
							Party[hurt].hp += heal;
							if (Party[hurt].hp > Party[hurt].hpMax)
								Party[hurt].hp = Party[hurt].hpMax;
							draw_portrait(hurt);
							hurt++;
						}
						if (!hurt)
							break;
					}
				}
				if (!hurt)
					break;
			}
		if (step == 6) {
			step = 0;
			hours++;
			GameTicks += 0x7FF8L;
			show_hours(hours, 0);
			for (i = 0; i < 6; i++)
				if (char_is_active(i, 1) && !char_is_active(i, 16)) {
					damage_character(i, 10);
					delay_ticks(5);
				}
			if (hours % 8 == 0) {
				changed = 0;
				for (i = 0; i < 6; i++) {
					sp = 0x1C;
					if (Party[i].cls == 2) {
						if ((p = memchr(Party[i].clericSpells, sp, 10)) != 0)
							*p = sp;
						else if ((p = memchr(Party[i].clericSpells, -sp, 10)) != 0)
							*p = sp;
						else if (!memchr(Party[i].clericSpells, sp, 10))
							*(signed char far *)memchr(Party[i].clericSpells, 0, 10) = sp;
					}
					if (!char_is_active(i, 3))
						continue;
					if (Party[i].food) {
						if (Party[i].hp < Party[i].hpMax) {
							Party[i].hp++;
							set_font(GameFont);
							draw_portrait(i);
						}
						if (has_ring(&Party[i], 2) < 0) {
							if (Party[i].food <= 5) {
								Party[i].food = 0;
								changed = 1;
							} else
								Party[i].food -= 5;
						}
					} else if (hours % 24 == 0 && Party[i].hp > -10) {
						damage_character(i, 1);
						changed = 1;
						set_font(GameFont);
						draw_portrait(i);
					}
				}
				if (changed) {
					if (!ask_party(0x2F)) {
						show_hours(hours, 1);
						healing = 0;
						hurt = 0;
						break;
					}
					show_hours(hours, 1);
				}
				hurt = most_hurt();
			}
		}
		if (!spells_to_regain() && !healing && hurt) {
			healing = ask_party(0x29);
			show_hours(hours, 1);
			if (!healing)
				break;
		}
		if (key_pressed())
			break;
		result = monsters_near();
		if (!result)
			result = check_party_alive(0);
	} while ((spells_to_regain() || ((healing || askHeal) && hurt)) && !key_pressed() && !result);
	flush_keys();
	select_window(4);
	set_font(Font8);
	if (!hurt && !result)
		rest_text_box(0x2B);
	if (!result && hours > 4)
		drop_extra_npc();
	return result;
}

/* 51C5:091D [asm] - the living character with the fewest hit points
 * below his maximum, + 1 (0: none) */
int far most_hurt(void)
{
	int found;
	register int i, min;

	found = -1;
	min = 900;
	for (i = 0; i < 6; i++)
		if (char_is_active(i, 3) && Party[i].hp < Party[i].hpMax && Party[i].hp < min) {
			found = i;
			min = Party[i].hp;
		}
	return found + 1;
}

/* 51C5:098E [asm] - "Hours rested" in window 10 */
void far show_hours(int hours, int redraw)
{
	int old;
	byte far *font;

	old = GuiStyle;
	font = CurFont;
	set_font(Font8);
	select_window(10);
	if (redraw) {
		Var_62A5 = 0;
		hide_mouse();
		gui_clear();
		draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
		copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 0, 2);
		print_text(get_script_string(MenuStrings, 0x2A), (WinX + 1) * 8, WinY + 5, 9, 0);
		show_mouse();
	}
	sprintf(Scratch, "Hours rested: %-4d", hours);
	Var_62A5 = 2;
	hide_mouse();
	print_text(Scratch, (WinX + 1) * 8, WinY + 20, 15, FrameFlat);
	Var_62A5 = 0;
	copy_page(WinX + 1, WinY + 20, WinX + 1, WinY + 20, 18, 8, 2, 0);
	show_mouse();
	wait_ticks(10);
	set_font(font);
	select_window(old);
}

/* 51C5:0B10 [asm] */
void far show_message(char far *s)
{
	int old;
	byte far *font;

	font = set_font(GameFont);
	old = select_window(7);
	Var_62A5 = 0;
	hide_mouse();
	gui_printf("\r%s\r", s);
	show_mouse();
	select_window(old);
	set_font(font);
}

/* 51C5:0B79 [asm] - 5 turns of the monsters: 1 if one comes near */
int far monsters_near(void)
{
	int k, old;
	byte far *font;
	register int i, snd;

	snd = SoundEnabled;
	SoundEnabled = 0;
	for (k = 0; k < 5; k++) {
		NoViewCopy = 1;
		font = set_font(GameFont);
		old = select_window(7);
		monsters_think(0);
		monsters_think(1);
		timer_thrown(0);
		set_font(font);
		select_window(old);
		NoViewCopy = 0;
		for (i = 0; i < 30; i++)
			if (Monsters[i].mode != 8 && distance(PartyBlock, Monsters[i].block) < 2) {
				show_message("\x06\x06You can't rest here, monsters are near.");
				SoundEnabled = snd;
				return 1;
			}
	}
	SoundEnabled = snd;
	return 0;
}

/* 51C5:0C5F [asm] - a spell still to regain */
int far spells_to_regain(void)
{
	signed char far *p;
	register int i, ch;

	for (ch = 0; ch < 6; ch++) {
		if (!char_is_active(ch, 0x43))
			continue;
		if ((class_part_index(2, Party[ch].cls) != -1 || class_part_index(4, Party[ch].cls) != -1)
		    && find_inventory_item(ch, 0x1E, -1) != -1) {
			p = Party[ch].clericSpells[0];
			for (i = 0; i < 80; i++)
				if (*p++ < 0)
					return 1;
		}
		if (class_part_index(1, Party[ch].cls) != -1 && find_inventory_item(ch, 0x1D, -1) != -1) {
			p = Party[ch].mageSpells[0];
			for (i = 0; i < 80; i++)
				if (*p++ < 0)
					return 1;
		}
	}
	return 0;
}

/* 51C5:0D73 [asm] - a text of MenuStrings in window 11, for 120 ticks or
 * until a key */
void far rest_text_box(int text)
{
	int old, oldPage;
	byte far *font;
	register int i;

	oldPage = Var_62A5;
	font = CurFont;
	old = GuiStyle;
	set_font(Font8);
	hide_mouse();
	Dims[11].b = FrameFlat;
	select_window_clear(11);
	Var_62A5 = 2;
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	print_text(get_script_string(MenuStrings, text), WinX * 8 + 5, WinY + 5, 15, 0);
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	show_mouse();
	for (i = 0; i < 120; i++) {
		if (key_pressed()) {
			flush_keys();
			break;
		}
		wait_ticks(1);
	}
	Var_62A5 = oldPage;
	set_font(font);
	select_window(old);
	flush_keys();
}

/* 51C5:0E9A [asm] - a button at the bottom of the window: wait for a
 * click on it or one of the keys */
void far button_box(char far *label, char far *keys, int colour)
{
	int w, done, key;
	register int x, y;

	x = WinX * 8 + WinW * 4;
	x = x - strlen(label) * 4;
	y = WinY + WinH - 21;
	w = strlen(label) * 8 + 8;
	hide_mouse();
	draw_frame(x, y, w, 14, 0, 0);
	print_text(label, x + 4, y + 3, colour, 0);
	show_mouse();
	flush_keys();
	done = 1;
	while (done) {
		key = read_key() & 0x8FF;
		if (key == 0x41 || key == 0x42)
			done = !in_rect(ClickUp[0], ClickUp[1], x, y, x + w, y + 14);
		else
			done = !strchr(keys, key);
	}
	hide_mouse();
	draw_frame3d(x, y, x + w, y + 14, FrameDark, FrameFlat, -1);
	wait_ticks(5);
	draw_frame(x, y, w, 14, 0, 0);
	print_text(label, x + 4, y + 3, colour, 0);
	show_mouse();
}

/* 51C5:1017 [asm] - after a long rest with 5 or more characters, the NPC
 * (portrait -1) leaves: his items go, and two texts are shown */
void far drop_extra_npc(void)
{
	int count, held, dest;
	register int i, npc;

	npc = -1;
	for (i = count = 0; i < 6; i++)
		if (char_is_active(i, 1)) {
			if (char_is_active(i, 2) && Party[i].portrait == -1)
				npc = i;
			count++;
		}
	if (npc == -1)
		return;
	if (count < 5)
		return;
	drop_character(npc);
	dest = npc;
	if (npc < 4) {
		if (char_is_active(5, 1))
			dest = 5;
		else
			dest = 4;
		swap_mem(&Party[dest], &Party[npc], ShrinkBuffer, sizeof(Character));
	}
	GlobalFlags &= 0xFFFFFFF9L;
	if (!remove_items(1, 1, 1, 1))
		remove_items(2, 1, 1, 1);
	remove_items(0x1F, 0, 1, 3);
	remove_items(0x27, 1, 0, 3);
	remove_items(0x2F, 0, 1, 2);
	hide_mouse();
	held = drop_new_item(0x1C);
	Items[held].value = 0x1A;
	draw_play_screen(0);
	Drv_65D2(0, 0x78, 0x16, 0x18, LoadBuffer + 20000);
	set_font(GameFont);
	redraw_party();
	set_font(Font8);
	show_mouse();
	select_window_clear(10);
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	select_window_clear(11);
	Var_62A5 = 2;
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	print_text(get_script_string(MenuStrings, 0x3F), WinX * 8 + 5, WinY + 5, 15, 0);
	Var_62A5 = 0;
	hide_mouse();
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	show_mouse();
	while (key_pressed())
		read_key();
	button_box("OK", Keys_2554, 6);
	Var_62A5 = 2;
	draw_frame(WinX * 8, WinY, WinW * 8, WinH, 0, 0);
	print_text(get_script_string(MenuStrings, 0x40), WinX * 8 + 5, WinY + 5, 15, 0);
	Var_62A5 = 0;
	hide_mouse();
	copy_page(WinX, WinY, WinX, WinY, WinW, WinH, 2, 0);
	show_mouse();
	while (key_pressed())
		read_key();
	button_box("OK", Keys_2554, 6);
}

/* 51C5:138A [asm] */
static void far wait_until(int ticks)
{
	unsigned long end;

	end = TimerTicks + ticks;
	while (end > TimerTicks)
		;
}

/* 51C5:13BE [asm] - Dranx the dragon appears (called by the monsters) */
void far dranx_appears(void)
{
	byte far *font;

	font = set_font(GameFont);
	start_dialogue();
	dialogue_picture("DRANX", 0, 0, 0, 0);
	clear_dialogue();
	dialogue_text(0x78, "MORE");
	play_sound(0x38);
	dialogue_picture("DRANX", 0, 0x14, 0, 0);
	wait_until(7);
	dialogue_picture("DRANX", 0, 0, 0x60, 0);
	wait_until(7);
	dialogue_picture("DRANX", 0, 0x14, 0x60, 0);
	wait_until(0x12);
	end_dialogue();
	set_font(font);
}

/* 51C5:1493 [asm] - knows a cure spell (3, 16, 20), ready or not */
int far knows_healing(int ch)
{
	signed char far *p;
	register int i;

	p = Party[ch].clericSpells[0];
	for (i = 0; i < 80; p++, i++)
		switch (*p < 0 ? -*p : *p) {
		case 3:
		case 16:
		case 20:
			return 1;
		}
	return 0;
}
