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
 * Test Name: getpid02
 *
 * Test Description:
 *  Verify that getpid() system call gets the process ID of the of the
 *  calling process.
 *
 * Expected Result:
 *  getpid() should return pid of the process on success.
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
 *  getpid02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

char *TCID = "getpid02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t proc_id;		/* process id of the test process */
	pid_t pid;		/* process id of the child process */
	pid_t pproc_id;		/* parent process id */
	int status;		/* exit status of child process */

	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(getpid());

		proc_id = TEST_RETURN;

		if (STD_FUNCTIONAL_TEST) {
			if ((pid = FORK_OR_VFORK()) == -1)
				tst_resm(TFAIL|TERRNO, "fork failed");
			else if (pid == 0) {
				pproc_id = getppid();

				if (pproc_id != proc_id)
					exit(1);
				exit(0);
			} else {
				if (wait(&status) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
					    "wait failed");
				if (!WIFEXITED(status) ||
				    WEXITSTATUS(status) != 0)
					tst_resm(TFAIL, "getpid() returned "
						 "invalid pid %d", proc_id);
				else
					tst_resm(TPASS,
					    "getpid functionality is correct");
			}
		} else
			tst_resm(TPASS, "call succeeded");
	}

	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup()
{
	TEST_CLEANUP;
}
