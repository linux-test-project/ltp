/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
 /*
  * futex_wake() returns 0 (0 woken up processes) when no processes wait on the mutex.
  */

#include <errno.h>
#include <limits.h>

#include "test.h"
#include "futextest.h"

const char *TCID="futex_wake01";

struct testcase {
	futex_t *f_addr;
	int nr_wake;
	int opflags;
};

static futex_t futex = FUTEX_INITIALIZER;

static struct testcase testcases[] = {
	/* nr_wake = 0 is noop */
	{&futex, 0, 0},
	{&futex, 0, FUTEX_PRIVATE_FLAG},
	{&futex, 1, 0},
	{&futex, 1, FUTEX_PRIVATE_FLAG},
	{&futex, INT_MAX, 0},
	{&futex, INT_MAX, FUTEX_PRIVATE_FLAG},
};

const int TST_TOTAL=ARRAY_SIZE(testcases);

static void verify_futex_wake(struct testcase *tc)
{
	int res;

	res = futex_wake(tc->f_addr, tc->nr_wake, tc->opflags);

	if (res != 0) {
		tst_resm(TFAIL, "futex_wake() returned %i, expected 0", res);
		return;
	}

	tst_resm(TPASS, "futex_wake() returned 0");
}

int main(int argc, char *argv[])
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			verify_futex_wake(testcases + i);
	}

	tst_exit();
}
