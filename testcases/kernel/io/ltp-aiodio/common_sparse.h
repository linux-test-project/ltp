/*
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef LTP_AIODIO_COMMON_SPARSE
#define LTP_AIODIO_COMMON_SPARSE

#include "common_checkzero.h"

/*
 * This code tries to create dirty free blocks on
 * the HDD so there is a chance that blocks to be allocated
 * for a file are filled with something else than zeroes.
 *
 * The usefulness of this is IMHO questionable.
 */
static void dirty_freeblocks(int size)
{
	int fd;
	void *p;
	int pg;
	char *filename = "dirty_freeblocks";

	pg = getpagesize();
	size = ((size + pg - 1) / pg) * pg;

	fd = open(filename, O_CREAT|O_RDWR|O_EXCL, 0600);

	if (fd < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "failed to open '%s'", filename);

	SAFE_FTRUNCATE(cleanup, fd, size);

	p = SAFE_MMAP(cleanup, NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_FILE, fd, 0);

	memset(p, 0xaa, size);
	msync(p, size, MS_SYNC);
	munmap(p, size);
	close(fd);
	unlink(filename);
}

/*
 * Scale value by kilo, mega, or giga.
 */
long long scale_by_kmg(long long value, char scale)
{
	switch (scale) {
	case 'g':
	case 'G':
		value *= 1024;
	case 'm':
	case 'M':
		value *= 1024;
	case 'k':
	case 'K':
		value *= 1024;
		break;
	case '\0':
		break;
	default:
		usage();
		break;
	}
	return value;
}

/*
 * Make sure we read only zeroes,
 * either there is a hole in the file,
 * or zeroes were actually written by parent.
 */
static void read_sparse(char *filename, int filesize)
{
	int fd;
	int  i, j, r;
	char buf[4096];

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		if (debug)
			fprintf(stderr, "Child %i failed to open '%s'\n",
			        getpid(), filename);
		exit(10);
	}

	if (debug)
		fprintf(stderr, "Child %i has opened '%s' for reading\n",
		        getpid(), filename);

	for (i = 0; i < 100000000; i++) {
		off_t offset = 0;
		char *badbuf;

		if (debug)
			fprintf(stderr, "Child %i loop %i\n", getpid(), i);

		lseek(fd, SEEK_SET, 0);
		for (j = 0; j < filesize+1; j += sizeof(buf)) {
			r = read(fd, buf, sizeof(buf));
			if (r > 0) {
				if ((badbuf = check_zero(buf, r))) {
					fprintf(stderr, "non-zero read at offset %u\n",
						(unsigned int)(offset + badbuf - buf));
					exit(10);
				}
			}
			offset += r;
		}
	}

	exit(0);
}

#endif /* LTP_AIODIO_COMMON_SPARSE */
