/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    03/2001 - Written by Wayne Boyer
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * Create a message queue, then issue the IPC_SET command to lower the
 * msg_qbytes value.
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int msg_id = -1;
struct msqid_ds orig_buf;

static void verify_msgctl(void)
{
	struct msqid_ds buf = orig_buf;

	buf.msg_qbytes -= 1;

	TEST(msgctl(msg_id, IPC_SET, &buf));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "msgctl(IPC_SET) failed");
		return;
	}

	tst_res(TPASS, "msgctl(IPC_SET) msg_qbytes - 1");

	memset(&buf, 0, sizeof(buf));
	SAFE_MSGCTL(msg_id, IPC_STAT, &buf);

	if (buf.msg_qbytes == orig_buf.msg_qbytes - 1) {
		tst_res(TPASS, "msg_qbytes = %lu",
			(unsigned long)buf.msg_qbytes);
	} else {
		tst_res(TFAIL, "msg_qbytes = %lu, expected %lu",
			(unsigned long)buf.msg_qbytes,
			(unsigned long)orig_buf.msg_qbytes - 1);
	}

	SAFE_MSGCTL(msg_id, IPC_SET, &orig_buf);
}

static void setup(void)
{
	key_t msgkey = GETIPCKEY();

	msg_id = SAFE_MSGGET(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW | 0660);

	SAFE_MSGCTL(msg_id, IPC_STAT, &orig_buf);
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
