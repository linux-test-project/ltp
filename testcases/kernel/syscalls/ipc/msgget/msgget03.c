/*
 * Copyright (c) International Business Machines  Corp., 2001
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
 * along with this program.
 */

/*
 * DESCRIPTION
 * test for an ENOSPC error by using up all available
 * message queues.
 *
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"

static int maxmsgs;
static int *queues;
static key_t msgkey;

static void verify_msgget(void)
{
	TEST(msgget(msgkey + maxmsgs, IPC_CREAT | IPC_EXCL));
	if (TEST_RETURN != -1)
		tst_res(TFAIL, "msgget() succeeded unexpectedly");

	if (TEST_ERRNO == ENOSPC) {
		tst_res(TPASS | TTERRNO, "msgget() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "msgget() failed unexpectedly,"
			" expected ENOSPC");
	}
}

static void setup(void)
{
	int res, num;

	msgkey = GETIPCKEY();

	SAFE_FILE_SCANF("/proc/sys/kernel/msgmni", "%i", &maxmsgs);

	queues = SAFE_MALLOC(maxmsgs * sizeof(int));

	for (num = 0; num < maxmsgs; num++) {
		queues[num] = -1;

		res = msgget(msgkey + num, IPC_CREAT | IPC_EXCL);
		if (res != -1)
			queues[num] = res;
	}

	tst_res(TINFO, "The maximum number of message queues (%d) reached",
		maxmsgs);
}

static void cleanup(void)
{
	int num;

	if (!queues)
		return;

	for (num = 0; num < maxmsgs; num++) {
		if (queues[num] != -1)
			SAFE_MSGCTL(queues[num], IPC_RMID, NULL);
	}

	free(queues);
}

static struct tst_test test = {
	.tid = "msgget03",
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_msgget
};
