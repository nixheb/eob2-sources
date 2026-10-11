/*
 * Eye of the Beholder II - START.EXE
 * The files of the Westwood library: open, read, write, seek, files kept
 * in memory ("resident"), files inside a library file, the disks:
 * module 1DCE (Ghidra 2DCE).
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y files.c
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * A handle (0-9) gives FileHandles[]: a DOS handle, or 0x8000 | the index
 * in FileData[] of a resident file; 0x4000: a file inside a library
 * (FileStart, FileSize: where); 0x3FFF: free.
 */
#include "eob2.h"

typedef unsigned char byte;

/* Borland's dos.h */
#define MK_FP( seg,ofs )( (void _seg * )( seg ) +( void near * )( ofs ))
#define FP_SEG( fp )( (unsigned )( void _seg * )( void far * )( fp ))
#define FP_OFF( fp )( (unsigned )( fp ))

/* A file the game knows: FileData[] (37D8:0000, far data of the data module) */
typedef struct FileEntry {
	char far     *name;          /* +00 ("" ends the table) */
	long          size;          /* +04 */
	byte far     *data;          /* +08 the file in memory, 0 if not resident */
	int           link;          /* +0C > 0: the disk; < 0: inside the library 0x7FFF & link */
	long          pos;           /* +0E the position, when resident */
	byte          flags;         /* +12 0x80 loaded at start, 0x40 loaded when opened,
	                                    0x3F: the priority (freed first when low) */
} FileEntry;                     /* 19 bytes */

/* ------------------------------------------------------------------ */
/* Globals (DGROUP)                                                    */
/* ------------------------------------------------------------------ */
extern FileEntry far  FileData[];        /* 37D8:0000 */
/* Uninitialised data: in bssefda.c (DS:EFDA-F04B) */
extern long FileSize[10];        /* EFDA */
extern char CurDrive;            /* F002 the drive at start (BSS) */
extern char Pad_F003;            /* F003 the alignment of the _BSS */
extern long FileStart[10];       /* F004 where the file starts in its library */
extern unsigned FileHandles[10]; /* F02C */
extern int StartDrive;           /* F040 the drive at start */
extern char LastDrive;           /* F042 */
extern char Pad_F043;            /* F043 the alignment of the _BSS */
extern long LibSize;             /* F044 of the file found by lib_find */
extern long LibStart;            /* F048 */
extern int (far      *FileErrorHandler)(int err, char far *name);   /* 6294 */
extern int (far      *FileErrorPrompt)(int err, char far *name);    /* 62A1 */

/* ------------------------------------------------------------------ */
/* Routines from other modules                                         */
/* ------------------------------------------------------------------ */
int  chsize(int handle, long size);
char far *strchr(char far *s, int c);
char far *strupr(char far *s);
char far *strcat(char far *dst, char far *src);
int  strcmp(char far *a, char far *b);
int  open(char far *path, int access, ...);
int  close(int handle);
int  read(int handle, void far *buf, unsigned len);
int  write(int handle, void far *buf, unsigned len);
long lseek(int handle, long offset, int from);
long filelength(int handle);
int  unlink(char far *path);
int  getdisk(void);
int  setdisk(int drive);
void exit(int status);
void system_shutdown(void);                                      /* 315F:0000 */
void far *normalize(void far *p);                                /* 3374:0071 */
void mem_copy(void far *src, void far *dst, long n);             /* 3374:0004 */
void far *mem_alloc(unsigned long size, int flags);              /* 2D0E:000B */
void mem_free(void far *p);                                      /* 2D0E:013F */
long mem_free_total(void);                                       /* 2D0E:0249 */
int  drive_ready(int drive);                                     /* 3263:0037 */
int far is_resident(char far *name);
int far find_resident(char far *name);
long far read_resident(int idx, void far *buf, long n);
int far file_exists(char far *name);
int far load_resident(int i);
int far find_entry(char far *name);
long far lib_find(int fd, char far *name);
int far free_cache(int size, char level);
int far cache_victim(int size, char level);
void far flush_cache(void);
void far init_files(void);
int far preload_file(char far *name);
int far file_on_disk(char far *name);
int far delete_file(char far *name);
int far create_file(char far *name);

