// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Test for EEXIST, ENOENT, EACCES errors.
 *
 * -  msgget(2) fails if a message queue exists for key and msgflg
 *    specified both IPC_CREAT and IPC_EXCL.
 * -  msgget(2) fails if no message queue exists for key and msgflg
 *    did not specify IPC_CREAT.
 * -  msgget(2) fails if a message queue exists for key, but the
 *    calling process does not have permission to access the queue,
 *    and does not have the CAP_IPC_OWNER capability.
 *
 */
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static key_t msgkey, msgkey1;
static int queue_id = -1;
static struct passwd *pw;

static struct tcase {
	int *key;
	int flags;
	int exp_err;
	/*1: nobody expected  0: root expected */
	int exp_user;
} tcases[] = {
	{&msgkey, IPC_CREAT | IPC_EXCL, EEXIST, 0},
	{&msgkey1, IPC_PRIVATE, ENOENT, 0},
	{&msgkey1, IPC_EXCL, ENOENT, 0},
	{&msgkey, MSG_RD, EACCES, 1},
	{&msgkey, MSG_WR, EACCES, 1},
	{&msgkey, MSG_RW, EACCES, 1}
};

static void verify_msgget(struct tcase *tc)
{
	TST_EXP_FAIL2(msgget(*tc->key, tc->flags), tc->exp_err, "msgget(%i, %i)",
		*tc->key, tc->flags);
}

static void do_test(unsigned int n)
{
	pid_t pid;
	struct tcase *tc = &tcases[n];

	if (tc->exp_user == 0) {
		verify_msgget(tc);
	} else {
		pid = SAFE_FORK();
		if (pid) {
			tst_reap_children();
		} else {
			SAFE_SETUID(pw->pw_uid);
			verify_msgget(tc);
			exit(0);
		}
	}
}

static void setup(void)
{
	msgkey = GETIPCKEY();
	msgkey1 = GETIPCKEY();

	queue_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL);

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
