// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic pidfd_open() test:
 * 1) Fetch the PID of the current process and try to get its file descriptor.
 * 2) Check that the close-on-exec flag is set on the file descriptor.
 */

#include <unistd.h>
#include <fcntl.h>
#include "tst_test.h"
#include "lapi/pidfd_open.h"

static void run(void)
{
	int flag;

	TEST(pidfd_open(getpid(), 0));

	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "pidfd_open(getpid(), 0) failed");

	flag = fcntl(TST_RET, F_GETFD);

	SAFE_CLOSE(TST_RET);

	if (flag == -1)
		tst_brk(TFAIL | TERRNO, "fcntl(F_GETFD) failed");

	if (!(flag & FD_CLOEXEC))
		tst_brk(TFAIL, "pidfd_open(getpid(), 0) didn't set close-on-exec flag");

	tst_res(TPASS, "pidfd_open(getpid(), 0) passed");
}

static struct tst_test test = {
	.min_kver = "5.3",
	.test_all = run,
};
