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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/syscall.h>
#include <sys/types.h>

#include "test.h"
#include "usctest.h"

char *TCID = "get_robust_list01";	/* test program identifier.              */
int TST_TOTAL = 5;		/* total number of tests in this file.   */

#ifdef __NR_get_robust_list

struct robust_list {
	struct robust_list *next;
};

struct robust_list_head {
	struct robust_list list;
	long futex_offset;
	struct robust_list *list_op_pending;
};

extern int Tst_count;		/* counter for tst_xxx routines.         */

int exp_enos[] = { ESRCH, EPERM, EFAULT, 0 };

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct robust_list_head head;
	size_t len_ptr;		/* size of structure struct robust_list_head */
	int retval;

	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	len_ptr = sizeof(struct robust_list_head);

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		Tst_count = 0;

		/*
		 * The get_robust_list function fails with EFAULT if the size of the
		 * struct robust_list_head can't be stored in the memory address space
		 * specified by len_ptr argument, or the head of the robust list can't
		 * be stored in the memory address space specified by the head_ptr
		 * argument.
		 */

		TEST(retval = syscall(__NR_get_robust_list, 0,
				      (struct robust_list_head *)&head,
				      (size_t *) NULL));

		if (TEST_RETURN) {
			if (TEST_ERRNO == EFAULT)
				tst_resm(TPASS,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EFAULT);
			else
				tst_resm(TFAIL,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EFAULT);
		} else {
			tst_resm(TFAIL,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, -1,
				 TEST_ERRNO, EFAULT);
		}

		TEST(retval = syscall(__NR_get_robust_list, 0,
				      (struct robust_list_head **)NULL,
				      &len_ptr));

		if (TEST_RETURN) {
			if (TEST_ERRNO == EFAULT)
				tst_resm(TPASS,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EFAULT);
			else
				tst_resm(TFAIL,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EFAULT);
		} else {
			tst_resm(TFAIL,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, -1,
				 TEST_ERRNO, EFAULT);
		}

		/*
		 * The get_robust_list function fails with ESRCH if it can't find the
		 * task specified by the pid argument. The value 65535 is used as the
		 * pid argument.
		 */

		TEST(retval = syscall(__NR_get_robust_list, 65535,
				      (struct robust_list_head *)&head,
				      &len_ptr));

		if (TEST_RETURN) {
			if (TEST_ERRNO == ESRCH)
				tst_resm(TPASS,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, ESRCH);
			else
				tst_resm(TFAIL,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, ESRCH);
		} else {
			tst_resm(TFAIL,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, -1,
				 TEST_ERRNO, ESRCH);
		}

		/*
		 * The get_robust_list function fails with EPERM if it has no
		 * permission to access the task specified by the pid argument.
		 * The current user id of the process is changed to 1 (bin), and the
		 * value of 1 (init) is used as the pid argument.
		 */

		/*
		 * Temporarily drop root privleges.
		 */
		seteuid(1);

		TEST(retval = syscall(__NR_get_robust_list, 1,
				      (struct robust_list_head *)&head,
				      &len_ptr));

		if (TEST_RETURN) {
			if (TEST_ERRNO == EPERM)
				tst_resm(TPASS,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EPERM);
			else
				tst_resm(TFAIL,
					 "get_robust_list: retval = %ld (expected %d), "
					 "errno = %d (expected %d)",
					 TEST_RETURN, -1, TEST_ERRNO, EPERM);
		} else {
			tst_resm(TFAIL,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, -1,
				 TEST_ERRNO, EPERM);
		}

		/*
		 * Regain root privileges.
		 */
		seteuid(0);

		/*
		 * This call to get_robust_list function should be sucessful.
		 */

		TEST(retval = syscall(__NR_get_robust_list, 0,
				      (struct robust_list_head **)&head,
				      &len_ptr));

		if (TEST_RETURN == 0) {
			tst_resm(TPASS,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, 0,
				 TEST_ERRNO, 0);
		} else {
			tst_resm(TFAIL,
				 "get_robust_list: retval = %ld (expected %d), "
				 "errno = %d (expected %d)", TEST_RETURN, 0,
				 TEST_ERRNO, 0);
		}

	}

	cleanup();

	exit(EXIT_SUCCESS);
}

void setup(void)
{
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

void cleanup(void)
{
	TEST_CLEANUP;

	tst_exit();
}

#else

int main()
{
	tst_resm(TCONF, "get_robust_list: system call not available");
	tst_exit();
}

#endif
