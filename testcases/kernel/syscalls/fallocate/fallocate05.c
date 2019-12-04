// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Tests that writing to fallocated file works when filesystem is full.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "tst_test.h"
#include "lapi/fallocate.h"

#define MNTPOINT "mntpoint"
#define FALLOCATE_SIZE (1024*1024)
#define TESTED_FLAGS "fallocate(FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE)"

static int fd;

static void run(void)
{
	char buf[FALLOCATE_SIZE];
	ssize_t ret;

	fd = SAFE_OPEN(MNTPOINT "/test_file", O_WRONLY | O_CREAT);

	if (fallocate(fd, 0, 0, FALLOCATE_SIZE)) {
		if (errno == EOPNOTSUPP) {
			tst_res(TCONF | TERRNO, "fallocate() not supported");
			SAFE_CLOSE(fd);
			return;
		}

		tst_brk(TBROK | TERRNO,
			"fallocate(fd, 0, 0, %i)", FALLOCATE_SIZE);
	}

	tst_fill_fs(MNTPOINT, 1);

	ret = write(fd, buf, sizeof(buf));

	if (ret < 0)
		tst_res(TFAIL | TERRNO, "write() failed unexpectedly");
	else
		tst_res(TPASS, "write() wrote %zu bytes", ret);

	ret = fallocate(fd, 0, FALLOCATE_SIZE, FALLOCATE_SIZE);
	if (ret != -1)
		tst_brk(TFAIL, "fallocate() succeeded unexpectedly");

	if (errno != ENOSPC)
		tst_brk(TFAIL | TERRNO, "fallocate() should fail with ENOSPC");

	tst_res(TPASS | TERRNO, "fallocate() on full FS");

	ret = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, FALLOCATE_SIZE);
	if (ret == -1) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, TESTED_FLAGS);

		tst_brk(TBROK | TERRNO, TESTED_FLAGS);
	}
	tst_res(TPASS, TESTED_FLAGS);

	ret = write(fd, buf, 10);
	if (ret == -1)
		tst_res(TFAIL | TERRNO, "write()");
	else
		tst_res(TPASS, "write()");

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.cleanup = cleanup,
	.test_all = run,
};
