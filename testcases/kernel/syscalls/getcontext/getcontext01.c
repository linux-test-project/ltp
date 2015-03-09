/*
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *  Author: Prashant P Yendigeri <prashant.yendigeri@wipro.com>
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

#include <features.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ucontext.h>

#include "test.h"

char *TCID = "getcontext01";

#if !defined(__UCLIBC__)

static void setup(void);
static void cleanup(void);

int TST_TOTAL = 1;

static void test_getcontext(void)
{
	ucontext_t ptr;

	TEST(getcontext(&ptr));

	if (TEST_RETURN == -1) {
		if (errno == ENOSYS)
			tst_resm(TCONF, "getcontext not implemented in libc");
		else
			tst_resm(TFAIL | TTERRNO, "getcontext failed");
	} else if (TEST_RETURN == 0) {
		tst_resm(TPASS, "getcontext passed");
	} else {
		tst_resm(TFAIL, "Unexpected return value %li", TEST_RETURN);
	}
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		test_getcontext();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

#else /* systems that dont support obsolete getcontext */
int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have getcontext support");
}
#endif
