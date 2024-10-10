// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Ma Xinjian <maxj.fnst@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that, rt_sigqueueinfo(2) sets errno to
 *
 * - EINVAL if sig is invalid
 * - EPERM if uinfo->si_code is invalid
 * - ESRCH if no thread group matching tgid is found
 */

#include <pwd.h>
#include <signal.h>
#include "config.h"
#include "tst_test.h"

#ifdef HAVE_STRUCT_SIGACTION_SA_SIGACTION
#include "rt_sigqueueinfo.h"

static siginfo_t siginfo_einval;
static siginfo_t siginfo_eperm;
static siginfo_t siginfo_esrch;

static pid_t tgid_notfound = -1;

static struct test_case_t {
	pid_t *tgid;
	int sig;
	siginfo_t *uinfo;
	int expected_errno;
	char *desc;
} tcases[] = {
	{NULL, -1, &siginfo_einval, EINVAL, "sig is invalid"},
	{NULL, SIGUSR1, &siginfo_eperm, EPERM, "uinfo->si_code is invalid"},
	{&tgid_notfound, SIGUSR1, &siginfo_esrch, ESRCH,
		"no thread group matching tgid is found"},
};

static void setup(void)
{
	siginfo_einval.si_code = SI_QUEUE;
	siginfo_eperm.si_code = 0;
	siginfo_esrch.si_code = SI_QUEUE;
}

static void parent_do(struct test_case_t *tc, pid_t pid)
{
	pid_t real_pid;

	if (tc->tgid)
		real_pid = *(tc->tgid);
	else
		real_pid = pid;

	TST_EXP_FAIL(sys_rt_sigqueueinfo(real_pid, tc->sig, tc->uinfo),
		tc->expected_errno, "%s", tc->desc);
	TST_CHECKPOINT_WAKE(0);
}

static void child_do(void)
{
	TST_CHECKPOINT_WAIT(0);
}

static void verify_rt_sigqueueinfo(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];
	pid_t pid = SAFE_FORK();

	if (!pid) {
		child_do();
		exit(0);
	}
	parent_do(tc, pid);
	SAFE_WAITPID(pid, NULL, 0);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_rt_sigqueueinfo,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.bufs = (struct tst_buffers []) {
		{&siginfo_einval, .size = sizeof(siginfo_einval)},
		{&siginfo_eperm, .size = sizeof(siginfo_eperm)},
		{&siginfo_esrch, .size = sizeof(siginfo_esrch)},
		{},
	}
};

#else
	TST_TEST_TCONF("This system does not support rt_sigqueueinfo()");
#endif /* HAVE_STRUCT_SIGACTION_SA_SIGACTION */
