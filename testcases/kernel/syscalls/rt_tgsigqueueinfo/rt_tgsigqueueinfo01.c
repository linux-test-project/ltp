// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Copyright (c) Linux Test Project, 2019-2024
 * Author: Sumit Garg <sumit.garg@linaro.org>
 */

/*\
 * Basic test for rt_tgsigqueueinfo(2) syscall. It sends the signal and data
 * to the single thread specified by the combination of tgid, a thread group
 * ID, and tid, a thread in that thread group.
 *
 * Also this implement 3 tests differing on the basis of signal sender:
 *
 * - Sender and receiver is the same thread.
 * - Sender is parent of the thread.
 * - Sender is different thread.
 */

#define _GNU_SOURCE

#include <err.h>
#include <pthread.h>
#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

static char sigval_send[] = "rt_tgsigqueueinfo data";
static volatile int signum_rcv;
static char *sigval_rcv;

static void sigusr1_handler(int signum, siginfo_t *uinfo,
			    void *p LTP_ATTRIBUTE_UNUSED)
{
	signum_rcv = signum;
	sigval_rcv = uinfo->si_ptr;
}

void *send_rcv_func(void *arg)
{
	siginfo_t uinfo;

	signum_rcv = 0;
	sigval_rcv = NULL;

	uinfo.si_errno = 0;
	uinfo.si_code = SI_QUEUE;
	uinfo.si_ptr = sigval_send;

	TEST(tst_syscall(__NR_rt_tgsigqueueinfo, getpid(),
			 syscall(__NR_gettid), SIGUSR1, &uinfo));
	if (TST_RET)
		tst_brk(TFAIL | TTERRNO, "rt_tgsigqueueinfo failed");

	while (!signum_rcv)
		usleep(1000);

	if ((signum_rcv == SIGUSR1) && (sigval_rcv == sigval_send))
		tst_res(TPASS, "Test signal to self succeeded");
	else
		tst_res(TFAIL, "Failed to deliver signal/data to self thread");

	return arg;
}

static void verify_signal_self(void)
{
	pthread_t pt;

	SAFE_PTHREAD_CREATE(&pt, NULL, send_rcv_func, NULL);

	SAFE_PTHREAD_JOIN(pt, NULL);
}

void *receiver_func(void *arg)
{
	pid_t *tid = arg;

	*tid = syscall(__NR_gettid);

	signum_rcv = 0;
	sigval_rcv = NULL;

	TST_CHECKPOINT_WAKE(0);

	while (!signum_rcv)
		usleep(1000);

	if ((signum_rcv == SIGUSR1) && (sigval_rcv == sigval_send))
		tst_res(TPASS, "Test signal to different thread succeeded");
	else
		tst_res(TFAIL,
			"Failed to deliver signal/data to different thread");

	return NULL;
}

static void verify_signal_parent_thread(void)
{
	pid_t tid = -1;
	pthread_t pt;
	siginfo_t uinfo;

	SAFE_PTHREAD_CREATE(&pt, NULL, receiver_func, &tid);

	TST_CHECKPOINT_WAIT(0);

	uinfo.si_errno = 0;
	uinfo.si_code = SI_QUEUE;
	uinfo.si_ptr = sigval_send;

	TEST(tst_syscall(__NR_rt_tgsigqueueinfo, getpid(),
			 tid, SIGUSR1, &uinfo));
	if (TST_RET)
		tst_brk(TFAIL | TTERRNO, "rt_tgsigqueueinfo failed");

	SAFE_PTHREAD_JOIN(pt, NULL);
}

void *sender_func(void *arg)
{
	pid_t *tid = arg;
	siginfo_t uinfo;

	uinfo.si_errno = 0;
	uinfo.si_code = SI_QUEUE;
	uinfo.si_ptr = sigval_send;

	TEST(tst_syscall(__NR_rt_tgsigqueueinfo, getpid(),
			 *tid, SIGUSR1, &uinfo));
	if (TST_RET)
		tst_brk(TFAIL | TTERRNO, "rt_tgsigqueueinfo failed");

	return NULL;
}

static void verify_signal_inter_thread(void)
{
	pid_t tid = -1;
	pthread_t pt1, pt2;

	SAFE_PTHREAD_CREATE(&pt1, NULL, receiver_func, &tid);

	TST_CHECKPOINT_WAIT(0);

	SAFE_PTHREAD_CREATE(&pt2, NULL, sender_func, &tid);

	SAFE_PTHREAD_JOIN(pt2, NULL);

	SAFE_PTHREAD_JOIN(pt1, NULL);
}

static struct tcase {
	void (*tfunc)(void);
} tcases[] = {
	{&verify_signal_self},
	{&verify_signal_parent_thread},
	{&verify_signal_inter_thread},
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
}

static void setup(void)
{
	struct sigaction sigusr1 = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = sigusr1_handler,
	};

	SAFE_SIGACTION(SIGUSR1, &sigusr1, NULL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_checkpoints = 1,
	.setup = setup,
	.test = run,
};
