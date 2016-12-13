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
 * NAME
 * 	getpgid02.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of getpgid().
 *
 * ALGORITHM
 * 	test 1: Does getpgid(-99) and expects ESRCH.
 * 	test 2: Searches an unused pid and expects ESRCH.
 *
 * USAGE:  <for command-line>
 *  getpgid02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */
#define _GNU_SOURCE 1

#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "test.h"

void setup(void);
void cleanup(void);

char *TCID = "getpgid02";
int TST_TOTAL = 2;

int pgid_0, pgid_1;
#define BADPID -99

struct test_case_t {
	int *id;
	int error;
} TC[] = {
	/* The pid value is negative */
	{
	&pgid_0, ESRCH},
	    /* The pid value does not match any process */
	{
	&pgid_1, ESRCH}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(getpgid(*TC[i].id));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	pgid_0 = BADPID;

	pgid_1 = tst_get_unused_pid(cleanup);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}
