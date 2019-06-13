// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * Check the basic functionality of the pwritev(2) for the file
 * opened with O_DIRECT in all filesystem.
 * pwritev(2) should succeed to write the expected content of data
 * and after writing the file, the file offset is not changed.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include "tst_test.h"
#include "pwritev.h"
#include "tst_safe_prw.h"

#define MNTPOINT	"mntpoint"
#define FNAME	MNTPOINT"/file"

static char *initbuf, *preadbuf;
static int fd;
static off_t blk_off, zero_off;
static ssize_t blksz;

static struct iovec wr_iovec[] = {
	{NULL, 0},
	{NULL, 0},
};

static struct tcase {
	int count;
	off_t *offset;
	ssize_t *size;
} tcases[] = {
	{1, &zero_off, &blksz},
	{2, &zero_off, &blksz},
	{1, &blk_off, &blksz},
};

static void verify_direct_pwritev(unsigned int n)
{
	int i;
	struct tcase *tc = &tcases[n];

	SAFE_PWRITE(1, fd, initbuf, blksz * 2, 0);

	TEST(pwritev(fd, wr_iovec, tc->count, *tc->offset));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "pwritev(O_DIRECT) fails");
		return;
	}

	if (TST_RET != *tc->size) {
		tst_res(TFAIL, "pwritev(O_DIRECT) wrote %li bytes, expected %zi",
			 TST_RET, *tc->size);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != 0) {
		tst_res(TFAIL, "pwritev(O_DIRECT) had changed file offset");
		return;
	}

	memset(preadbuf, 0x00, blksz);
	SAFE_PREAD(1, fd, preadbuf, *tc->size, *tc->offset);

	for (i = 0; i < *tc->size; i++) {
		if (preadbuf[i] != 0x61)
			break;
	}

	if (i != *tc->size) {
		tst_res(TFAIL, "Buffer wrong at %i have %02x expected 0x61",
			 i, preadbuf[i]);
		return;
	}

	tst_res(TPASS, "pwritev(O_DIRECT) wrote %zi bytes successfully "
		 "with content 'a' expectedly ", *tc->size);
}

static void setup(void)
{
	int dev_fd, ret;
	
	dev_fd = SAFE_OPEN(tst_device->dev, O_RDWR);
	SAFE_IOCTL(dev_fd, BLKSSZGET, &ret);
	SAFE_CLOSE(dev_fd);

	if (ret <= 0)
		tst_brk(TBROK, "BLKSSZGET returned invalid block size %i", ret);

	tst_res(TINFO, "Using block size %i", ret);

	blksz = ret;
	blk_off = ret;

	fd = SAFE_OPEN(FNAME, O_RDWR | O_CREAT | O_DIRECT, 0644);

	initbuf = SAFE_MEMALIGN(blksz, blksz * 2);
	memset(initbuf, 0x00, blksz * 2);

	preadbuf = SAFE_MEMALIGN(blksz, blksz);

	wr_iovec[0].iov_base = SAFE_MEMALIGN(blksz, blksz);
	wr_iovec[0].iov_len = blksz;
	memset(wr_iovec[0].iov_base, 0x61, blksz);
}

static void cleanup(void)
{
	free(initbuf);
	free(preadbuf);
	free(wr_iovec[0].iov_base);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_direct_pwritev,
	.min_kver = "2.6.30",
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
};
