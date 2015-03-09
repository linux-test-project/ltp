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
 * Test Description:
 *  Verify that,
 *	The calling process does not have any unwaited-for children,
 *	ECHILD would return.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"

char *TCID = "wait01";
int TST_TOTAL = 1;
static void setup(void);
static void wait_verify(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		wait_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void wait_verify(void)
{
	TEST(wait(NULL));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL | TTERRNO, "wait failed unexpectedly: %ld",
			TEST_RETURN);
		return;
	}

	if (TEST_ERRNO == ECHILD) {
		tst_resm(TPASS | TTERRNO, "wait failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "wait failed unexpectedly; expected: %d - %s",
			 ECHILD, strerror(ECHILD));
	}
}

static void cleanup(void)
{
}
