// SPDX-License-Identifier: GPL-2.0-or-later
/*
* Copyright (c) 2015 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*/

/*\
 * [Description]
 *
 * Testcase to check the basic functionality of the pwritev(2).
 *
 * pwritev(2) should succeed to write the expected content of data
 * and after writing the file, the file offset is not changed.
 */

#define _GNU_SOURCE
#include <string.h>
#include <sys/uio.h>
#include "tst_test.h"
#include "pwritev.h"
#include "tst_safe_prw.h"

#define	CHUNK		64

static char buf[CHUNK];
static char initbuf[CHUNK * 2];
static char preadbuf[CHUNK];
static int fd;

static struct iovec wr_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct tcase {
	int count;
	off_t offset;
	ssize_t size;
} tcases[] = {
	{1, 0, CHUNK},
	{2, 0, CHUNK},
	{1, CHUNK/2, CHUNK},
};

static void verify_pwritev(unsigned int n)
{
	int i;
	struct tcase *tc = &tcases[n];

	SAFE_PWRITE(1, fd, initbuf, sizeof(initbuf), 0);

	SAFE_LSEEK(fd, 0, SEEK_SET);

	TEST(pwritev(fd, wr_iovec, tc->count, tc->offset));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "pwritev() failed");
		return;
	}

	if (TST_RET != tc->size) {
		tst_res(TFAIL, "pwritev() wrote %li bytes, expected %zi",
			 TST_RET, tc->size);
		return;
	}

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != 0) {
		tst_res(TFAIL, "pwritev() had changed file offset");
		return;
	}

	SAFE_PREAD(1, fd, preadbuf, tc->size, tc->offset);

	for (i = 0; i < tc->size; i++) {
		if (preadbuf[i] != 0x61)
			break;
	}

	if (i != tc->size) {
		tst_res(TFAIL, "buffer wrong at %i have %02x expected 61",
			 i, preadbuf[i]);
		return;
	}

	tst_res(TPASS, "writev() wrote %zi bytes successfully "
		 "with content 'a' expectedly ", tc->size);
}

static void setup(void)
{
	memset(&buf, 0x61, CHUNK);

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
	.test = verify_pwritev,
	.needs_tmpdir = 1,
};
