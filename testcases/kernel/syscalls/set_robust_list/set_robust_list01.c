/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: set_robust_list01
 *
 * Test Description:
 *  Verify that set_robust_list() returns the proper errno for various failure
 *  cases
 *
 * Usage:  <for command-line>
 *  set_robust_list01 [-c n] [-e][-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2008 Ramon de Carvalho Valle <rcvalle@br.ibm.com>
 *		-Created
 *
 * Restrictions:
 *  None.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/syscall.h>
#include <sys/types.h>

#ifdef __NR_set_robust_list
#ifndef __user
#define __user
#endif

struct robust_list {
	struct robust_list __user *next;
};

struct robust_list_head {
	struct robust_list list;
	long futex_offset;
	struct robust_list __user *list_op_pending;
};
#endif

#include "test.h"

char *TCID = "set_robust_list01";
int TST_TOTAL = 2;

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
#ifdef __NR_set_robust_list
	int lc;
#endif
#ifdef __NR_set_robust_list
	struct robust_list_head head;
	size_t len;		/* size of structure struct robust_list_head */
	int retval;
#endif

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

#ifdef __NR_set_robust_list

	len = sizeof(struct robust_list_head);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		/*
		 * The set_robust_list function fails with EINVAL if the len argument
		 * doesn't match the size of structure struct robust_list_head.
		 */

		TEST(retval = syscall(__NR_set_robust_list, &head, -1));

		if (TEST_RETURN) {
			if (TEST_ERRNO == EINVAL)
				tst_resm(TPASS,
					 "set_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EINVAL);
			else
				tst_resm(TFAIL,
					 "set_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EINVAL);
		} else {
			tst_resm(TFAIL,
				 "set_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, -1,
				 TEST_ERRNO, EINVAL);
		}

		/*
		 * This call to set_robust_list function should be sucessful.
		 */

		TEST(retval = syscall(__NR_set_robust_list, &head, len));

		if (TEST_RETURN == 0) {
			tst_resm(TPASS,
				 "set_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, 0,
				 TEST_ERRNO, 0);
		} else {
			tst_resm(TFAIL,
				 "set_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, 0,
				 TEST_ERRNO, 0);
		}

	}

#else

	tst_resm(TCONF, "set_robust_list: system call not available.");

#endif

	cleanup();

	exit(EXIT_SUCCESS);
}

void setup(void)
{
	TEST_PAUSE;
}

void cleanup(void)
{
}
