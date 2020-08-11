// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */
/*
 * DESCRIPTION
 *	check mkdir() with various error conditions that should produce
 *	EFAULT, ENAMETOOLONG, EEXIST, ENOENT, ENOTDIR, ELOOP and EROFS
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mount.h>

#include "tst_test.h"

#define TST_EEXIST	"tst_eexist"
#define TST_ENOENT	"tst_enoent/tst"
#define TST_ENOTDIR_FILE "tst_enotdir"
#define TST_ENOTDIR_DIR	"tst_enotdir/tst"
#define MODE		0777

#define MNT_POINT	"mntpoint"
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define TST_EROFS      "mntpoint/tst_erofs"

static char long_dir[PATH_MAX + 2] = {[0 ... PATH_MAX + 1] = 'a'};
static char loop_dir[PATH_MAX] = ".";

struct tcase;

static struct tcase {
	char *pathname;
	int exp_errno;
} TC[] = {
	{NULL, EFAULT},
	{long_dir, ENAMETOOLONG},
	{TST_EEXIST, EEXIST},
	{TST_ENOENT, ENOENT},
	{TST_ENOTDIR_DIR, ENOTDIR},
	{loop_dir, ELOOP},
	{TST_EROFS, EROFS},
};

static void verify_mkdir(unsigned int n)
{
	struct tcase *tc = TC + n;

	TEST(mkdir(tc->pathname, MODE));
	if (TST_RET != -1) {
		tst_res(TFAIL, "mkdir() returned %ld, expected -1, errno=%d",
			TST_RET, tc->exp_errno);
		return;
	}

	if (TST_ERR == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "mkdir() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"mkdir() failed unexpectedly; expected: %d - %s",
			 tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void setup(void)
{
	unsigned int i;

	SAFE_TOUCH(TST_EEXIST, MODE, NULL);
	SAFE_TOUCH(TST_ENOTDIR_FILE, MODE, NULL);

	for (i = 0; i < ARRAY_SIZE(TC); i++) {
		if (TC[i].exp_errno == EFAULT)
                       TC[i].pathname = 0;
	}

	SAFE_MKDIR("test_eloop", DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(loop_dir, "/test_eloop");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(TC),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = MNT_POINT,
	.setup = setup,
	.test = verify_mkdir,
};
