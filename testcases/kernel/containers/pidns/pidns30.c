// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Bull S.A.S. 2008
 *               01/12/08  Nadia Derbey <Nadia.Derbey@bull.net>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone a process with CLONE_NEWPID flag, register notification on a posix
 * mqueue and send a mqueue message from the parent. Then check if signal
 * notification contains si_pid of the parent.
 */

#define _GNU_SOURCE
#include <signal.h>
#include <mqueue.h>
#include "tst_test.h"
#include "tst_safe_posix_ipc.h"
#include "lapi/sched.h"

#define MQNAME "/LTP_PIDNS30_MQ"

static mqd_t mqd = -1;
static siginfo_t info;
static volatile int received;

static void remove_mqueue(mqd_t mqd)
{
	if (mqd != -1)
		SAFE_MQ_CLOSE(mqd);

	mq_unlink(MQNAME);
}

static void child_signal_handler(LTP_ATTRIBUTE_UNUSED int sig, siginfo_t *si, LTP_ATTRIBUTE_UNUSED void *unused)
{
	received = 1;
	memcpy(&info, si, sizeof(info));
}

static void child_func(void)
{
	pid_t cpid, ppid;
	struct sigaction sa;
	struct sigevent notif;
	mqd_t mqd_child;

	cpid = tst_getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Register notification on posix mqueue");

	mqd_child = SAFE_MQ_OPEN(MQNAME, O_RDONLY, 0, NULL);
	notif.sigev_notify = SIGEV_SIGNAL;
	notif.sigev_signo = SIGUSR1;
	notif.sigev_value.sival_int = mqd_child;

	SAFE_MQ_NOTIFY(mqd_child, &notif);

	sa.sa_flags = SA_SIGINFO;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (received)
		tst_res(TPASS, "Signal notification has been received");
	else
		tst_res(TFAIL, "Signal notification has not been received");

	TST_EXP_EQ_LI(info.si_signo, SIGUSR1);
	TST_EXP_EQ_LI(info.si_code, SI_MESGQ);
	TST_EXP_EQ_LI(info.si_pid, 0);
}

static void cleanup(void)
{
	remove_mqueue(mqd);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};

	remove_mqueue(mqd);
	received = 0;

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}

	mqd = SAFE_MQ_OPEN(MQNAME, O_RDWR | O_CREAT | O_EXCL, 0777, 0);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_res(TINFO, "Send mqueue message");

	SAFE_MQ_SEND(mqd, "pippo", 5, 1);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
