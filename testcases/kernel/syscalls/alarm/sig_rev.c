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
 * send_rev.c - This is program is execed by "alarm04" process. When
 *		"alarm04" executed, this process sleeps till receipt
 *		of SIGALRM signal which was generated when alarm timer expires
 *		at the calling function "alarm04".
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

char *TCID="sig_rev";		/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
int almreceived = 0;		/* flag to indicate SIGALRM received or not */

void setup();			/* Setup function for test */
void cleanup();			/* Cleanup function for test */
void sigproc(int);		/* Signal catching function */

int
main(int ac, char **av)
{
	int sleep_time = 10;	/* waiting time for the SIGALRM signal */
	int time_sec;		 /* alarm timer */

	if (ac < 2) {
		tst_brkm(TBROK, NULL,
			 "'sig_rev' should be execed by 'alarm04' process!!");
		tst_exit();
	}

	/* Get the alarm timer value from caller process */
	time_sec = atoi(av[1]);

	/* Perform global setup for test */
	setup();

	/*
	 * Wait for SIGALRM to be generated when alarm timer
	 * expires in "sig_send" caller process.
	 */
	sleep(sleep_time);

	if (STD_FUNCTIONAL_TEST) {
		/*
		 * Verify whether this process received "SIGALRM"
		 * by checking the value of almreceived variable.
		 */
		if (almreceived == 1) {
			tst_resm(TPASS,
				 "Functionality of alarm(%u) successful",
				 time_sec);
		} else {
			tst_resm(TFAIL,
				 "alarm() fails to send SIGALRM to execede"
				 " process");
		}
	}

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}	/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Setup signal handler to catch SIGALRM.
 */
void 
setup()
{
	/* Set the signal catching function */
	if (signal(SIGALRM, sigproc) == SIG_ERR) {
		tst_brkm(TFAIL, cleanup,
			 "signal() fails, errno:%d : %s",
			 errno, strerror(errno));
	}
}	/* End setup() */

/*
 * void
 * sigproc(int) - This function defines the action that has to be taken
 *                when the SIGALRM signal is caught.
 *   It also sets the variable which is used to check whether the
 *   alarm system call was successful.
 */
void
sigproc(int sig)
{
	almreceived = almreceived + 1;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void 
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}	/* End cleanup() */
