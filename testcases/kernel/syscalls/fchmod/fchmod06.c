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
#define _GNU_SOURCE
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
#include "usctest.h"
#include "safe_macros.h"

static int fd1;
static int fd2;
static int fd3;
static char *fstype = "ext2";
static char *device;
static int dflag;
static int mount_flag;

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &dflag, &device},
	{NULL, NULL, NULL}
};

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
static int exp_enos[] = { EPERM, EBADF, EROFS, 0 };

static void setup(void);
static void cleanup(void);
static void help(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int i;

	msg = parse_opts(ac, av, options, help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (!dflag) {
		tst_brkm(TBROK, NULL,
			 "you must specify the device used for mounting with "
			 "-D option");
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

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
	static struct passwd *ltpuser;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	tst_mkfs(NULL, device, fstype, NULL);

	SAFE_MKDIR(cleanup, "mntpoint", 0755);

	if (mount(device, "mntpoint", fstype, 0, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag = 1;

	/* Create a file in the file system, then remount it as read-only */
	SAFE_TOUCH(cleanup, "mntpoint/tfile_3", 0644, NULL);

	if (mount(device, "mntpoint", fstype,
		  MS_REMOUNT | MS_RDONLY, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag = 1;

	fd3 = SAFE_OPEN(cleanup, "mntpoint/tfile_3", O_RDONLY);

	fd1 = SAFE_OPEN(cleanup, "tfile_1", O_RDWR | O_CREAT, 0666);

	fd2 = SAFE_OPEN(cleanup, "tfile_2", O_RDWR | O_CREAT, 0666);

	SAFE_CLOSE(cleanup, fd2);

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup(void)
{
	SAFE_SETEUID(NULL, 0);

	TEST_CLEANUP;

	SAFE_CLOSE(NULL, fd1);

	SAFE_CLOSE(NULL, fd3);

	if (mount_flag && umount("mntpoint") < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "umount device:%s failed", device);
	}

	tst_rmdir();
}

static void help(void)
{
	printf("-T type   : specifies the type of filesystem to be mounted. "
	       "Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}
