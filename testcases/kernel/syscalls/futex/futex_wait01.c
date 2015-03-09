/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Based on futextest (futext_wait_timeout.c and futex_wait_ewouldblock.c)
 * written by Darren Hart <dvhltc@us.ibm.com>
 *            Gowrishankar <gowrishankar.m@in.ibm.com>
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
  * 1. Block on a futex and wait for timeout.
  * 2. Test if FUTEX_WAIT op returns -EWOULDBLOCK if the futex value differs
  *    from the expected one.
  */

#include <errno.h>

#include "test.h"
#include "futextest.h"

const char *TCID="futex_wait01";

struct testcase {
	futex_t *f_addr;
	futex_t f_val;
	int opflags;
	int exp_errno;
};

static futex_t futex = FUTEX_INITIALIZER;
static struct timespec to = {.tv_sec = 0, .tv_nsec = 10000};

static struct testcase testcases[] = {
	{&futex, FUTEX_INITIALIZER, 0, ETIMEDOUT},
	{&futex, FUTEX_INITIALIZER+1, 0, EWOULDBLOCK},
	{&futex, FUTEX_INITIALIZER, FUTEX_PRIVATE_FLAG, ETIMEDOUT},
	{&futex, FUTEX_INITIALIZER+1, FUTEX_PRIVATE_FLAG, EWOULDBLOCK},
};

const int TST_TOTAL=ARRAY_SIZE(testcases);

static void verify_futex_wait(struct testcase *tc)
{
	int res;

	res = futex_wait(tc->f_addr, tc->f_val, &to, tc->opflags);

	if (res != -1) {
		tst_resm(TFAIL, "futex_wait() returned %i, expected -1", res);
		return;
	}

	if (errno != tc->exp_errno) {
		tst_resm(TFAIL | TERRNO, "expected errno=%s",
		         tst_strerrno(tc->exp_errno));
		return;
	}

	tst_resm(TPASS | TERRNO, "futex_wait()");
}

int main(int argc, char *argv[])
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			verify_futex_wait(testcases + i);
	}

	tst_exit();
}
