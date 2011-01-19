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
 *	waitpid02.c
 *
 * DESCRIPTION
 *	Check that when a child gets killed by an integer zero
 *	divide exception, the waiting parent is correctly notified.
 *
 * ALGORITHM
 *	Fork a child and send a SIGFPE to it. The parent waits for the
 *	death of the child and checks that SIGFPE was returned.
 *
 * USAGE:  <for command-line>
 *      waitpid02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *	10/2002 Paul Larson
 *		Div by zero doesn't cause SIGFPE on some archs, fixed
 *		to send the signal with kill
 *
 * Restrictions
 *	None
 */

#include <sys/file.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

void do_child(void);
void setup(void);
void cleanup(void);

char *TCID = "waitpid02";
int TST_TOTAL = 1;

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int pid, npid, sig, nsig;
	int exno, nexno, status;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) !=
	    NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "");
#endif

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		exno = 1;
		sig = SIGFPE;

		pid = FORK_OR_VFORK();

		if (pid == 0) {
#ifdef UCLINUX
			self_exec(argv[0], "");
			/* No fork() error check is done so don't check here */
#else
			do_child();
#endif
		} else {
			kill(pid, sig);
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
						 "unexpected signal returned");
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
				} else {
					tst_resm(TPASS, "recieved expected "
						 "exit value");
				}
			}
		}

		if (access("core", F_OK) == 0) {
			unlink("core");
		}
	}
	cleanup();
	tst_exit();

}

/*
 * do_child()
 */
void do_child()
{
	int exno = 1;

	while (1)
		usleep(10);

	exit(exno);
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
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

 }