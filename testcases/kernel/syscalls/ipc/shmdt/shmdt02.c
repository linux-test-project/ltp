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
 *	shmdt02.c
 *
 * DESCRIPTION
 *	shmdt02 - check for EINVAL error
 *
 * ALGORITHM
 *	loop if that option was specified
 *	  call shmdt() using an invalid shared memory address
 *	  check the errno value
 *	    issue a PASS message if we get EINVAL
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmdt02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcshm.h"

char *TCID = "shmdt02";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	int unshared;		/* a local variable to use to produce *//* the error in the shmdt() call */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * make the call using the TEST() macro - attempt to
		 * remove an invalid shared memory address
		 */

		TEST(shmdt(&unshared));

		if (TEST_RETURN != -1) {
			tst_brkm(TFAIL, cleanup, "call succeeded unexpectedly");
		}

		switch (TEST_ERRNO) {
		case EINVAL:
			tst_resm(TPASS, "expected failure - errno = %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed with an unexpected error "
				 "- %d : %s", TEST_ERRNO, strerror(TEST_ERRNO));

		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}
