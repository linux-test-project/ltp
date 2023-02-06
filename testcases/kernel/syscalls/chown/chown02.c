// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Verify that chown(2) invoked by super-user:
 *
 *  - clears setuid and setgid bits set on an executable file
 *  - preserves setgid bit set on a non-group-executable file
 */

#include "tst_test.h"
#include "compat_tst_16.h"
#include "tst_safe_macros.h"

#define NEW_PERMS1	(S_IFREG|S_IRWXU|S_IRWXG|S_ISUID|S_ISGID)
#define NEW_PERMS2	(S_IFREG|S_IRWXU|S_ISGID)
#define EXP_PERMS	(S_IFREG|S_IRWXU|S_IRWXG)
#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

struct test_case_t {
	const char *filename;
	mode_t set_mode;
	mode_t exp_mode;
} tc[] = {
	{TESTFILE1, NEW_PERMS1, EXP_PERMS},
	{TESTFILE2, NEW_PERMS2, NEW_PERMS2}
};

static void run(unsigned int i)
{
	uid_t uid;
	gid_t gid;

	UID16_CHECK((uid = geteuid()), "chown");
	GID16_CHECK((gid = getegid()), "chown");

	SAFE_CHMOD(tc[i].filename, tc[i].set_mode);

	TST_EXP_PASS(CHOWN(tc[i].filename, uid, gid), "chown(%s, %d, %d)",
		     tc[i].filename, uid, gid);

	struct stat stat_buf;
	SAFE_STAT(tc[i].filename, &stat_buf);

	if (stat_buf.st_uid != uid || stat_buf.st_gid != gid) {
		tst_res(TFAIL, "%s: owner set to (uid=%d, gid=%d), expected (uid=%d, gid=%d)",
			tc[i].filename, stat_buf.st_uid, stat_buf.st_gid, uid, gid);
	}

	if (stat_buf.st_mode != tc[i].exp_mode) {
		tst_res(TFAIL, "%s: wrong mode permissions %#o, expected %#o",
			tc[i].filename, stat_buf.st_mode, tc[i].exp_mode);
	}
}

static void setup(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(tc); i++)
		SAFE_TOUCH(tc[i].filename, tc[i].set_mode, NULL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.test = run,
};
