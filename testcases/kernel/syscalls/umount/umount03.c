/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 *	Verify that umount(2) returns -1 and sets errno to  EPERM if the user
 *	is not the super-user.
 */

#include <errno.h>
#include <pwd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag;

static void verify_umount(void)
{
	TEST(umount(MNTPOINT));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "umount() succeeds unexpectedly");
		return;
	}

	if (TEST_ERRNO != EPERM) {
		tst_res(TFAIL | TTERRNO, "umount() should fail with EPERM");
		return;
	}

	tst_res(TPASS | TTERRNO, "umount() fails as expected");
}

static void setup(void)
{
	struct passwd *pw;

	SAFE_MKDIR(MNTPOINT, 0775);
	SAFE_MOUNT(tst_device->dev, MNTPOINT, tst_device->fs_type, 0, NULL);
	mount_flag = 1;

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pw->pw_uid);
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_res(TWARN | TERRNO, "seteuid(0) Failed");

	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.tid = "umount03",
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_umount,
};
