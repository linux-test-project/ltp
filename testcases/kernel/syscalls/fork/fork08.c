// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Check that the parent's file descriptors will not be affected by being
 * closed in the child.
 */

#include <stdlib.h>

#include "tst_test.h"

#define TESTFILE "testfile_fork08"

static int fd;
static char buf;

static void run(void)
{
	int ret;

	fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	if (!SAFE_FORK()) {
		SAFE_CLOSE(fd);
		exit(0);
	}

	if (!SAFE_FORK()) {
		SAFE_READ(1, fd, &buf, 1);
		if (buf != 'a')
			tst_res(TFAIL, "%6d: read '%c' instead of 'a'",
				getpid(), buf);
		SAFE_CLOSE(fd);
		exit(0);
	}
	tst_reap_children();

	ret = read(fd, &buf, 1);
	if (ret == 0)
		tst_res(TPASS, "read the end of file correctly");
	else
		tst_res(TFAIL | TERRNO, "read() returns %d, expected 0", ret);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	tst_fill_file(TESTFILE, 'a', 1, 1);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
	.test_all = run,
};
