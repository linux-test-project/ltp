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
 *   1. The clockid argument is neither CLOCK_MONOTONIC nor CLOCK_REALTIME,
 *	EINVAL would return.
 *   2. flags is invalid, EINVAL would return.
 */

#define _GNU_SOURCE

#include <errno.h>

#include "test.h"
#include "lapi/timerfd.h"

char *TCID = "timerfd_create01";

static struct test_case_t {
	int clockid;
	int flags;
	int exp_errno;
} test_cases[] = {
	{-1, 0, EINVAL},
	{0, -1, EINVAL},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static void setup(void);
static void timerfd_create_verify(const struct test_case_t *);
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
			timerfd_create_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void timerfd_create_verify(const struct test_case_t *test)
{
	TEST(timerfd_create(test->clockid, test->flags));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "timerfd_create() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			 "timerfd_create() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "timerfd_create() failed unexpectedly; expected: "
			 "%d - %s", test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
}
