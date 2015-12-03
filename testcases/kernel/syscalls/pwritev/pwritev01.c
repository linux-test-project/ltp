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
* Test Name: pwritev01
*
* Test Description:
* Testcase to check the basic functionality of the pwritev(2).
* pwritev(2) should succeed to write the expected content of data
* and after writing the file, the file offset is not changed.
*/

#include <errno.h>

#include "test.h"
#include "pwritev.h"
#include "safe_macros.h"

#define	CHUNK		64

static char buf[CHUNK];
static char initbuf[CHUNK * 2];
static char preadbuf[CHUNK];
static int fd;

static struct iovec wr_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
};

static struct test_case_t {
	int count;
	off_t offset;
	ssize_t size;
} tc[] = {
	{1, 0, CHUNK},
	{2, 0, CHUNK},
	{1, CHUNK/2, CHUNK},
};

void verify_pwritev(struct test_case_t *);
void setup(void);
void cleanup(void);

char *TCID = "pwritev01";
int TST_TOTAL =  ARRAY_SIZE(tc);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify_pwritev(&tc[i]);
	}
	cleanup();
	tst_exit();
}

void verify_pwritev(struct test_case_t *tc)
{
	int i;

	SAFE_PWRITE(cleanup, 1, fd, initbuf, sizeof(initbuf), 0);

	SAFE_LSEEK(cleanup, fd, 0, SEEK_SET);

	TEST(pwritev(fd, wr_iovec, tc->count, tc->offset));
	if (TEST_RETURN < 0) {
		tst_resm(TFAIL | TTERRNO, "Pwritev(2) failed");
		return;
	}

	if (TEST_RETURN != tc->size) {
		tst_resm(TFAIL, "Pwritev(2) write %li bytes, expected %li",
			 TEST_RETURN, tc->size);
		return;
	}

	if (SAFE_LSEEK(cleanup, fd, 0, SEEK_CUR) != 0) {
		tst_resm(TFAIL, "Pwritev(2) has changed file offset");
		return;
	}

	SAFE_PREAD(cleanup, 1, fd, preadbuf, tc->size, tc->offset);

	for (i = 0; i < tc->size; i++) {
		if (preadbuf[i] != 0x61)
			break;
	}

	if (i != tc->size) {
		tst_resm(TFAIL, "Buffer wrong at %i have %02x expected 61",
			 i, preadbuf[i]);
		return;
	}

	tst_resm(TPASS, "Pwritev(2) write %li bytes successfully "
		 "with content 'a' expectedly ", tc->size);
}

void setup(void)
{
	if ((tst_kvercmp(2, 6, 30)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			"that are 2.6.30 and higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	memset(&buf, 0x61, CHUNK);

	fd = SAFE_OPEN(cleanup, "file", O_RDWR | O_CREAT, 0644);
}

void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close file");

	tst_rmdir();
}
