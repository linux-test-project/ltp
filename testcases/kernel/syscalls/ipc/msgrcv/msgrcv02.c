// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Basic error test for msgrcv(2).
 *
 * 1)msgrcv(2) fails and sets errno to E2BIG if the message text length is
 *    greater than msgsz and MSG_NOERROR isn't specified in msgflg.
 *
 * 2)The calling process does not have read permission on the message
 *    queue, so msgrcv(2) fails and sets errno to EACCES.
 *
 * 3)msgrcv(2) fails and sets errno to EFAULT if the message buffer address
 *    isn't accessible.
 *
 * 4)msgrcv(2) fails and sets errno to EINVAL if msqid was invalid(<0).
 *
 * 5)msgrcv(2) fails and sets errno to EINVAL if msgsize is less than 0.
 *
 * 6)msgrcv(2) fails and sets errno to ENOMSG if IPC_NOWAIT was specified in
 *   msgflg and no message of the requested type existed on the message queue.
 */

#define _GNU_SOURCE

#include <string.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1;
static int bad_id = -1;
struct passwd *pw;

static struct buf {
	long type;
	char mtext[MSGSIZE];
} rcv_buf, snd_buf = {2, "hello"};

static struct tcase {
	int *id;
	struct buf *buffer;
	int msgsz;
	long msgtyp;
	int msgflag;
	int exp_user;
	int exp_err;
} tcases[] = {
	{&queue_id, &rcv_buf, MSGSIZE - 1, 2, 0, 0, E2BIG},
	{&queue_id, &rcv_buf, MSGSIZE,     2, 0, 1, EACCES},
	{&queue_id, NULL,     MSGSIZE,     2, 0, 0, EFAULT},
	{&bad_id,   &rcv_buf, MSGSIZE,     2, 0, 0, EINVAL},
	{&queue_id, &rcv_buf, -1,          2, 0, 0, EINVAL},
	{&queue_id, &rcv_buf, MSGSIZE,  3, IPC_NOWAIT,              0, ENOMSG},
	{&queue_id, &rcv_buf, MSGSIZE, -1, IPC_NOWAIT,              0, ENOMSG},
	{&queue_id, &rcv_buf, MSGSIZE, -1, IPC_NOWAIT | MSG_EXCEPT, 0, ENOMSG},
};

static void verify_msgrcv(struct tcase *tc)
{
	TST_EXP_FAIL2(msgrcv(*tc->id, tc->buffer, tc->msgsz, tc->msgtyp, tc->msgflag), tc->exp_err,
		"msgrcv(%i, %p, %i, %ld, %i)", *tc->id, tc->buffer, tc->msgsz, tc->msgtyp, tc->msgflag);
}

static void do_test(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid;

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);

	SAFE_MSGSND(queue_id, &snd_buf, MSGSIZE, 0);
	pid = SAFE_FORK();
	if (pid == 0) {
		if (tc->exp_user)
			SAFE_SETUID(pw->pw_uid);
		verify_msgrcv(tc);
		exit(0);
	}
	tst_reap_children();
	SAFE_MSGCTL(queue_id, IPC_RMID, NULL);
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	pw = SAFE_GETPWNAM("nobody");
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
