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
 *	waitpid01.c
 *
 * DESCRIPTION
 *	Check that when a child kills itself by generating an alarm
 *	exception, the waiting parent is correctly notified.
 *
 * ALGORITHM
 *	Fork a child that sets an alarm. When the alarm goes off, causing
 *	the death of the child, the parent checks that SIG_ALRM was returned
 *
 * USAGE:  <for command-line>
 *      waitpid01 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>

void setup(void);
void cleanup(void);

char *TCID = "waitpid01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int pid, npid, sig, nsig;
	int exno, nexno, status;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		exno = 1;
		sig = 14;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TFAIL, cleanup, "Fork Failed");
		} else if (pid == 0) {
			alarm(2);
			pause();
			exit(exno);
		} else {
			errno = 0;
			while (((npid = waitpid(pid, &status, 0)) != -1) ||
			       (errno == EINTR)) {
				if (errno == EINTR) {
					continue;
				}

				if (npid != pid) {
					tst_resm(TFAIL, "waitpid error: "
						 "unexpected pid returned");
				} else {
					tst_resm(TPASS,
						 "recieved expected pid");
				}

				nsig = WTERMSIG(status);

				/*
				 * nsig is the signal number returned by
				 * waitpid
				 */
				if (nsig != sig) {
					tst_resm(TFAIL, "waitpid error: "
						 "unexpected signal "
						 "returned");
				} else {
					tst_resm(TPASS, "recieved expected "
						 "signal");
				}

				/*
				 * nexno is the exit number returned by
				 * waitpid
				 */
				nexno = WEXITSTATUS(status);
				if (nexno != 0) {
					tst_resm(TFAIL, "signal error: "
						 "unexpected exit number "
						 "returned");
				}
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup()
 *      performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup()
 *      performs all ONE TIME cleanup for this test at
 *      completion or premature exit
 */
void cleanup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
