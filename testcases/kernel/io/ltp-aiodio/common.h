// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef AIODIO_COMMON_H__
#define AIODIO_COMMON_H__

#include <stdlib.h>
#include "tst_test.h"

static inline char *check_zero(char *buf, int size)
{
	char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			tst_res(TINFO,
				"non zero buffer at buf[%lu] => 0x%02x,%02x,%02x,%02x",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			tst_res(TINFO, "buf %p, p %p", buf, p);
			return buf;
		}
		buf++;
		size--;
	}

	return 0;
}

static inline void io_append(const char *path, char pattern, int flags, size_t bs, size_t bcount)
{
	int fd;
	size_t i;
	char *bufptr;

	bufptr = SAFE_MEMALIGN(getpagesize(), bs);
	memset(bufptr, pattern, bs);

	fd = SAFE_OPEN(path, flags, 0666);

	for (i = 0; i < bcount; i++) {
		SAFE_WRITE(1, fd, bufptr, bs);

		if (!tst_remaining_runtime())
			break;
	}

	free(bufptr);
	SAFE_CLOSE(fd);
}

static inline void io_read(const char *filename, int filesize, volatile int *run_child)
{
	char buff[4096];
	int fd;
	int i;
	int r;

	while ((fd = open(filename, O_RDONLY, 0666)) < 0)
		usleep(100);

	tst_res(TINFO, "child %i reading file", getpid());

	for (;;) {
		off_t offset = 0;
		char *bufoff;

		SAFE_LSEEK(fd, SEEK_SET, 0);

		for (i = 0; i < filesize + 1; i += sizeof(buff)) {
			r = SAFE_READ(0, fd, buff, sizeof(buff));
			if (r > 0) {
				bufoff = check_zero(buff, r);
				if (bufoff) {
					tst_res(TINFO, "non-zero read at offset %zu",
						offset + (bufoff - buff));
					break;
				}
				offset += r;
			}

			if (!*run_child)
				goto exit;
		}
	}

exit:
	SAFE_CLOSE(fd);
}

static inline void io_read_eof(const char *filename, volatile int *run_child)
{
	char buff[4096];
	int fd;
	int r;

	while ((fd = open(filename, O_RDONLY, 0666)) < 0)
		usleep(100);

	tst_res(TINFO, "child %i reading file", getpid());

	while (*run_child) {
		off_t offset;
		char *bufoff;

		offset = SAFE_LSEEK(fd, SEEK_END, 0);

		r = SAFE_READ(0, fd, buff, sizeof(buff));
		if (r > 0) {
			bufoff = check_zero(buff, r);
			if (bufoff) {
				tst_res(TINFO, "non-zero read at offset %p", offset + bufoff);
				break;
			}
		}
	}

	SAFE_CLOSE(fd);
}

/*
 * This code tries to create dirty free blocks on
 * the HDD so there is a chance that blocks to be allocated
 * for a file are filled with something else than zeroes.
 *
 * The usefulness of this is IMHO questionable.
 */
static inline void dirty_freeblocks(int size)
{
	char *filename = "dirty_file";
	int fd;
	void *p;
	int pg;

	pg = getpagesize();
	size = LTP_ALIGN(size, pg);

	fd = SAFE_OPEN(filename, O_CREAT | O_RDWR, 0600);
	SAFE_FTRUNCATE(fd, size);

	p = SAFE_MMAP(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_FILE, fd, 0);
	memset(p, 0xaa, size);
	msync(p, size, MS_SYNC);
	munmap(p, size);

	SAFE_CLOSE(fd);
	SAFE_UNLINK(filename);
}

#endif /* AIODIO_COMMON_H__ */
