// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 lufei <lufei@uniontech.com>
 */

/*\
 * This test case is to verify unshare(CLONE_NEWNS) also unshares process
 * working directory.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/sched.h"

#define TESTDIR "test_dir"

static char *cwd;
static char *buff;
static size_t size = 1024;

static void setup(void)
{
	cwd = SAFE_MALLOC(size);
	SAFE_GETCWD(cwd, size);

	SAFE_MKDIR(TESTDIR, 0700);

	buff = SAFE_MALLOC(size);
}

static void cleanup(void)
{
	free(buff);
	free(cwd);
}

static void run(void)
{
	struct tst_clone_args args = {
		.flags = CLONE_FS,
		.exit_signal = SIGCHLD,
	};

	if (!SAFE_CLONE(&args)) {

		TST_EXP_PASS(unshare(CLONE_NEWNS));

		SAFE_CHDIR(TESTDIR);
		SAFE_GETCWD(buff, size);

		if (strcmp(cwd, buff) == 0)
			tst_res(TFAIL, "current dir not changed");
		else
			tst_res(TPASS, "current dir changed to %s", buff);
	} else {
		SAFE_WAIT(NULL);

		SAFE_GETCWD(buff, size);

		if (strcmp(cwd, buff) == 0)
			tst_res(TPASS, "cwd unshared");
		else
			tst_res(TFAIL, "cwd not unshared as expected");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