/* ------------------------------------------------------------------ */
/* Module 2DCE, in address order                                       */
/* ------------------------------------------------------------------ */

/* Initialised data of the module (DS:61F4) */
char SearchDrives = 1;                   /* 61F4 look for a file on every drive */

/* 2DCE:0005 [asm] - the DOS handle, -1 if none (or resident) */
int far file_handle(register int fd)
{
	register unsigned h;

	if (fd != -1) {
		h = FileHandles[fd];
		if (!(h & 0x8000))
			return h;
	}
	return -1;
}

/* 2DCE:002B [asm] */
int far file_resize(int fd, long size)
{
	register unsigned h;

	h = FileHandles[fd];
	if (!(h & 0x8000))
		return chsize(h & 0x3FFF, size) != -1;
	return 0;
}

/* 2DCE:0065 [asm] - ask for the disk of a file (the handler of main()).
 * ORIGINAL BUG: meant to keep only the name after the last '\', it never
 * moves past the '\' it finds: a name with a path would loop forever (the
 * game never gives one) */
int far ask_disk(int err, char far *name)
{
	char far *p, far *q;

	q = p = name;
	while ((q = strchr(p, '\\')) != 0)
		p = q;
	strupr(p);
	return FileErrorPrompt(err, p);
}

/* 2DCE:00C9 [asm] - a fatal file error: message, then back to DOS */
void far file_fatal(register int err, char far *name)
{
	if (err != 12)
		FileErrorHandler(err, name);
	FileErrorHandler(12, name);
	system_shutdown();
	exit(5);
}

/* 2DCE:0107 [asm] - mode 1: read, 2: write (a new file), 3: read and write */
int far open_file(char far *name, int mode)
{
	int access, slot, entry;
	char path[50];
	register int i, h;

	path[0] = 0;
	strcat(path, name);
	switch (mode) {
	case 1:
		if (!is_resident(name)) {
			while (!file_on_disk(path))
				if (!ask_disk(2, name))
					return -1;
			preload_file(name);
		}
		access = 0x8001;
		break;
	case 2:
		access = 0x8002;
		delete_file(path);
		create_file(path);
	case 3:
		if (!file_on_disk(path))
			while (!create_file(path))
				if (!ask_disk(0, path))
					return -1;
		if (mode == 3)
			access = 0x8004;
		break;
	default:
		ask_disk(1, name);
		file_fatal(12, name);
		system_shutdown();
		exit(5);
	}
	if (is_resident(name) && mode != 1) {
		slot = find_resident(name);
		mem_free(FileData[slot].data);
		FileData[slot].data = 0;
	}
	if (!is_resident(name)) {
		if ((entry = find_entry(name)) != -1) {
			if (FileData[entry].link < 0)
				name = FileData[FileData[entry].link & 0x7FFF].name;
			else
				entry = -1;
		}
		h = open(name, access, 0x180);
		while (h == -1) {
			h = open(name, access, 0x180);
			if (h == -1 && !ask_disk(2, name))
				return -1;
		}
	} else {
		h = find_resident(name);
		FileData[h].pos = 0;
		h = h | 0x8000;
	}
	slot = 10;
	for (i = 0; i < 10; i++)
		if (FileHandles[i] == 0x3FFF) {
			slot = i;
			break;
		}
	if (slot == 10)
		file_fatal(3, name);
	FileHandles[slot] = h;
	FileStart[slot] = LibStart;
	FileSize[slot] = LibSize;
	return slot;
}

/* 2DCE:0406 [asm] */
void far close_file(register int fd)
{
	register unsigned h;

	h = FileHandles[fd];
	if (h == 0x3FFF)
		file_fatal(4, 0L);
	if (!(h & 0x8000))
		close(h & 0x3FFF);
	FileHandles[fd] = 0x3FFF;
}

/* 2DCE:044E [asm] - by blocks of 32000 bytes; returns the size read, 0
 * on an error */
