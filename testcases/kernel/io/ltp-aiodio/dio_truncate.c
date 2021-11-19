// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *				 2004 Open Source Development Lab
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test is mixing direct I/O and truncate operations checking if they can
 * be used together at the same time. Multiple children are spawned to read a
 * file that is written to using direct I/O and truncated in a loop.
 *
 * [Algorithm]
 *
 * - Spawn multiple children which start to read on 'file'
 * - Parent start to fill and truncate 'file' many times with zero char when
 *   children are reading
 * - Parent start to fill and truncate a junk file many times with non-zero char
 *
 * If no issues occur on direct IO/truncate operations and the file always
 * contains zero characters, test PASS. Otherwise, test will FAIL.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "tst_test.h"

#define NUM_CHILDREN 16
#define FILE_SIZE (64 * 1024)

static int *run_child;

static char *check_zero(char *buf, int size)
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

static void dio_read(const char *filename, size_t bs)
{
	int fd;
	int r;
	char *bufptr;

	bufptr = SAFE_MEMALIGN(getpagesize(), bs);

	while ((fd = open(filename, O_RDONLY | O_DIRECT, 0666)) < 0)
		usleep(100);

	tst_res(TINFO, "child %i reading file", getpid());
	while (*run_child) {
		off_t offset;
		char *bufoff;

		offset = SAFE_LSEEK(fd, SEEK_SET, 0);
		do {
			r = read(fd, bufptr, 64 * 1024);
			if (r > 0) {
				bufoff = check_zero(bufptr, r);
				if (bufoff) {
					tst_res(TINFO, "non-zero read at offset %zu",
						offset + (bufoff - bufptr));
					free(bufptr);
					SAFE_CLOSE(fd);
					return;
				}
				offset += r;
			}
		} while (r > 0);
	}

	free(bufptr);
	SAFE_CLOSE(fd);
}

static void dio_append(const char *path, char pattern, size_t bs, size_t bcount)
{
	int fd;
	size_t i;
	char *bufptr;

	bufptr = SAFE_MEMALIGN(getpagesize(), bs);
	memset(bufptr, pattern, bs);

	fd = SAFE_OPEN(path, O_CREAT | O_WRONLY | O_DIRECT, 0666);

	for (i = 0; i < bcount; i++)
		SAFE_WRITE(1, fd, bufptr, bs);

	free(bufptr);
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	run_child = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(run_child, sizeof(int));
}

static void run(void)
{
	char *filename = "file";
	int filesize = FILE_SIZE;
	int num_children = NUM_CHILDREN;
	int status;
	int i;
	int fail = 0;

	*run_child = 1;

	for (i = 0; i < num_children; i++) {
		if (!SAFE_FORK()) {
			dio_read(filename, filesize);
			return;
		}
	}

	tst_res(TINFO, "parent writes/truncates the file");

	for (i = 0; i < 100; i++) {
		dio_append(filename, 0, filesize, 100);
		SAFE_TRUNCATE(filename, 0);
		dio_append("junkfile", 0xaa, filesize, 100);
		SAFE_TRUNCATE("junkfile", 0);

		if (SAFE_WAITPID(-1, &status, WNOHANG)) {
			fail = 1;
			break;
		}
	}

	if (fail)
		tst_res(TFAIL, "Non zero bytes read");
	else
		tst_res(TPASS, "All bytes read were zeroed");

	*run_child = 0;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
