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
#include "usctest.h"

#include <errno.h>
#include <signal.h>

void cleanup(void);
void setup(void);

char *TCID = "signal02";
int TST_TOTAL = 3;
extern int Tst_count;

typedef void (*sighandler_t) (int);

sighandler_t Tret;
int sigs[] = { _NSIG + 1, SIGKILL, SIGSTOP };
int exp_enos[] = { 22, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

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
			 /*NOTREACHED*/}

			TEST_ERROR_LOG(TEST_ERRNO);

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
		Tst_count++;	/* incr. TEST_LOOP counter */
	}

	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set expected errnos for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
