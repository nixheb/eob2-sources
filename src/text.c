/*
 * Eye of the Beholder II - START.EXE
 * The text in the windows: module 21B0 (Ghidra 31B0).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -d text.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * gui_printf writes into the current window (select_window: Dims[]),
 * from its cursor, breaking the lines between words. Its own codes:
 *   %s, %d    as printf (only these two, no width)
 *   1         the "more" prompt now
 *   2 c, 6 c  the background, the colour of the text
 *   9         to the next tab (8 characters), 19 the same with spaces
 *   12        clear the window, 13 new line
 *   24 x, 25 y   move the cursor
 * The other codes below 32 are ignored. A byte with bit 7 set stands for
 * two letters (Westwood's text compression): PairFirst[(b >> 3) & 15],
 * then PairNext[(b >> 3) & 15][b & 7].
 * After WinLines - 1 lines, the "more" function (MoreFunc) is called with
 * MoreText...: by default more_prompt, "<MORE>" at the end of the line.
 */
#include "eob2.h"
#include <stdarg.h>

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern char           MorePrompt;        /* 62A0 the "more" is on */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern char           CharWidth;         /* 6288 */
extern char           LineHeight;        /* 6289 */
extern void (far     *MoreFunc)(char far *text, int len, int a, int b);   /* 6367 */
extern int            TextBg;            /* 636B */
extern int            TextColor;         /* 636D */
extern int            GuiStyle;          /* 636F the current window */
extern char           PairFirst[];       /* 71AF */
extern char           PairNext[];        /* E445 */
extern int            CursorX, CursorY;  /* F094, F096 the text cursor in the window */
extern int            WinH, WinW, WinX, WinY;   /* F100, F102 (chars), F104 (chars), F106: the text window */
/* Uninitialised data: in bssf1e4.c (DS:F1E4-F255) */
extern char Pend[2];      /* F1E4 the next two characters */
extern char PendSaved;    /* F1E6 the one after the %s or %d */
extern char Pad_F1E7;     /* F1E7 the alignment of the _BSS */
extern char far *SubText; /* F1E8 the text of a %s or %d */
extern char Line[100];    /* F1EC the line being made */
extern char far *FmtText; /* F250 */
extern int LineLen;       /* F254 */
extern void (far     *Drv_65F2)(char far *text, int x, int y, int fg, int bg);   /* 65F2 prints */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);    /* 657A fills */

void mouse_in_window(int w);                                     /* 323F:0009 */
void mouse_free(void);                                           /* 323F:006C */
void flush_keys(void);                                           /* 3295:0516 */
int  get_key(void);                                              /* 3295:05D0 */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
int  tolower(int c);                                             /* 1000:68C7 */
int  sprintf(char far *buf, char far *fmt, ...);                 /* 1000:64A1 */
char far *strcpy(char far *d, char far *s);                      /* 1000:7570 */
unsigned strlen(char far *s);                                    /* 1000:7599 */
void far gui_clear(void);
void far gui_printf(char far *fmt, ...);
void far text_scroll(void);
void far flush_line(void);
void far read_pair(char far *out);
char far text_next_char(void);

/* Initialised data of the module (DS:6502...) */
int LineCount = 0;                       /* 6502 the lines since the last "more" */
int WinLines = 25;                       /* 6504 */
int WinCols = 40;                        /* 6506 */
char far *MoreText = "<MORE>";           /* 6508 */
int MoreLen = 7;                         /* 650C */
int MoreA = 0;                           /* 650E */
int MoreB = 0;                           /* 6510 */

/* 31B0:0006 [asm] - the default "more": text at the end of the last line,
 * wait for a key, then erase it */
void far more_prompt(char far *s, int len, register int c1, register int c2)
{
	int x, y;

	x = (WinX + WinW) * 8 - (len - 1) * CharWidth;
	y = WinY + (CursorY - 1) * LineHeight;
	Drv_65F2(s, x, y, c1 ? c1 : Dims[GuiStyle].b, c2 ? c2 : Dims[GuiStyle].a);
	if (!Var_62A5)
		mouse_free();
	flush_keys();
	get_key();
	if (!Var_62A5)
		mouse_in_window(GuiStyle);
	Drv_65F2(s, x, y, TextBg, TextBg);
}

