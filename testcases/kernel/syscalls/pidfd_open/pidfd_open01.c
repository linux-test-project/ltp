// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Basic pidfd_open() test:
 *
 * - Fetch the PID of the current process and try to get its file descriptor.
 * - Check that the close-on-exec flag is set on the file descriptor.
 */

#include <unistd.h>
#include "tst_test.h"
#include "lapi/pidfd.h"

static int pidfd = -1;

static void run(void)
{
	int flag;

	TST_EXP_FD_SILENT(pidfd_open(getpid(), 0), "pidfd_open(getpid(), 0)");

	pidfd = TST_RET;
	flag = SAFE_FCNTL(pidfd, F_GETFD);

	SAFE_CLOSE(pidfd);

	if (!(flag & FD_CLOEXEC))
		tst_brk(TFAIL, "pidfd_open(getpid(), 0) didn't set close-on-exec flag");

	tst_res(TPASS, "pidfd_open(getpid(), 0) passed");
}

static void cleanup(void)
{
	if (pidfd > -1)
		SAFE_CLOSE(pidfd);
}

static struct tst_test test = {
	.setup = pidfd_open_supported,
	.cleanup = cleanup,
	.test_all = run,
};
