/* Copyright (c) International Business Machines  Corp., 2001
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Description:
 *  1) create a directory tstdir1, create a file under it.
 *     call rmdir(tstdir1), verify the return value is -1
 *     and the errno is ENOTEMPTY.
 *  2) create a directory with long path, call rmdir(tstdir1),
 *     verify the return value is -1 and the errno is ENAMETOOLONG.
 *  3) pass a pathname containing non-exist directory component
 *     to rmdir(), verify the return value is -1 and the errno
 *     is ENOENT.
 *  4) pass a pathname containing a file component to rmdir(),
 *     verify the return value is -1 and the errno is ENOTDIR.
 *  5) attempt to pass an invalid pathname with an address
 *     pointing outside the address space of the process, as the
 *     argument to rmdir(), verify the return value is -1 and
 *     the errno is EFAULT.
 *  6) pass a pathname with too many symbolic links to rmdir(),
 *     verify the return value is -1 and the errno is ELOOP.
 *  7) pass a pathname which refers to a directory on a read-only
 *     file system to rmdir(), verify the return value is -1 and
 *     the errno is EROFS.
 *  8) pass a pathname which is currently used as a mount point
 *     to rmdir(), verify the return value is -1 and the errno is
 *     EBUSY.
 *  9) pass a pathname which points to the current directory(.)
 *     to  rmdir(), verify the return value is -1 and the errno is
 *     EINVAL.
 */

#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include "tst_test.h"

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
#define CURRENTDIR  "."

static char longpathname[PATH_MAX + 2];
static char looppathname[sizeof(TESTDIR4) * 43] = ".";

static struct testcase {
	char *dir;
	int exp_errno;
} tcases[] =  {
	{TESTDIR, ENOTEMPTY},
	{longpathname, ENAMETOOLONG},
	{TESTDIR2, ENOENT},
	{TESTDIR3, ENOTDIR},
	{NULL, EFAULT},
	{looppathname, ELOOP},
	{TESTDIR5, EROFS},
	{MNTPOINT, EBUSY},
	{CURRENTDIR, EINVAL},
};

static void setup(void)
{
	unsigned int i;

	SAFE_MKDIR(TESTDIR5, DIR_MODE);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type,
				 MS_REMOUNT | MS_RDONLY, NULL);
	SAFE_MKDIR(TESTDIR, DIR_MODE);
	SAFE_TOUCH(TESTFILE, FILE_MODE, NULL);

	memset(longpathname, 'a', PATH_MAX + 1);

	SAFE_TOUCH(TESTFILE2, FILE_MODE, NULL);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].exp_errno == EFAULT) {
			tcases[i].dir = SAFE_MMAP(0, 1, PROT_NONE,
			                          MAP_PRIVATE | MAP_ANONYMOUS,
			                          0, 0);
		}
	}

	/*
	* NOTE: the ELOOP test is written based on that the
	* consecutive symlinks limit in kernel is hardwire
	* to 40.
	*/
	SAFE_MKDIR("loopdir", DIR_MODE);
	SAFE_SYMLINK("../loopdir", "loopdir/loopdir");
	for (i = 0; i < 43; i++)
		strcat(looppathname, TESTDIR4);
}

static void verify_rmdir(unsigned int n)
{
	struct testcase *tc = &tcases[n];

	TEST(rmdir(tc->dir));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "rmdir() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_res(TPASS | TTERRNO, "rmdir() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"rmdir() failed unexpectedly; expected: %d, got ",
			tc->exp_errno);
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_rmdir,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mntpoint = MNTPOINT,
	.mount_device = 1,

};

