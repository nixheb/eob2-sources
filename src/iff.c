/*
 * Eye of the Beholder II - START.EXE
 * IFF files ("FORM", chunks with a big-endian size): module 1F44 (Ghidra 2F44).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y iff.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A chunk: its name (4 characters), its size (big-endian long), its data,
 * padded to an even size. A null name is skipped.
 */
#include "eob2.h"

#define ID_FORM 0x4D524F46L              /* "FORM" */

int  open_file(char far *name, int mode);                        /* 2DCE:0107 */
void close_file(int fd);                                         /* 2DCE:0406 */
long read_file(int fd, void far *buf, long n);                   /* 2DCE:044E */
long seek_file(int fd, long pos, int from);                      /* 2DCE:0750 */
long swap_long(long v);                                          /* 33DF:0006 (assembler) */

/* 2F44:0008 [asm] - -1 if it is not an IFF file */
int far iff_open(char far *name)
{
	long id;
	register int fd;

	fd = open_file(name, 1);
	close_file(fd);
	fd = open_file(name, 3);
	read_file(fd, &id, 4L);
	if (id == ID_FORM)
		seek_file(fd, 4L, 1);
	else {
		close_file(fd);
		fd = -1;
	}
	return fd;
}

/* 2F44:008A [asm] */
void far iff_close(register int fd)
{
	if (fd != -1)
		close_file(fd);
}

/* 2F44:00A1 [asm] - find a chunk: its size, the file left on its name;
 * 0 if none */
long far iff_find(register int fd, long id)
{
	long cid, size;
	char first;

	first = 1;
	for (;;) {
		if (read_file(fd, &cid, 4L) != 4 && !first)
			break;
		if (!cid)
			if (read_file(fd, &cid, 4L) != 4 && !first)
				break;
		if (read_file(fd, &size, 4L) != 4 && !first)
			break;
		size = swap_long(size);
		if (id == cid) {
			seek_file(fd, -8L, 1);
			return size;
		}
		if (first) {
			seek_file(fd, 12L, 0);
			first = 0;
		} else {
			size = (size + 1) & 0xFFFFFFFEL;
			seek_file(fd, size, 1);
		}
	}
	return 0;
}

/* 2F44:01C9 [asm] - read a chunk (max bytes at most): the size read */
long far iff_read(register int fd, long id, void far *buf, long max)
{
	long cid, size;
	char first;

	first = 1;
	for (;;) {
		if (read_file(fd, &cid, 4L) != 4 && !first)
			break;
		if (!cid)
			if (read_file(fd, &cid, 4L) != 4 && !first)
				break;
		if (read_file(fd, &size, 4L) != 4 && !first)
			break;
		size = swap_long(size);
		if (id == cid) {
			max = max < size ? max : size;
			read_file(fd, buf, max);
			size = (size + 1) & 0xFFFFFFFEL;
			if (max < size)
				seek_file(fd, size - max, 1);
			return max;
		}
		if (first) {
			seek_file(fd, 12L, 0);
			first = 0;
		} else {
			size = (size + 1) & 0xFFFFFFFEL;
			seek_file(fd, size, 1);
		}
	}
	return 0;
}
