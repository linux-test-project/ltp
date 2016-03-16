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
 *	This is a Phase I test for the mount(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 */

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "mount01";
int TST_TOTAL = 1;

#define DIR_MODE (S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP)
#define MNTPOINT "mntpoint"

static const char *device;
static const char *fs_type;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(mount(device, MNTPOINT, fs_type, 0, NULL));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL | TTERRNO, "mount(2) failed");
		} else {
			tst_resm(TPASS, "mount(2) passed ");
			TEST(tst_umount(MNTPOINT));
			if (TEST_RETURN != 0) {
				tst_brkm(TBROK | TTERRNO, cleanup,
					 "umount(2) failed");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (device)
		tst_release_device(device);

	tst_rmdir();
}
