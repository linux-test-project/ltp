/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 * 1. Verify that the system call utime() fails to set the modification
 *    and access times of a file to the current time, under the following
 *    constraints,
 *	 - The times argument is null.
 *	 - The user ID of the process is not "root".
 * 2. Verify that the system call utime() fails to set the modification
 *    and access times of a file if the specified file doesn't exist.
 * 3. Verify that the system call utime() fails to set the modification
 *    and access times of a file to the current time, under the following
 *    constraints,
 *	 - The times argument is not null.
 *	 - The user ID of the process is not "root".
 * 4. Verify that the system call utime() fails to set the modification
 *    and access times of a file that resides on a read-only file system.
 */

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"
#define MNT_POINT	"mntpoint"

char *TCID = "utime06";
static struct passwd *ltpuser;
static const struct utimbuf times;
static const char *dev;
static int mount_flag;
static void setup_nobody(void);
static void cleanup_nobody(void);

struct test_case_t {
	char *pathname;
	int exp_errno;
	const struct utimbuf *times;
	void (*setup_func)(void);
	void (*cleanup_func)(void);
} Test_cases[] = {
	{TEMP_FILE, EACCES, NULL, setup_nobody, cleanup_nobody},
	{"", ENOENT, NULL, NULL, NULL},
	{TEMP_FILE, EPERM, &times, setup_nobody, cleanup_nobody},
	{MNT_POINT, EROFS, NULL, NULL, NULL},
};

int TST_TOTAL = ARRAY_SIZE(Test_cases);
static void setup(void);
static void utime_verify(const struct test_case_t *);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			utime_verify(&Test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	const char *fs_type;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TEMP_FILE, 0644, NULL);

	fs_type = tst_dev_fs_type();
	dev = tst_acquire_device(cleanup);
	if (!dev)
		tst_brkm(TCONF, cleanup, "Failed to acquire test device");

	tst_mkfs(cleanup, dev, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNT_POINT, 0644);
	SAFE_MOUNT(cleanup, dev, MNT_POINT, fs_type, MS_RDONLY, NULL);
	mount_flag = 1;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
}

static void utime_verify(const struct test_case_t *test)
{
	if (test->setup_func != NULL)
		test->setup_func();

	TEST(utime(test->pathname, test->times));

	if (test->cleanup_func != NULL)
		test->cleanup_func();

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "utime succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "utime failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "utime failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void setup_nobody(void)
{
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup_nobody(void)
{
	SAFE_SETEUID(cleanup, 0);
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNT_POINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", dev);

	if (dev)
		tst_release_device(dev);

	tst_rmdir();
}
