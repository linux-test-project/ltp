// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify the basic functionality of the preadv2(2):
 *
 * 1. If the file offset argument is not -1, preadv2() should succeed
 * in reading the expected content of data and the file offset is not
 * changed after reading.
 * 2. If the file offset argument is -1, preadv2() should succeed in
 * reading the expected content of data and the current file offset
 * is used and changed after reading.
 */

#define _GNU_SOURCE
#include <string.h>
#include <sys/uio.h>

#include "tst_test.h"
#include "lapi/uio.h"

#define CHUNK           64

static int fd;
static char buf[CHUNK];

static struct iovec rd_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct tcase {
	off_t seek_off;
	int count;
	off_t read_off;
	ssize_t size;
	char content;
	off_t exp_off;
} tcases[] = {
	{0,     1, 0,           CHUNK,     'a', 0},
	{CHUNK, 2, 0,           CHUNK,     'a', CHUNK},
	{0,     1, CHUNK*3 / 2, CHUNK / 2, 'b', 0},
	{0,     1, -1,          CHUNK,     'a', CHUNK},
	{0,     2, -1,          CHUNK,     'a', CHUNK},
	{CHUNK, 1, -1,          CHUNK,     'b', CHUNK * 2},
};

static void verify_preadv2(unsigned int n)
{
	int i;
	char *vec;
	struct tcase *tc = &tcases[n];

	vec = rd_iovec[0].iov_base;
	memset(vec, 0x00, CHUNK);

	SAFE_LSEEK(fd, tc->seek_off, SEEK_SET);

	TEST(preadv2(fd, rd_iovec, tc->count, tc->read_off, 0));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "preadv2() failed");
		return;
	}

	if (TST_RET != tc->size) {
		tst_res(TFAIL, "preadv2() read %li bytes, expected %zi",
			 TST_RET, tc->size);
		return;
	}

	for (i = 0; i < tc->size; i++) {
		if (vec[i] != tc->content)
			break;
	}

	if (i < tc->size) {
		tst_res(TFAIL, "Buffer wrong at %i have %02x expected %02x",
			 i, vec[i], tc->content);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != tc->exp_off) {
		tst_res(TFAIL, "preadv2() has changed file offset");
		return;
	}

	tst_res(TPASS, "preadv2() read %zi bytes with content '%c' expectedly",
		tc->size, tc->content);
}

static void setup(void)
{
	char buf[CHUNK];

	fd = SAFE_OPEN("file", O_RDWR | O_CREAT, 0644);

	memset(buf, 'a', sizeof(buf));
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, sizeof(buf));

	memset(buf, 'b', sizeof(buf));
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, sizeof(buf));
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_preadv2,
	.needs_tmpdir = 1,
};
