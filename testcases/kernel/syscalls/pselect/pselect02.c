/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
/*
 * Test Description:
 *  Verify that,
 *   1. pselect() fails with -1 return value and sets errno to EBADF
 *      if a file descriptor that was already closed.
 *   2. pselect() fails with -1 return value and sets errno to EINVAL
 *      if nfds was negative.
 *   3. pselect() fails with -1 return value and sets errno to EINVAL
 *      if the value contained within timeout was invalid.
 */

#include <errno.h>
#include "test.h"
#include "safe_macros.h"

TCID_DEFINE(pselect02);

static fd_set read_fds;
static struct timespec time_buf;

static struct test_case_t {
	int nfds;
	fd_set *readfds;
	struct timespec *timeout;
	int exp_errno;
} test_cases[] = {
	{128, &read_fds, NULL, EBADF},
	{-1, NULL, NULL, EINVAL},
	{128, NULL, &time_buf, EINVAL},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void cleanup(void);
static void pselect_verify(const struct test_case_t *);

int main(int argc, char **argv)
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			pselect_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, "test_file", O_RDWR | O_CREAT, 0777);

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	SAFE_CLOSE(cleanup, fd);

	time_buf.tv_sec = -1;
	time_buf.tv_nsec = 0;
}

static void pselect_verify(const struct test_case_t *test)
{
	TEST(pselect(test->nfds, test->readfds, NULL, NULL, test->timeout,
	     NULL));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "pselect() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "pselect() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "pselect() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
