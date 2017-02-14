/*
* Copyright (c) 2015 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
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
* Test Name: preadv01
*
* Test Description:
* Testcase to check the basic functionality of the preadv(2).
* Preadv(2) should succeed to read the expected content of data
* and after reading the file, the file offset is not changed.
*/

#include <string.h>
#include <sys/uio.h>

#include "tst_test.h"
#include "preadv.h"

#define CHUNK           64

static int fd;
static char buf[CHUNK];

static struct iovec rd_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct tcase {
	int count;
	off_t offset;
	ssize_t size;
	char content;
} tcases[] = {
	{1, 0, CHUNK, 'a'},
	{2, 0, CHUNK, 'a'},
	{1, CHUNK*3/2, CHUNK/2, 'b'}
};

void verify_preadv(unsigned int n)
{
	int i;
	char *vec;
	struct tcase *tc = &tcases[n];

	vec = rd_iovec[0].iov_base;
	memset(vec, 0x00, CHUNK);

	SAFE_LSEEK(fd, 0, SEEK_SET);

	TEST(preadv(fd, rd_iovec, tc->count, tc->offset));
	if (TEST_RETURN < 0) {
		tst_res(TFAIL | TTERRNO, "Preadv(2) failed");
		return;
	}

	if (TEST_RETURN != tc->size) {
		tst_res(TFAIL, "Preadv(2) read %li bytes, expected %li",
			 TEST_RETURN, tc->size);
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

	if (SAFE_LSEEK(fd, 0, SEEK_CUR) != 0) {
		tst_res(TFAIL, "Preadv(2) has changed file offset");
		return;
	}

	tst_res(TPASS, "Preadv(2) read %li bytes successfully "
		 "with content '%c' expectedly", tc->size, tc->content);
}

void setup(void)
{
	char buf[CHUNK];

	fd = SAFE_OPEN("file", O_RDWR | O_CREAT, 0644);

	memset(buf, 'a', sizeof(buf));
	SAFE_WRITE(1, fd, buf, sizeof(buf));

	memset(buf, 'b', sizeof(buf));
	SAFE_WRITE(1, fd, buf, sizeof(buf));
}

void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tid = "preadv01",
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_preadv,
	.min_kver = "2.6.30",
	.needs_tmpdir = 1,
};
