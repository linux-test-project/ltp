// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Red Hat, Inc.  All rights reserved.
 *
 * Attempt to run a trivial binary with stack < 1MB.
 *
 * Early patches for stack guard gap caused that gap size was
 * contributing to stack limit. This caused failures
 * for new processes (E2BIG) when ulimit was set to anything
 * lower than size of gap. commit 1be7107fbe18 "mm: larger
 * stack guard gap, between vmas" sets default gap size to 1M
 * (for systems with 4k pages), so let's set stack limit to 512kB
 * and confirm we can still run some trivial binary.
 */

#define _GNU_SOURCE
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "tst_test.h"

#define STACK_LIMIT (5 * 1024)

static void test_setrlimit(void)
{
	int status;
	struct rlimit rlim;

	rlim.rlim_cur = STACK_LIMIT;
	rlim.rlim_max = STACK_LIMIT;

	SAFE_SETRLIMIT(RLIMIT_STACK, &rlim);
	rlim.rlim_cur = 0;
	rlim.rlim_max = 0;
	getrlimit(RLIMIT_STACK, &rlim);
	if (rlim.rlim_cur == STACK_LIMIT && rlim.rlim_max == STACK_LIMIT) {
		tst_res(TPASS, "STACK_LIMIT is set accurately");
	}
	else {
		tst_res(TFAIL, "STACK_LIMIT not set proper");
	}
}

static struct tst_test test = {
	.test_all     = test_setrlimit,
	.needs_root = 1,
};
