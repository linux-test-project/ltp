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
 *	kill04.c
 *
 * DESCRIPTION
 *	Test case to check that kill() fails when passed a non-existant pid.
 *
 * ALGORITHM
 *	call setup
 *	loop if the -i option was given
 *	fork a child
 *	execute the kill system call with a non-existant PID
 *	check the return value
 *	if return value is not -1
 *		issue a FAIL message, break remaining tests and cleanup
 *	if we are doing functional testing
 *		if the errno was set to 3 (No such proccess)
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE
 *  kill04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	This test should be run by a non-root user
 */

#include "test.h"

#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

void cleanup(void);
void setup(void);
void do_child(void);

char *TCID = "kill04";
int TST_TOTAL = 1;

#define TEST_SIG SIGKILL

int main(int ac, char **av)
{
	int lc;
	pid_t fake_pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		fake_pid = tst_get_unused_pid(cleanup);
		TEST(kill(fake_pid, TEST_SIG));

		if (TEST_RETURN != -1) {
			tst_brkm(TFAIL, cleanup, "%s failed - errno = %d : %s "
				 "Expected a return value of -1 got %ld",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO),
				 TEST_RETURN);
		}

		/*
		 * Check to see if the errno was set to the expected
		 * value of 3 : ESRCH
		 */
		if (TEST_ERRNO == ESRCH) {
			tst_resm(TPASS, "errno set to %d : %s, as "
				 "expected", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		} else {
			tst_resm(TFAIL, "errno set to %d : %s expected "
				 "%d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO), 3, strerror(3));
		}
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * or premature exit.
 */
void cleanup(void)
{

}
