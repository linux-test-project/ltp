// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *    03/2001 - Written by Wayne Boyer
 *    12/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * Test for EACCES, EFAULT and EINVAL errors using a variety of incorrect
 * calls.
 */
#include <errno.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int msg_id1 = -1;
static int msg_id2 = -1;
static int msg_id3 = -1;
static int bad_q = -1;

struct msqid_ds q_buf;

struct tcase {
	int *msg_id;
	int cmd;
	struct msqid_ds *buf;
	int error;
} tc[] = {
	/* EACCES - there is no read permission for the queue */
	{&msg_id1, IPC_STAT, &q_buf, EACCES},
	/* EFAULT - the structure address is invalid - IPC_STAT */
	{&msg_id2, IPC_STAT, (struct msqid_ds *)-1, EFAULT},
	/* EFAULT - the structure address is invalid - IPC_SET */
	{&msg_id2, IPC_SET, (struct msqid_ds *)-1, EFAULT},
	/* EINVAL - the command (-1) is invalid */
	{&msg_id2, -1, &q_buf, EINVAL},
	/* EINVAL - the queue id is invalid - IPC_STAT */
	{&bad_q, IPC_STAT, &q_buf, EINVAL},
	/* EINVAL - the queue id is invalid - IPC_SET */
	{&bad_q, IPC_SET, &q_buf, EINVAL},
	/* EPERM - cannot delete root owned queue */
	{&msg_id3, IPC_RMID, NULL, EPERM},
};

static void verify_msgctl(unsigned int i)
{
	TEST(msgctl(*(tc[i].msg_id), tc[i].cmd, tc[i].buf));

	if (TST_RET != -1) {
		tst_res(TFAIL, "msgctl() returned %li", TST_RET);
		return;
	}

	if (TST_ERR == tc[i].error) {
		tst_res(TPASS | TTERRNO, "msgctl(%i, %i, %p)",
			*tc[i].msg_id, tc[i].cmd, tc[i].buf);
		return;
	}

	tst_res(TFAIL | TTERRNO, "msgctl(%i, %i, %p) expected %s",
		*tc[i].msg_id, tc[i].cmd, tc[i].buf, tst_strerrno(tc[i].error));
}

static void setup(void)
{
	key_t msgkey1, msgkey2;
	struct passwd *ltpuser;

	msg_id3 = SAFE_MSGGET(IPC_PRIVATE, IPC_CREAT | MSG_RW);

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);

	msgkey1 = GETIPCKEY();
	msgkey2 = GETIPCKEY();

	msg_id1 = SAFE_MSGGET(msgkey1, IPC_CREAT | IPC_EXCL);
	msg_id2 = SAFE_MSGGET(msgkey2, IPC_CREAT | IPC_EXCL | MSG_RD | MSG_WR);
}

static void cleanup(void)
{
	if (msg_id1 >= 0)
		SAFE_MSGCTL(msg_id1, IPC_RMID, NULL);

	if (msg_id2 >= 0)
		SAFE_MSGCTL(msg_id2, IPC_RMID, NULL);

	if (msg_id3 >= 0) {
		SAFE_SETEUID(0);
		SAFE_MSGCTL(msg_id3, IPC_RMID, NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_msgctl,
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.needs_root = 1,
};