/* 31B0:00D3 [asm] - the text of the "more" (0: "<MORE>") */
void far set_more(char far *text, int len, int a, int b)
{
	if (text) {
		MoreText = text;
		MoreLen = len;
		MoreA = a;
		MoreB = b;
	} else {
		MoreText = "<MORE>";
		MoreLen = 7;
		MoreA = MoreB = 0;
	}
}

/* 31B0:0119 [asm] */
void far more_on(void)
{
	MorePrompt = 1;
	LineCount = 0;
}

/* 31B0:0129 [asm] */
void far more_off(void)
{
	MorePrompt = 0;
}

/* 31B0:0133 [asm] - returns the previous window */
int far select_window(register int w)
{
	int old;
	int far *p;

	old = GuiStyle;
	GuiStyle = w;
	p = (int far *)&Dims[w];
	WinX = *p++;
	WinY = *p++;
	WinW = *p++;
	WinH = *p++;
	TextColor = *p++;
	TextBg = *p++;
	CursorX = *p++;
	CursorY = *p++;
	LineCount = 0;
	WinLines = WinH / LineHeight;
	WinCols = WinW * 8 / CharWidth;
	return old;
}

/* 31B0:01F2 [asm] */
int far select_window_clear(int w)
{
	int old;

	old = select_window(w);
	gui_clear();
	return old;
}

/* 31B0:020F [asm] */
void far gui_clear(void)
{
	int x1, y1, x2, y2;

	x1 = WinX * 8;
	y1 = WinY;
	x2 = (WinX + WinW) * 8;
	y2 = WinY + WinH;
	Drv_657A(x1, y1, x2 - 1, y2 - 1, TextBg);
	CursorX = 0;
	CursorY = 0;
	LineCount = 0;
}

/* 31B0:026F [asm] */
void far gui_print_number(int n)
{
	gui_printf("%d", n);
}

/* 31B0:0284 [asm] */
void far gui_printf(char far *fmt, ...)
{
	char c;
	char buf[10];
	int fg, bg;
	va_list ap;
	register int end;

	va_start(ap, fmt);
	LineLen = 0;
	Line[0] = 0;
	Pend[0] = Pend[1] = 0;
	FmtText = fmt;
	SubText = 0;
	fg = TextColor;
	bg = TextBg;
	if (!Var_62A5)
		mouse_in_window(GuiStyle);
	for (;;) {
		if (!(c = text_next_char()))
			break;
		if (!SubText && c == '%')
			switch (tolower(Pend[0])) {
			case 's':
				SubText = va_arg(ap, char far *);
				if (SubText) {
					PendSaved = Pend[1];
					Pend[0] = Pend[1] = 0;
					c = text_next_char();
				}
				break;
			case 'd':
				SubText = buf;
				sprintf(buf, "%d", va_arg(ap, int));
				PendSaved = Pend[1];
				Pend[0] = Pend[1] = 0;
				c = text_next_char();
			}
		switch (c) {
		case 3: case 4: case 5: case 7: case 10: case 11: case 14: case 15:
		case 16: case 17: case 18: case 20: case 21: case 22: case 23:
		case 26: case 27: case 28: case 29: case 30: case 31:
			break;
		case 24:
			flush_line();
			CursorX = text_next_char();
			CursorX = CursorX < 0 ? 0 : CursorX;
			CursorX = WinCols - 1 < CursorX ? WinCols - 1 : CursorX;
			break;
		case 25:
			flush_line();
			CursorY = text_next_char();
			CursorY = CursorY < 0 ? 0 : CursorY;
			CursorY = WinLines - 1 < CursorY ? WinLines - 1 : CursorY;
			break;
		case 1:
			flush_line();
			if (MoreFunc) {
				if (!Var_62A5)
					mouse_free();
				MoreFunc(MoreText, MoreLen, MoreA, MoreB);
				if (!Var_62A5)
					mouse_in_window(GuiStyle);
			}
			break;
		case 12:
			gui_clear();
			break;
		case 13:
			flush_line();
			LineCount++;
			CursorX = 0;
			CursorY++;
			break;
		case 2:
			flush_line();
			TextBg = text_next_char();
			break;
		case 6:
			flush_line();
			TextColor = text_next_char();
			break;
		case 9:
			flush_line();
			CursorX = ((CursorX + LineLen + 8) & 0xFFF8) - 1;
			if (CursorX + LineLen >= WinCols)
				CursorX = 0;
			break;
		case 19:
			flush_line();
			end = ((CursorX + LineLen + 8) & 0xFFF8) - 1;
			while (CursorX + LineLen < end) {
				Line[LineLen] = ' ';
				LineLen++;
			}
			Line[LineLen] = 0;
			break;
		default:
			Line[LineLen] = c;
			LineLen++;
			Line[LineLen] = 0;
			if (CursorX + LineLen > WinCols + 1)
				flush_line();
		}
	}
	if (LineLen)
		flush_line();
	Dims[GuiStyle].c = CursorX;
	Dims[GuiStyle].d = CursorY;
	Dims[GuiStyle].a = TextColor = fg;
	Dims[GuiStyle].b = TextBg = bg;
	if (!Var_62A5)
		mouse_free();
}

