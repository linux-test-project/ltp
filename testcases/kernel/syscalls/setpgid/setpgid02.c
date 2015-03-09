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
 * 	setpgid02.c
 *
 * DESCRIPTION
 *	Testcase to check that setpgid() sets errno correctly.
 *
 * CALLS
 * 	setpgid
 *
 * ALGORITHM
 * 	Checks that setpgid returns the correct errno values in case of
 * 	negative testing.
 * 	test 1: EINVAL - Pass '-1' as the pgid parameter to setpgid
 * 	test 2: ESRCH - Pass '-1' as the pid parameter to setpgid
 *	test 3: EPERM - Pass an invalid pgid parameter to setpgid
 *
 * USAGE:  <for command-line>
 *  setpgid02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 * 	None
 */
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "test.h"

static void setup(void);
static void cleanup(void);

char *TCID = "setpgid02";
int TST_TOTAL = 3;

static pid_t pgid, pid;
static pid_t bad_pid = -1;
static pid_t zero_pid;
static pid_t unused_pid;
static pid_t inval_pid = 99999;

struct test_case_t {
	pid_t *pid;
	pid_t *pgid;
	int error;
} TC[] = {
	/* pgid is less than zero - EINVAL */
	{
	&pid, &bad_pid, EINVAL},
	    /* pid doesn't match any process - ESRCH */
	{
	&unused_pid, &pgid, ESRCH},
	    /* pgid doesn't exist - EPERM */
	{
	&zero_pid, &inval_pid, EPERM}
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

			TEST(setpgid(*TC[i].pid, *TC[i].pgid));

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
 * setup - performs all ONE TIME setup for this test
 */
static void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	pgid = getpgrp();
	pid = getpid();

	unused_pid = tst_get_unused_pid(cleanup);
}

/*
 * cleanup - Performs all ONE TIME cleanup for this test at completion or
 * 	     premature exit
 */
static void cleanup(void)
{

}
