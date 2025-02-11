// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 */

/*\
 * Verify that:
 *
 * sched_getparam(2) gets correct scheduling parameters for
 * the specified process:
 *
 * - If pid is zero, sched_getparam(2) gets the scheduling parameters
 * for the calling process.
 * - If pid is not zero, sched_getparam(2) gets the scheduling
 * parameters for the specified [pid] process.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sched.h"

static pid_t pids[2] = {0, 0};

static void verify_sched_getparam(unsigned int n)
{
	pid_t child_pid;
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct sched_param param = {
		.sched_priority = 100,
	};

	child_pid = SAFE_FORK();
	if (!child_pid) {
		TST_EXP_PASS_SILENT(tv->sched_getparam(pids[n], &param),
				   "sched_getparam(%d)", pids[n]);
		if (!TST_PASS)
			exit(0);

		/*
		 * For normal process, scheduling policy is SCHED_OTHER.
		 * For this scheduling policy, only allowed priority value is 0.
		 */
		if (param.sched_priority)
			tst_res(TFAIL,
				"sched_getparam(%d) got wrong sched_priority %d, expected 0",
				pids[n], param.sched_priority);
		else
			tst_res(TPASS, "sched_getparam(%d) got expected sched_priority 0", pids[n]);

		exit(0);
	}

	tst_reap_children();
}

static void setup(void)
{
	struct sched_variant *tv = &sched_variants[tst_variant];

	tst_res(TINFO, "Testing %s variant", tv->desc);

	pids[1] = getpid();
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.tcnt = ARRAY_SIZE(pids),
	.test = verify_sched_getparam,
};
