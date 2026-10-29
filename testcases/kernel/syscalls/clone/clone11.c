// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Stephen Bertram <sbertram@redhat.com>
 */

/*\
 * This test verifies that :manpage:`clone(2)` fails with EPERM when CAP_SYS_ADMIN
 * has been dropped.
 */

#define _GNU_SOURCE
#define DESC(x) .flags = x, .sflags = #x

#include "tst_test.h"
#include "clone_platform.h"
#include "lapi/sched.h"

static void *child_stack;
static int *child_pid;

static struct tcase {
	uint64_t flags;
	const char *sflags;
	int skip;
} tcases[] = {
	{ DESC(CLONE_NEWPID) },
	{ DESC(CLONE_NEWCGROUP) },
	{ DESC(CLONE_NEWIPC) },
	{ DESC(CLONE_NEWNET) },
	{ DESC(CLONE_NEWNS) },
	{ DESC(CLONE_NEWUTS) },
};

static int child_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	*child_pid = getpid();
	_exit(0);
}

static void tcases_filter(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		struct tcase *tc = &tcases[i];

		switch (tc->flags) {
		case CLONE_NEWCGROUP:
			if (tst_kvercmp(4, 6, 0) < 0)
				tc->skip = 1;
			break;
		default:
			break;
		}
	}
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->skip) {
		tst_res(TCONF, "%s is not supported", tc->sflags);
		return;
	}

	TST_EXP_FAIL(ltp_clone(tc->flags, child_fn, NULL, CHILD_STACK_SIZE, child_stack),
		EPERM, "clone(%s) should fail with EPERM",
		tc->sflags);
}

static void setup(void)
{
	child_pid = SAFE_MMAP(NULL, sizeof(*child_pid),
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS,
			-1, 0);

	tcases_filter();
}

static void cleanup(void)
{
	if (child_pid)
		SAFE_MUNMAP(child_pid, sizeof(*child_pid));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.caps = (struct tst_cap[]) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{},
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
	.bufs = (struct tst_buffers[]) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{},
	},
};
