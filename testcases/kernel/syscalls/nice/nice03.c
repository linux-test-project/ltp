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
 *  Verify that any user can successfully increase the nice value of
 *  the process by passing an increment value (< max. applicable limits) to
 *  nice() system call.
 */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include "tst_test.h"

#define	NICEINC	2

static void nice_test(void)
{
	int new_nice;
	int orig_nice;

	orig_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	TEST(nice(NICEINC));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "nice(%d) returned -1", NICEINC);
		return;
	}

	if (TEST_ERRNO) {
		tst_res(TFAIL | TTERRNO, "nice(%d) failed", NICEINC);
		return;
	}

	new_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	if (new_nice != (orig_nice + NICEINC)) {
		tst_res(TFAIL, "Process priority %i, expected %i",
		        new_nice, orig_nice + NICEINC);
		return;
	}

	tst_res(TPASS, "nice(%d) passed", NICEINC);

	exit(0);
}

static void verify_nice(void)
{
	if (!SAFE_FORK())
		nice_test();

	tst_reap_children();
}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = verify_nice,
};
