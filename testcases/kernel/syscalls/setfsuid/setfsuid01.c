/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Ported by Wayne Boyer
 *
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
 * Testcase to test the basic functionality of the setfsuid(2) system call.
 */

#include <sys/types.h>
#include <sys/fsuid.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "compat_16.h"

static void setup(void);
static void cleanup(void);

TCID_DEFINE(setfsuid01);
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;

	uid_t uid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	uid = geteuid();
	UID16_CHECK(uid, setfsuid, cleanup);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(SETFSUID(cleanup, uid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				"setfsuid() failed unexpectedly");
			continue;
		}

		if (TEST_RETURN != uid) {
			tst_resm(TFAIL, "setfsuid() returned %ld, expected %d",
				 TEST_RETURN, uid);
		} else {
			tst_resm(TPASS,
				"setfsuid() returned expected value : %ld",
				TEST_RETURN);
		}
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
