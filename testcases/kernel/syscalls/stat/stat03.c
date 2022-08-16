// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
 *		-Ported
 * Copyright (c) Linux Test Project, 2002-2022
 */

/*\
 * [Description]
 *
 * check stat() with various error conditions that should produce
 * EACCES, EFAULT, ENAMETOOLONG,  ENOENT, ENOTDIR, ELOOP
 */

#include <pwd.h>
#include "tst_test.h"

#define TST_EACCES_DIR   "tst_eaccesdir"
#define TST_EACCES_FILE  "tst_eaccesdir/tst"
#define TST_ENOENT       "tst_enoent/tst"
#define TST_ENOTDIR_DIR  "tst_enotdir/tst"
#define TST_ENOTDIR_FILE "tst_enotdir"

#define MODE_RW	        0666
#define DIR_MODE        0755

static struct passwd *ltpuser;

static char long_dir[PATH_MAX + 2] = {[0 ... PATH_MAX + 1] = 'a'};
static char loop_dir[PATH_MAX] = ".";

static struct tcase{
	char *pathname;
	int exp_errno;
} TC[] = {
	{TST_EACCES_FILE, EACCES},
	{NULL, EFAULT},
	{long_dir, ENAMETOOLONG},
	{TST_ENOENT, ENOENT},
	{TST_ENOTDIR_DIR, ENOTDIR},
	{loop_dir, ELOOP}
};

static void verify_stat(unsigned int n)
{
	struct tcase *tc = TC + n;
	struct stat stat_buf;

	TST_EXP_FAIL(stat(tc->pathname, &stat_buf), tc->exp_errno);
}

static void setup(void)
{
	unsigned int i;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);

	SAFE_MKDIR(TST_EACCES_DIR, DIR_MODE);
	SAFE_TOUCH(TST_EACCES_FILE, DIR_MODE, NULL);
	SAFE_CHMOD(TST_EACCES_DIR, MODE_RW);

	for (i = 0; i < ARRAY_SIZE(TC); i++) {
		if (TC[i].exp_errno == EFAULT)
			TC[i].pathname = tst_get_bad_addr(NULL);
	}

	SAFE_TOUCH(TST_ENOTDIR_FILE, DIR_MODE, NULL);

	SAFE_MKDIR("test_eloop", DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(loop_dir, "/test_eloop");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(TC),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.test = verify_stat,
};
