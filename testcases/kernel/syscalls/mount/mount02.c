/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *  AUTHOR: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 *
 */

/*

   DESCRIPTION
	Check for basic errors returned by mount(2) system call.

	Verify that mount(2) returns -1 and sets errno to
	1) ENODEV if filesystem type not configured
	2) ENOTBLK if specialfile is not a block device
	3) EBUSY if specialfile is already mounted or
		it  cannot  be remounted read-only, because it still holds
		files open for writing.
	4) EINVAL if specialfile or device is invalid or
		 a remount was attempted, while source was not already
		 mounted on target.
	5) EFAULT if specialfile or device file points to invalid address space.
	6) ENAMETOOLONG if pathname was longer than MAXPATHLEN.
	7) ENOENT if pathname was empty or has a nonexistent component.
	8) ENOTDIR if not a directory.
*/

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "mount02";

#define DIR_MODE	(S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)
#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO)

static char path[PATH_MAX + 2];
static const char *long_path = path;
static const char *fs_type;
static const char *wrong_fs_type = "error";
static const char *mntpoint = "mntpoint";
static const char *device;
static const char *null = NULL;
static const char *fault = (void*)-1;
static const char *nonexistent = "nonexistent";
static const char *char_dev = "char_device";
static const char *file = "filename";
static int fd;

static void do_umount(void);
static void close_umount(void);
static void do_mount(void);
static void mount_open(void);

static struct test_case {
	const char **device;
	const char **mntpoint;
	const char **fs_type;
	unsigned long flag;
	int exp_errno;
	void (*setup)(void);
	void (*cleanup)(void);
} tc[] = {
	{&device, &mntpoint, &wrong_fs_type, 0, ENODEV, NULL, NULL},
	{&char_dev, &mntpoint, &fs_type, 0, ENOTBLK, NULL, NULL},
	{&device, &mntpoint, &fs_type, 0, EBUSY, do_mount, do_umount},
	{&device, &mntpoint, &fs_type, MS_REMOUNT | MS_RDONLY, EBUSY,
	 mount_open, close_umount},
	{&null, &mntpoint, &fs_type, 0, EINVAL, NULL, NULL},
	{&device, &mntpoint, &null, 0, EINVAL, NULL, NULL},
	{&device, &mntpoint, &fs_type, MS_REMOUNT, EINVAL, NULL, NULL},
	{&fault, &mntpoint, &fs_type, 0, EFAULT, NULL, NULL},
	{&device, &mntpoint, &fault, 0, EFAULT, NULL, NULL},
	{&device, &long_path, &fs_type, 0, ENAMETOOLONG, NULL, NULL},
	{&device, &nonexistent, &fs_type, 0, ENOENT, NULL, NULL},
	{&device, &file, &fs_type, 0, ENOTDIR, NULL, NULL},
};

int TST_TOTAL = ARRAY_SIZE(tc);

static void verify_mount(struct test_case *tc)
{
	if (tc->setup)
		tc->setup();

	TEST(mount(*tc->device, *tc->mntpoint, *tc->fs_type, tc->flag, NULL));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mount() succeded unexpectedly (ret=%li)",
		         TEST_RETURN);
		goto cleanup;
	}

	if (TEST_ERRNO != tc->exp_errno) {
		tst_resm(TFAIL | TTERRNO,
		         "mount() was expected to fail with %s(%i)",
		         tst_strerrno(tc->exp_errno), tc->exp_errno);
		goto cleanup;
	}

	tst_resm(TPASS | TTERRNO, "mount() failed expectedly");

cleanup:
	if (tc->cleanup)
		tc->cleanup();
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i)
			verify_mount(tc + i);
	}

	cleanup();
	tst_exit();
}

static void do_mount(void)
{
	if (mount(device, mntpoint, fs_type, 0, NULL))
		tst_brkm(TBROK | TERRNO, cleanup, "Failed to mount(mntpoint)");
}

static void mount_open(void)
{
	do_mount();

	fd = SAFE_OPEN(cleanup, "mntpoint/file", O_CREAT | O_RDWR, S_IRWXU);
}

static void close_umount(void)
{
	SAFE_CLOSE(cleanup, fd);
	do_umount();
}

static void do_umount(void)
{
	if (tst_umount(mntpoint))
		tst_brkm(TBROK | TERRNO, cleanup, "Failed to umount(mntpoint)");
}

static void setup(void)
{
	dev_t dev;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	tst_tmpdir();

	SAFE_TOUCH(cleanup, file, FILE_MODE, NULL);

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, mntpoint, DIR_MODE);

	memset(path, 'a', PATH_MAX + 1);

	dev = makedev(1, 3);
	if (mknod(char_dev, S_IFCHR | FILE_MODE, dev)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to mknod(char_dev, S_IFCHR | FILE_MODE, %lu)",
			 dev);
	}

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (device)
		tst_release_device(device);

	tst_rmdir();
}
