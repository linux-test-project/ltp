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
 * Test Name: stime02
 *
 * Test Description:
 *   Verify that the system call stime() fails to set the system's idea
 *   of data and time if invoked by "non-root" user.
 *
 * Expected Result:
 *  stime() should fail with return value -1 and set errno to EPERM.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  stime02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"

#define INCR_TIME	10	/* increment in the system's current time */

char *TCID = "stime02";
int TST_TOTAL = 1;

time_t curr_time;		/* system's current time in seconds */
time_t new_time;		/* system's new time */
time_t tloc;			/* argument var. for time() */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Invoke stime(2) to set the system's time
		 * to the specified new_time as non-root user.
		 */
		TEST(stime(&new_time));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EPERM) {
				tst_resm(TPASS, "stime(2) fails, Caller not "
					 "root, errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "stime(2) fails, Caller not "
					 "root, errno:%d, expected errno:%d",
					 TEST_ERRNO, EPERM);
			}
		} else {
			tst_resm(TFAIL, "stime(2) returned %ld, expected -1, "
				 "errno:%d", TEST_RETURN, EPERM);
		}
		tst_count++;	/* incr TEST_LOOP counter */
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Get the current time and system's new time.
 */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	TEST_PAUSE;

	/* Get the current time */
	if ((curr_time = time(&tloc)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "time() failed to get current time, errno=%d", errno);
	}

	/* Get the system's new time */
	new_time = curr_time + INCR_TIME;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

}
