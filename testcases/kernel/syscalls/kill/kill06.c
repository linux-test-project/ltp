// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 */

/*\
 * Test case to check the basic functionality of kill() when killing an
 * entire process group with a negative pid.
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "tst_test.h"

static void verify_kill(void)
{
	int nsig, i, status;

	if (!SAFE_FORK()) {
		setpgrp();
		for (i = 0; i < 5; i++) {
			if (!SAFE_FORK())
				pause();
		}

		TEST(kill(-getpgrp(), SIGKILL));
		if (TST_RET != 0)
			tst_res(TFAIL | TTERRNO, "kill failed");
		exit(0);
	}

	SAFE_WAITPID(-1, &status, 0);
	nsig = WTERMSIG(status);
	if (nsig != SIGKILL) {
		tst_brk(TFAIL, "wait: unexpected signal %d returned, "
			"expected SIGKILL(9)", nsig);
	}

	tst_res(TPASS, "receive expected signal SIGKILL(9)");
}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = verify_kill,
};
