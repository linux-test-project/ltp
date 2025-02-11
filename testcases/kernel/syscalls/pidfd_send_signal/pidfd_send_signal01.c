// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*\
 * Tests if the pidfd_send_signal syscall behaves
 * like rt_sigqueueinfo when a pointer to a siginfo_t
 * struct is passed.
 */

#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/pidfd.h"
#include "tst_safe_pthread.h"

#define SIGNAL  SIGUSR1
#define DATA	777

static struct sigaction *sig_action;
static int sig_rec;
static siginfo_t *uinfo;
static int pidfd;

static void received_signal(int sig, siginfo_t *info, void *ucontext)
{
	if (info && ucontext) {
		if (sig == SIGNAL && info->si_value.sival_int == DATA) {
			tst_res(TPASS, "Received correct signal and data!");
			sig_rec = 1;
		} else {
			tst_res(TFAIL, "Received wrong signal and/or data!");
		}
	} else {
		tst_res(TFAIL, "Signal handling went wrong!");
	}
}

static void *handle_thread(void *arg)
{
	SAFE_SIGACTION(SIGNAL, sig_action, NULL);
	TST_CHECKPOINT_WAKE_AND_WAIT(0);
	return arg;
}

static void verify_pidfd_send_signal(void)
{
	pthread_t thr;

	SAFE_PTHREAD_CREATE(&thr, NULL, handle_thread, NULL);

	TST_CHECKPOINT_WAIT(0);

	TEST(pidfd_send_signal(pidfd, SIGNAL, uinfo, 0));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "pidfd_send_signal() failed");
		return;
	}

	TST_CHECKPOINT_WAKE(0);
	SAFE_PTHREAD_JOIN(thr, NULL);

	if (sig_rec) {
		tst_res(TPASS,
			"pidfd_send_signal() behaved like rt_sigqueueinfo()");
	}
}

static void setup(void)
{
	pidfd_send_signal_supported();

	pidfd = SAFE_OPEN("/proc/self", O_DIRECTORY | O_CLOEXEC);

	sig_action = SAFE_MALLOC(sizeof(struct sigaction));

	memset(sig_action, 0, sizeof(*sig_action));
	sig_action->sa_sigaction = received_signal;
	sig_action->sa_flags = SA_SIGINFO;

	uinfo = SAFE_MALLOC(sizeof(siginfo_t));

	memset(uinfo, 0, sizeof(*uinfo));
	uinfo->si_signo = SIGNAL;
	uinfo->si_code = SI_QUEUE;
	uinfo->si_pid = getpid();
	uinfo->si_uid = getuid();
	uinfo->si_value.sival_int = DATA;
}

static void cleanup(void)
{
	free(uinfo);
	free(sig_action);
	if (pidfd > 0)
		SAFE_CLOSE(pidfd);
}

static struct tst_test test = {
	.test_all = verify_pidfd_send_signal,
	.setup = setup,
	.cleanup = cleanup,
	.needs_checkpoints = 1,
};
