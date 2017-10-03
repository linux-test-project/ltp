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
 *  Verify that nanosleep() will fail to suspend the execution
 *  of a process for a specified time if interrupted by a non-blocked signal.
 *
 * Expected Result:
 *  nanosleep() should return with -1 value and sets errno to EINTR.
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "nanosleep03";
int TST_TOTAL = 1;

static void do_child(void);
static void setup(void);
static void sig_handler();

int main(int ac, char **av)
{
	int lc;
	pid_t cpid;
	int status;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "dddd", &timereq.tv_sec, &timereq.tv_nsec,
			&timerem.tv_sec, &timerem.tv_nsec);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/*
		 * Creat a child process and suspend its
		 * execution using nanosleep()
		 */
		if ((cpid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK, NULL, "fork() failed");

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

		sleep(1);

		/* Now send signal to child */
		SAFE_KILL(NULL, cpid, SIGINT);

		/* Wait for child to execute */
		wait(&status);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			tst_resm(TPASS, "nanosleep() failed, interrupted"
				 " by signal (%d) as expected", EINTR);
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

	/*
	 * Call nanosleep() to suspend child process
	 * for specified time 'tv_sec'.
	 * Call should return before suspending execution
	 * for the specified time due to receipt of signal
	 * from Parent.
	 */
	TEST(nanosleep(&timereq, &timerem));

	if (TEST_RETURN == -1) {

		/* Check for expected errno is set */
		if (TEST_ERRNO != EINTR) {
			tst_resm(TFAIL | TTERRNO,
				 "nanosleep() failed; expected errno: %d",
				 EINTR);
			exit(1);
		}
	} else {
		tst_resm(TFAIL, "nanosleep() returns %ld, "
			 "expected -1, errno:%d", TEST_RETURN, EINTR);
		exit(1);
	}

	exit(0);
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
