// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*
 * This tests the rt_sigqueueinfo() syscall.
 *
 * It does so by creating a thread which registers the corresponding
 * signal handler. After that the main thread sends a signal and data
 * to the handler thread. If the correct signal and data is received,
 * the test is successful.
 */

#include <signal.h>
#include <stdlib.h>
#include "config.h"
#include "tst_test.h"
#include "tst_safe_pthread.h"

#ifdef HAVE_STRUCT_SIGACTION_SA_SIGACTION
#include "rt_sigqueueinfo.h"

#define SIGNAL	SIGUSR1
#define DATA	777

static struct sigaction *sig_action;
static int sig_rec;
static siginfo_t *uinfo;
static pid_t tid;

static void received_signal(int sig, siginfo_t *info, void *ucontext)
{
	if (info && ucontext) {
		if (sig == SIGNAL && info->si_value.sival_int == DATA) {
			tst_res(TPASS, "Received correct signal and data!");
			sig_rec = 1;
		} else
			tst_res(TFAIL, "Received wrong signal and/or data!");
	} else
		tst_res(TFAIL, "Signal handling went wrong!");
}

static void *handle_thread(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret;

	tid = tst_syscall(__NR_gettid);

	ret = sigaction(SIGNAL, sig_action, NULL);
	if (ret)
		tst_brk(TBROK, "Failed to set sigaction for handler thread!");

	TST_CHECKPOINT_WAKE(0);
	TST_CHECKPOINT_WAIT(1);
	return arg;
}

static void verify_sigqueueinfo(void)
{
	pthread_t thr;

	SAFE_PTHREAD_CREATE(&thr, NULL, handle_thread, NULL);

	TST_CHECKPOINT_WAIT(0);

	TEST(sys_rt_sigqueueinfo(tid, SIGNAL, uinfo));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "rt_sigqueueinfo() failed");
		return;
	}

	TST_CHECKPOINT_WAKE(1);
	SAFE_PTHREAD_JOIN(thr, NULL);

	if (sig_rec)
		tst_res(TPASS, "rt_sigqueueinfo() was successful!");
}

static void setup(void)
{
	sig_action = SAFE_MALLOC(sizeof(struct sigaction));

	memset(sig_action, 0, sizeof(*sig_action));
	sig_action->sa_sigaction = received_signal;
	sig_action->sa_flags = SA_SIGINFO;

	uinfo = SAFE_MALLOC(sizeof(siginfo_t));

	memset(uinfo, 0, sizeof(*uinfo));
	uinfo->si_code = SI_QUEUE;
	uinfo->si_pid = getpid();
	uinfo->si_uid = getuid();
	uinfo->si_value.sival_int = DATA;

	sig_rec = 0;
}

static void cleanup(void)
{
	free(uinfo);
	free(sig_action);
}

static struct tst_test test = {
	.test_all = verify_sigqueueinfo,
	.setup = setup,
	.cleanup = cleanup,
	.needs_checkpoints = 1,
	.timeout = 20,
};

#else
	TST_TEST_TCONF(
		"This system does not support rt_sigqueueinfo().");
#endif /* HAVE_STRUCT_SIGACTION_SA_SIGACTION */
