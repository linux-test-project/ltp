// SPDX-License-Identifier: GPL-2.0-or-late
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 *
 * Author: Ulrich Drepper <drepper@redhat.com>
 *
 * History:
 *  Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>
 *  Ported to LTP - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>
 *  Converted into new api - Apri 15 2020 - Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "lapi/fcntl.h"
#include "tst_test.h"

static int fds[2];

static struct tcase {
	int flags;
	int cmd;
	int check_read_end;
	char *message;
} tcases[] = {
	{0, F_GETFD, 1, "Test pipe2 with 0 flag"},
	{O_CLOEXEC, F_GETFD, 1, "Test pipe2 using O_CLOEXEC flag"},
	/*
	 * It may get EINVAL error on older kernel because this flag was
	 * introduced since kernel 3.4. We only test flag in write end
	 * because this flag was used to make pipe buffer marked with the
	 * PIPE_BUF_FLAG_PACKET flag. In read end, kernel also checks buffer
	 * flag instead of O_DIRECT. So it make no sense to check this flag
	 * in fds[0].
	 */
	{O_DIRECT, F_GETFL, 0, "Test pipe2 using O_DIRECT flag"},
	{O_NONBLOCK, F_GETFL, 1, "Test pipe2 using O_NONBLOCK flag"},
};

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 1)
		SAFE_CLOSE(fds[1]);
}

static void verify_pipe2(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int get_flag = 0, i = 0;

	tst_res(TINFO, "%s ", tc->message);
	if ((tc->flags ==  O_DIRECT) && (tst_kvercmp(3, 4, 0)) < 0) {
		tst_res(TCONF, "O_DIRECT needs Linux 3.4 or newer");
		return;
	}

	SAFE_PIPE2(fds, tc->flags);
	for (i = 0; i < 2; i++) {
		if (i == 0 && !tc->check_read_end)
			continue;
		get_flag = SAFE_FCNTL(fds[i], tc->cmd);
		if ((get_flag && tc->flags) || (tc->flags == get_flag))
			tst_res(TPASS, "pipe2 fds[%d] gets expected flag(%d)", i, tc->flags);
		else
			tst_res(TFAIL, "pipe2 fds[%d] doesn't get expected flag(%d), get flag(%d)",
					i, tc->flags, get_flag);
	}
	cleanup();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_pipe2,
	.cleanup = cleanup,
};
