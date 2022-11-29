// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * [Description]
 *
 * Basic clone3() test to check various failures.
 */

#define _GNU_SOURCE

#include <stdlib.h>

#include "tst_test.h"
#include "lapi/sched.h"

static struct clone_args *valid_args, *invalid_args;
unsigned long stack;
static int *invalid_address;

static struct tcase {
	const char *name;
	struct clone_args **args;
	size_t size;
	uint64_t flags;
	int **pidfd;
	int exit_signal;
	unsigned long stack;
	unsigned long stack_size;
	unsigned long tls;
	int exp_errno;
} tcases[] = {
	{"invalid args", &invalid_args, sizeof(*valid_args), 0, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"zero size", &valid_args, 0, 0, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"short size", &valid_args, sizeof(*valid_args) - 1, 0, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"extra size", &valid_args, sizeof(*valid_args) + 1, 0, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"sighand-no-VM", &valid_args, sizeof(*valid_args), CLONE_SIGHAND, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"thread-no-sighand", &valid_args, sizeof(*valid_args), CLONE_THREAD, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"fs-newns", &valid_args, sizeof(*valid_args), CLONE_FS | CLONE_NEWNS, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"invalid pidfd", &valid_args, sizeof(*valid_args), CLONE_PIDFD, &invalid_address, SIGCHLD, 0, 0, 0, EFAULT},
	{"invalid signal", &valid_args, sizeof(*valid_args), 0, NULL, CSIGNAL + 1, 0, 0, 0, EINVAL},
	{"zero-stack-size", &valid_args, sizeof(*valid_args), 0, NULL, SIGCHLD, (unsigned long)&stack, 0, 0, EINVAL},
	{"invalid-stack", &valid_args, sizeof(*valid_args), 0, NULL, SIGCHLD, 0, 4, 0, EINVAL},
	/*
	 * Don't test CLONE_CHILD_SETTID and CLONE_PARENT_SETTID:
	 * When the parent tid is written to the memory location for
	 * CLONE_PARENT_SETTID we're past the point of no return of process
	 * creation, i.e. the return value from put_user() isn't checked and
	 * can't be checked anymore so you'd never receive EFAULT for a bogus
	 * parent_tid memory address.
	 *
	 * https://lore.kernel.org/linux-m68k/20200627122332.ki2otaiw3v7wndbl@wittgenstein/T/#u
	 */
};

static void setup(void)
{
	clone3_supported_by_kernel();

	void *p = tst_get_bad_addr(NULL);

	invalid_args = p;
	invalid_address = p;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct clone_args *args = *tc->args;

	if (args == valid_args) {
		args->flags = tc->flags;
		if (tc->pidfd)
			args->pidfd = (uint64_t)(*tc->pidfd);
		else
			args->pidfd = 0;
		args->exit_signal = tc->exit_signal;
		args->stack = tc->stack;
		args->stack_size = tc->stack_size;
		args->tls = tc->tls;
	}

	TEST(clone3(args, tc->size));

	if (!TST_RET)
		exit(EXIT_SUCCESS);

	if (TST_RET >= 0) {
		tst_res(TFAIL, "%s: clone3() passed unexpectedly", tc->name);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: clone3() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: clone3() failed as expected", tc->name);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&valid_args, .size = sizeof(*valid_args)},
		{},
	}
};
