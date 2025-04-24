// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Al Viro <viro@zeniv.linux.org.uk>
 * Copyright (C) 2024 Wei Gao <wegao@suse.com>
 */

/*\
 * This test case based on kernel self-test unshare_test.c to check that
 * the kernel handles the EMFILE error when a parent process changes file
 * descriptor limits and the child process tries to unshare (CLONE_FILES).
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "config.h"
#include "lapi/sched.h"

#define FS_NR_OPEN "/proc/sys/fs/nr_open"

#ifdef HAVE_UNSHARE

static void run(void)
{
	struct tst_clone_args args = {
		.flags = CLONE_FILES,
		.exit_signal = SIGCHLD,
	};

	int nr_open = sizeof(long long) * 8;

	SAFE_DUP2(2, nr_open + 1);

	if (!SAFE_CLONE(&args)) {
		SAFE_FILE_PRINTF(FS_NR_OPEN, "%d", nr_open);
		TST_EXP_FAIL(unshare(CLONE_FILES), EMFILE);
		exit(0);
	}
}

static void setup(void)
{
	clone3_supported_by_kernel();
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.test_all = run,
	.setup = setup,
	.save_restore = (const struct tst_path_val[]) {
		{FS_NR_OPEN, NULL, TST_SR_TCONF},
		{}
	},
};

#else
TST_TEST_TCONF("unshare syscall is undefined.");
#endif
