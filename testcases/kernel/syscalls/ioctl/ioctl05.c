/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Basic test for the BLKGETSIZE and BLKGETSIZE64 ioctls.
 *
 * - BLKGETSIZE returns size in 512 byte blocks BLKGETSIZE64 in bytes
 *   compare that they return the same value.
 * - lseek to the end of the device, this should work
 * - try to read from the device, read should return 0
 */

#include <stdint.h>
#include <errno.h>
#include <sys/mount.h>
#include "tst_test.h"

static int fd;

static void verify_ioctl(void)
{
	unsigned long size = 0;
	uint64_t size64 = 0;
	char buf;
	int ret;

	fd = SAFE_OPEN(tst_device->dev, O_RDONLY);

	SAFE_IOCTL(fd, BLKGETSIZE, &size);
	SAFE_IOCTL(fd, BLKGETSIZE64, &size64);

	if (size == size64/512) {
		tst_res(TPASS, "BLKGETSIZE returned %lu, BLKGETSIZE64 %llu",
			size, (unsigned long long)size64);
	} else {
		tst_res(TFAIL,
			"BLKGETSIZE returned %lu, BLKGETSIZE64 returned %llu",
			size, (unsigned long long)size64);
	}

	if (lseek(fd, size * 512, SEEK_SET) !=  (off_t)size * 512) {
		tst_res(TFAIL | TERRNO,
			"Cannot lseek to the end of the device");
	} else {
		tst_res(TPASS, "Could lseek to the end of the device");
	}

	ret = read(fd, &buf, 1);

	if (ret == 0) {
		tst_res(TPASS,
			"Got EOF when trying to read after the end of device");
	} else {
		tst_res(TFAIL | TERRNO,
			"Read at the end of device returned %i", ret);
	}

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tid = "ioctl05",
	.needs_device = 1,
	.needs_root = 1,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
};
