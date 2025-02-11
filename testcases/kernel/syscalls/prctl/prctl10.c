// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * Basic test to test behaviour of PR_GET_TSC and PR_SET_TSC.
 *
 * Set the state of the flag determining whether the timestamp counter can
 * be read by the process.
 *
 * - Pass PR_TSC_ENABLE to arg2 to allow it to be read.
 * - Pass PR_TSC_SIGSEGV to arg2 to generate a SIGSEGV when read.
 */

#include <sys/prctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/prctl.h"

#define TCASE_ENTRY(tsc_read_stat) { .name = #tsc_read_stat, .read_stat = tsc_read_stat}

static const char * const tsc_read_stat_names[] = {
	[0] = "[not set]",
	[PR_TSC_ENABLE] = "PR_TSC_ENABLE",
	[PR_TSC_SIGSEGV] = "PR_TSC_SIGSEGV",
};

static struct tcase {
	char *name;
	int read_stat;
} tcases[] = {
	TCASE_ENTRY(PR_TSC_ENABLE),
	TCASE_ENTRY(PR_TSC_SIGSEGV)
};

static uint64_t rdtsc(void)
{
	uint32_t lo = 0, hi = 0;

#if (defined(__x86_64__) || defined(__i386__))
	/* We cannot use "=A", since this would use %rax on x86_64 */
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
#endif

	return (uint64_t)hi << 32 | lo;
}


static int expected_status(int status, int exp_status)
{
	if (!exp_status && WIFEXITED(status))
		return 0;

	if (exp_status && WIFSIGNALED(status) && WTERMSIG(status) == exp_status)
		return 0;

	return 1;
}

static void verify_prctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	unsigned long long time1, time2;
	int tsc_val = 0, pid, status;

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_PASS_SILENT(prctl(PR_SET_TSC, tc->read_stat));
		TST_EXP_PASS_SILENT(prctl(PR_GET_TSC, &tsc_val));
		if (tsc_val == tc->read_stat)
			tst_res(TPASS, "current state is %s(%d)",
					tc->name, tc->read_stat);
		else
			tst_res(TFAIL, "current state is %s(%d), expect %s(%d)",
					tsc_read_stat_names[tsc_val],
					tsc_val, tc->name, tc->read_stat);

		time1 = rdtsc();
		time2 = rdtsc();
		if (time2 > time1)
			tst_res(TPASS, "rdtsc works correctly, %lld ->%lld",
				time1, time2);
		else
			tst_res(TFAIL, "rdtsc works incorrectly, %lld ->%lld",
				time1, time2);
		exit(0);
	}
	SAFE_WAITPID(pid, &status, 0);

	if (expected_status(status, tc->read_stat == PR_TSC_SIGSEGV ? SIGSEGV : 0))
		tst_res(TFAIL, "Test %s failed", tc->name);
	else
		tst_res(TPASS, "Test %s succeeded", tc->name);
}

static struct tst_test test = {
	.forks_child = 1,
	.test = verify_prctl,
	.tcnt = ARRAY_SIZE(tcases),
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
};
