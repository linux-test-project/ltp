// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 *
 * Test basic error handling for fcntl(2) using F_SETPIPE_SZ, F_GETPIPE_SZ
 * argument.
 * 1)fcntl fails with EINVAL when cmd is F_SETPIPE_SZ and the arg is
 * beyond 1<<31.
 * 2)fcntl fails with EBUSY when cmd is F_SETPIPE_SZ and the arg is smaller
 * than the amount of the current used buffer space.
 * 3)fcntl fails with EPERM when cmd is F_SETPIPE_SZ and the arg is over
 * /proc/sys/fs/pipe-max-size limit under unprivileged users.
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/capability.h"

static int fds[2];
static unsigned int invalid_value, half_value, sys_value;

static struct tcase {
	unsigned int *setvalue;
	int exp_err;
	char *message;
} tcases[] = {
	{&invalid_value, EINVAL, "F_SETPIPE_SZ and size is beyond 1<<31"},
	{&half_value, EBUSY, "F_SETPIPE_SZ and size < data stored in pipe"},
	{&sys_value, EPERM, "F_SETPIPE_SZ and size is over limit for unpriviledged user"},
};

static void verify_fcntl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);

	TEST(fcntl(fds[1], F_SETPIPE_SZ, *(tc->setvalue)));
	if (TST_RET != -1) {
		tst_res(TFAIL, "F_SETPIPE_SZ succeed and return %ld", TST_RET);
		return;
	}
	if (TST_ERR == tc->exp_err)
		tst_res(TPASS | TTERRNO, "F_SETPIPE_SZ failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "F_SETPIPE_SZ failed expected %s got",
				tst_strerrno(tc->exp_err));
}

static void setup(void)
{
	char *wrbuf;
	unsigned int orig_value;

	SAFE_PIPE(fds);

	TEST(fcntl(fds[1], F_GETPIPE_SZ));
	if (TST_ERR == EINVAL)
		tst_brk(TCONF, "kernel doesn't support F_GET/SETPIPE_SZ");

	orig_value = TST_RET;

	wrbuf = SAFE_MALLOC(orig_value);
	memset(wrbuf, 'x', orig_value);
	SAFE_WRITE(1, fds[1], wrbuf, orig_value);
	free(wrbuf);

	SAFE_FILE_SCANF("/proc/sys/fs/pipe-max-size", "%d", &sys_value);
	sys_value++;

	half_value = orig_value / 2;
	invalid_value = (1U << 31) + 1;
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_fcntl,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_RESOURCE),
		{}
	},
};
