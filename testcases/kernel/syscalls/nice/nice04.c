/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *  Verify that, nice(2) fails when, a non-root user attempts to increase
 *  the priority of a process by specifying a negative increment value.
 */
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include "tst_test.h"

#define NICEINC -10

static void verify_nice(void)
{
	TEST(nice(NICEINC));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "nice(%i) succeded unexpectedly (returned %li)",
			NICEINC, TEST_RETURN);
		return;
	}

	if (TEST_ERRNO != EPERM) {
		tst_res(TFAIL | TTERRNO, "nice(%i) should fail with EPERM",
			NICEINC);
		return;
	}

	tst_res(TPASS, "nice(%i) failed with EPERM", NICEINC);
}

static void setup(void)
{
	struct passwd *ltpuser;

	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETUID(ltpuser->pw_uid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_nice,
	.needs_root = 1,
};