long far read_file(register int fd, void far *buf, unsigned long n)
{
	long total, left, pos, got;
	register unsigned h;

	h = FileHandles[fd];
	if (h == 0x3FFF)
		file_fatal(5, 0L);
	if (h & 0x8000)
		return read_resident(h & 0x3FFF, buf, n);
	if (h & 0x4000) {
		pos = lseek(h & 0x3FFF, 0L, 1);
		pos -= FileStart[fd];
		left = FileSize[fd] - pos;
		if (n > left)
			n = left;
	}
	h &= 0x3FFF;
	total = 0;
	while (n > 32000) {
		buf = normalize(buf);
		got = read(h, buf, 32000);
		n -= 32000;
		if ((int)got == -1)
			return 0;
		total = total + got;
		buf = MK_FP(FP_SEG(buf), FP_OFF(buf) + got);
	}
	buf = normalize(buf);
	got = read(h, buf, n);
	if ((int)got == -1)
		return 0;
	return total + got;
}

/* 2DCE:05DE [asm] */
long far write_file(int fd, void far *buf, unsigned long n)
{
	long total, pos, left;
	register unsigned h, w;

	h = FileHandles[fd];
	if (h == 0x3FFF)
		file_fatal(6, 0L);
	if (h & 0x8000)
		file_fatal(9, 0L);
	if (h & 0x4000) {
		pos = lseek(h & 0x3FFF, 0L, 1);
		pos -= FileStart[fd];
		left = FileSize[fd] - pos;
		if (n > left)
			n = left;
	}
	h &= 0x3FFF;
	total = 0;
	while (n > 32000) {
		buf = normalize(buf);
		w = write(h, buf, 32000);
		if (w != 32000)
			return 0;
		n -= 32000;
		total += 32000;
		buf = MK_FP(FP_SEG(buf), FP_OFF(buf) + 32000);
	}
	buf = normalize(buf);
	w = write(h, buf, n);
	if (w != n)
		return 0;
	return total + n;
}

/* 2DCE:0750 [asm] - from 0: the start, 1: here, 2: the end */
long far seek_file(int fd, long pos, int from)
{
	long newpos, r;
	register unsigned h;
	register int idx;

	h = FileHandles[fd];
	if (h == 0x3FFF)
		file_fatal(7, 0L);
	if (h & 0x8000) {
		idx = h & 0x3FFF;
		newpos = FileData[idx].pos;
		switch (from) {
		case 0:
			if (FileData[idx].size < pos)
				newpos = FileData[idx].size;
			else
				newpos = pos;
			break;
		case 2:
			newpos = FileData[idx].size;
		case 1:
			if (FileData[idx].size < newpos + pos)
				newpos = FileData[idx].size;
			else if (newpos + pos < 0)
				newpos = 0;
			else
				newpos += pos;
			break;
		default:
			file_fatal(8, 0L);
		}
		FileData[idx].pos = newpos;
		return newpos;
	} else {
		switch (from) {
		case 0:
			if (h & 0x4000)
				pos += FileStart[fd];
			r = lseek(h & 0x3FFF, pos, from);
			break;
		case 1:
			r = lseek(h & 0x3FFF, pos, from);
			break;
		case 2:
			if (h & 0x4000) {
				pos -= FileStart[fd];
				pos += FileSize[fd];
			}
			r = lseek(h & 0x3FFF, pos, from);
			break;
		default:
			file_fatal(8, 0L);
		}
		return r;
	}
}

/* 2DCE:09B6 [asm] - is the file there (on some drive), whole? */
int far file_on_disk(char far *name)
{
	register int entry, fd;

	entry = -1;
	name = strupr(name);
	entry = find_entry(name);
	if (entry != -1 && FileData[entry].link < 0)
		name = FileData[FileData[entry].link & 0x7FFF].name;
	if (!file_exists(name))
		return 0;
	fd = open(name, 0x8001);
	if (fd == -1)
		file_fatal(15, name);
	if (entry == -1) {
		close(fd);
		return 1;
	}
	if (FileData[entry].link > 0 && filelength(fd) != FileData[entry].size && FileData[entry].size) {
		close(fd);
		file_fatal(14, name);
		return 0;
	}
	close(fd);
	return 1;
}

/* 2DCE:0AF9 [asm] */
int far delete_file(char far *name)
{
	register int i;

	if (is_resident(name)) {
		i = find_resident(name);
		mem_free(FileData[i].data);
		FileData[i].data = 0;
	}
	return !unlink(name);
}

