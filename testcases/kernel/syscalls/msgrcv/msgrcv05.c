// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * msgrcv error test for EINTR.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"

static key_t msgkey;
static int queue_id = -1;
static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf;

static void sighandler(int sig)
{
	if (sig == SIGHUP)
		return;
	else
		_exit(TBROK);
}

static void verify_msgrcv(void)
{
	TST_EXP_FAIL2(msgrcv(queue_id, &rcv_buf, MSGSIZE, 1, 0), EINTR,
		"msgrcv(%i, %p, %d, 1, 0)", queue_id, &rcv_buf, MSGSIZE);
}

static void do_test(void)
{
	int pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		verify_msgrcv();
		exit(0);
	}
	TST_PROCESS_STATE_WAIT(pid, 'S', 0);
	SAFE_KILL(pid, SIGHUP);
	tst_reap_children();
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);
	SAFE_SIGNAL(SIGHUP, sighandler);
}

static void cleanup(void)
{
	if (queue_id != -1)
		SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
