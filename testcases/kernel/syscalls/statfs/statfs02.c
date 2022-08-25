// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *
 */

/*\
 * [Description]
 *
 * Tests for failures:
 *
 * - ENOTDIR A component of the pathname, which is not a directory.
 * - ENOENT A filename which doesn't exist.
 * - ENAMETOOLONG A pathname which is longer than MAXNAMLEN.
 * - EFAULT A pathname pointer outside the address space of the process.
 * - EFAULT A buf pointer outside the address space of the process.
 * - ELOOP A filename which has too many symbolic links.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#define TEST_FILE		"statfs_file"
#define TEST_FILE1		TEST_FILE"/statfs_file1"
#define TEST_NOEXIST		"statfs_noexist"
#define TEST_SYMLINK		"statfs_symlink"

static int fd;
static char test_toolong[PATH_MAX+2];
static struct statfs buf;

static struct test_case_t {
	char *path;
	struct statfs *buf;
	int exp_error;
} tests[] = {
	{TEST_FILE1, &buf, ENOTDIR},
	{TEST_NOEXIST, &buf, ENOENT},
	{test_toolong, &buf, ENAMETOOLONG},
	{(char *)-1, &buf, EFAULT},
	{TEST_FILE, (struct statfs *)-1, EFAULT},
	{TEST_SYMLINK, &buf, ELOOP},
};

static void statfs_verify(unsigned int n)
{
	int pid, status;

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_FAIL(statfs(tests[n].path, tests[n].buf), tests[n].exp_error, "statfs()");
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	if (tests[n].exp_error == EFAULT &&
	    WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Got SIGSEGV instead of EFAULT");
		return;
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static void setup(void)
{
	unsigned int i;

	fd = SAFE_CREAT(TEST_FILE, 0444);

	memset(test_toolong, 'a', PATH_MAX+1);

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (tests[i].path == (char *)-1)
			tests[i].path = tst_get_bad_addr(NULL);
	}

	SAFE_SYMLINK(TEST_SYMLINK, "statfs_symlink_2");
	SAFE_SYMLINK("statfs_symlink_2", TEST_SYMLINK);
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);
}

static struct tst_test test = {
	.test = statfs_verify,
	.tcnt = ARRAY_SIZE(tests),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
