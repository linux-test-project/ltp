/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
/*
 * Test Description:
 *  Verify that,
 *   1. link() fails with -1 return value and sets errno to EPERM
 *      if oldpath is a directory.
 *   2. link() fails with -1 return value and sets errno to EXDEV
 *      if oldpath and newpath are not on the same mounted file system( Linux
 *      permits a file system to be mounted at multiple points, but link()
 *      does not work across different mount points, even if the same
 *      file system is mounted on both. ).
 *   3. link() fails with -1 return value and sets errno to EROFS
 *      if the file is on a read-only file system.
 *   4. link() fails with -1 return value and sets errno to ELOOP
 *      if too many symbolic links were encountered in resolving path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define MNT_POINT	"mntpoint"
#define TEST_FILE	"testfile"
#define TEST_FILE1	"testfile1"
#define TEST_FILE2	"mntpoint/testfile3"
#define TEST_FILE3	"mntpoint/testfile4"

static char test_file4[PATH_MAX] = ".";
static void setup(void);
static void cleanup(void);

static const char *device;
static int mount_flag;

static struct test_case_t {
	char *oldpath;
	char *newpath;
	int exp_errno;
} test_cases[] = {
	{TEST_FILE1, TEST_FILE, EPERM},
	{TEST_FILE2, TEST_FILE, EXDEV},
	{TEST_FILE2, TEST_FILE3, EROFS},
	{test_file4, TEST_FILE, ELOOP},
};

static void link_verify(const struct test_case_t *);

char *TCID = "link08";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			link_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();

}

static void link_verify(const struct test_case_t *tc)
{
	TEST(link(tc->oldpath, tc->newpath));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "link succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "link failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "link failed unexpectedly; expected: %d - %s",
			 tc->exp_errno, strerror(tc->exp_errno));
	}
}


static void setup(void)
{
	int i;
	const char *fs_type;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to acquire device");

	SAFE_MKDIR(cleanup, TEST_FILE1, DIR_MODE);

	SAFE_MKDIR(cleanup, "test_eloop", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(test_file4, "/test_eloop");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);
	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNT_POINT, fs_type, 0, NULL);
	mount_flag = 1;

	SAFE_TOUCH(cleanup, TEST_FILE2, 0644, NULL);
	SAFE_MOUNT(cleanup, device, MNT_POINT, fs_type,
		   MS_REMOUNT | MS_RDONLY, NULL);
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNT_POINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
