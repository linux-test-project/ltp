// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Jinhui Huang <huangjh.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Testcase to check the basic functionality of the pwritev2(2).
 *
 * - If the file offset argument is not -1, pwritev2() should succeed
 *   in writing the expected content of data and the file offset is
 *   not changed after writing.
 * - If the file offset argument is -1, pwritev2() should succeed in
 *   writing the expected content of data and the current file offset
 *   is used and changed after writing.
 */

#define _GNU_SOURCE
#include <string.h>
#include <sys/uio.h>

#include "tst_test.h"
#include "lapi/uio.h"
#include "tst_safe_prw.h"

#define CHUNK	64

static int fd;
static char initbuf[CHUNK * 2];
static char buf[CHUNK];

static struct iovec wr_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct tcase {
	off_t seek_off;
	int count;
	off_t write_off;
	ssize_t size;
	off_t exp_off;
} tcases[] = {
	{0,     1, 0,          CHUNK, 0},
	{CHUNK, 2, 0,          CHUNK, CHUNK},
	{0,     1, CHUNK / 2,  CHUNK, 0},
	{0,     1, -1,         CHUNK, CHUNK},
	{0,     2, -1,         CHUNK, CHUNK},
	{CHUNK, 1, -1,         CHUNK, CHUNK * 2},
};

static void verify_pwritev2(unsigned int n)
{
	int i;
	char preadbuf[CHUNK];
	struct tcase *tc = &tcases[n];

	SAFE_PWRITE(1, fd, initbuf, sizeof(initbuf), 0);
	SAFE_LSEEK(fd, tc->seek_off, SEEK_SET);

	TEST(pwritev2(fd, wr_iovec, tc->count, tc->write_off, 0));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "pwritev2() failed");
		return;
	}

	if (TST_RET != tc->size) {
		tst_res(TFAIL, "pwritev2() wrote %li bytes, expected %zi",
			 TST_RET, tc->size);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != tc->exp_off) {
		tst_res(TFAIL, "pwritev2() had changed file offset");
		return;
	}

	memset(preadbuf, 0, CHUNK);

	if (tc->write_off != -1)
		SAFE_PREAD(1, fd, preadbuf, tc->size, tc->write_off);
	else
		SAFE_PREAD(1, fd, preadbuf, tc->size, tc->seek_off);

	for (i = 0; i < tc->size; i++) {
		if (preadbuf[i] != 0x61)
			break;
	}

	if (i != tc->size) {
		tst_res(TFAIL, "buffer wrong at %i have %c expected 'a'",
			 i, preadbuf[i]);
		return;
	}

	tst_res(TPASS, "pwritev2() wrote %zi bytes successfully "
		 "with content 'a' expectedly ", tc->size);
}

static void setup(void)
{
	memset(buf, 0x61, CHUNK);
	memset(initbuf, 0, CHUNK * 2);

	fd = SAFE_OPEN("file", O_RDWR | O_CREAT, 0644);
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
	.test = verify_pwritev2,
	.needs_tmpdir = 1,
};
