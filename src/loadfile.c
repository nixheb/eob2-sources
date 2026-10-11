/*
 * Eye of the Beholder II - START.EXE
 * A whole file in a new block (fonts...): module 208C (Ghidra 308C).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Compiled with  bcc -c -ml -G -O -Z -y loadfile.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 */
#include "eob2.h"

typedef unsigned char byte;

int  file_exists(char far *name);                                /* 2DCE:11CF (asks for the disk) */
int  open(char far *path, int access, ...);
int  read(int handle, void far *buf, unsigned len);
int  close(int handle);
void far *mem_alloc(unsigned long size, int flags);              /* 2D0E:000B */

/* 308C:0005 [asm] - the file starts with its size (a word), which is kept
 * at the start of the block. sic: page is never used */
byte far * far load_file_alloc(char far *name, register int page)
{
	byte far *p;
	unsigned size;
	register int fd;

	p = 0;
	file_exists(name);
	fd = open(name, 0x8001);
	if (fd != -1) {
		if (read(fd, &size, 2) != 2)
			return 0;
		p = mem_alloc(size, 0x10);
		read(fd, p + 2, size);
		close(fd);
	}
	return p;
}