/* 2DCE:0B6F [asm] */
int far create_file(char far *name)
{
	register int fd;

	delete_file(name);
	fd = open(name, 0x8102, 0x180);
	if (fd == -1)
		return 0;
	close(fd);
	return 1;
}

/* 2DCE:0BAF [asm] - the resident file to free for size bytes: the one of
 * lowest priority, not above level */
int far cache_victim(int size, char level)
{
	register int i, best;
	int n;

	best = -1;
	i = 0;
	for (n = 0; *FileData[n].name; n++)
		;
	for (i = 0; i < n; i++)
		if (FileData[i].data && FileData[i].size >= size && FileData[i].flags <= level)
			if (best == -1)
				best = i;
			else if ((char)FileData[i].flags < (char)FileData[best].flags)
				best = i;
	return best;
}

/* 2DCE:0C71 [asm] - free a resident file to make room.
 * ORIGINAL BUG: to keep an open file, it compares each handle with the
 * index of the loop instead of the file chosen (victim) */
int far free_cache(int size, char level)
{
	char used;
	register int i = 0, victim;

	victim = cache_victim(size, level);
	if (victim == -1)
		return 0;
	used = 0;
	for (i = 0; i < 10; i++)
		if ((FileHandles[i] & 0x3FFF) == i) {
			used = 1;
			i = 10;
		}
	if (!used && i == 10) {
		mem_free(FileData[victim].data);
		FileData[victim].data = 0;
		return 1;
	}
	return 0;
}

/* 2DCE:0D0B [asm] - at the end: free the resident files; complain about
 * the files still open (sic: with a null name) */
void far flush_cache(void)
{
	register int i;

	for (i = 0; *FileData[i].name; i++)
		if (FileData[i].data) {
			mem_free(FileData[i].data);
			FileData[i].data = 0;
		}
	for (i = 0; i < 10; i++)
		if (FileHandles[i] != 0x3FFF)
			ask_disk(11, 0L);
}

/* 2DCE:0DAD [asm] - at start: load the files marked 0x80, the highest
 * priority first (a bubble sort) */
void far init_files(void)
{
	int count;
	int far *order;
	byte far *levels;
	int j, k, last, t1, t2;
	register int n;
	int i;

	count = 0;
	for (n = 0; n < 10; n++)
		FileHandles[n] = 0x3FFF;
	for (n = 0; *FileData[n].name; n++) {
		strupr(FileData[n].name);
		FileData[n].data = 0;
		if (FileData[n].flags & 0x80)
			count++;
	}
	if (!count)
		return;
	order = mem_alloc(count * 2, 0);
	levels = mem_alloc(count, 0);
	for (j = k = 0; j < n; j++)
		if (FileData[j].flags & 0x80) {
			order[k] = j;
			levels[k] = FileData[j].flags & 0x3F;
			k++;
		}
	for (last = count - 1; last >= 0; last--)
		for (i = 0; i < last; i++)
			if (levels[i] < levels[i + 1]) {
				t1 = levels[i];
				t2 = order[i];
				levels[i] = levels[i + 1];
				order[i] = *(order + i + 1);
				levels[i + 1] = t1;
				*(order + i + 1) = t2;
			}
	for (j = 0; j < count; j++) {
		if (order[j] < 0 || order[j] >= n)
			file_fatal(17, 0L);
		load_resident(order[j]);
	}
	mem_free(levels);
	mem_free(order);
}

/* 2DCE:0FC6 [asm] */
int far is_resident(char far *name)
{
	register int i = 0;

	name = strupr(name);
	for (; *FileData[i].name; i++)
		if (!strcmp(FileData[i].name, name)) {
			if (FileData[i].data)
				return 1;
			return 0;
		}
	return 0;
}

/* 2DCE:104C [asm] */
int far find_resident(char far *name)
{
	register int i = 0;

	name = strupr(name);
	for (; *FileData[i].name; i++)
		if (!strcmp(FileData[i].name, name))
			return i;
	ask_disk(10, name);
	return 0;
}

