// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 * 1) The calling process does not have write permission on the message
 *    queue, so msgsnd(2) fails and sets errno to EACCES.
 * 2) msgsnd(2) fails and sets errno to EFAULT if the message buffer address
 *    is invalid.
 * 3) msgsnd(2) fails and sets errno to EINVAL if the queue ID is invalid.
 * 4) msgsnd(2) fails and sets errno to EINVAL if the message type is not
 *    positive (0).
 * 5) msgsnd(2) fails and sets errno to EINVAL if the message type is not
 *    positive (>0).
 * 6) msgsnd(2) fails and sets errno to EINVAL if the message size is less
 *    than zero.
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static key_t msgkey;
static int queue_id = -1;
static int bad_id = -1;
static struct passwd *pw;
static struct buf {
	long type;
	char text[MSGSIZE];
} snd_buf[] = {
	{1, "hello"},
	{0, "hello"},
	{-1, "hello"}
};

static struct tcase {
	int *id;
	struct buf *buffer;
	int msgsz;
	int exp_err;
	/*1: nobody expected  0: root expected */
	int exp_user;
} tcases[] = {
	{&queue_id, &snd_buf[0], MSGSIZE, EACCES, 1},
	{&queue_id, NULL, MSGSIZE, EFAULT, 0},
	{&bad_id, &snd_buf[0], MSGSIZE, EINVAL, 0},
	{&queue_id, &snd_buf[1], MSGSIZE, EINVAL, 0},
	{&queue_id, &snd_buf[2], MSGSIZE, EINVAL, 0},
	{&queue_id, &snd_buf[0], -1, EINVAL, 0}
};

static void verify_msgsnd(struct tcase *tc)
{
	TEST(msgsnd(*tc->id, tc->buffer, tc->msgsz, 0));
	if (TST_RET != -1) {
		tst_res(TFAIL, "smgsnd() succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "msgsnd() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "msgsnd() failed unexpectedly,"
			" expected %s", tst_strerrno(tc->exp_err));
	}
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
	if (pid) {
		tst_reap_children();
	} else {
		SAFE_SETUID(pw->pw_uid);
		verify_msgsnd(tc);
	}
}

static void setup(void)
{
	msgkey = GETIPCKEY();

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW);

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
