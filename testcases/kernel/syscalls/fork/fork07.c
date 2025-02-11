// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 07/2002 Limited forking and split "infinite forks" testcase to fork12.c by
 * Nate Straz
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Check that all children inherit parent's file descriptor.
 *
 * Parent opens a file and forks children. Each child reads a byte and checks
 * that the value is correct. Parent checks that correct number of bytes was
 * consumed from the file.
 */

#include <stdlib.h>

#include "tst_test.h"

#define NFORKS 100
#define TESTFILE "testfile_fork07"

static int fd;
static char buf;

static void run(void)
{
	int ret, i;

	fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	for (i = 0; i < NFORKS; ++i) {
		if (!SAFE_FORK()) {
			SAFE_READ(1, fd, &buf, 1);
			if (buf != 'a')
				tst_res(TFAIL, "%6d: read '%c' instead of 'a'",
					getpid(), buf);
			exit(0);
		}
	}
	tst_reap_children();

	ret = read(fd, &buf, 1);
	if (ret == 0)
		tst_res(TPASS, "read the end of file correctly");
	else
		tst_res(TFAIL, "read() returns %d, expected 0", ret);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	tst_fill_file(TESTFILE, 'a', NFORKS, 1);
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
