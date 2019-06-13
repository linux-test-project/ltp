// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 */

/*
 * Description:
 * The testcase checks the various errnos of the unlink(2).
 * 1) unlink() returns EACCES when deleting file in unwritable directory
 *    as an unprivileged user.
 * 2) unlink() returns EACCES when deleting file in "unsearchable directory
 *    as an unprivileged user.
 * 3) unlink() returns EISDIR when deleting directory for root
 * 4) unlink() returns EISDIR when deleting directory for regular user
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
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
	TEST(unlink(tc->name));
	if (TST_RET != -1) {
		tst_res(TFAIL, "unlink(<%s>) succeeded unexpectedly",
			tc->desc);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "unlink(<%s>) failed as expected",
			tc->desc);
	} else {
		tst_res(TFAIL | TTERRNO,
			"unlink(<%s>) failed, expected errno: %s",
			tc->desc, tst_strerrno(tc->exp_errno));
	}
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
