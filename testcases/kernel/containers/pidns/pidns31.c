// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Bull S.A.S. 2008
 *               01/12/08  Nadia Derbey <Nadia.Derbey@bull.net>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag, register notification on a posix
 * mqueue and send a mqueue message from the child. Then check if signal
 * notification contains si_pid of the child.
 */

#define _GNU_SOURCE 1
#include <signal.h>
#include <mqueue.h>
#include "tst_test.h"
#include "tst_safe_posix_ipc.h"
#include "lapi/sched.h"

#define MQNAME "/LTP_PIDNS30_MQ"

static mqd_t mqd = -1;
static volatile int received;
static siginfo_t info;

static void remove_mqueue(mqd_t mqd)
{
	if (mqd != -1)
		SAFE_MQ_CLOSE(mqd);

	mq_unlink(MQNAME);
}

static void signal_handler(LTP_ATTRIBUTE_UNUSED int sig, siginfo_t *si, LTP_ATTRIBUTE_UNUSED void *unused)
{
	memcpy(&info, si, sizeof(info));
	received++;
}

static void child_func(void)
{
	pid_t cpid, ppid;
	mqd_t mqd_child;

	cpid = getpid();
	ppid = getppid();

	TST_EXP_EQ_LI(cpid, 1);
	TST_EXP_EQ_LI(ppid, 0);

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Send mqueue message from child");

	mqd_child = SAFE_MQ_OPEN(MQNAME, O_WRONLY, 0, NULL);
	SAFE_MQ_SEND(mqd_child, "pippo", 5, 1);

	TST_CHECKPOINT_WAKE(0);
}

static void cleanup(void)
{
	remove_mqueue(mqd);
}

static void run(void)
{
	pid_t cpid;
	struct sigaction sa;
	struct sigevent notif;
	const struct tst_clone_args args = { CLONE_NEWPID, SIGCHLD };

	remove_mqueue(mqd);
	received = 0;

	cpid = SAFE_CLONE(&args);
	if (!cpid) {
		child_func();
		return;
	}

	tst_res(TINFO, "Register notification on posix mqueue");

	mqd = SAFE_MQ_OPEN(MQNAME, O_RDWR | O_CREAT | O_EXCL, 0777, NULL);

	notif.sigev_notify = SIGEV_SIGNAL;
	notif.sigev_signo = SIGUSR1;

	SAFE_MQ_NOTIFY(mqd, &notif);

	sa.sa_flags = SA_SIGINFO;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	sa.sa_sigaction = signal_handler;
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_reap_children();

	TST_EXP_EQ_LI(received, 1);
	TST_EXP_EQ_LI(info.si_signo, SIGUSR1);
	TST_EXP_EQ_LI(info.si_code, SI_MESGQ);
	TST_EXP_EQ_LI(info.si_pid, cpid);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
};
