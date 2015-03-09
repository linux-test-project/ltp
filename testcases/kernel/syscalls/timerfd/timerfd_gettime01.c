/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * DESCRIPTION
 *  Verify that,
 *   1. fd is not a valid file descriptor, EBADF would return.
 *   2. curr_value is not valid a pointer, EFAULT would return.
 *   3. fd is not a valid timerfd file descriptor, EINVAL would return.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/timerfd.h"

char *TCID = "timerfd_gettime01";

static int bad_clockfd = -1;
static int clockfd;
static int fd;

static struct test_case_t {
	int *fd;
	struct itimerspec *curr_value;
	int exp_errno;
} test_cases[] = {
	{&bad_clockfd, NULL, EBADF},
	{&clockfd, (struct itimerspec *)-1, EFAULT},
	{&fd, NULL, EINVAL},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static void setup(void);
static void timerfd_gettime_verify(const struct test_case_t *);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			timerfd_gettime_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 25)) < 0)
		tst_brkm(TCONF, NULL, "This test needs kernel 2.6.25 or newer");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	clockfd = timerfd_create(CLOCK_REALTIME, 0);
	if (clockfd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "timerfd_create() fail");

	fd = SAFE_OPEN(cleanup, "test_file", O_RDWR | O_CREAT, 0644);
}

static void timerfd_gettime_verify(const struct test_case_t *test)
{
	TEST(timerfd_gettime(*test->fd, test->curr_value));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "timerfd_gettime() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			 "timerfd_gettime() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "timerfd_gettime() failed unexpectedly; expected: "
			 "%d - %s", test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	if (clockfd > 0)
		close(clockfd);

	if (fd > 0)
		close(fd);

	tst_rmdir();
}