/* 2DCE:10C6 [asm] */
long far read_resident(register int idx, void far *buf, long n)
{
	void far *src;

	if (FileData[idx].pos + n > FileData[idx].size)
		n = FileData[idx].size - FileData[idx].pos;
	src = normalize(FileData[idx].data + FileData[idx].pos);
	mem_copy(src, buf, n);
	FileData[idx].pos += n;
	return n;
}

/* 2DCE:11CF [asm] - the drive (1 = A) where the file is, 0 if none */
int far file_exists(char far *name)
{
	int drive;
	register int d, fd;

	drive = getdisk();
	name = strupr(name);
	fd = open(name, 0x8001);
	if (fd != -1) {
		close(fd);
		return drive + 1;
	}
	if (SearchDrives == 0)
		return 0;
	for (d = LastDrive; d != -1; d--)
		if (drive_ready(d)) {
			setdisk(d);
			fd = open(name, 0x8001);
			if (fd != -1) {
				close(fd);
				return d + 1;
			}
		}
	setdisk(drive);
	return 0;
}

/* 2DCE:127C [asm] */
void far files_init(void)
{
	StartDrive = getdisk();
	init_files();
}

/* 2DCE:128D [asm] */
void far files_shutdown(void)
{
	flush_cache();
	setdisk(StartDrive);
}

/* 2DCE:12A1 [asm] - a file marked 0x40 is loaded when opened */
int far preload_file(char far *name)
{
	register int i;

	i = find_entry(name);
	if (i == -1)
		return 0;
	if (FileData[i].flags & 0x40) {
		load_resident(i);
		return 1;
	}
	return 0;
}

/* 2DCE:12E4 [asm] - load a file in memory */
int far load_resident(register int i)
{
	byte far *buf;
	FileEntry far *e;
	FileEntry far *lib;
	register int fd;

	buf = 0;
	e = &FileData[i];
	if (e->size <= 0) {
		file_fatal(16, e->name);
		return 0;
	}
	if (mem_free_total() >= e->size)
		buf = mem_alloc(e->size, 0);
	else
		buf = 0;
	if (!buf) {
		if (free_cache(e->size, e->flags))
			buf = mem_alloc(e->size, 0);
		else
			return 0;
	}
	if (e->link > 0)
		fd = open(e->name, 0x8001, 0x180);
	else {
		lib = &FileData[FileData[i].link & 0x7FFF];
		fd = open(lib->name, 0x8001, 0x180);
		e->size = lib_find(fd, e->name);
	}
	if (fd == -1) {
		mem_free(buf);
		return 0;
	}
	if (e->link > 0 && filelength(fd) != e->size) {
		file_fatal(14, e->name);
		return 0;
	}
	if (read(fd, buf, e->size) == e->size) {
		e->data = buf;
		close(fd);
	} else {
		mem_free(buf);
		return 0;
	}
	return 1;
}

/* 2DCE:14E8 [asm] */
int far find_entry(char far *name)
{
	register int i;

	for (i = 0; *FileData[i].name; i++)
		if (!strcmp(FileData[i].name, name))
			return i;
	return -1;
}

/* 2DCE:153E [asm] - the disk of a file (following the libraries), -1 */
int far file_disk(char far *name)
{
	register int i = 0;

	name = strupr(name);
	for (; *FileData[i].name; i++)
		if (!strcmp(FileData[i].name, name)) {
			while (FileData[i].link < 0)
				i = FileData[i].link & 0x7FFF;
			return FileData[i].link;
		}
	return -1;
}

/* 2DCE:15EC [asm] */
void far drives_search_off(void)
{
	SearchDrives = 0;
}

/* 2DCE:15F6 [asm] */
void far drives_search_on(void)
{
	SearchDrives = 1;
}

/* 2DCE:1600 [asm] - find a file in a library: entries (long offset,
 * name); the next offset gives the size. Returns the size */
long far lib_find(register int h, char far *name)
{
	long start, next;
	char entry[14];
	register int i;

	h &= 0x3FFF;
	do {
		read(h, &start, 4);
		i = 0;
		do
			read(h, entry + i, 1);
		while (entry[i++]);
	} while (strcmp(entry, name));
	read(h, &next, 4);
	lseek(h, start, 0);
	next -= start;
	LibSize = next;
	LibStart = start;
	return next;
}
