// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2012-2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * wait401 - check that a call to wait4() correctly waits for a child
 *           process to exit
 */

#include <stdlib.h>
#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "tst_test.h"

static void run(void)
{
	pid_t pid;
	int status = 1;
	struct rusage rusage;

	pid = SAFE_FORK();
	if (!pid) {
		TST_PROCESS_STATE_WAIT(getppid(), 'S', 0);
		exit(0);
	}

	TEST(wait4(pid, &status, 0, &rusage));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "wait4() failed");
		return;
	}

	if (TST_RET != pid) {
		tst_res(TFAIL, "waitpid() returned wrong pid %li, expected %i",
			TST_RET, pid);
	} else {
		tst_res(TPASS, "waitpid() returned correct pid %i", pid);
	}

	if (!WIFEXITED(status)) {
		tst_res(TFAIL, "WIFEXITED() not set in status (%s)",
		        tst_strstatus(status));
		return;
	}

	tst_res(TPASS, "WIFEXITED() is set in status");

	if (WEXITSTATUS(status))
		tst_res(TFAIL, "WEXITSTATUS() != 0 but %i", WEXITSTATUS(status));
	else
		tst_res(TPASS, "WEXITSTATUS() == 0");

}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = run,
};
