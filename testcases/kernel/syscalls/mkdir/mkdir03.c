/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
static void prot_none_pathname(struct tcase *tc);

static struct tcase {
	char *pathname;
	int exp_errno;
	void (*setupfunc)(struct tcase *tc);
} TC[] = {
	{NULL, EFAULT, prot_none_pathname},
	{long_dir, ENAMETOOLONG, NULL},
	{TST_EEXIST, EEXIST, NULL},
	{TST_ENOENT, ENOENT, NULL},
	{TST_ENOTDIR_DIR, ENOTDIR, NULL},
	{loop_dir, ELOOP, NULL},
	{TST_EROFS, EROFS, NULL},
};

static void verify_mkdir(unsigned int n)
{
	struct tcase *tc = TC + n;

	TEST(mkdir(tc->pathname, MODE));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "mkdir() returned %ld, expected -1, errno=%d",
			TEST_RETURN, tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "mkdir() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"mkdir() failed unexpectedly; expected: %d - %s",
			 tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void prot_none_pathname(struct tcase *tc)
{
	tc->pathname = SAFE_MMAP(0, 1, PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

static void setup(void)
{
	unsigned int i;

	SAFE_TOUCH(TST_EEXIST, MODE, NULL);
	SAFE_TOUCH(TST_ENOTDIR_FILE, MODE, NULL);

	SAFE_MKDIR("test_eloop", DIR_MODE);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(loop_dir, "/test_eloop");

	for (i = 0; i < ARRAY_SIZE(TC); i++) {
		if (TC[i].setupfunc)
			TC[i].setupfunc(&TC[i]);
	}
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
