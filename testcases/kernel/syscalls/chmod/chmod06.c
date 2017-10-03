/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: chmod06
 *
 * Test Description:
 *   Verify that,
 *   1) chmod(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) chmod(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   3) chmod(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) chmod(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) chmod(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) chmod(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mount.h>

#include "test.h"
#include "safe_macros.h"

#define MODE_RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define TEST_FILE3	"t_file/tfile_3"
#define TEST_FILE4	"test_file4"
#define MNT_POINT	"mntpoint"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)

static char long_path[PATH_MAX + 2];

static const char *device;
static int mount_flag;
static uid_t nobody_uid;

static void set_root(void);
static void set_nobody(void);

struct test_case_t {
	char *pathname;
	mode_t mode;
	int exp_errno;
	void (*setup)(void);
	void (*cleanup)(void);
} tc[] = {
	{TEST_FILE1, FILE_MODE, EPERM, set_nobody, set_root},
	{TEST_FILE2, FILE_MODE, EACCES, set_nobody, set_root},
	{(char *)-1, FILE_MODE, EFAULT, NULL, NULL},
	{(char *)-2, FILE_MODE, EFAULT, NULL, NULL},
	{long_path, FILE_MODE, ENAMETOOLONG, NULL, NULL},
	{"", FILE_MODE, ENOENT, NULL, NULL},
	{TEST_FILE3, FILE_MODE, ENOTDIR, NULL, NULL},
	{MNT_POINT, FILE_MODE, EROFS, NULL, NULL},
	{TEST_FILE4, FILE_MODE, ELOOP, NULL, NULL},
};

char *TCID = "chmod06";
int TST_TOTAL = ARRAY_SIZE(tc);

static char *bad_addr = 0;

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

			if (tc[i].setup)
				tc[i].setup();

			TEST(chmod(tc[i].pathname, tc[i].mode));

			if (tc[i].cleanup)
				tc[i].cleanup();

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "chmod succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == tc[i].exp_errno)
				tst_resm(TPASS | TTERRNO,
					 "chmod failed as expected");
			else
				tst_resm(TFAIL | TTERRNO,
					 "chmod failed unexpectedly; "
					 "expected %d - %s",
					 tc[i].exp_errno,
					 tst_strerrno(tc[i].exp_errno));
		}

	}

	cleanup();
	tst_exit();
}

void set_root(void)
{
	SAFE_SETEUID(cleanup, 0);
}

void set_nobody(void)
{
	SAFE_SETEUID(cleanup, nobody_uid);
}

void setup(void)
{
	struct passwd *nobody;
	const char *fs_type;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	nobody = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = nobody->pw_uid;

	bad_addr = SAFE_MMAP(NULL, 0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	tc[3].pathname = bad_addr;

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	SAFE_TOUCH(cleanup, TEST_FILE1, 0666, NULL);
	SAFE_MKDIR(cleanup, DIR_TEMP, MODE_RWX);
	SAFE_TOUCH(cleanup, TEST_FILE2, 0666, NULL);
	SAFE_CHMOD(cleanup, DIR_TEMP, FILE_MODE);
	SAFE_TOUCH(cleanup, "t_file", MODE_RWX, NULL);

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);

	/*
	 * mount a read-only file system for test EROFS
	 */
	SAFE_MOUNT(cleanup, device, MNT_POINT, fs_type, MS_RDONLY, NULL);
	mount_flag = 1;

	memset(long_path, 'a', PATH_MAX+1);

	/*
	 * create two symbolic links who point to each other for
	 * test ELOOP.
	 */
	SAFE_SYMLINK(cleanup, "test_file4", "test_file5");
	SAFE_SYMLINK(cleanup, "test_file5", "test_file4");
}

static void cleanup(void)
{
	if (chmod(DIR_TEMP, MODE_RWX) == -1)
		tst_resm(TBROK | TERRNO, "chmod(%s) failed", DIR_TEMP);

	if (mount_flag && tst_umount(MNT_POINT) < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "umount device:%s failed", device);
	}

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
