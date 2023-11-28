// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Before kernel 2.6.26 we could not trace init(1) process and ptrace() would
 * fail with EPERM. This case just checks whether we can trace init(1) process
 * successfully.
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "tst_test.h"

static void verify_ptrace(void)
{
	TEST(ptrace(PTRACE_ATTACH, 1, NULL, NULL));
	if (TST_RET == 0)
		tst_res(TPASS, "ptrace() traces init process successfully");
	else
		tst_res(TFAIL | TTERRNO,
			"ptrace() returns %ld, failed unexpectedly", TST_RET);

	/*
	 * Wait until tracee is stopped by SIGSTOP otherwise detach will fail
	 * with ESRCH.
	 */
	SAFE_WAITPID(1, NULL, 0);

	SAFE_PTRACE(PTRACE_DETACH, 1, NULL, NULL);
}

static struct tst_test test = {
	.test_all = verify_ptrace,
	.needs_root = 1,
};
