// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * Fork a process that creates a file and writes a few bytes, and
 * calls exit WITHOUT calling close(). The parent then reads the
 * file.  If everything that was written is present in the file, then
 * the test passes.
 */

#include <stdlib.h>
#include "tst_test.h"

#define FNAME "test_file"

static void child_write(void)
{
	int fd;

	fd = SAFE_CREAT(FNAME, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, FNAME, sizeof(FNAME));
	exit(0);
}

static void check_file(void)
{
	int fd, len;
	char buf[256];

	fd = SAFE_OPEN(FNAME, O_RDONLY);
	len = SAFE_READ(0, fd, buf, sizeof(buf));

	if (len != sizeof(FNAME)) {
		tst_res(TFAIL, "Wrong length %i expected %zu", len, sizeof(buf));
		goto out;
	}

	if (memcmp(buf, FNAME, sizeof(FNAME))) {
		tst_res(TFAIL, "Wrong data read back");
		goto out;
	}

	tst_res(TPASS, "File written by child read back correctly");
out:
	SAFE_CLOSE(fd);
}

static void run(void)
{
	int pid;

	pid = SAFE_FORK();
	if (!pid)
		child_write();

	tst_reap_children();

	check_file();

	SAFE_UNLINK(FNAME);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.forks_child = 1,
	.test_all = run,
};
