/*
 * Eye of the Beholder II - START.EXE
 * Out of memory: module 1F39 (Ghidra 2F39).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y memerr.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

void set_text_mode(int mode);                                    /* 31A9:0033 */
void dos_puts(char far *s);                                      /* 3246:0006 */
int  get_key(void);                                              /* 3295:05D0 */
void system_shutdown(void);                                      /* 315F:0000 */
void exit(int status);
int  printf(char far *fmt, ...);

/* 2F39:0001 [asm] - the handler installed by main() */
int far out_of_memory_handler(void)
{
	set_text_mode(9);
	dos_puts("Insufficient memory error! Press any key to exit to DOS.\r\n");
	get_key();
	system_shutdown();
	exit(5);
	return 0;
}

/* 2F39:0035 [asm] */
int far alloc_error(void)
{
	printf("A memory allocation error has occured - exiting\r\n");
	system_shutdown();
	exit(5);
	return 0;
}
