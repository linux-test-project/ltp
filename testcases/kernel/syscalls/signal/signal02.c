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
 *	signal02.c
 *
 * DESCRIPTION
 *	signal02 - Test that we get an error using illegal signals
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the return value
 *	  if return value != -1
 *	    issue a FAIL message, break remaining tests and cleanup
 *	  if we get an EINVAL
 *	    issue a PASS message
 *	  else
 *	    issue a FAIL message, break remaining tests and cleanup
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  signal02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on error logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	none
 */

#include "test.h"

#include <errno.h>
#include <signal.h>

void cleanup(void);
void setup(void);

char *TCID = "signal02";
int TST_TOTAL = 3;

typedef void (*sighandler_t) (int);

sighandler_t Tret;
int sigs[] = { _NSIG + 1, SIGKILL, SIGSTOP };

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * There are three cases where we should get an EINVAL
		 */
		for (i = 0; i < TST_TOTAL; i++) {

			errno = 0;
			Tret = signal(sigs[i], SIG_IGN);
			TEST_ERRNO = errno;

			if (Tret != SIG_ERR) {
				tst_brkm(TFAIL, cleanup, "%s call failed - "
					 "errno = %d : %s", TCID, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}

			switch (TEST_ERRNO) {
			case EINVAL:
				tst_resm(TPASS, "expected failure - errno = "
					 "%d - %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
				break;
			default:
				tst_resm(TFAIL, "call failed to produce "
					 "expected error - errno = %d "
					 "- %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			}
		}
		tst_count++;	/* incr. TEST_LOOP counter */
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
