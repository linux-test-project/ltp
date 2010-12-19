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
 * Test Name: alarm07
 *
 * Test Description:
 *  Check the functionality of the alarm() when the time input
 *  parameter is non-zero and the process does a fork.
 *
 * Expected Result:
 *  The alarm request should be cleared in the child process.
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
 *  alarm07 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
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
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

char *TCID = "alarm07";
int TST_TOTAL = 1;
int alarms_received = 0;

void setup();
void cleanup();
void sigproc(int sig);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int sleep_time = 5;
	int status;
	int time_sec = 3;
	pid_t cpid;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call First alarm() with non-zero time parameter
		 * 'time_sec' to send SIGALRM to the process.
		 */
		TEST(alarm(time_sec));

		/* Now, fork a child process */
		cpid = FORK_OR_VFORK();
		if (cpid < 0) {
			tst_resm(TFAIL|TERRNO, "fork() failed");
		}

		sleep(sleep_time);

		if (STD_FUNCTIONAL_TEST) {
			if (cpid == 0) {
				if (alarms_received == 0)
					exit(0);
				else {
					printf("alarm request not cleared in "
					    "child; alarms received:%d\n",
					    alarms_received);
					exit(1);
				}
			} else {
				/* Wait for child to complete execution */
				if (wait(&status) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "wait failed");
				if (!WIFEXITED(status) ||
				    WEXITSTATUS(status) != 0)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "child exited abnormally");
			}
		} else
			tst_resm(TPASS, "call returned %ld", TEST_RETURN);
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Setup signal handler to catch SIGALRM signal.
 */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Set the signal catching function */
	if (signal(SIGALRM, sigproc) == SIG_ERR) {
		tst_brkm(TFAIL|TERRNO, cleanup, "signal(SIGALRM, ..) failed");
	}
}

/*
 * sigproc(int) - This function defines the action that has to be taken
 *	          when the SIGALRM signal is caught.
 *   It also sets the variable which is used to check whether the
 *   alarm system call was successful.
 */
void sigproc(int sig)
{
	alarms_received++;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
}