/* 31B0:05D8 [asm] - the window goes up one line ("more" first, if due) */
void far text_scroll(void)
{
	int y;

	if (WinLines - 1 <= LineCount && MorePrompt) {
		LineCount = 0;
		if (MoreFunc) {
			if (!Var_62A5)
				mouse_free();
			MoreFunc(MoreText, MoreLen, MoreA, MoreB);
			if (!Var_62A5)
				mouse_in_window(GuiStyle);
		}
	}
	y = (WinH / LineHeight - 1) * LineHeight;
	copy_page(WinX, WinY + LineHeight, WinX, WinY, WinW, WinH - LineHeight, Var_62A5, Var_62A5);
	Drv_657A(WinX * 8, WinY + y, (WinX + WinW) * 8 - 1, WinY + WinH - 1, TextBg);
}

/* 31B0:06BE [asm] - print the line made so far, up to the last space
 * that fits; the rest goes on the next line */
void far flush_line(void)
{
	int width, x, y;
	char save;
	register int n, i;

	while (CursorY >= WinLines) {
		text_scroll();
		if (CursorY >= WinLines)
			CursorY--;
	}
	x = WinX * 8 + CharWidth * CursorX;
	y = WinY + LineHeight * CursorY;
	width = WinCols;
	if (WinLines - 3 <= LineCount && MorePrompt)
		width -= MoreLen;
	n = LineLen;
	if (CursorX + LineLen > width) {
		width -= CursorX;
		for (i = width; i > 0; i--)
			if (Line[i] == ' ') {
				n = i;
				break;
			}
		if (!i) {
			if (CursorX)
				n = 0;
			else
				n = width;
		}
	}
	save = Line[n];
	Line[n] = 0;
	Drv_65F2(Line, x, y, TextColor, TextBg);
	CursorX += n;
	Line[n] = save;
	if (save == ' ')
		n++;
	if (Line[n] == ' ')
		n++;
	strcpy(Line, Line + n);
	LineLen = strlen(Line);
	if (LineLen || CursorX >= WinCols) {
		LineCount++;
		CursorX = 0;
		CursorY++;
		flush_line();
	}
}

/* 31B0:07F3 [asm] - the next byte of the text (the %s or %d first): two
 * characters, the second one 0 unless the byte was a pair */
void far read_pair(char far *out)
{
	register char c, d;

	c = d = 0;
	if (SubText) {
		if (*SubText == 0) {
			SubText = 0;
			c = PendSaved;
		} else {
			c = *SubText;
			SubText++;
		}
	}
	if (!c && FmtText) {
		if (*FmtText == 0)
			FmtText = 0;
		else {
			c = *FmtText;
			FmtText++;
		}
	}
	if (c & 0x80) {
		c &= 0x7F;
		d = c & 7;
		c = (c & 0x78) >> 3;
		d = PairNext[c * 8 + d];
		c = PairFirst[c];
	}
	*out = c;
	out++;
	*out = d;
}

/* 31B0:08B4 [asm] */
char far text_next_char(void)
{
	char c;

	if (!Pend[0])
		read_pair(Pend);
	c = Pend[0];
	Pend[0] = Pend[1];
	Pend[1] = 0;
	if (!Pend[0])
		read_pair(Pend);
	return c;
}
