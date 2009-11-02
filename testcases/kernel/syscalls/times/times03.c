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
 *	times03.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the times() system call.
 *
 * ALGORITHM
 *	This testcase checks the values that times(2) system call returns.
 *	Start a process, and spend some CPU time by performing a spin in
 *	a for-loop. Then use the times() system call, to determine the
 *	cpu time/sleep time, and other statistics.
 *
 * USAGE:  <for command-line>
 *	times03 [-c n] [-f] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	NONE
 */

#include <sys/types.h>
#include <sys/times.h>
#include <errno.h>
#include <wait.h>
#include <time.h>
#include <test.h>
#include <usctest.h>
#include <signal.h>
#include <stdint.h>

char *TCID = "times03";
int TST_TOTAL = 1;
extern int Tst_count;
int exp_enos[] = { 0 };
volatile int timeout;		/* Did we timeout in alarm() ? */

void work(void);
void sighandler(int signal, siginfo_t * info, void *uc);

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	char *msg;		/* message returned from parse_opts */

	struct tms buf1, buf2;
	time_t start_time, end_time;
	int pid2, status, fail = 0;
	struct sigaction sa;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
		/*NOT REACHED */
	}

	setup();

	/*
	 * We spend time in userspace using the following mechanism :
	 * Setup an alarm() for 3 secs and do some simple loop operations
	 * until we get the signal. This makes the test independent of
	 * processor speed.
	 */
	sa.sa_sigaction = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;

	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		tst_brkm(TBROK, cleanup, "Sigaction failed !\n");
		/* NOT REACHED */
	}

	timeout = 0;
	alarm(3);

	work();

	/*
	 * At least some CPU time must be used in system space. This is
	 * achieved by executing the times(2) call for
	 * atleast 5 secs. This logic makes it independant
	 * of the processor speed.
	 */
	start_time = time(NULL);
	for (;;) {
		if (times(&buf1) == -1) {
			TEST_ERROR_LOG(errno);
			tst_resm(TFAIL, "Call to times(2) "
				 "failed, errno = %d", errno);
		}
		end_time = time(NULL);
		if ((end_time - start_time) > 5) {
			break;
		}
	}
	if (times(&buf1) == -1) {
		TEST_ERROR_LOG(errno);
		tst_resm(TFAIL, "Call to times(2) failed, "
			 "errno = %d", errno);
	} else {
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			if (buf1.tms_utime == 0) {
				tst_resm(TFAIL, "Error: times() report "
					 "0 user time");
				fail = 1;
			}
			if (buf1.tms_stime == 0) {
				tst_resm(TFAIL, "Error: times() report "
					 "0 system time");
				fail = 1;
			}
			if (buf1.tms_cutime != 0) {
				tst_resm(TFAIL, "Error: times() report "
					 "%ld child user time",
					 buf1.tms_cutime);
				fail = 1;
			}
			if (buf1.tms_cstime != 0) {
				tst_resm(TFAIL,
					 "Error: times() report "
					 "%ld child system time",
					 buf1.tms_cstime);
				fail = 1;
			}

			pid2 = FORK_OR_VFORK();
			if (pid2 < 0) {
				tst_brkm(TFAIL, cleanup, "Fork failed");
			 /*NOTREACHED*/} else if (pid2 == 0) {

				/* Spend some cycles in userspace */

				timeout = 0;
				alarm(3);

				work();

				/*
				 * Atleast some CPU system ime must be used
				 * even in the child process (thereby
				 * making it independent of the
				 * processor speed). In fact the child
				 * uses twice as much CPU time.
				 */
				start_time = time(NULL);
				for (;;) {
					if (times(&buf2) == -1) {
						tst_resm(TFAIL,
							 "Call to times "
							 "failed, "
							 "errno = %d", errno);
						exit(1);
					}
					end_time = time(NULL);
					if ((end_time - start_time)
					    > 10) {
						break;
					}
				}
				exit(0);
			}

			waitpid(pid2, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "Call to times(2) "
					 "failed in child");
			}
			if (times(&buf2) == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "Call to times failed "
					 "errno = %d", errno);
				fail = 1;
			}
			if (buf1.tms_utime > buf2.tms_utime) {
				tst_resm(TFAIL, "Error: parents's "
					 "user time(%ld) before child "
					 "> parent's user time (%ld) "
					 "after child",
					 (intmax_t)buf1.tms_utime, (intmax_t)buf2.tms_utime);
				fail = 1;
			}
			if (buf2.tms_cutime == 0) {
				tst_resm(TFAIL, "Error: times() "
					 "report %ld child user "
					 "time should be > than "
					 "zero", (intmax_t)buf2.tms_cutime);
				fail = 1;
			}
			if (buf2.tms_cstime == 0) {
				tst_resm(TFAIL, "Error: times() "
					 "report %ld child system time "
					 "should be > than zero",
					 (intmax_t)buf2.tms_cstime);
				fail = 1;
			}
			if (fail == 0) {
				tst_resm(TPASS, "%s: Functionality "
					 "test passed", TCID);
			}

		} else {
			tst_resm(TPASS, "%s call succeeded", TCID);
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * sighandler
 *	Set the timeout to indicate we timed out in the alarm().
 */

void sighandler(int signal, siginfo_t * info, void *uc)
{
	if (signal == SIGALRM)
		timeout = 1;
	else
		tst_brkm(TBROK, cleanup, "Unexpected signal %d\n", signal);
}

/*
 * work
 *	Do some work in user space, until we get a timeout.
 */

void work(void)
{
	int i, j, k;

	while (!timeout)
		for (i = 0; i < 10000; i++)
			for (j = 0; j < 100; j++)
				k = i * j;
	timeout = 0;
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

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

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
