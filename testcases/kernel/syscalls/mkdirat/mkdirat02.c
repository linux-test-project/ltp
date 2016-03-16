/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * DESCRIPTION
 *	check mkdirat() with various error conditions that should produce
 *	ELOOP and EROFS.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mount.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/mkdirat.h"

static void setup(void);
static void cleanup(void);

#define TEST_FILE1	"mntpoint/test_file1"
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)

char *TCID = "mkdirat02";

static int dir_fd;
static int cur_fd = AT_FDCWD;
static char test_file2[PATH_MAX] = ".";
static const char *device;
static int mount_flag_dir;
static int mount_flag_cur;

static struct test_case_t {
	int *dirfd;
	char *pathname;
	int exp_errno;
} TC[] = {
	{&dir_fd, TEST_FILE1, EROFS},
	{&cur_fd, TEST_FILE1, EROFS},
	{&dir_fd, test_file2, ELOOP},
	{&cur_fd, test_file2, ELOOP},
};

int TST_TOTAL = ARRAY_SIZE(TC);
static void mkdirat_verify(const struct test_case_t *);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			mkdirat_verify(&TC[i]);
	}

	cleanup();
	tst_exit();
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

	SAFE_MKDIR(cleanup, "test_dir", DIR_MODE);
	dir_fd = SAFE_OPEN(cleanup, "test_dir", O_DIRECTORY);

	SAFE_MKDIR(cleanup, "test_eloop", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");

	SAFE_MKDIR(cleanup, "test_dir/test_eloop", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../test_eloop",
		     "test_dir/test_eloop/test_eloop");
	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	for (i = 0; i < 43; i++)
		strcat(test_file2, "/test_eloop");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, "test_dir/mntpoint", DIR_MODE);
	if (mount(device, "test_dir/mntpoint", fs_type, MS_RDONLY, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag_dir = 1;

	SAFE_MKDIR(cleanup, "mntpoint", DIR_MODE);
	if (mount(device, "mntpoint", fs_type, MS_RDONLY, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag_cur = 1;
}

static void mkdirat_verify(const struct test_case_t *test)
{
	TEST(mkdirat(*test->dirfd, test->pathname, 0777));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mkdirat() returned %ld, expected -1, errno=%d",
			 TEST_RETURN, test->exp_errno);
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "mkdirat() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mkdirat() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	if (mount_flag_dir && tst_umount("mntpoint") < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (mount_flag_cur && tst_umount("test_dir/mntpoint") < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
