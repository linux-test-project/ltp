// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 *
 * Test simple tgkill() error cases.
 */

#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>

#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tgkill.h"

#define CHECK_ENOENT(x) ((x) == -1 && errno == ENOENT)

static pthread_t child_thread;

static pid_t parent_tgid;
static pid_t parent_tid;
static pid_t child_tid;
static pid_t defunct_tid;

static const int invalid_pid = -1;

static void *child_thread_func(void *arg)
{
	child_tid = sys_gettid();

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	return arg;
}

static void *defunct_thread_func(void *arg)
{
	defunct_tid = sys_gettid();

	return arg;
}

static void setup(void)
{
	sigset_t sigusr1;
	pthread_t defunct_thread;
	char defunct_tid_path[PATH_MAX];
	int ret;

	sigemptyset(&sigusr1);
	sigaddset(&sigusr1, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &sigusr1, NULL);

	parent_tgid = getpid();
	parent_tid = sys_gettid();

	SAFE_PTHREAD_CREATE(&child_thread, NULL, child_thread_func, NULL);

	TST_CHECKPOINT_WAIT(0);

	SAFE_PTHREAD_CREATE(&defunct_thread, NULL, defunct_thread_func, NULL);
	SAFE_PTHREAD_JOIN(defunct_thread, NULL);
	sprintf(defunct_tid_path, "/proc/%d/task/%d", getpid(), defunct_tid);
	ret = TST_RETRY_FN_EXP_BACKOFF(access(defunct_tid_path, R_OK),
		CHECK_ENOENT, 15);
	if (!CHECK_ENOENT(ret))
		tst_brk(TBROK, "Timeout, %s still exists", defunct_tid_path);
}

static void cleanup(void)
{
	TST_CHECKPOINT_WAKE(0);

	SAFE_PTHREAD_JOIN(child_thread, NULL);
}

static const struct testcase {
	const char *desc;
	const int *tgid;
	const int *tid;
	const int sig;
	const int err;
} testcases[] = {
	{ "Invalid tgid", &invalid_pid, &parent_tid, SIGUSR1, EINVAL },
	{ "Invalid tid", &parent_tgid, &invalid_pid, SIGUSR1, EINVAL },
	{ "Invalid signal", &parent_tgid, &parent_tid, -1, EINVAL },
	{ "Defunct tid", &parent_tgid, &defunct_tid, SIGUSR1, ESRCH },
	{ "Defunct tgid", &defunct_tid, &child_tid, SIGUSR1, ESRCH },
	{ "Valid tgkill call", &parent_tgid, &child_tid, SIGUSR1, 0 },
};

static void run(unsigned int i)
{
	const struct testcase *tc = &testcases[i];

	TEST(sys_tgkill(*tc->tgid, *tc->tid, tc->sig));
	if (tc->err) {
		if (TST_RET < 0 && TST_ERR == tc->err)
			tst_res(TPASS | TTERRNO, "%s failed as expected",
				tc->desc);
		else
			tst_res(TFAIL | TTERRNO,
				"%s should have failed with %s", tc->desc,
				tst_strerrno(tc->err));
	} else {
		if (TST_RET == 0)
			tst_res(TPASS, "%s succeeded", tc->desc);
		else
			tst_res(TFAIL | TTERRNO, "%s failed", tc->desc);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(testcases),
	.needs_checkpoints = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
};
