// SPDX-License-Identifier: GPL-2.0-later
/*
 * Copyright (c) Kerlabs 2008.
 * Copyright (c) International Business Machines  Corp., 2008
 * Created by Renaud Lottiaux
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Check if setreuid behaves correctly with file permissions.
 * The test creates a file as ROOT with permissions 0644, does a setreuid
 * and then tries to open the file with RDWR permissions.
 * The same test is done in a fork to check if new UIDs are correctly
 * passed to the child process.
 */

#include <pwd.h>
#include <stdlib.h>

#include "tst_test.h"
#include "compat_tst_16.h"

#define TEMPFILE "testfile"

static struct passwd *ltpuser;

static void setup(void)
{
	int fd;

	ltpuser = SAFE_GETPWNAM("nobody");

	UID16_CHECK(ltpuser->pw_uid, setreuid);
	fd = SAFE_OPEN(TEMPFILE, O_CREAT | O_RDWR, 0644);
	SAFE_CLOSE(fd);
}

static void run(void)
{
	pid_t pid;

	TST_EXP_PASS_SILENT(SETREUID(-1, ltpuser->pw_uid));
	TST_EXP_FAIL2(open(TEMPFILE, O_RDWR), EACCES);

	pid = SAFE_FORK();
	if (pid == 0) {
		TST_EXP_FAIL2(open(TEMPFILE, O_RDWR), EACCES);
		exit(0);
	}
	tst_reap_children();

	TST_EXP_PASS_SILENT(SETREUID(-1, 0));
	TST_EXP_FD(open(TEMPFILE, O_RDWR));
	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_tmpdir = 1,
};
