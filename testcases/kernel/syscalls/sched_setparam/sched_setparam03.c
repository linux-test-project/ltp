// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

/*\
 * Checks functionality for sched_setparam(2) for pid != 0
 *
 * This test forks a child and changes its parent's scheduling priority.
 */

#include <stdlib.h>

#include "tst_test.h"
#include "tst_sched.h"

static void run(void)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct sched_param p5 = { .sched_priority = 5 };
	struct sched_param p;
	pid_t child_pid;

	child_pid = SAFE_FORK();

	if (!child_pid) {
		TST_EXP_PASS_SILENT(tv->sched_setparam(getppid(), &p5));
		exit(0);
	}
	tst_reap_children();

	TST_EXP_PASS_SILENT(tv->sched_getparam(0, &p));

	if (p.sched_priority == p5.sched_priority)
		tst_res(TPASS, "got expected priority %d", p.sched_priority);
	else
		tst_res(TFAIL, "got priority %d, expected 5", p.sched_priority);
}

void setup(void)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct sched_param p = { .sched_priority = 1 };

	tst_res(TINFO, "Testing %s variant", tv->desc);

	if (tv->sched_setscheduler(0, SCHED_FIFO, &p))
		tst_brk(TBROK | TERRNO, "sched_setscheduler(0, SCHED_FIFO, 1)");
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.test_all = run,
};
