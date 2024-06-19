// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright 2023 Mike Galbraith <efault-AT-gmx.de> */
/* Copyright 2023 Wei Gao <wegao@suse.com> */
/*\
 *
 * [Description]
 *
 * Thread starvation test. On fauluty kernel the test timeouts.
 *
 * Original reproducer taken from:
 * https://lore.kernel.org/lkml/9fd2c37a05713c206dcbd5866f67ce779f315e9e.camel@gmx.de/
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sched.h>

#include "tst_test.h"

static char *str_loop;
static long loop = 2000000;
static char *str_timeout;
static int timeout = 240;

static int wait_for_pid(pid_t pid)
{
	int status, ret;

again:
	ret = waitpid(pid, &status, 0);
	if (ret == -1) {
		if (errno == EINTR)
			goto again;

		return -1;
	}

	if (WIFSIGNALED(status))
		return 0;

	return -1;
}

static void setup(void)
{
	cpu_set_t mask;

	CPU_ZERO(&mask);

	CPU_SET(0, &mask);

	if (sched_setaffinity(0, sizeof(mask), &mask) < 0)
		tst_brk(TBROK | TERRNO, "sched_setaffinity() failed");

	if (tst_parse_long(str_loop, &loop, 1, LONG_MAX))
		tst_brk(TBROK, "Invalid number of loop number '%s'", str_loop);

	if (tst_parse_int(str_timeout, &timeout, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of timeout '%s'", str_timeout);

	tst_set_max_runtime(timeout);
}

static void handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	if (loop > 0)
		--loop;
}

static void child(void)
{
	pid_t ppid = getppid();

	TST_CHECKPOINT_WAIT(0);

	while (1)
		SAFE_KILL(ppid, SIGUSR1);
}

static void do_test(void)
{
	pid_t child_pid;

	child_pid = SAFE_FORK();

	if (!child_pid)
		child();

	SAFE_SIGNAL(SIGUSR1, handler);
	TST_CHECKPOINT_WAKE(0);

	while (loop)
		sleep(1);

	SAFE_KILL(child_pid, SIGTERM);
	TST_EXP_PASS(wait_for_pid(child_pid));
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"l:", &str_loop, "Number of loops (default 2000000)"},
		{"t:", &str_timeout, "Max timeout (default 240s)"},
		{}
	},
	.needs_checkpoints = 1,
};
