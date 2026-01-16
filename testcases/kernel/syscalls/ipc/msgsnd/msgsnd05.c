// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Linux Test Project, 2002-2024
 */

/*\
 * Verify that msgsnd(2) fails and sets correct errno:
 *
 * - EAGAIN if the message can't be sent due to the msg_qbytes limit for the
 *   queue and IPC_NOWAIT is specified
 * - EINTR if msgsnd(2) sleeps on a full message queue condition and the process
 *   catches a signal
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char text[MSGSIZE];
} snd_buf = {1, "hello"};

static struct tcase {
	int flag;
	int exp_err;
	/*1: nobody expected  0: root expected */
	int exp_user;
} tcases[] = {
	{IPC_NOWAIT, EAGAIN, 0},
	{0, EINTR, 1}
};

static void verify_msgsnd(struct tcase *tc)
{
	TST_EXP_FAIL(msgsnd(queue_id, &snd_buf, MSGSIZE, tc->flag), tc->exp_err,
		"msgsnd(%i, %p, %i, %i)", queue_id, &snd_buf, MSGSIZE, tc->flag);
}

static void sighandler(int sig)
{
	if (sig == SIGHUP)
		return;
	else
		_exit(TBROK);
}

static void do_test(unsigned int n)
{
	pid_t pid;
	struct tcase *tc = &tcases[n];

	if (tc->exp_user == 0) {
		verify_msgsnd(tc);
		return;
	}

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_SIGNAL(SIGHUP, sighandler);
		verify_msgsnd(tc);
		_exit(0);
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_KILL(pid, SIGHUP);
	tst_reap_children();
}

static void setup(void)
{
	msgkey = GETIPCKEY();

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);

	while (msgsnd(queue_id, &snd_buf, MSGSIZE, IPC_NOWAIT) != -1)
		snd_buf.type += 1;
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test
};
