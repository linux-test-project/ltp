// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Basic test for the BLKROSET and BLKROGET ioctls.
 *
 * - Set the device read only, read the value back.
 * - Try to mount the device read write, expect failure.
 * - Try to mount the device read only, expect success.
 */

#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

static int fd;

static void verify_ioctl(void)
{
	int ro = 1;

	SAFE_IOCTL(fd, BLKROGET, &ro);

	if (ro == 0)
		tst_res(TPASS, "BLKROGET returned 0");
	else
		tst_res(TFAIL, "BLKROGET returned %i", ro);

	ro = 1;
	SAFE_IOCTL(fd, BLKROSET, &ro);

	ro = 0;
	SAFE_IOCTL(fd, BLKROGET, &ro);

	if (ro == 0)
		tst_res(TFAIL, "BLKROGET returned 0");
	else
		tst_res(TPASS, "BLKROGET returned %i", ro);

	TEST(mount(tst_device->dev, "mntpoint", tst_device->fs_type, 0, NULL));

	if (TST_RET != -1) {
		tst_res(TFAIL, "Mounting RO device RW succeeded");
		tst_umount("mntpoint");
		goto next;
	}

	if (TST_ERR == EACCES) {
		tst_res(TPASS | TTERRNO, "Mounting RO device RW failed");
		goto next;
	}

	tst_res(TFAIL | TTERRNO,
		"Mounting RO device RW failed unexpectedly expected EACCES");

next:
	TEST(mount(tst_device->dev, "mntpoint", tst_device->fs_type, MS_RDONLY, NULL));

	if (TST_RET == 0) {
		tst_res(TPASS, "Mounting RO device RO works");
		tst_umount("mntpoint");
	} else {
		tst_res(TFAIL | TTERRNO, "Mounting RO device RO failed");
	}

	ro = 0;
	SAFE_IOCTL(fd, BLKROSET, &ro);
}

static void setup(void)
{
	SAFE_MKDIR("mntpoint", 0777);
	fd = SAFE_OPEN(tst_device->dev, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.timeout = 1,
	.format_device = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
};
