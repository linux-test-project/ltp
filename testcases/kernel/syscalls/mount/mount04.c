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
 * DESCRIPTION
 *	Verify that mount(2) returns -1 and sets errno to  EPERM if the user
 *	is not the super-user.
 */

#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "mount04";

#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP

static char *mntpoint = "mntpoint";
static const char *fs_type;
static const char *device;

int TST_TOTAL = 1;

static void verify_mount(void)
{

	TEST(mount(device, mntpoint, fs_type, 0, NULL));

	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == EPERM)
			tst_resm(TPASS | TTERRNO, "mount() failed expectedly");
		else
			tst_resm(TFAIL | TTERRNO,
			         "mount() was expected to fail with EPERM");
		return;
	}

	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "mount() succeeded unexpectedly");
		if (tst_umount(mntpoint))
			tst_brkm(TBROK, cleanup, "umount() failed");
		return;
	}

	tst_resm(TFAIL | TTERRNO, "mount() returned %li", TEST_RETURN);
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		verify_mount();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	ltpuser = SAFE_GETPWNAM(cleanup, nobody_uid);
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
	SAFE_MKDIR(cleanup, mntpoint, DIR_MODE);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (seteuid(0))
		tst_resm(TWARN | TERRNO, "seteuid(0) failed");

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
