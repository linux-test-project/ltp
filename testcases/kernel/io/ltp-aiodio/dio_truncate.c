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
#include "common.h"

static volatile int *run_child;

static char *str_numchildren;
static char *str_filesize;
static char *str_numappends;
static char *str_numwrites;

static int numchildren = 16;
static long long filesize = 64 * 1024;
static long long alignment;
static int numappends = 100;
static int numwrites = 100;

static void dio_read(const char *filename, long long align, size_t bs)
{
	int fd;
	int r;
	char *bufptr;

	while ((fd = open(filename, O_RDONLY | O_DIRECT, 0666)) < 0)
		usleep(100);

	bufptr = SAFE_MEMALIGN(align, bs);

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

static void setup(void)
{
	struct stat sb;

	if (tst_parse_int(str_numchildren, &numchildren, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of children '%s'", str_numchildren);

	if (tst_parse_filesize(str_filesize, &filesize, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid file size '%s'", str_filesize);

	if (tst_parse_int(str_numappends, &numappends, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of appends '%s'", str_numappends);

	if (tst_parse_int(str_numwrites, &numwrites, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of truncate/append '%s'", str_numwrites);

	SAFE_STAT(".", &sb);
	alignment = sb.st_blksize;

	run_child = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (run_child) {
		*run_child = 0;
		SAFE_MUNMAP((void *)run_child, sizeof(int));
	}
}

static void run(void)
{
	char *filename = "file.bin";
	int wflags = O_DIRECT | O_WRONLY | O_CREAT;
	int status;
	int i;
	int fail = 0;

	*run_child = 1;

	for (i = 0; i < numchildren; i++) {
		if (!SAFE_FORK()) {
			dio_read(filename, alignment, filesize);
			return;
		}
	}

	tst_res(TINFO, "Parent writes/truncates the file");

	for (i = 0; i < numwrites; i++) {
		io_append(filename, 0, wflags, filesize, numappends);
		SAFE_TRUNCATE(filename, 0);
		io_append("junkfile", 0xaa, wflags, filesize, numappends);
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
	.options = (struct tst_option[]) {
		{"n:", &str_numchildren, "Number of threads (default 16)"},
		{"s:", &str_filesize, "Size of file (default 64K)"},
		{"a:", &str_numappends, "Number of appends (default 100)"},
		{"c:", &str_numwrites, "Number of append & truncate (default 100)"},
		{}
	},
	.skip_filesystems = (const char *[]) {
		"tmpfs",
		NULL
	},
};
