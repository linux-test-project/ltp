// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Kerlabs 2008.
 * Copyright (c) International Business Machines  Corp., 2008
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that setresuid() behaves correctly with file permissions.
 * The test creates a file as ROOT with permissions 0644, does a setresuid
 * to change euid to a non-root user and tries to open the file with RDWR
 * permissions, which should fail with EACCES errno.
 * The same test is done in a fork also to check that child process also
 * inherits new euid and open fails with EACCES.
 * Test verifies the successful open action after reverting the euid back
 * ROOT user.
 */

#include <sys/wait.h>
#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

#define TEMP_FILE	"testfile"
static char nobody_uid[] = "nobody";
static struct passwd *ltpuser;
static int fd = -1;

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM(nobody_uid);
	UID16_CHECK(ltpuser->pw_uid, "setresuid");

	fd = SAFE_OPEN(TEMP_FILE, O_CREAT | O_RDWR, 0644);
}

static void run(void)
{
	pid_t pid;
	int status;

	TST_EXP_PASS_SILENT(SETRESUID(-1, ltpuser->pw_uid, -1));
	TST_EXP_FAIL2(open(TEMP_FILE, O_RDWR), EACCES);

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_FAIL2(open(TEMP_FILE, O_RDWR), EACCES);
		return;
	}
	SAFE_WAITPID(pid, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_res(TFAIL, "child process exited with status: %d", status);

	TST_EXP_PASS_SILENT(SETRESUID(-1, 0, -1));
	TST_EXP_FD(open(TEMP_FILE, O_RDWR));
	SAFE_CLOSE(TST_RET);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1
};
