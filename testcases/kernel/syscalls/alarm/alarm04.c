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
 * Test Name: alarm04
 *
 * Test Description:
 *  Check that when an alarm request is made, the signal SIGALRM is received
 *  even after the process has done an exec().
 * 
 * Expected Result:
 *  The signal SIGALRM should be received in the execed process.
 *  Since, this process execs another process, hence never returns unless
 *  exec fails.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call	
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  alarm04 [-c n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
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

char *TCID="alarm04";
int TST_TOTAL=1;
char Cmd_buffer[PATH_MAX];	/* buffer to hold "alarm04" name */

void setup();			/* Setup function for the test */
void cleanup();			/* Cleanup function for the test */

int
main(int ac, char **av)
{
	char *msg;              /* message returned from parse_opts */
	int time_sec = 3;	/* time for which alarm is set */ 

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/*
	 * Call alarm() with non-zero timer which will
	 * expire before the execed process "sig_rev" resumes
	 * exection after sleep. Hence, SIGALRM which is generated
	 * should be received by "sig_rev" process.
	 */
	TEST(alarm(time_sec));

	if (STD_FUNCTIONAL_TEST) {
		/* exec a process "sig_rev" */
		execl(Cmd_buffer, "sig_rev", "3", NULL);
	}

	/* This message never printed unless exec fails */
	tst_resm(TFAIL, "exec of %s Failed, errno=%d",
			 Cmd_buffer, errno);

	/* Call cleanup function for the test */
	cleanup();

	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Get the TESTHOME variable and set the path of execing process.
 */
void
setup()
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	sprintf(Cmd_buffer, "sig_rev");
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  This function is executed if and only if exec() fails in the test.
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
}
