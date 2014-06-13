/*
 * Copyright (c) Crackerjack Project., 2007 ,Hitachi, Ltd
 * Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>
 * Ported to LTP: Manas Kumar Nayak maknayak@in.ibm.com>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Description:
 *   Verify that,
 *   1) utimes() returns -1 and sets errno to EACCES if times
 *      is NULL, the caller's effective user ID does not match
 *      the owner of the file, the caller does not have write
 *      access to the file, and the caller is not privileged.
 *   2) utimes() returns -1 and sets errno to ENOENT if filename
 *      does not exist.
 *   3) utimes() returns -1 and sets errno to EFAULT if filename
 *      is NULL.
 *   4) utimes() returns -1 and sets errno to EPERM if times is
 *      not NULL, the caller's effective UID does not match the
 *      owner of the file, and the caller is not privileged.
 *   5) utimes() returns -1 and sets errno to EROFS if path resides
 *      on a read-only file system.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/mount.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

#define MNTPOINT "mntpoint"
#define TESTFILE1 "testfile1"
#define TESTFILE2 "testfile2"
#define TESTFILE3 "mntpoint/testfile"
#define FILE_MODE (S_IRWXU | S_IRGRP | S_IXGRP | \
					S_IROTH | S_IXOTH)
#define DIR_MODE (S_IRWXU | S_IRWXG | S_IRWXO)

#define LTPUSER1 "nobody"
#define LTPUSER2 "bin"

static char *fstype = "ext2";
static char *device;
static int mount_flag;

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", NULL, &device},
	{NULL, NULL, NULL}
};

static struct timeval a_tv[2] = { {0, 0}, {1000, 0} };
static struct timeval m_tv[2] = { {1000, 0}, {0, 0} };
static struct timeval tv[2] = { {1000, 0}, {2000, 0} };

static struct test_case_t {
	char *pathname;
	struct timeval *times;
	int exp_errno;
} test_cases[] = {
	{ TESTFILE1, a_tv, 0 },
	{ TESTFILE1, m_tv, 0 },
	{ TESTFILE2, NULL, EACCES },
	{ "notexistfile", tv, ENOENT },
	{ NULL, tv, EFAULT },
	{ TESTFILE2, tv, EPERM },
	{ TESTFILE3, tv, EROFS },
};

static void setup(void);
static void cleanup(void);
static void utimes_verify(const struct test_case_t *);
static void help(void);

char *TCID = "utimes01";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { EACCES, ENOENT, EFAULT,
							EPERM, EROFS, 0 };

int main(int ac, char **av)
{
	int i, lc;
	const char *msg;

	msg = parse_opts(ac, av, options, help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (!device) {
		tst_brkm(TCONF, NULL, "you must specify the device "
			"used for mounting with -D option");
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			utimes_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	struct passwd *ltpuser;

	tst_require_root(NULL);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	SAFE_TOUCH(cleanup, TESTFILE1, FILE_MODE, NULL);
	ltpuser = SAFE_GETPWNAM(cleanup, LTPUSER1);
	SAFE_CHOWN(cleanup, TESTFILE1, ltpuser->pw_uid,
		ltpuser->pw_gid);

	SAFE_TOUCH(cleanup, TESTFILE2, FILE_MODE, NULL);
	ltpuser = SAFE_GETPWNAM(cleanup, LTPUSER2);
	SAFE_CHOWN(cleanup, TESTFILE2, ltpuser->pw_uid,
		ltpuser->pw_gid);

	tst_mkfs(NULL, device, fstype, NULL);
	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);
	if (mount(device, MNTPOINT, fstype, 0, NULL) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"mount device:%s failed", device);
	}
	SAFE_TOUCH(cleanup, TESTFILE3, FILE_MODE, NULL);
	ltpuser = SAFE_GETPWNAM(cleanup, LTPUSER1);
	SAFE_CHOWN(cleanup, TESTFILE3, ltpuser->pw_uid,
		ltpuser->pw_gid);
	if (mount(device, MNTPOINT, fstype,
			MS_REMOUNT | MS_RDONLY, NULL) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"mount device:%s failed", device);
	}
	mount_flag = 1;

	ltpuser = SAFE_GETPWNAM(cleanup, LTPUSER1);
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void utimes_verify(const struct test_case_t *tc)
{
	struct stat st;
	struct timeval tmp_tv[2];

	if (tc->exp_errno == 0) {
		SAFE_STAT(cleanup, tc->pathname, &st);

		tmp_tv[0].tv_sec = st.st_atime;
		tmp_tv[0].tv_usec = 0;
		tmp_tv[1].tv_sec = st.st_mtime;
		tmp_tv[1].tv_usec = 0;
	}

	TEST(utimes(tc->pathname, tc->times));

	TEST_ERROR_LOG(TEST_ERRNO);

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "utimes() worked as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"utimes() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}

	if (TEST_ERRNO == 0 && utimes(tc->pathname, tmp_tv) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "utimes() failed.");
}

void cleanup(void)
{
	if (seteuid(0) == -1)
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	if (mount_flag && umount(MNTPOINT) == -1)
		tst_resm(TWARN | TERRNO, "umount %s failed", MNTPOINT);

	TEST_CLEANUP;

	tst_rmdir();
}

static void help(void)
{
	printf("-T type   : specifies the type of filesystem to be mounted. "
		"Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}
