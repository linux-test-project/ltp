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
 * Test Name: nanosleep02
 *
 * Test Description:
 *  Verify that nanosleep() will be successful to suspend the execution
 *  of a process, returns after the receipt of a signal and writes the
 *  remaining sleep time into the structure.
 *
 * Expected Result:
 *  nanosleep() should return with after receipt of a signal and write
 *  remaining sleep time into a structure. if called again, succeeds to
 *  suspend execution of process for the specified sleep time.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  nanosleep02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	05/2004 Changed USEC_PRECISION from 100 to 2000 to get around glibc failure
 *		that's years old.
 *
 * RESTRICTIONS:
 *  None.
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <wait.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>

#include "test.h"
#include "usctest.h"

char *TCID = "nanosleep02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

extern int Tst_count;		/* Test Case counter for tst_* routines */

struct timespec timereq;	/* time struct. buffer for nanosleep() */
struct timespec timerem;	/* time struct. buffer for nanosleep() */

void do_child();		/* Child process */
void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal catching function */

/*
 * Define here the "rem" precision in microseconds,
 * Various implementations will provide different
 * precisions. The -aa tree provides up to usec precision.
 * NOTE: all the trees that don't provide a precision of
 * the order of the microseconds are subject to an userspace
 * live lock condition with glibc under a flood of signals,
 * the "rem" field would never change without the increased
 * usec precision in the -aa tree.
 */
#define USEC_PRECISION 250000	/* Error margin allowed in usec */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t cpid;		/* Child process id */
	int status;		/* child exit status */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child, "dddd", &timereq.tv_sec, &timereq.tv_nsec,
			&timerem.tv_sec, &timerem.tv_nsec);
#endif

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Creat a child process and suspend its
		 * execution using nanosleep()
		 */
		if ((cpid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup,
				 "fork() failed to create child process");
		}

		if (cpid == 0) {	/* Child process */
#ifdef UCLINUX
			if (self_exec(av[0], "dddd",
				      timereq.tv_sec, timereq.tv_nsec,
				      timerem.tv_sec, timerem.tv_nsec) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		}

		/* wait for child to time slot for execution */
		sleep(1);

		/* Now send signal to child */
		if (kill(cpid, SIGINT) < 0) {
			tst_brkm(TBROK, cleanup,
				 "kill() fails send signal to child");
		}

		/* Wait for child to execute */
		wait(&status);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			tst_resm(TPASS, "Functionality of nanosleep() is "
					"correct");
		} else {
			tst_resm(TFAIL, "child process exited abnormally; "
					"status = %d", status);
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * do_child()
 */
void do_child()
{
	unsigned long req, rem, elapsed;	/* usec */
	struct timeval otime;	/* time before child execution suspended */
	struct timeval ntime;	/* time after child resumes execution */

	/* Note down the current time */
	gettimeofday(&otime, NULL);
	/*
	 * Call nanosleep() to suspend child process
	 * for specified time 'tv_sec'.
	 * Call should return before suspending execution
	 * for the specified time due to receipt of signal
	 * from Parent.
	 */
	TEST(nanosleep(&timereq, &timerem));
	/* time after child resumes execution */
	gettimeofday(&ntime, NULL);

	/*
	 * Check whether the remaining sleep of child updated
	 * in 'timerem' structure.
	 * The time remaining should be equal to the
	 * Total time for sleep - time spent on sleep bfr signal
	 * Change precision from msec to usec.
	 */
	req = timereq.tv_sec * 1000000 + timereq.tv_nsec / 1000;
	rem = timerem.tv_sec * 1000000 + timerem.tv_nsec / 1000;
	elapsed =
	    (ntime.tv_sec - otime.tv_sec) * 1000000 + ntime.tv_usec -
	    otime.tv_usec;

	if (rem - (req - elapsed) > USEC_PRECISION) {
		tst_resm(TWARN,
			 "This test could fail if the system was under load");
		tst_resm(TWARN,
			 "due to the limitation of the way it calculates the");
		tst_resm(TWARN, "system call execution time.");
		tst_resm(TFAIL, "Remaining sleep time %lu usec doesn't "
			 "match with the expected %lu usec time",
			 rem, (req - elapsed));
	} else {

		/* Record the time before suspension */
		gettimeofday(&otime, NULL);

		/*
		 * Invoke nanosleep() again to suspend child
		 * for the specified sleep time specified by
		 * 'timereq' structure.
		 */
		TEST(nanosleep(&timereq, &timerem));

		/* Record the time after suspension */
		gettimeofday(&ntime, NULL);

		/* check return code of nanosleep() */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "nanosleep() failed");
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		else if (STD_FUNCTIONAL_TEST) {
			/*
			 * Verify whether child execution was
			 * actually suspended for the remaining
			 * sleep time specified by 'timerem'
			 * structure.
			 */
			req = timereq.tv_sec * 1000000 + timereq.tv_nsec / 1000;
			elapsed =
			    (ntime.tv_sec - otime.tv_sec) * 1000000 + ntime.tv_usec -
			    otime.tv_usec;
			if (elapsed - req > USEC_PRECISION) {
				tst_resm(TWARN,
					 "This test could fail if the system "
					 "was under load due to the limitation "
					 "of the way it calculates the system "
					 "call execution time.");
				tst_resm(TFAIL, "Child execution not suspended "
						"for %jd seconds %lu "
						"nanoseconds",
						(intmax_t)timereq.tv_sec,
						timereq.tv_nsec);
			} else {
				tst_resm(TINFO, "call succeeded");
			}

		}

	}

	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Setup signal handler to catch the interrupt signal sent by parent
 *  to child process.
 *  Initialise time structure elements.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Setup signal handler */
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		tst_brkm(TBROK, cleanup,
			 "signal() fails to setup signal handler");
	}

	/* Initialise time variables which used to suspend child execution */
	timereq.tv_sec = 5;
	timereq.tv_nsec = 9999;

	/* Initialise 'time remaining' structure elements to NULL */
	timerem.tv_sec = 0;
	timerem.tv_nsec = 0;
}

/*
 * sig_handler() - signal catching function.
 *   This function gets executed when a parnet sends a signal 'SIGINT'
 *   to child to awake it from sleep.
 *   This function just returns without doing anything.
 */
void sig_handler()
{
}

/*
 * void
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

	/* exit with return code appropriate for results */
	tst_exit();
}
