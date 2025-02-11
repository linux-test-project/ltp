// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
 *		-Ported
 * Copyright (c) Linux Test Project, 2002-2022
 */

/*\
 * Verify that, stat(2) succeeds to get the status of a file and fills the
 * stat structure elements regardless of whether process has or doesn't
 * have read access to the file.
 */

#include <pwd.h>
#include "tst_test.h"

#define FILE_SIZE	 1024
#define TST_FILEREAD     "test_fileread"
#define TST_FILENOREAD   "test_filenoread"
#define READ_MODE        0666
#define NEW_MODE         0222
#define MASK             0777

static uid_t user_id;
static gid_t group_id;
static struct passwd *ltpuser;

static struct tcase{
	char *pathname;
	unsigned int mode;
} TC[] = {
	{TST_FILEREAD, READ_MODE},
	{TST_FILENOREAD, NEW_MODE}
};

static void verify_stat(unsigned int n)
{
	struct tcase *tc = TC + n;
	struct stat stat_buf;

	TST_EXP_PASS(stat(tc->pathname, &stat_buf));

	TST_EXP_EQ_LU(stat_buf.st_uid, user_id);
	TST_EXP_EQ_LU(stat_buf.st_gid, group_id);
	TST_EXP_EQ_LI(stat_buf.st_size, FILE_SIZE);
	TST_EXP_EQ_LU(stat_buf.st_mode & MASK, tc->mode);
	TST_EXP_EQ_LU(stat_buf.st_nlink, 1);
}

static void setup(void)
{
	unsigned int i;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);

	for (i = 0; i < ARRAY_SIZE(TC); i++) {
		if (tst_fill_file(TC[i].pathname, 'a', 256, 4))
			tst_brk(TBROK, "Failed to create tst file %s",
				TC[i].pathname);
		SAFE_CHMOD(TC[i].pathname, TC[i].mode);
	}

	user_id = getuid();
	group_id = getgid();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(TC),
	.setup = setup,
	.test = verify_stat,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
