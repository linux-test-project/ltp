/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Author: Wayne Boyer
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
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test that fchmod() fails and sets the proper errno values.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

static int fd1;
static int fd2;
static int fd3;
static const char *device;
static int mount_flag;

static struct test_case_t {
	char *name;
	int *fd;
	int mode;
	int exp_errno;
} test_cases[] = {
	{"EPERM", &fd1, 0644, EPERM},
	{"EBADF", &fd2, 0644, EBADF},
	{"EROFS", &fd3, 0644, EROFS},
};

char *TCID = "fchmod06";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(fchmod(*test_cases[i].fd, test_cases[i].mode));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == test_cases[i].exp_errno) {
					tst_resm(TPASS | TTERRNO,
						 "fchmod: test %s success",
						 test_cases[i].name);
				} else {
					tst_resm(TFAIL | TTERRNO,
						 "fchmod: test %s FAILED with "
						 "unexpect errno: %d",
						 test_cases[i].name,
						 TEST_ERRNO);
				}
			} else {
				tst_resm(TFAIL,
					 "fchmod: test %s success unexpectly",
					 test_cases[i].name);
			}
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct passwd *ltpuser;
	const char *fs_type;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, "mntpoint", 0755);

	SAFE_MOUNT(cleanup, device, "mntpoint", fs_type, 0, NULL);
	mount_flag = 1;

	/* Create a file in the file system, then remount it as read-only */
	SAFE_TOUCH(cleanup, "mntpoint/tfile_3", 0644, NULL);

	SAFE_MOUNT(cleanup, device, "mntpoint", fs_type,
		   MS_REMOUNT | MS_RDONLY, NULL);

	fd3 = SAFE_OPEN(cleanup, "mntpoint/tfile_3", O_RDONLY);

	fd1 = SAFE_OPEN(cleanup, "tfile_1", O_RDWR | O_CREAT, 0666);

	fd2 = SAFE_OPEN(cleanup, "tfile_2", O_RDWR | O_CREAT, 0666);

	SAFE_CLOSE(cleanup, fd2);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	if (fd1 > 0 && close(fd1))
		tst_resm(TWARN | TERRNO, "close(fd1) failed");

	if (fd3 > 0 && close(fd3))
		tst_resm(TWARN | TERRNO, "close(fd1) failed");

	if (mount_flag && tst_umount("mntpoint") < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "umount device:%s failed", device);
	}

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
