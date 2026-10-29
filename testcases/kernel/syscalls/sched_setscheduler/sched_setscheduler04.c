// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Federico Bonfiglio <federico.bonfiglio@protonmail.ch>
 */

/*
 * Testcases that test if sched_setscheduler with flag
 * SCHED_RESET_ON_FORK restores children policy to
 * SCHED_NORMAL.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>

#include "tst_test.h"
#include "tst_sched.h"

struct test_case_t {
	int policy;
	char *desc;
};

static struct test_case_t cases[] = {
	{
		.policy = SCHED_FIFO,
		.desc = "SCHED_FIFO"
	},
	{
		.policy = SCHED_RR,
		.desc = "SCHED_RR"
	}
};

static void test_reset_on_fork(unsigned int i)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct test_case_t *tc = &cases[i];

	tst_res(TINFO, "Testing %s variant %s policy", tv->desc, tc->desc);

	struct sched_param param = { .sched_priority = 10 };

	tv->sched_setscheduler(getpid(), tc->policy | SCHED_RESET_ON_FORK, &param);

	pid_t pid = SAFE_FORK();

	if (pid) {
		if (sched_getscheduler(pid) == SCHED_NORMAL)
			tst_res(TPASS, "Policy reset to SCHED_NORMAL");
		else
			tst_res(TFAIL, "Policy NOT reset to SCHED_NORMAL");

		sched_getparam(pid, &param);

		/* kernel will return sched_priority 0 if task is not RT Policy */
		if (param.sched_priority == 0)
			tst_res(TPASS, "Priority set to 0");
		else
			tst_res(TFAIL, "Priority not set to 0");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.caps = (struct tst_cap[]) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_NICE),
		{}
	},
	.test_variants = ARRAY_SIZE(sched_variants),
	.tcnt = ARRAY_SIZE(cases),
	.test = test_reset_on_fork,
};
