// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

/*\
 * [Description]
 *
 * Verify that:
 *
 * 1. sched_setparam(2) returns -1 and sets errno to ESRCH if the
 *    process with specified pid could not be found.
 * 2. sched_setparam(2) returns -1 and sets errno to EINVAL if
 *    the parameter pid is an invalid value (-1).
 * 3. sched_setparam(2) returns -1 and sets errno to EINVAL if the
 *    parameter p is an invalid address.
 * 4. sched_setparam(2) returns -1 sets errno to EINVAL if the
 *    value for p.sched_priority is other than 0 for scheduling
 *    policy, SCHED_OTHER.
 */

#include "tst_test.h"
#include "tst_sched.h"

static struct sched_param p = { .sched_priority = 0 };
static struct sched_param p1 = { .sched_priority = 1 };

static pid_t unused_pid;
static pid_t inval_pid = -1;
static pid_t zero_pid;

static struct test_cases_t {
	char *desc;
	pid_t *pid;
	struct sched_param *p;
	int exp_errno;
} tcases[] = {
	{
	"test with non-existing pid", &unused_pid, &p, ESRCH}, {
	"test invalid pid value", &inval_pid, &p, EINVAL,}, {
	"test with invalid address for p", &zero_pid, NULL, EINVAL}, {
	"test with invalid p.sched_priority", &zero_pid, &p1, EINVAL}
};

static void setup(void)
{
	tst_res(TINFO, "Testing %s variant", sched_variants[tst_variant].desc);

	unused_pid = tst_get_unused_pid();
}

static void run(unsigned int n)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct test_cases_t *tc = &tcases[n];

	TST_EXP_FAIL(tv->sched_setparam(*tc->pid, tc->p), tc->exp_errno, "%s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.test_variants = ARRAY_SIZE(sched_variants),
	.tcnt = ARRAY_SIZE(tcases),
};
