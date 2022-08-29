// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that acces after guarded buffer causes segfault.
 */

#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

#define BUF1_LEN 10
#define BUF2_LEN 4096
#define BUF3_LEN 12004

static char *buf1;
static char *buf2;
static char *buf3;

static void do_test(unsigned int n)
{
	int pid;
	int status;

	if (n == 6) {
		buf1[-1] = 0;
		buf3[-1] = 0;
		tst_res(TPASS, "Buffers dirtied!");
	}

	pid = SAFE_FORK();
	if (!pid) {
		switch (n) {
		case 0:
			buf1[BUF1_LEN - 1] = 0;
		break;
		case 1:
			buf2[BUF2_LEN - 1] = 0;
		break;
		case 2:
			buf3[BUF3_LEN - 1] = 0;
		break;
		case 3:
			buf1[BUF1_LEN] = 0;
		break;
		case 4:
			buf2[BUF2_LEN] = 0;
		break;
		case 5:
			buf3[BUF3_LEN] = 0;
		break;
		case 6:
			buf1[-2] = 0;
			buf3[-2] = 0;
		break;
		}

		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (n == 6)
		return;

	if (n < 3) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			tst_res(TPASS, "exited normally");
			return;
		}
	} else {
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
			tst_res(TPASS, "Killed by SIGSEGV");
			return;
		}
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.forks_child = 1,
	.test = do_test,
	.tcnt = 7,
	.bufs = (struct tst_buffers []) {
		{&buf1, .size = BUF1_LEN},
		{&buf2, .size = BUF2_LEN},
		{&buf3, .size = BUF3_LEN},
		{}
	}
};
