// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2002-2022
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*\
 * [Description]
 *
 * Verify that unlink(2) fails with
 *
 * - EACCES when no write access to the directory containing pathname
 * - EACCES when one of the directories in pathname did not allow search
 * - EISDIR when deleting directory as root user
 * - EISDIR when deleting directory as non-root user
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static struct passwd *pw;

static struct test_case_t {
	char *name;
	char *desc;
	int exp_errno;
	int exp_user;
} tcases[] = {
	{"unwrite_dir/file", "unwritable directory", EACCES, 1},
	{"unsearch_dir/file", "unsearchable directory", EACCES, 1},
	{"regdir", "directory", EISDIR, 0},
	{"regdir", "directory", EISDIR, 1},
};

static void verify_unlink(struct test_case_t *tc)
{
	TST_EXP_FAIL(unlink(tc->name), tc->exp_errno, "%s", tc->desc);
}

static void do_unlink(unsigned int n)
{
	struct test_case_t *cases = &tcases[n];
	pid_t pid;

	if (cases->exp_user) {
		pid = SAFE_FORK();
		if (!pid) {
			SAFE_SETUID(pw->pw_uid);
			verify_unlink(cases);
			exit(0);
		}
		SAFE_WAITPID(pid, NULL, 0);
	} else {
		verify_unlink(cases);
	}
}

static void setup(void)
{
	SAFE_MKDIR("unwrite_dir", 0777);
	SAFE_TOUCH("unwrite_dir/file", 0777, NULL);
	SAFE_CHMOD("unwrite_dir", 0555);

	SAFE_MKDIR("unsearch_dir", 0777);
	SAFE_TOUCH("unsearch_dir/file", 0777, NULL);
	SAFE_CHMOD("unsearch_dir", 0666);

	SAFE_MKDIR("regdir", 0777);

	pw = SAFE_GETPWNAM("nobody");
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = do_unlink,
};
