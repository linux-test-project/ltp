/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

/*
 * DESCRIPTION
 *  Test for feature MNT_DETACH of umount2().
 *  "Perform a lazy unmount: make the mount point unavailable for
 *   new accesses, and actually perform the unmount when the mount
 *   point ceases to be busy."
 */

#include <errno.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/mount.h"

static void setup(void);
static void umount2_verify(void);
static void cleanup(void);

char *TCID = "umount2_01";
int TST_TOTAL = 1;

#define DIR_MODE	(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define FILE_MODE	(S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)
#define MNTPOINT	"mntpoint"

static int fd;
static int mount_flag;

static const char *device;
static const char *fs_type;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		umount2_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, NULL);

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	TEST_PAUSE;
}

static void umount2_verify(void)
{
	int ret;
	char buf[256];
	const char *str = "abcdefghijklmnopqrstuvwxyz";

	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
	mount_flag = 1;

	fd = SAFE_CREAT(cleanup, MNTPOINT "/file", FILE_MODE);

	TEST(umount2(MNTPOINT, MNT_DETACH));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "umount2(2) Failed");
		goto EXIT;
	}

	mount_flag = 0;

	/* check the unavailability for new access */
	ret = access(MNTPOINT "/file", F_OK);

	if (ret != -1) {
		tst_resm(TFAIL, "umount2(2) MNT_DETACH flag "
			"performed abnormally");
		goto EXIT;
	}

	/*
	 * check the old fd still points to the file
	 * in previous mount point and is available
	 */
	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, str, strlen(str));

	SAFE_CLOSE(cleanup, fd);

	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);
	mount_flag = 1;

	fd = SAFE_OPEN(cleanup, MNTPOINT "/file", O_RDONLY);

	memset(buf, 0, sizeof(buf));

	SAFE_READ(cleanup, 1, fd, buf, strlen(str));

	if (strcmp(str, buf)) {
		tst_resm(TFAIL, "umount2(2) MNT_DETACH flag "
			"performed abnormally");
		goto EXIT;
	}

	tst_resm(TPASS, "umount2(2) Passed");

EXIT:
	SAFE_CLOSE(cleanup, fd);
	fd = 0;

	if (mount_flag) {
		if (tst_umount(MNTPOINT))
			tst_brkm(TBROK, cleanup, "umount() failed");
		mount_flag = 0;
	}
}

static void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	if (mount_flag && tst_umount(MNTPOINT))
		tst_resm(TWARN | TERRNO, "Failed to unmount");

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
