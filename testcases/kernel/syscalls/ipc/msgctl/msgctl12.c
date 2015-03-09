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
 *	msgctl12 - test for IPC_INFO MSG_INFO and MSG_STAT.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "test.h"
#include "ipcmsg.h"

static int msg_q;
static int index_q;
static struct msginfo msginfo_buf;
static struct msqid_ds msgqid_buf;

static struct test_case_t {
	int *queue_id;
	int ipc_cmd;
	char *name;
	void *buf;
} test_cases[] = {
	{&msg_q, IPC_INFO, "IPC_INFO", &msginfo_buf},
	{&msg_q, MSG_INFO, "MSG_INFO", &msginfo_buf},
	{&index_q, MSG_STAT, "MSG_STAT", &msgqid_buf},
};

char *TCID = "msgctl12";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(msgctl(*test_cases[i].queue_id,
				    test_cases[i].ipc_cmd, test_cases[i].buf));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "msgctl() test %s failed with errno: "
					 "%d", test_cases[i].name, TEST_ERRNO);
			} else {
				tst_resm(TPASS, "msgctl() test %s succeeded",
					 test_cases[i].name);
			}
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	msg_q = msgget(IPC_PRIVATE, MSG_RW);
	if (msg_q < 0)
		tst_brkm(TBROK, cleanup, "Can't create message queue");

	index_q = msgctl(msg_q, IPC_INFO, (struct msqid_ds *)&msginfo_buf);
	if (index_q < 0)
		tst_brkm(TBROK, cleanup, "Can't create message queue");
}

void cleanup(void)
{
	rm_queue(msg_q);
}
