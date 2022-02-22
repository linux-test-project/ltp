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

static void run(void)
{
	int flag;

	TST_EXP_FD_SILENT(pidfd_open(getpid(), 0), "pidfd_open(getpid(), 0)");

	flag = fcntl(TST_RET, F_GETFD);

	SAFE_CLOSE(TST_RET);

	if (flag == -1)
		tst_brk(TFAIL | TERRNO, "fcntl(F_GETFD) failed");

	if (!(flag & FD_CLOEXEC))
		tst_brk(TFAIL, "pidfd_open(getpid(), 0) didn't set close-on-exec flag");

	tst_res(TPASS, "pidfd_open(getpid(), 0) passed");
}

static struct tst_test test = {
	.setup = pidfd_open_supported,
	.test_all = run,
};
