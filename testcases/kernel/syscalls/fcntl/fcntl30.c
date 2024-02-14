// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, fetching and changing the capacity of a pipe works as
 * expected with fcntl(2) syscall using F_GETPIPE_SZ, F_SETPIPE_SZ arguments.
 */

#include "tst_test.h"
#include "lapi/fcntl.h"

static int fds[2];
static int max_size_unpriv;

static void run(void)
{
	SAFE_PIPE(fds);

	TST_EXP_POSITIVE(fcntl(fds[1], F_GETPIPE_SZ));

	TST_EXP_POSITIVE(fcntl(fds[1], F_SETPIPE_SZ, max_size_unpriv));
	TST_EXP_POSITIVE(fcntl(fds[1], F_GETPIPE_SZ));
	TST_EXP_EXPR(TST_RET >= max_size_unpriv,
				"new pipe size (%ld) >= requested size (%d)",
				TST_RET, max_size_unpriv);

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static void setup(void)
{
	SAFE_FILE_SCANF("/proc/sys/fs/pipe-max-size", "%d", &max_size_unpriv);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup
};
