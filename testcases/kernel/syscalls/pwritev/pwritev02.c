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
* Test Name: pwritev02
*
* Description:
* 1) pwritev(2) fails if iov_len is invalid.
* 2) pwritev(2) fails if the vector count iovcnt is less than zero.
* 3) pwritev(2) fails if offset is negative.
*
* Expected Result:
* 1) pwritev(2) should return -1 and set errno to EINVAL.
* 2) pwritev(2) should return -1 and set errno to EINVAL.
* 3) pwritev(2) should return -1 and set errno to EINVAL.
*/

#include <errno.h>

#include "test.h"
#include "pwritev.h"
#include "safe_macros.h"

#define CHUNK           64

static int fd;
static char buf[CHUNK];

static struct iovec wr_iovec1[] = {
	{buf, -1},
};

static struct iovec wr_iovec2[] = {
	{buf, CHUNK},
};

static struct test_case_t {
	struct iovec *name;
	int count;
	off_t offset;
} tc[] = {
	/* test1 */
	{wr_iovec1, 1, 0},
	/* test2 */
	{wr_iovec2, -1, 0},
	/* test3 */
	{wr_iovec2, 1, -1}
};

void verify_pwritev(struct test_case_t *tc);
void setup(void);
void cleanup(void);

char *TCID = "pwritev02";
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
			verify_pwritev(&tc[i]);
	}

	cleanup();
	tst_exit();
}

void verify_pwritev(struct test_case_t *tc)
{
	TEST(pwritev(fd, tc->name, tc->count, tc->offset));
	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "pwritev(2) succeed unexpectedly");
	} else {
		if (TEST_ERRNO == EINVAL) {
			tst_resm(TPASS | TTERRNO, "pwritev(2) fails as expected");
		} else {
			tst_resm(TFAIL | TTERRNO, "pwritev(2) fails unexpectedly,"
				 " expected errno is EINVAL");
		}
	}
}

void setup(void)
{
	if ((tst_kvercmp(2, 6, 30)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels"
			 " that are 2.6.30 or higher.");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, "file", O_RDWR | O_CREAT, 0644);
}

void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "failed to close file");

	tst_rmdir();
}
