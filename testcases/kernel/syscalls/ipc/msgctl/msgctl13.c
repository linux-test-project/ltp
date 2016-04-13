/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */
/*
 * DESCRIPTION
 *	msgctl13 - test for IPC_RMID
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "test.h"
#include "ipcmsg.h"

char *TCID = "msgctl13";
int TST_TOTAL = 1;
static struct msqid_ds buf;

static void msgctl_verify(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		msgctl_verify();
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void msgctl_verify(void)
{
	int msg_q;

	msg_q = msgget(IPC_PRIVATE, MSG_RW);
	if (msg_q == -1)
		tst_brkm(TBROK, cleanup, "Can't create message queue");

	TEST(msgctl(msg_q, IPC_RMID, NULL));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL, "msgctl() test IPC_RMID failed with errno: %d",
			 TEST_ERRNO);
		return;
	}

	TEST(msgctl(msg_q, IPC_STAT, &buf));
	if (TEST_ERRNO == EINVAL)
		tst_resm(TPASS, "msgctl() test IPC_RMID succeeded");
	else
		tst_resm(TFAIL, "msgctl() test IPC_RMID failed unexpectedly");
}

void cleanup(void)
{
}
