// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Basic test for :manpage:`ioctl(2)` with BLKRASET and BLKRAGET.
 *
 * Sets device read-ahead, reads it back and compares the values.
 *
 * The read-ahead value was choosen to be multiple of 512, since it's rounded
 * based on page size on BLKRASET and 512 should be safe enough for everyone.
 */

#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

static int fd;

static void verify_ioctl(void)
{
	unsigned long ra, rab, rao;

	SAFE_IOCTL(fd, BLKRAGET, &rao);

	tst_res(TINFO, "BLKRAGET original value %lu", rao);

	for (ra = 0; ra <= 4096; ra += 512) {
		SAFE_IOCTL(fd, BLKRASET, ra);
		SAFE_IOCTL(fd, BLKRAGET, &rab);

		if (ra == rab)
			tst_res(TPASS, "BLKRASET %lu read back correctly", ra);
		else
			tst_res(TFAIL, "BLKRASET %lu read back %lu", ra, rab);
	}

	tst_res(TINFO, "BLKRASET restoring original value %lu", rao);

	SAFE_IOCTL(fd, BLKRASET, rao);
}

static void setup(void)
{
	fd = SAFE_OPEN(tst_device->dev, O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_device = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
};
