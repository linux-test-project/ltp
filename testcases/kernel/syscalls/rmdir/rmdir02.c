/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Description:
 *   1) create a directory tstdir1, create a file under it.
 *      call rmdir(tstdir1), verify the return value is -1
 *      and the errno is ENOTEMPTY.
 *   2) create a directory with long path, call rmdir(tstdir1),
 *      verify the return value is -1 and the errno is ENAMETOOLONG.
 *   3) pass a pathname containing non-exist directory component
 *      to rmdir(), verify the return value is -1 and the errno
 *      is ENOENT.
 *   4) pass a pathname containing a file component to rmdir(),
 *      verify the return value is -1 and the errno is ENOTDIR.
 *   5) attempt to pass an invalid pathname with an address
 *      pointing outside the address space of the process, as the
 *      argument to rmdir(), verify the return value is -1 and
 *      the errno is EFAULT.
 *   6) attempt to pass an invalid pathname with NULL, as the
 *      argument to rmdir(), verify the return value is -1 and
 *      the errno is EFAULT.
 *   7) pass a pathname with too many symbolic links to rmdir(),
 *      verify the return value is -1 and the errno is ELOOP.
 *   8) pass a pathname which refers to a directory on a read-only
 *      file system to rmdir(), verify the return value is -1 and
 *      the errno is EROFS.
 *   9) pass a pathname which is currently used as a mount point
 *      to rmdir(), verify the return value is -1 and the errno is
 *      EBUSY.
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)
#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)

#define TESTDIR		"testdir"
#define TESTDIR2	"nosuchdir/testdir2"
#define TESTDIR3	"testfile2/testdir3"
#define TESTDIR4	"/loopdir"
#define MNTPOINT	"mntpoint"
#define TESTDIR5	"mntpoint/testdir5"
#define TESTFILE    "testdir/testfile"
#define TESTFILE2   "testfile2"

static char longpathname[PATH_MAX + 2];
static char looppathname[sizeof(TESTDIR4) * 43] = ".";

static const char *device;
static int mount_flag;

static struct test_case_t {
	char *dir;
	int exp_errno;
} test_cases[] =  {
	{TESTDIR, ENOTEMPTY},
	{longpathname, ENAMETOOLONG},
	{TESTDIR2, ENOENT},
	{TESTDIR3, ENOTDIR},
#if !defined(UCLINUX)
	{(char *)-1, EFAULT},
#endif
	{NULL, EFAULT},
	{looppathname, ELOOP},
	{TESTDIR5, EROFS},
	{MNTPOINT, EBUSY},
};

static void setup(void);
static void rmdir_verify(struct test_case_t *tc);
static void cleanup(void);

char *TCID = "rmdir02";
int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			rmdir_verify(&test_cases[i]);
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

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);
	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
	SAFE_MKDIR(cleanup, TESTDIR5, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, MS_REMOUNT | MS_RDONLY,
		   NULL);
	mount_flag = 1;

	SAFE_MKDIR(cleanup, TESTDIR, DIR_MODE);
	SAFE_TOUCH(cleanup, TESTFILE, FILE_MODE, NULL);

	memset(longpathname, 'a', PATH_MAX + 1);

	SAFE_TOUCH(cleanup, TESTFILE2, FILE_MODE, NULL);

#if !defined(UCLINUX)
	test_cases[4].dir = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
#endif

	/*
	 * NOTE: the ELOOP test is written based on that the
	 * consecutive symlinks limit in kernel is hardwired
	 * to 40.
	 */
	SAFE_MKDIR(cleanup, "loopdir", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../loopdir", "loopdir/loopdir");
	for (i = 0; i < 43; i++)
		strcat(looppathname, TESTDIR4);
}

static void rmdir_verify(struct test_case_t *tc)
{
	TEST(rmdir(tc->dir));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "rmdir() returned %ld, "
			"expected -1, errno:%d", TEST_RETURN,
			tc->exp_errno);
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "rmdir() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"rmdir() failed unexpectedly; expected: %d - %s",
			tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNTPOINT) == -1)
		tst_resm(TWARN | TERRNO, "umount %s failed", MNTPOINT);

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
