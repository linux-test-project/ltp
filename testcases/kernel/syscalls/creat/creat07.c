// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2012-2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Check that :manpage:`creat(2)` sets ETXTBSY correctly.
 *
 * NOTE: write to executed file was allowed in 6.11-rc1:
 * 2a010c412853 ("fs: don't block i_writecount during exec")
 * but then reverted in v6.13-rc1:
 * 3b832035387f ("Revert "fs: don't block i_writecount during exec""),
 * backported into v6.11.11 and v6.12.2.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "tst_test.h"

#define TEST_APP "creat07_child"

static void verify_creat(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		SAFE_EXECL(TEST_APP, TEST_APP, NULL);
		exit(1);
	}

	TST_CHECKPOINT_WAIT(0);

	TEST(creat(TEST_APP, O_WRONLY));

	if (TST_RET != -1) {
		tst_res(TFAIL, "creat() succeeded unexpectedly");
		goto kill;
	}

	if (TST_ERR == ETXTBSY)
		tst_res(TPASS, "creat() received EXTBSY");
	else
		tst_res(TFAIL | TTERRNO, "creat() failed unexpectedly");

kill:
	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, NULL, 0);
}

static struct tst_test test = {
	.test_all = verify_creat,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3b832035387ff508fdcf0fba66701afc78f79e3d"},
		{}
	},
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	}
};
