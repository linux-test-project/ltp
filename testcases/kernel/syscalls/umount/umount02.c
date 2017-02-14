/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    DESCRIPTION
 *	Check for basic errors returned by umount(2) system call.
 *
 *	Verify that umount(2) returns -1 and sets errno to
 *
 *	1) EBUSY if it cannot be umounted, because dir is still busy.
 *	2) EFAULT if specialfile or device file points to invalid address space.
 *	3) ENOENT if pathname was empty or has a nonexistent component.
 *	4) EINVAL if specialfile or device is invalid or not a mount point.
 *	5) ENAMETOOLONG if pathname was longer than MAXPATHLEN.
 *****************************************************************************/

#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static char long_path[PATH_MAX + 2];
static int mount_flag;
static int fd;

static struct tcase {
	const char *err_desc;
	const char *mntpoint;
	int exp_errno;
} tcases[] = {
	{"Already mounted/busy", MNTPOINT, EBUSY},
	{"Invalid address", NULL, EFAULT},
	{"Directory not found", "nonexistent", ENOENT},
	{"Invalid  device", "./", EINVAL},
	{"Pathname too long", long_path, ENAMETOOLONG}
};

static void verify_umount(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(umount(tc->mntpoint));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "umount() succeeds unexpectedly");
		return;
	}

	if (tc->exp_errno != TEST_ERRNO) {
		tst_res(TFAIL | TTERRNO, "umount() should fail with %s",
			tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "umount() fails as expected: %s",
		tc->err_desc);
}

static void setup(void)
{
	memset(long_path, 'a', PATH_MAX + 1);

	SAFE_MKDIR(MNTPOINT, 0775);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL);
	mount_flag = 1;

	fd = SAFE_CREAT(MNTPOINT "/file", 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.tid = "umount02",
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_umount,
};
