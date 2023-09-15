// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that IPC_STAT command succeeds and the buffer is filled with
 * correct data.
 */
#include <errno.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int msg_id = -1;
static time_t creat_time;
static key_t msgkey;
static uid_t uid;
static gid_t gid;
unsigned short mode = 0660;

static void verify_msgctl(void)
{
	struct msqid_ds buf;

	memset(&buf, 'a', sizeof(buf));
	TEST(msgctl(msg_id, IPC_STAT, &buf));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "msgctl() returned %li", TST_RET);
		return;
	}

	tst_res(TPASS, "msgctl(IPC_STAT)");

	if (buf.msg_stime == 0)
		tst_res(TPASS, "msg_stime = 0");
	else
		tst_res(TFAIL, "msg_stime = %lu", (unsigned long)buf.msg_stime);

	if (buf.msg_rtime == 0)
		tst_res(TPASS, "msg_rtime = 0");
	else
		tst_res(TFAIL, "msg_rtime = %lu", (unsigned long)buf.msg_rtime);

	if (buf.msg_ctime <= creat_time && buf.msg_ctime >= creat_time - 2) {
		tst_res(TPASS, "msg_ctime = %lu, expected %lu",
			(unsigned long)buf.msg_ctime, (unsigned long)creat_time);
	} else {
		tst_res(TFAIL, "msg_ctime = %lu, expected %lu",
			(unsigned long)buf.msg_ctime, (unsigned long)creat_time);
	}

	if (buf.msg_qnum == 0)
		tst_res(TPASS, "msg_qnum = 0");
	else
		tst_res(TFAIL, "msg_qnum = %li", (long)buf.msg_qnum);

	if (buf.msg_qbytes > 0)
		tst_res(TPASS, "msg_qbytes = %li", (long)buf.msg_qbytes);
	else
		tst_res(TFAIL, "msg_qbytes = %li", (long)buf.msg_qbytes);

	if (buf.msg_lspid == 0)
		tst_res(TPASS, "msg_lspid = 0");
	else
		tst_res(TFAIL, "msg_lspid = %u", (unsigned)buf.msg_lspid);

	if (buf.msg_lrpid == 0)
		tst_res(TPASS, "msg_lrpid = 0");
	else
		tst_res(TFAIL, "msg_lrpid = %u", (unsigned)buf.msg_lrpid);

	if (buf.msg_perm.__key == msgkey) {
		tst_res(TPASS, "msg_perm.__key == %u", (unsigned)msgkey);
	} else {
		tst_res(TFAIL, "msg_perm.__key == %u, expected %u",
			(unsigned)buf.msg_perm.__key, (unsigned)msgkey);
	}

	if (buf.msg_perm.uid == uid) {
		tst_res(TPASS, "msg_perm.uid = %u", (unsigned)uid);
	} else {
		tst_res(TFAIL, "msg_perm.uid = %u, expected %u",
			(unsigned)buf.msg_perm.uid, (unsigned)uid);
	}

	if (buf.msg_perm.gid == gid) {
		tst_res(TPASS, "msg_perm.gid = %u", (unsigned)gid);
	} else {
		tst_res(TFAIL, "msg_perm.gid = %u, expected %u",
			(unsigned)buf.msg_perm.gid, (unsigned)gid);
	}

	if (buf.msg_perm.cuid == uid) {
		tst_res(TPASS, "msg_perm.cuid = %u", (unsigned)uid);
	} else {
		tst_res(TFAIL, "msg_perm.cuid = %u, expected %u",
			(unsigned)buf.msg_perm.cuid, (unsigned)uid);
	}

	if (buf.msg_perm.cgid == gid) {
		tst_res(TPASS, "msg_perm.cgid = %u", (unsigned)gid);
	} else {
		tst_res(TFAIL, "msg_perm.cgid = %u, expected %u",
			(unsigned)buf.msg_perm.cgid, (unsigned)gid);
	}

	if ((buf.msg_perm.mode & MODE_MASK) == (mode & MODE_MASK)) {
		tst_res(TPASS, "msg_perm.mode = 0%ho", mode & MODE_MASK);
	} else {
		tst_res(TFAIL, "msg_perm.mode = 0%ho, expected %hx",
			buf.msg_perm.mode, (mode & MODE_MASK));
	}
}

static void setup(void)
{
	msgkey = GETIPCKEY();

	msg_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW | mode);
	time(&creat_time);

	uid = geteuid();
	gid = getegid();
}

static void cleanup(void)
{
	if (msg_id >= 0)
		SAFE_MSGCTL(msg_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgctl,
	.needs_tmpdir = 1
};
