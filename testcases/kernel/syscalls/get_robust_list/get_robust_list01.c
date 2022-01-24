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
 * Test Name: get_robust_list01
 *
 * Test Description:
 *  Verify that get_robust_list() returns the proper errno for various failure
 *  cases
 *
 * Usage:  <for command-line>
 *  get_robust_list01 [-c n] [-e][-i n] [-I x] [-p x] [-t]
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

#include <sys/types.h>
#include <sys/syscall.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

char *TCID = "get_robust_list01";
int TST_TOTAL = 5;

struct robust_list {
	struct robust_list *next;
};

struct robust_list_head {
	struct robust_list list;
	long futex_offset;
	struct robust_list *list_op_pending;
};
static pid_t unused_pid;

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;
	struct robust_list_head head;
	size_t len_ptr;		/* size of structure struct robust_list_head */

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	len_ptr = sizeof(struct robust_list_head);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		/*
		 * The get_robust_list function fails with EFAULT if the size of the
		 * struct robust_list_head can't be stored in the memory address space
		 * specified by len_ptr argument, or the head of the robust list can't
		 * be stored in the memory address space specified by the head_ptr
		 * argument.
		 */

		TEST(tst_syscall(__NR_get_robust_list, 0,
				      (struct robust_list_head *)&head,
				      NULL));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EFAULT)
				tst_resm(TPASS,
					 "get_robust_list failed as expected with "
					 "EFAULT");
			else
				tst_resm(TFAIL | TTERRNO,
					 "get_robust_list failed unexpectedly");
		} else
			tst_resm(TFAIL,
				 "get_robust_list succeeded unexpectedly");

		TEST(tst_syscall(__NR_get_robust_list, 0,
				      NULL,
				      &len_ptr));

		if (TEST_RETURN) {
			if (TEST_ERRNO == EFAULT)
				tst_resm(TPASS,
					 "get_robust_list failed as expected with "
					 "EFAULT");
			else
				tst_resm(TFAIL | TTERRNO,
					 "get_robust_list failed unexpectedly");
		} else
			tst_resm(TFAIL,
				 "get_robust_list succeeded unexpectedly");

		/*
		 * The get_robust_list function fails with ESRCH if it can't
		 * find the task specified by the pid argument.
		 */

		TEST(tst_syscall(__NR_get_robust_list, unused_pid,
				      (struct robust_list_head *)&head,
				      &len_ptr));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == ESRCH)
				tst_resm(TPASS,
					 "get_robust_list failed as expected with "
					 "ESRCH");
			else
				tst_resm(TFAIL | TTERRNO,
					 "get_robust_list failed unexpectedly");
		} else
			tst_resm(TFAIL,
				 "get_robust_list succeeded unexpectedly");

		TEST(tst_syscall(__NR_get_robust_list, 0,
				      (struct robust_list_head **)&head,
				      &len_ptr));

		if (TEST_RETURN == 0)
			tst_resm(TPASS, "get_robust_list succeeded");
		else
			tst_resm(TFAIL | TTERRNO,
				 "get_robust_list failed unexpectedly");

		SAFE_SETUID(cleanup, 1);

		TEST(tst_syscall(__NR_get_robust_list, 1,
				      (struct robust_list_head *)&head,
				      &len_ptr));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EPERM)
				tst_resm(TPASS,
					 "get_robust_list failed as expected with "
					 "EPERM");
			else
				tst_resm(TFAIL | TERRNO,
					 "get_robust_list failed unexpectedly");
		} else
			tst_resm(TFAIL,
				 "get_robust_list succeeded unexpectedly");
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	tst_require_root();

	unused_pid = tst_get_unused_pid(cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
