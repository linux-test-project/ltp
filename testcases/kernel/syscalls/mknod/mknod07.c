/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
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
 *
 * Test Description:
 * Verify that,
 *   1) mknod(2) returns -1 and sets errno to EPERM if the process id of
 *	the caller is not super-user.
 *   2) mknod(2) returns -1 and sets errno to EACCES if parent directory
 *	does not allow  write  permission  to  the process.
 *   3) mknod(2) returns -1 and sets errno to EROFS if pathname refers to
 *	a file on a read-only file system.
 *   4) mknod(2) returns -1 and sets errno to ELOOP if too many symbolic
 *	links were encountered in resolving pathname.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

#define DIR_TEMP		"testdir_1"
#define DIR_TEMP_MODE		(S_IRUSR | S_IXUSR)
#define DIR_MODE		(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
				 S_IXGRP|S_IROTH|S_IXOTH)
#define MNT_POINT		"mntpoint"

#define FIFO_MODE	(S_IFIFO | S_IRUSR | S_IRGRP | S_IROTH)
#define SOCKET_MODE	(S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO)
#define CHR_MODE	(S_IFCHR | S_IRUSR | S_IWUSR)
#define BLK_MODE	(S_IFBLK | S_IRUSR | S_IWUSR)

#define ELOPFILE	"/test_eloop"

static char elooppathname[sizeof(ELOPFILE) * 43] = ".";

static const char *device;
static int mount_flag;

static struct test_case_t {
	char *pathname;
	int mode;
	int exp_errno;
} test_cases[] = {
	{ "testdir_1/tnode_1", SOCKET_MODE, EACCES },
	{ "testdir_1/tnode_2", FIFO_MODE, EACCES },
	{ "tnode_3", CHR_MODE, EPERM },
	{ "tnode_4", BLK_MODE, EPERM },
	{ "mntpoint/tnode_5", SOCKET_MODE, EROFS },
	{ elooppathname, FIFO_MODE, ELOOP },
};

char *TCID = "mknod07";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void mknod_verify(const struct test_case_t *test_case);
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
			mknod_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;
	struct passwd *ltpuser;
	const char *fs_type;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to acquire device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	TEST_PAUSE;

	/* mount a read-only file system for EROFS test */
	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNT_POINT, fs_type, MS_RDONLY, NULL);
	mount_flag = 1;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);

	SAFE_MKDIR(cleanup, DIR_TEMP, DIR_TEMP_MODE);

	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	SAFE_MKDIR(cleanup, "test_eloop", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(elooppathname, ELOPFILE);
}

static void mknod_verify(const struct test_case_t *test_case)
{
	TEST(mknod(test_case->pathname, test_case->mode, 0));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mknod succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test_case->exp_errno) {
		tst_resm(TPASS | TTERRNO, "mknod failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mknod failed unexpectedly; expected: "
			 "%d - %s", test_case->exp_errno,
			 strerror(test_case->exp_errno));
	}
}

static void cleanup(void)
{
	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	if (mount_flag && tst_umount(MNT_POINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
