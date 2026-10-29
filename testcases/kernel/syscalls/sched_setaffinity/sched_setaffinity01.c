// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2019 FUJITSU LIMITED.
 *
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

/*\
 * Check various errnos for sched_setaffinity():
 *
 * 1. EFAULT, if the supplied memory address is invalid.
 * 2. EINVAL, if the mask doesn't contain at least one permitted cpu.
 * 3. ESRCH, if the process whose id is pid could not be found.
 * 4. EPERM, if the calling process doesn't have appropriate privileges.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/cpuset.h"
#include "lapi/syscalls.h"

static cpu_set_t *mask, *emask, *fmask;
static size_t mask_size, emask_size;
static pid_t self_pid, privileged_pid, free_pid;
static struct passwd *ltpuser;

static struct tcase {
	pid_t *pid;
	size_t *size;
	cpu_set_t **mask;
	int exp_errno;
} tcases[] = {
	{&self_pid, &mask_size, &fmask, EFAULT},
	{&self_pid, &emask_size, &emask, EINVAL},
	{&free_pid, &mask_size, &mask, ESRCH},
	{&privileged_pid, &mask_size, &mask, EPERM},
};

static void kill_pid(void)
{
	SAFE_KILL(privileged_pid, SIGKILL);
	SAFE_WAITPID(privileged_pid, NULL, 0);
	SAFE_SETEUID(0);
}

static void verify_test(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (tc->exp_errno == EPERM) {
		privileged_pid = SAFE_FORK();
		if (privileged_pid == 0) {
			pause();
			exit(0);
		}

		SAFE_SETEUID(ltpuser->pw_uid);
	}

	TEST(tst_syscall(__NR_sched_setaffinity,
			*tc->pid, *tc->size, *tc->mask));

	if (TST_RET != -1) {
		tst_res(TFAIL, "sched_setaffinity() succeded unexpectedly");
		kill_pid();
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"sched_setaffinity() should fail with %s, got",
			tst_strerrno(tc->exp_errno));
	} else {
		tst_res(TPASS | TTERRNO, "sched_setaffinity() failed");
	}

	if (tc->exp_errno == EPERM)
		kill_pid();
}

static void setup(void)
{
	long ncpus;
	ncpus = tst_ncpus_max();
	fmask = tst_get_bad_addr(NULL);

	mask = CPU_ALLOC(ncpus);
	if (!mask)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC() failed");

	mask_size = CPU_ALLOC_SIZE(ncpus);

	if (sched_getaffinity(0, mask_size, mask) < 0)
		tst_brk(TBROK | TERRNO, "sched_getaffinity() failed");

	emask = CPU_ALLOC(ncpus + 1);
	if (!emask)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC() failed");

	emask_size = CPU_ALLOC_SIZE(ncpus + 1);
	CPU_ZERO_S(emask_size, emask);
	CPU_SET_S(ncpus, emask_size, emask);

	ltpuser = SAFE_GETPWNAM("nobody");

	free_pid = tst_get_unused_pid();
}

static void cleanup(void)
{
	if (mask)
		CPU_FREE(mask);

	if (emask)
		CPU_FREE(emask);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_test,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_root = 1,
};
