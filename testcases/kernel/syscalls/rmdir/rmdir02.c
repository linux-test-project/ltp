// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */

/*\
 * [Description]
 *
 * Verify that, rmdir(2) returns -1 and sets errno to
 *
 * 1. ENOTEMPTY when removing a non-empty directory
 * 2. ENAMETOOLONG when removing a directory with long path name
 * 3. ENOENT when removing a non-existing directory
 * 4. ENOTDIR when removing a a file
 * 5. EFAULT when removing a invalid pointer
 * 6. ELOOP when removing a symlink loop
 * 7. EROFS when removing a dir on RO mounted FS
 * 8. EBUSY when removing a mount point
 * 9. EINVAL when removing "." (current directory)
 */

#include <errno.h>

#include "tst_test.h"

#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)
#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)

#define TESTDIR		"testdir"
#define TESTDIR2	"nosuchdir/testdir2"
#define TESTDIR3	"testfile2/testdir3"
#define TESTDIR4	"/loopdir"
#define MNT_POINT	"mntpoint"
#define TESTDIR5	"mntpoint/dir"
#define TESTFILE    "testdir/testfile"
#define TESTFILE2   "testfile2"

static char longpathname[PATH_MAX + 2];
static char looppathname[sizeof(TESTDIR4) * 43] = ".";

static struct testcase {
	char *dir;
	int exp_errno;
} tcases[] =  {
	{TESTDIR, ENOTEMPTY},
	{longpathname, ENAMETOOLONG},
	{TESTDIR2, ENOENT},
	{TESTDIR3, ENOTDIR},
	{NULL, EFAULT},
	{looppathname, ELOOP},
	{TESTDIR5, EROFS},
	{MNT_POINT, EBUSY},
	{".", EINVAL}
};

static void setup(void)
{
	unsigned int i;

	SAFE_MKDIR(TESTDIR, DIR_MODE);
	SAFE_TOUCH(TESTFILE, FILE_MODE, NULL);

	memset(longpathname, 'a', PATH_MAX + 1);

	SAFE_TOUCH(TESTFILE2, FILE_MODE, NULL);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (!tcases[i].dir)
			tcases[i].dir = tst_get_bad_addr(NULL);
	}

	/*
	 * NOTE: the ELOOP test is written based on that the
	 * consecutive symlinks limit in kernel is hardwired
	 * to 40.
	 */
	SAFE_MKDIR("loopdir", DIR_MODE);
	SAFE_SYMLINK("../loopdir", "loopdir/loopdir");
	for (i = 0; i < 43; i++)
		strcat(looppathname, TESTDIR4);
}

static void verify_rmdir(unsigned int n)
{
	struct testcase *tc = &tcases[n];

	TEST(rmdir(tc->dir));

	if (TST_RET != -1) {
		tst_res(TFAIL, "rmdir() succeeded unexpectedly (%li)",
			TST_RET);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "rmdir() failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"rmdir() failed unexpectedly; expected: %d - %s",
		tc->exp_errno, tst_strerrno(tc->exp_errno));
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_rmdir,
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
};
