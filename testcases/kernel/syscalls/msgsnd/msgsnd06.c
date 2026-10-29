// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 * Tests if EIDRM is returned when message queue was removed while
 * msgsnd() was trying to send a message.
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

static void verify_msgsnd(void)
{
	TST_EXP_FAIL(msgsnd(queue_id, &snd_buf, MSGSIZE, 0), EIDRM,
		"msgsnd(%i, %p, %i, 0)", queue_id, &snd_buf, MSGSIZE);
}

static void do_test(void)
{
	pid_t pid;

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);

	while (msgsnd(queue_id, &snd_buf, MSGSIZE, IPC_NOWAIT) != -1)
		snd_buf.type += 1;

	pid = SAFE_FORK();
	if (!pid) {
		verify_msgsnd();
		_exit(0);
	}

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);

	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);

	tst_reap_children();
}

static void setup(void)
{
	msgkey = GETIPCKEY();
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
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test
};
