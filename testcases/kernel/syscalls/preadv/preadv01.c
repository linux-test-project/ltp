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

#include <errno.h>

#include "test.h"
#include "preadv.h"
#include "safe_macros.h"

#define CHUNK           64

static int fd;
static char buf[CHUNK];

static struct iovec rd_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct test_case_t {
	int count;
	off_t offset;
	ssize_t size;
	char content;
} tc[] = {
	{1, 0, CHUNK, 'a'},
	{2, 0, CHUNK, 'a'},
	{1, CHUNK*3/2, CHUNK/2, 'b'}
};

void verify_preadv(struct test_case_t *tc);
void setup(void);
void cleanup(void);

char *TCID = "preadv01";
int TST_TOTAL = ARRAY_SIZE(tc);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify_preadv(&tc[i]);
	}

	cleanup();
	tst_exit();
}

void verify_preadv(struct test_case_t *tc)
{
	int i;
	char *vec;

	vec = rd_iovec[0].iov_base;
	memset(vec, 0x00, CHUNK);

	SAFE_LSEEK(cleanup, fd, 0, SEEK_SET);

	TEST(preadv(fd, rd_iovec, tc->count, tc->offset));
	if (TEST_RETURN < 0) {
		tst_resm(TFAIL | TTERRNO, "Preadv(2) failed");
		return;
	}

	if (TEST_RETURN != tc->size) {
		tst_resm(TFAIL, "Preadv(2) read %li bytes, expected %li",
			 TEST_RETURN, tc->size);
		return;
	}

	for (i = 0; i < tc->size; i++) {
		if (vec[i] != tc->content)
			break;
	}

	if (i < tc->size) {
		tst_resm(TFAIL, "Buffer wrong at %i have %02x expected %02x",
			 i, vec[i], tc->content);
		return;
	}

	if (SAFE_LSEEK(cleanup, fd, 0, SEEK_CUR) != 0) {
		tst_resm(TFAIL, "Preadv(2) has changed file offset");
		return;
	}

	tst_resm(TPASS, "Preadv(2) read %li bytes successfully "
		 "with content '%c' expectedly", tc->size, tc->content);
}

void setup(void)
{
	char buf[CHUNK];

	if ((tst_kvercmp(2, 6, 30)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			"that are 2.6.30 and higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, "file", O_RDWR | O_CREAT, 0644);

	memset(buf, 'a', sizeof(buf));
	SAFE_WRITE(cleanup, 1, fd, buf, sizeof(buf));

	memset(buf, 'b', sizeof(buf));
	SAFE_WRITE(cleanup, 1, fd, buf, sizeof(buf));
}

void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	tst_rmdir();
}
