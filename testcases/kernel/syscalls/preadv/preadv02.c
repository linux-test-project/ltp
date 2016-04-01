/*
* Copyright (c) 2015-2016 Fujitsu Ltd.
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
* Test Name: preadv02
*
* Description:
* 1) preadv(2) fails if iov_len is invalid.
* 2) preadv(2) fails if the vector count iovcnt is less than zero.
* 3) preadv(2) fails if offset is negative.
* 4) preadv(2) fails when attempts to read into a invalid address.
* 5) preadv(2) fails if file descriptor is invalid.
* 6) preadv(2) fails if file descriptor is not open for reading.
*
* Expected Result:
* 1) preadv(2) should return -1 and set errno to EINVAL.
* 2) preadv(2) should return -1 and set errno to EINVAL.
* 3) preadv(2) should return -1 and set errno to EINVAL.
* 4) preadv(2) should return -1 and set errno to EFAULT.
* 5) preadv(2) should return -1 and set errno to EBADF.
* 6) preadv(2) should return -1 and set errno to EBADF.
*/

#include <errno.h>
#include <sys/uio.h>

#include "test.h"
#include "preadv.h"
#include "safe_macros.h"

#define CHUNK           64

static int fd1;
static int fd2;
static int fd3 = -1;
static char buf[CHUNK];

static struct iovec rd_iovec1[] = {
	{buf, -1},
};

static struct iovec rd_iovec2[] = {
	{buf, CHUNK},
	{(char *)-1, CHUNK},
};

static struct test_case_t {
	int *fd;
	struct iovec *name;
	int count;
	off_t offset;
	int exp_err;
} tc[] = {
	/* test1 */
	{&fd1, rd_iovec1, 1, 0, EINVAL},
	/* test2 */
	{&fd1, rd_iovec2, -1, 0, EINVAL},
	/* test3 */
	{&fd1, rd_iovec2, 1, -1, EINVAL},
	/* test4 */
	{&fd1, rd_iovec2, 2, 0, EFAULT},
	/* test5 */
	{&fd3, rd_iovec2, 1, 0, EBADF},
	/* test6 */
	{&fd2, rd_iovec2, 1, 0, EBADF}
};

static void verify_preadv(struct test_case_t *tc);
static void setup(void);
static void cleanup(void);

char *TCID = "preadv02";
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

static void verify_preadv(struct test_case_t *tc)
{
	TEST(preadv(*tc->fd, tc->name, tc->count, tc->offset));
	if (TEST_RETURN == 0) {
		tst_resm(TFAIL, "preadv() succeeded unexpectedly");
	} else {
		if (TEST_ERRNO == tc->exp_err) {
			tst_resm(TPASS | TTERRNO,
				 "preadv() failed as expected");
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "preadv() failed unexpectedly, expected"
				 " errno is %s", tst_strerrno(tc->exp_err));
		}
	}
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 30)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels"
			 " that are 2.6.30 or higher.");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd1 = SAFE_OPEN(cleanup, "file1", O_RDWR | O_CREAT, 0644);

	fd2 = SAFE_OPEN(cleanup, "file2", O_WRONLY | O_CREAT, 0644);
}

static void cleanup(void)
{
	if (fd1 > 0 && close(fd1))
		tst_resm(TWARN | TERRNO, "failed to close file");

	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "failed to close file");

	tst_rmdir();
}
