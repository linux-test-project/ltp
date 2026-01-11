// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Stephen Bertram <sbertram@redhat.com>
 */

/*\
 * This test verifies that :manpage:`clone3(2)` fails with EPERM when CAP_SYS_ADMIN
 * has been dropped and ``clone_args.set_tid_size`` is greater than zero.
 */

#define _GNU_SOURCE
#define DESC(x) .flags = x, .sflags = #x

#include "tst_test.h"
#include "lapi/sched.h"

enum case_type {
	K_SET_TID,   /* flags = 0 || CLONE_NEW*, set_tid_size > 0 => EPERM */
	K_NAMESPACE_ONLY, /* flags = CLONE_NEW*, set_tid_size = 0 => EPERM */
};

static struct clone_args args = {0};
static pid_t tid_array[1] = {1};

static struct tcase {
	uint64_t flags;
	const char *sflags;
	enum case_type type;
} tcases[] = {
	{ DESC(CLONE_NEWPID), K_NAMESPACE_ONLY },
	{ DESC(CLONE_NEWCGROUP), K_NAMESPACE_ONLY },
	{ DESC(CLONE_NEWIPC), K_NAMESPACE_ONLY },
	{ DESC(CLONE_NEWNET), K_NAMESPACE_ONLY },
	{ DESC(CLONE_NEWNS), K_NAMESPACE_ONLY },
	{ DESC(CLONE_NEWUTS), K_NAMESPACE_ONLY },

	{ DESC(CLONE_NEWPID), K_SET_TID },
	{ DESC(CLONE_NEWCGROUP), K_SET_TID },
	{ DESC(CLONE_NEWIPC), K_SET_TID },
	{ DESC(CLONE_NEWNET), K_SET_TID },
	{ DESC(CLONE_NEWNS), K_SET_TID },
	{ DESC(CLONE_NEWUTS), K_SET_TID },

	{ DESC(0), K_SET_TID },
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	args.flags = tc->flags;

	if (tc->type == K_NAMESPACE_ONLY) {
		args.set_tid = 0;
		args.set_tid_size = 0;
	} else {
		args.set_tid = (uint64_t)(uintptr_t)tid_array;
		args.set_tid_size = 1;
	}

	TST_EXP_FAIL(clone3(&args, sizeof(args)), EPERM,
			"clone3(%s) set_tid_size=%ld",
			tc->sflags, args.set_tid_size);
}

static void setup(void)
{
	clone3_supported_by_kernel();

	memset(&args, 0, sizeof(args));
	SAFE_UNSHARE(CLONE_NEWUSER | CLONE_NEWNS);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = run,
	.needs_root = 1,
	.caps = (struct tst_cap[]) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{},
	},
	.bufs = (struct tst_buffers[]) {
		{&args, .size = sizeof(struct clone_args)},
		{},
	},
};
