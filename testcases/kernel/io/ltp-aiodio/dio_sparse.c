// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *                 2004 Open Source Development Lab
 *
 *   Copyright (c) 2004 Marty Ridgeway <mridge@us.ibm.com>
 *
 *   Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *
 *   Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Create a sparse file using O_DIRECT while other processes are doing
 * buffered reads and check if the buffer reads always see zero.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "common.h"

static int *run_child;

static char *str_numchildren;
static char *str_writesize;
static char *str_filesize;
static char *str_offset;

static int numchildren;
static long long writesize;
static long long filesize;
static long long offset;
static long long alignment;

static void dio_sparse(int fd, int align, long long fs, int ws, long long off)
{
	void *bufptr = NULL;
	long long i;
	int w;

	bufptr = SAFE_MEMALIGN(align, ws);

	memset(bufptr, 0, ws);
	SAFE_LSEEK(fd, off, SEEK_SET);

	for (i = off; i < fs;) {
		w = SAFE_WRITE(0, fd, bufptr, ws);
		i += w;
	}
}

static void setup(void)
{
	struct stat sb;

	numchildren = 1000;
	writesize = 1024;
	filesize = 100 * 1024 * 1024;
	offset = 0;
	alignment = 512;

	if (tst_parse_int(str_numchildren, &numchildren, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of children '%s'", str_numchildren);

	if (tst_parse_filesize(str_writesize, &writesize, 1, LONG_LONG_MAX))
		tst_brk(TBROK, "Invalid write blocks size '%s'", str_writesize);

	if (tst_parse_filesize(str_filesize, &filesize, 1, LONG_LONG_MAX))
		tst_brk(TBROK, "Invalid file size '%s'", str_filesize);

	if (tst_parse_filesize(str_offset, &offset, 0, LONG_LONG_MAX))
		tst_brk(TBROK, "Invalid file offset '%s'", str_offset);

	SAFE_STAT(".", &sb);
	alignment = sb.st_blksize;

	run_child = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	tst_res(TINFO, "Dirtying free blocks");
	dirty_freeblocks(100 * 1024 * 1024);
}

static void cleanup(void)
{
	*run_child = 0;
	SAFE_MUNMAP(run_child, sizeof(int));
}

static void run(void)
{
	char *filename = "dio_sparse";
	int status;
	int fd;
	int i;

	fd = SAFE_OPEN(filename, O_DIRECT | O_WRONLY | O_CREAT, 0666);
	SAFE_FTRUNCATE(fd, filesize);

	*run_child = 1;

	for (i = 0; i < numchildren; i++) {
		if (!SAFE_FORK()) {
			io_read(filename, filesize, run_child);
			return;
		}
	}

	dio_sparse(fd, alignment, filesize, writesize, offset);

	if (SAFE_WAITPID(-1, &status, WNOHANG))
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
		{"n:", &str_numchildren, "Number of threads (default 1000)"},
		{"w:", &str_writesize, "Size of writing blocks (default 1K)"},
		{"s:", &str_filesize, "Size of file (default 100M)"},
		{"o:", &str_offset, "File offset (default 0)"},
		{}
	},
};
