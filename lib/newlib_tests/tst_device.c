/*
 * Copyright (c) 2016 Linux Test Project
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

#include <stdlib.h>
#include <sys/mount.h>
#include <stdint.h>

#include "tst_test.h"

static void do_test(void)
{
	int fd;
	const char *dev;
	uint64_t ltp_dev_size;

	dev = tst_device->dev;
	if (!dev)
		tst_brk(TCONF, "Failed to acquire test device");

	SAFE_MKFS(dev, "ext2", NULL, NULL);

	fd = SAFE_OPEN(dev, O_RDONLY);
	SAFE_IOCTL(fd, BLKGETSIZE64, &ltp_dev_size);
	SAFE_CLOSE(fd);

	if (ltp_dev_size/1024/1024 == 300)
		tst_res(TPASS, "Got expected device size");
	else
		tst_res(TFAIL, "Got unexpected device size");
}

static struct tst_test test = {
	.tid = "tst_device",
	.needs_tmpdir = 1,
	.needs_device = 1,
	.dev_min_size = 300,
	.test_all = do_test,
};
