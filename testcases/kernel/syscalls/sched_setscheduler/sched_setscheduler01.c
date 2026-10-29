// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Testcase to test whether sched_setscheduler(2) sets the errnos
 * correctly.
 *
 * [Algorithm]
 *
 * 1. Call sched_setscheduler with an invalid pid, and expect
 *    ESRCH to be returned.
 * 2. Call sched_setscheduler with an invalid scheduling policy,
 *    and expect EINVAL to be returned.
 * 3. Call sched_setscheduler with an invalid "param" address,
 *    which lies outside the address space of the process, and expect
 *    EFAULT to be returned.
 * 4. Call sched_setscheduler with an invalid priority value
 *    in "param" and expect EINVAL to be returned
 */

#include <stdio.h>
#include <errno.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_sched.h"

#define SCHED_INVALID	99

static struct sched_param p;
static struct sched_param p1 = { .sched_priority = 1 };
static pid_t unused_pid;
static pid_t init_pid = 1;
static pid_t zero_pid;

struct test_cases_t {
	pid_t *pid;
	int policy;
	struct sched_param *p;
	int error;
} tcases[] = {
	/* The pid is invalid - ESRCH */
	{&unused_pid, SCHED_OTHER, &p, ESRCH},
	/* The policy is invalid - EINVAL */
	{&init_pid, SCHED_INVALID, &p, EINVAL},
	/* The param address is invalid - EFAULT */
	{&init_pid, SCHED_OTHER, (struct sched_param *)-1, EFAULT},
	/* The priority value in param invalid - EINVAL */
	{&zero_pid, SCHED_OTHER, &p1, EINVAL}
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

	TST_EXP_FAIL(tv->sched_setscheduler(*tc->pid, tc->policy, tc->p),
		     tc->error, "sched_setscheduler(%d, %d, %p)",
		     *tc->pid, tc->policy, tc->p);
}

static struct tst_test test = {
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
};
