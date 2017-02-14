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
 * AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 * DESCRIPTION
 *	This is a Phase I test for the umount(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *****************************************************************************/

#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

#define MNTPOINT	"mntpoint"

static int mount_flag;

static void verify_umount(void)
{
	if (mount_flag != 1) {
		SAFE_MOUNT(tst_device->dev, MNTPOINT,
			tst_device->fs_type, 0, NULL);
		mount_flag = 1;
	}

	TEST(umount(MNTPOINT));

	if (TEST_RETURN != 0 && TEST_ERRNO == EBUSY) {
		tst_res(TINFO, "umount() Failed with EBUSY "
			"possibly some daemon (gvfsd-trash) "
			"is probing newly mounted dirs");
	}

	if (TEST_RETURN != 0) {
		tst_res(TFAIL | TTERRNO, "umount() Failed");
		return;
	}

	tst_res(TPASS, "umount() Passed");
	mount_flag = 0;
}

static void setup(void)
{
	SAFE_MKDIR(MNTPOINT, 0775);
}

static void cleanup(void)
{
	if (mount_flag)
		tst_umount(MNTPOINT);
}

static struct tst_test test = {
	.tid = "umount01",
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_umount,
};
