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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	waitpid05.c
 *
 * DESCRIPTION
 *	Check that when a child kills itself with a kill statement after
 *	determining its process id by using getpid, the parent receives a
 *	correct report of the cause of its death. This also indirectly
 *	checks that getpid returns the correct process id.
 *
 * ALGORITHM
 *	For signals 1 - 15: fork a child that determines it's own process
 *	id, then sends the signal to itself.  The parent waits to see if the
 *	demise of the child results in the signal number being returned to
 *	the parent.
 *
 * USAGE:  <for command-line>
 *      waitpid05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 *	None
 */

#include <sys/file.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"

static void do_child(int);
static void setup(void);
static void cleanup(void);

char *TCID = "waitpid05";
int TST_TOTAL = 1;

#ifdef UCLINUX
static void do_child_uclinux(void);
static int sig_uclinux;
#endif

int main(int ac, char **av)
{
	int pid, npid, sig, nsig;
	int exno, nexno, status;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "d", &sig_uclinux);
#endif

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * Set SIGTERM to SIG_DFL as test driver sets up to ignore
		 * SIGTERM
		 */
		if (signal(SIGTERM, SIG_DFL) == SIG_ERR) {
			tst_resm(TFAIL, "Sigset SIGTERM failed, errno = %d",
				 errno);

		}

		exno = 1;
		for (sig = 1; sig <= 15; sig++) {
			if (sig == SIGUSR1 || sig == SIGUSR2 || sig == SIGBUS)
				continue;

			/*Initialize signal to its default action */
			signal(sig, SIG_DFL);
			pid = FORK_OR_VFORK();

			if (pid == 0) {
#ifdef UCLINUX
				self_exec(av[0], "d", sig);
				/* No fork() error check is done so don't */
				/* do an error check here */
#else
				do_child(sig);
#endif
			} else {
				errno = 0;
				while (((npid = waitpid(pid, &status, 0)) !=
					-1) || (errno == EINTR)) {
					if (errno == EINTR)
						continue;

					if (npid != pid) {
						tst_resm(TFAIL, "waitpid "
							 "error: unexpected "
							 "pid returned");
					} else {
						tst_resm(TPASS, "received "
							 "expected pid.");
					}

					nsig = status % 256;

					/*
					 * to check if the core dump bit has
					 * been set, bit #7
					 */
					if (nsig >= 128) {
						nsig -= 128;
						if ((sig == 1) || (sig == 2) ||
						    (sig == 9) || (sig == 13) ||
						    (sig == 14) ||
						    (sig == 15)) {
							tst_resm(TFAIL,
								 "signal "
								 "error : "
								 "core dump "
								 "bit set for"
								 " exception "
								 "number %d",
								 sig);
						}
					} else if ((sig == 3) || (sig == 4) ||
						   (sig == 5) || (sig == 6) ||
						   (sig == 8) || (sig == 11)) {
						tst_resm(TFAIL,
							 "signal error: "
							 "core dump bit not "
							 "set for exception "
							 "number %d", sig);
					}

					/*
					 * nsig is the signal number returned
					 * by waitpid
					 */
					if (nsig != sig) {
						tst_resm(TFAIL, "waitpid "
							 "error: unexpected "
							 "signal returned");
						tst_resm(TINFO, "got signal "
							 "%d, expected  "
							 "%d", nsig, sig);
					}

					/*
					 * nexno is the exit number returned
					 * by waitpid
					 */
					nexno = status / 256;
					if (nexno != 0) {
						tst_resm(TFAIL, "signal "
							 "error: unexpected "
							 "exit number "
							 "returned");
					} else {
						tst_resm(TPASS, "received "
							 "expected exit number.");
					}
				}
			}
		}

		if (access("core", F_OK) == 0)
			unlink("core");
	}

	cleanup();
	tst_exit();
}

static void do_child(int sig)
{
	int exno = 1;
	int pid = getpid();

	if (kill(pid, sig) == -1) {
		tst_resm(TFAIL, "kill error: kill unsuccessful");
		exit(exno);
	}
}

#ifdef UCLINUX
/*
 * do_child_uclinux()
 *	run do_child with the appropriate sig variable
 */
static void do_child_uclinux(void)
{
	do_child(sig_uclinux);
}
#endif

static void setup(void)
{
	struct rlimit newlimit;

	TEST_PAUSE;

	tst_tmpdir();

	newlimit.rlim_max = newlimit.rlim_cur = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &newlimit) != 0)
		tst_resm(TWARN,
			 "setrlimit(RLIMIT_CORE,RLIM_INFINITY) failed; this may cause some false core-dump test failures");
}

static void cleanup(void)
{
	tst_rmdir();
}
