// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic clone3() test to check various failures.
 */
#define _GNU_SOURCE

#include <stdlib.h>

#include "tst_test.h"
#include "lapi/clone.h"

static struct clone_args *valid_args, *invalid_args;
unsigned long stack;
static int *invalid_address;

static struct tcase {
	const char *name;
	struct clone_args **args;
	size_t size;
	uint64_t flags;
	int **pidfd;
	int **child_tid;
	int **parent_tid;
	int exit_signal;
	unsigned long stack;
	unsigned long stack_size;
	unsigned long tls;
	int exp_errno;
} tcases[] = {
	{"invalid args", &invalid_args, sizeof(*valid_args), 0, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"zero size", &valid_args, 0, 0, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"short size", &valid_args, sizeof(*valid_args) - 1, 0, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"extra size", &valid_args, sizeof(*valid_args) + 1, 0, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"sighand-no-VM", &valid_args, sizeof(*valid_args), CLONE_SIGHAND, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"thread-no-sighand", &valid_args, sizeof(*valid_args), CLONE_THREAD, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"fs-newns", &valid_args, sizeof(*valid_args), CLONE_FS | CLONE_NEWNS, NULL, NULL, NULL, SIGCHLD, 0, 0, 0, EINVAL},
	{"invalid pidfd", &valid_args, sizeof(*valid_args), CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | CLONE_PIDFD, &invalid_address, NULL, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"invalid childtid", &valid_args, sizeof(*valid_args), CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | CLONE_PIDFD, NULL, &invalid_address, NULL, SIGCHLD, 0, 0, 0, EFAULT},
	{"invalid parenttid", &valid_args, sizeof(*valid_args), CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | CLONE_PIDFD, NULL, NULL, &invalid_address, SIGCHLD, 0, 0, 0, EFAULT},
	{"invalid signal", &valid_args, sizeof(*valid_args), 0, NULL, NULL, NULL, CSIGNAL + 1, 0, 0, 0, EINVAL},
	{"zero-stack-size", &valid_args, sizeof(*valid_args), 0, NULL, NULL, NULL, SIGCHLD, (unsigned long)&stack, 0, 0, EINVAL},
	{"invalid-stack", &valid_args, sizeof(*valid_args), 0, NULL, NULL, NULL, SIGCHLD, 0, 4, 0, EINVAL},
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
		if (tc->child_tid)
			args->child_tid = (uint64_t)(*tc->child_tid);
		if (tc->parent_tid)
			args->parent_tid = (uint64_t)(*tc->parent_tid);
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
