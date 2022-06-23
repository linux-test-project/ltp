// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *		07/2001 John George
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that system call utime() fails with
 * - EACCES when times argument is NULL and user does not have rights
 * to modify the file.
 * - ENOENT when specified file does not exist.
 * - EPERM when times argument is not NULL and user does not have rights
 * to modify the file.
 * - EROFS when the path resides on a read-only filesystem.
 */

#include <pwd.h>
#include <utime.h>

#include "tst_test.h"

#define TEMP_FILE	"tmp_file"
#define MNT_POINT	"mntpoint"
#define FILE_MODE	0644
#define TEST_USERNAME "nobody"

static const struct utimbuf times;

static struct tcase {
	char *pathname;
	int exp_errno;
	const struct utimbuf *utimbuf;
	char *err_desc;
} tcases[] = {
	{TEMP_FILE, EACCES, NULL, "No write access"},
	{"", ENOENT, NULL, "File not exist"},
	{TEMP_FILE, EPERM, &times, "Not file owner"},
	{MNT_POINT, EROFS, NULL, "Read-only filesystem"}
};


static void setup(void)
{
	struct passwd *pw;

	SAFE_TOUCH(TEMP_FILE, FILE_MODE, NULL);

	pw = SAFE_GETPWNAM(TEST_USERNAME);
	tst_res(TINFO, "Switching effective user ID to user: %s", pw->pw_name);
	SAFE_SETEUID(pw->pw_uid);
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(utime(tc->pathname, tc->utimbuf),
				tc->exp_errno, "%s", tc->err_desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mntpoint = MNT_POINT,
	.needs_rofs = 1
};
