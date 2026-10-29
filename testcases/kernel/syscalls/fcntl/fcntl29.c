// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * Basic test for fcntl(2) using F_DUPFD_CLOEXEC and getting FD_CLOEXEC.
 */

#include "lapi/fcntl.h"
#include <tst_test.h>

static int fd;

static void setup(void)
{
	fd = SAFE_CREAT("testfile", 0644);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	TST_EXP_FD(fcntl(fd, F_DUPFD_CLOEXEC, 0));
	int dup_fd = TST_RET;

	TST_EXP_POSITIVE(fcntl(dup_fd, F_GETFD), "fcntl test F_GETFD");
	if (TST_RET & FD_CLOEXEC)
		tst_res(TPASS, "fcntl() set FD_CLOEXEC");
	else
		tst_res(TFAIL, "fcntl() did not set FD_CLOEXEC");

	SAFE_CLOSE(dup_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
