/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *  AUTHOR		: William Roske
 *  CO-PILOT		: Dave Fenner
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/*
 * DESCRIPTION
 *	1.) test sbrk(8192) should return successfully.
 *	2.) test sbrk(-8192) should return successfully.
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "test.h"

char *TCID = "sbrk01";

static struct test_case_t {
	long increment;
} test_cases[] = {
	{8192},
	{-8192},
};

static void setup(void);
static void sbrk_verify(const struct test_case_t *);
static void cleanup(void);

int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			sbrk_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();

}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void sbrk_verify(const struct test_case_t *test)
{
	void *tret;

	tret = sbrk(test->increment);
	TEST_ERRNO = errno;

	if (tret == (void *)-1) {
		tst_resm(TFAIL | TTERRNO, "sbrk - Increase by %ld bytes failed",
			 test->increment);
	} else {
		tst_resm(TPASS, "sbrk - Increase by %ld bytes returned %p",
			 test->increment, tret);
	}
}

static void cleanup(void)
{
}
