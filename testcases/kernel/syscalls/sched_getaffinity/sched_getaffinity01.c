// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Description: This case tests the sched_getaffinity() syscall
 * History:     Porting from Crackerjack to LTP is done by
 *		 Manas Kumar Nayak maknayak@in.ibm.com>
 */
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/cpuset.h"

static long ncpu;

static void *bad_addr;

static void errno_test(pid_t pid, size_t cpusize, void *mask, int exp_errno)
{
	TEST(sched_getaffinity(pid, cpusize, mask));

	if (TST_RET != -1) {
		tst_res(TFAIL,
			"sched_getaffinity() returned %ld, expected -1",
			TST_RET);
		return;
	}

	if (TST_ERR != exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"sched_getaffinity() should fail with %s",
			tst_strerrno(exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "sched_getaffinity() failed");
}

static void do_test(void)
{
	cpu_set_t *mask;
	int nrcpus = 1024;
	unsigned len;

realloc:
	mask = CPU_ALLOC(nrcpus);
	if (!mask)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC()");

	len = CPU_ALLOC_SIZE(nrcpus);
	CPU_ZERO_S(len, mask);

	TEST(sched_getaffinity(0, len, mask));
	if (TST_RET == -1) {
		CPU_FREE(mask);
		if (TST_ERR == EINVAL && nrcpus < (1024 << 8)) {
			nrcpus = nrcpus << 2;
			goto realloc;
		}
		tst_brk(TBROK | TTERRNO, "fail to get cpu affinity");
	}

	long i, af_cpus = 0;

	for (i = 0; i < nrcpus; i++)
		af_cpus += !!CPU_ISSET_S(i, len, mask);

	if (af_cpus == 0)
		tst_res(TFAIL, "No cpus enabled in mask");
	else if (af_cpus > ncpu)
		tst_res(TFAIL, "Enabled cpus = %li > system cpus %li", af_cpus, ncpu);
	else
		tst_res(TPASS, "cpuset size = %u, enabled cpus %ld", len, af_cpus);

	errno_test(0, len, bad_addr, EFAULT);
	errno_test(0, 0, mask, EINVAL);
	errno_test(tst_get_unused_pid(), len, mask, ESRCH);

	CPU_FREE(mask);
}

static void setup(void)
{
	ncpu = SAFE_SYSCONF(_SC_NPROCESSORS_CONF);
	tst_res(TINFO, "system has %ld processor(s).", ncpu);

	bad_addr = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_test,
};
