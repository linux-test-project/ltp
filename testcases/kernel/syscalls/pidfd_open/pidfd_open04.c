// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that the PIDFD_NONBLOCK flag works with pidfd_open() and
 * that waitid() with a non-blocking pidfd returns EAGAIN.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/pidfd.h"

#ifndef P_PIDFD
#define P_PIDFD  3
#endif

static void run(void)
{
	int flag, pid, pidfd, ret;
	siginfo_t info;

	pid = SAFE_FORK();
	if (!pid) {
		TST_CHECKPOINT_WAIT(0);
		exit(EXIT_SUCCESS);
	}

	TST_EXP_FD_SILENT(pidfd_open(pid, PIDFD_NONBLOCK),
				"pidfd_open(%d,  PIDFD_NONBLOCK)", pid);

	pidfd = TST_RET;
	flag = fcntl(pidfd, F_GETFL);
	if (flag == -1)
		tst_brk(TFAIL | TERRNO, "fcntl(F_GETFL) failed");

	if (!(flag & O_NONBLOCK))
		tst_brk(TFAIL, "pidfd_open(%d, O_NONBLOCK) didn't set O_NONBLOCK flag", pid);

	tst_res(TPASS, "pidfd_open(%d, O_NONBLOCK) sets O_NONBLOCK flag", pid);

	TST_EXP_FAIL(waitid(P_PIDFD, pidfd, &info, WEXITED), EAGAIN,
			"waitid(P_PIDFD,...,WEXITED)");

	TST_CHECKPOINT_WAKE(0);

	ret = TST_RETRY_FUNC(waitid(P_PIDFD, pidfd, &info, WEXITED), TST_RETVAL_EQ0);
	if (ret == 0) {
		tst_res(TPASS, "waitid(P_PIDFD) succeeded after child process terminated");
	} else {
		tst_res(TFAIL, "waitid(P_PIDFD) failed after child process terminated");
		SAFE_WAIT(NULL);
	}

	SAFE_CLOSE(pidfd);
}

static void setup(void)
{
	pidfd_open_supported();

	TEST(pidfd_open(getpid(), PIDFD_NONBLOCK));
	if (TST_RET == -1) {
		if (TST_ERR == EINVAL) {
			tst_brk(TCONF, "PIDFD_NONBLOCK was supported since linux 5.10");
			return;
		}
		tst_brk(TFAIL | TTERRNO,
			"pidfd_open(getpid(),PIDFD_NONBLOCK) failed unexpectedly");
	}
	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.setup = setup,
	.test_all = run,
};
