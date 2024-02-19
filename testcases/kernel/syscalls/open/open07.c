// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Test functionality and error conditions of open(O_NOFOLLOW) system call.
 */

#include "tst_test.h"
#include "tst_safe_macros.h"

#define TESTFILE "testfile"
#define TESTDIR "testdir"
#define SYMFILE1 "symfile1"
#define SYMFILE2 "symfile2"
#define SYMDIR1 "symdir1"
#define SYMDIR2 "symdir2"
#define PASSFILE "symdir1/testfile"

static struct testcase {
	const char *path;
	int err;
	const char *desc;
} testcase_list[] = {
	{SYMFILE1, ELOOP, "open(O_NOFOLLOW) a symlink to file"},
	{SYMFILE2, ELOOP, "open(O_NOFOLLOW) a double symlink to file"},
	{SYMDIR1, ELOOP, "open(O_NOFOLLOW) a symlink to directory"},
	{SYMDIR2, ELOOP, "open(O_NOFOLLOW) a double symlink to directory"},
	{PASSFILE, 0, "open(O_NOFOLLOW) a file in symlinked directory"},
};

static void setup(void)
{
	int fd;

	umask(0);
	fd = SAFE_CREAT(TESTFILE, 0644);
	SAFE_CLOSE(fd);
	SAFE_MKDIR(TESTDIR, 0755);

	SAFE_SYMLINK(TESTFILE, SYMFILE1);
	SAFE_SYMLINK(SYMFILE1, SYMFILE2);
	SAFE_SYMLINK(TESTDIR, SYMDIR1);
	SAFE_SYMLINK(SYMDIR1, SYMDIR2);

	fd = SAFE_CREAT(PASSFILE, 0644);
	SAFE_CLOSE(fd);
}

static void run(unsigned int n)
{
	const struct testcase *tc = testcase_list + n;

	if (tc->err) {
		TST_EXP_FAIL2(open(tc->path, O_NOFOLLOW | O_RDONLY), tc->err,
			"%s", tc->desc);
	} else {
		TST_EXP_FD(open(tc->path, O_NOFOLLOW | O_RDONLY),
			"%s", tc->desc);
	}

	if (TST_RET >= 0)
		SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1
};
