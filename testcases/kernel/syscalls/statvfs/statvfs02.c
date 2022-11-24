// SPDX-License-Identifier: GPL-2.0

/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that statvfs() fails with:
 * - EFAULT when path points to an invalid address.
 * - ELOOP when too many symbolic links were encountered in translating path.
 * - ENAMETOOLONG when path is too long.
 * - ENOENT when the file referred to by path does not exist.
 * - ENOTDIR a component of the path prefix of path is not a directory.
 */

#include <sys/statvfs.h>

#include "tst_test.h"

#define TEST_SYMLINK	"statvfs_symlink"
#define TEST_FILE	"statvfs_file"

static struct statvfs buf;
static char nametoolong[PATH_MAX+2];

static struct tcase {
	char *path;
	struct statvfs *buf;
	int exp_errno;
} tcases[] = {
	{(char *)-1, &buf, EFAULT},
	{TEST_SYMLINK, &buf, ELOOP},
	{nametoolong, &buf, ENAMETOOLONG},
	{"filenoexist", &buf, ENOENT},
	{"statvfs_file/test", &buf, ENOTDIR},
};

static void setup(void)
{
	unsigned int i;

	SAFE_SYMLINK(TEST_SYMLINK, "symlink_2");
	SAFE_SYMLINK("symlink_2", TEST_SYMLINK);

	memset(nametoolong, 'a', PATH_MAX+1);
	SAFE_TOUCH(TEST_FILE, 0644, NULL);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].path == (char *)-1)
			tcases[i].path = tst_get_bad_addr(NULL);
	}
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(statvfs(tc->path, tc->buf), tc->exp_errno);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1
};
