// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * Basic tests for the tkill() errors.
 *
 * [Algorithm]
 *
 * - EINVAL on an invalid thread ID
 * - ESRCH when no process with the specified thread ID exists
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static pid_t unused_tid;
static pid_t inval_tid = -1;

struct test_case_t {
	int *tid;
	int exp_errno;
} tc[] = {
	{&inval_tid, EINVAL},
	{&unused_tid, ESRCH}
};

static void setup(void)
{
	unused_tid = tst_get_unused_pid();
}

static void run(unsigned int i)
{
	TST_EXP_FAIL(tst_syscall(__NR_tkill, *(tc[i].tid), SIGUSR1),
		     tc[i].exp_errno, "tst_syscall(__NR_tkill) expecting %s",
			 tst_strerrno(tc[i].exp_errno));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.setup = setup,
	.test = run,
};
