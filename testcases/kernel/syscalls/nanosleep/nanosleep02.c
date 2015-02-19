/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that nanosleep() will be successful to suspend the execution
 *  of a process, returns after the receipt of a signal and writes the
 *  remaining sleep time into the structure.
 *
 * Expected Result:
 *  nanosleep() should return with after receipt of a signal and write
 *  remaining sleep time into a structure. if called again, succeeds to
 *  suspend execution of process for the specified sleep time.
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

char *TCID = "nanosleep02";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void sig_handler();

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
	int lc;
	const char *msg;
	pid_t cpid;
	int status;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

#ifdef UCLINUX
	maybe_run_child(&do_child, "dddd", &timereq.tv_sec, &timereq.tv_nsec,
			&timerem.tv_sec, &timerem.tv_nsec);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		if ((cpid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, NULL,
				 "fork() failed to create child process");
		}

		if (cpid == 0) {
#ifdef UCLINUX
			if (self_exec(av[0], "dddd",
				      timereq.tv_sec, timereq.tv_nsec,
				      timerem.tv_sec, timerem.tv_nsec) < 0) {
				tst_brkm(TBROK, NULL, "self_exec failed");
			}
#else
			do_child();
#endif
		}

		/* wait for child to time slot for execution */
		sleep(1);

		/* Now send signal to child */
		if (kill(cpid, SIGINT) < 0) {
			tst_brkm(TBROK, NULL,
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
	}

	tst_exit();
}

static void do_child(void)
{
	struct timespec timereq = {.tv_sec = 5, .tv_nsec = 9999};
	struct timespec timerem;

	unsigned long req, rem, elapsed;
	struct timeval otime;
	struct timeval ntime;

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

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "nanosleep() failed");
		} else {
			/*
			 * Verify whether child execution was
			 * actually suspended for the remaining
			 * sleep time specified by 'timerem'
			 * structure.
			 */
			req = timereq.tv_sec * 1000000 + timereq.tv_nsec / 1000;
			elapsed =
			    (ntime.tv_sec - otime.tv_sec) * 1000000 +
			    ntime.tv_usec - otime.tv_usec;
			if (elapsed - req > USEC_PRECISION) {
				tst_resm(TWARN,
					 "This test could fail if the system "
					 "was under load due to the limitation "
					 "of the way it calculates the system "
					 "call execution time.");
				tst_resm(TFAIL, "Child execution not suspended "
					 "for %jd seconds %lu "
					 "nanoseconds",
					 (intmax_t) timereq.tv_sec,
					 timereq.tv_nsec);
			} else {
				tst_resm(TINFO, "call succeeded");
			}

		}

	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "signal() fails to setup signal handler");
	}
}

static void sig_handler(void)
{
}
