// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *	11/2010 Code cleanup by Cyril Hrubis chrubis@suse.cz
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that, lchown(2) succeeds to change the owner and group of a file
 * specified by path to any numeric owner(uid)/group(gid) values when invoked
 * by super-user.
 *
 * lchown(2) should return 0 and the ownership set on the file should match
 * the numeric values contained in owner and group respectively.
 */

#include "tst_test.h"

#define TESTFILE "testfile"
#define SFILE "slink_file"

static struct test_case_t {
	char *desc;
	uid_t user_id;
	gid_t group_id;
} test_cases[] = {
	{ "Change Owner/Group ids", 700, 701 },
	{ "Change Owner id only", 702, -1 },
	{ "Change Owner/Group ids", 703, 701 },
	{ "Change Group id only", -1, 704 },
	{ "Change Group/Group ids", 703, 705 },
	{ "Change none", -1, -1 },
};

static void run(unsigned int i)
{
	struct stat stat_buf;
	struct test_case_t *tc = &test_cases[i];
	uid_t user_id = tc->user_id;
	gid_t group_id = tc->group_id;

	SAFE_LSTAT(SFILE, &stat_buf);
	uid_t cmp_usr_id = user_id == -1 ? stat_buf.st_uid : user_id;
	gid_t cmp_grp_id = group_id == -1 ? stat_buf.st_gid : group_id;

	tst_res(TINFO, "%s", tc->desc);
	SAFE_LCHOWN(SFILE, user_id, group_id);
	SAFE_LSTAT(SFILE, &stat_buf);
	TST_EXP_EQ_LI(stat_buf.st_uid, cmp_usr_id);
	TST_EXP_EQ_LI(stat_buf.st_gid, cmp_grp_id);
}

static void setup(void)
{
	SAFE_TOUCH(TESTFILE, 0644, NULL);
	SAFE_SYMLINK(TESTFILE, SFILE);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.test = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
