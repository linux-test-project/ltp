// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Description:
 * Check the basic functionality of the preadv(2) for the file
 * opened with O_DIRECT in all filesystem.
 * preadv(2) should succeed to read the expected content of data
 * and after reading the file, the file offset is not changed.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include "tst_test.h"
#include "preadv.h"

#define MNTPOINT	"mntpoint"
#define FNAME	MNTPOINT"/file"

static int fd;
static off_t blk_off, zero_off;
static ssize_t blksz;
static char *pop_buf;

static struct iovec rd_iovec[] = {
	{NULL, 0},
	{NULL, 0},
};

static struct tcase {
	int count;
	off_t *offset;
	ssize_t *size;
	char content;
} tcases[] = {
	{1, &zero_off, &blksz, 0x61},
	{2, &zero_off, &blksz, 0x61},
	{1, &blk_off, &blksz, 0x62},
};

static void verify_direct_preadv(unsigned int n)
{
	int i;
	char *vec;
	struct tcase *tc = &tcases[n];

	vec = rd_iovec[0].iov_base;
	memset(vec, 0x00, blksz);

	SAFE_LSEEK(fd, 0, SEEK_SET);

	TEST(preadv(fd, rd_iovec, tc->count, *tc->offset));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "preadv(O_DIRECT) fails");
		return;
	}

	if (TST_RET != *tc->size) {
		tst_res(TFAIL, "preadv(O_DIRECT) read %li bytes, expected %zi",
			 TST_RET, *tc->size);
		return;
	}

	for (i = 0; i < *tc->size; i++) {
		if (vec[i] != tc->content)
			break;
	}

	if (i < *tc->size) {
		tst_res(TFAIL, "Buffer wrong at %i have %02x expected %02x",
			 i, vec[i], tc->content);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != 0) {
		tst_res(TFAIL, "preadv(O_DIRECT) has changed file offset");
		return;
	}

	tst_res(TPASS, "preadv(O_DIRECT) read %zi bytes successfully "
		 "with content '%c' expectedly", *tc->size, tc->content);
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

	pop_buf = SAFE_MEMALIGN(blksz, blksz);
	memset(pop_buf, 0x61, blksz);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, pop_buf, blksz);

	memset(pop_buf, 0x62, blksz);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, pop_buf, blksz);

	rd_iovec[0].iov_base = SAFE_MEMALIGN(blksz, blksz);
	rd_iovec[0].iov_len = blksz;
}

static void cleanup(void)
{
	free(pop_buf);
	free(rd_iovec[0].iov_base);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_direct_preadv,
	.mntpoint = MNTPOINT,
	.mount_device = 1,
	.all_filesystems = 1,
	.skip_filesystems = (const char *[]) {
		"tmpfs",
		NULL
	}
};
