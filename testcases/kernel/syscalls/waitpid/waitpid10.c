/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 John George - Ported
 *  04/2002 wjhuie sigset cleanups
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
 * Tests to see if pid's returned from fork and waitpid are same
 *
 * Set up to catch SIGINTs, SIGALRMs, and the real time timer. Until the timer
 * interrupts, do the following. Fork 8 kids, 2 will immediately exit, 2 will
 * sleep, 2 will be compute bound, and 2 will fork another child, both which
 * will do mkdirs on the same directory 50 times. When the timer expires, kill
 * all kids and remove the directory.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "test.h"

#define	MAXKIDS	8

char *TCID = "waitpid10";
int TST_TOTAL = 1;

volatile int intintr;

static void setup(void);
static void cleanup(void);
static void inthandlr();
static void wait_for_parent(void);
static void do_exit(void);
static void do_compute(void);
static void do_fork(void);
static void do_sleep(void);

static int fail;
static int fork_kid_pid[MAXKIDS];

#ifdef UCLINUX
static char *argv0;
#endif

int main(int ac, char **av)
{
	int kid_count, ret_val, status, nkids;
	int i, j, k, found;
	int wait_kid_pid[MAXKIDS];

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	argv0 = av[0];

	maybe_run_child(&do_exit, "n", 1);
	maybe_run_child(&do_compute, "n", 2);
	maybe_run_child(&do_fork, "n", 3);
	maybe_run_child(&do_sleep, "n", 4);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		fail = 0;
		kid_count = 0;
		intintr = 0;
		for (i = 0; i < MAXKIDS; i++) {
			ret_val = FORK_OR_VFORK();
			if (ret_val < 0)
				tst_brkm(TBROK|TERRNO, cleanup,
					 "Fork kid %d failed.", i);

			if (ret_val == 0) {
				if (i == 0 || i == 1) {
#ifdef UCLINUX
					if (self_exec(argv0, "n", 1) < 0)
						tst_brkm(TBROK|TERRNO, cleanup,
							 "self_exec %d failed",
							 i);
#else
					do_exit();
#endif
				}

				if (i == 2 || i == 3) {
#ifdef UCLINUX
					if (self_exec(argv0, "n", 2) < 0)
						tst_brkm(TBROK|TERRNO, cleanup,
							 "self_exec %d failed",
							 i);
#else
					do_compute();
#endif
				}

				if (i == 4 || i == 5) {
#ifdef UCLINUX
					if (self_exec(argv0, "n", 3) < 0)
						tst_brkm(TBROK|TERRNO, cleanup,
							 "self_exec %d failed",
							 i);
#else
					do_fork();
#endif
				}

				if (i == 6 || i == 7) {
#ifdef UCLINUX
					if (self_exec(argv0, "n", 4) < 0)
						tst_brkm(TBROK|TERRNO, cleanup,
							 "self_exec %d failed",
							 i);
#else
					do_sleep();
#endif

				}

			}

			fork_kid_pid[kid_count++] = ret_val;
		}

		nkids = kid_count;

		/*
		 * Now send all the kids a SIGUSR1 to tell them to
		 * proceed. We sleep for a while first to allow the
		 * children to initialize their "intintr" variables
		 * and get set up.
		 */
		sleep(15);

		for (i = 0; i < nkids; i++) {
			if (kill(fork_kid_pid[i], SIGUSR1) < 0) {
				tst_brkm(TBROK|TERRNO, cleanup, "Kill of child "
						"%d failed", i);
			}
		}

		/* Wait till all kids have terminated. */
		kid_count = 0;
		errno = 0;
		for (i = 0; i < nkids; i++) {
			while (((ret_val = waitpid(fork_kid_pid[i],
							&status, 0)) != -1)
					|| (errno == EINTR)) {
				if (ret_val == -1)
					continue;

				wait_kid_pid[kid_count++] = ret_val;
			}
		}

		/*
		 * Check that for every entry in the fork_kid_pid
		 * array, there is a matching pid in the
		 * wait_kid_pid array.
		 */
		for (i = 0; i < MAXKIDS; i++) {
			found = 0;
			for (j = 0; j < MAXKIDS; j++) {
				if (fork_kid_pid[i] == wait_kid_pid[j]) {
					found = 1;
					break;
				}
			}
			if (!found) {
				tst_resm(TFAIL, "Did not find a "
						"wait_kid_pid for the "
						"fork_kid_pid of %d",
						fork_kid_pid[i]);
				for (k = 0; k < nkids; k++) {
					tst_resm(TFAIL,
							"fork_kid_pid[%d] = "
							"%d", k,
							fork_kid_pid[k]);
				}
				for (k = 0; k < kid_count; k++) {
					tst_resm(TFAIL,
							"wait_kid_pid[%d] = "
							"%d", k,
							wait_kid_pid[k]);
				}
				fail = 1;
			}
		}

		memset(fork_kid_pid, 0, sizeof(fork_kid_pid));

		if (fail)
			tst_resm(TFAIL, "Test FAILED");
		else
			tst_resm(TPASS, "Test PASSED");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct sigaction act;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	act.sa_handler = inthandlr;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGUSR1, &act, NULL) < 0)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "sigaction(SIGUSR1, ...) failed");

	intintr = 0;

}

static void cleanup(void)
{
	int i;

	for (i = 0; i < MAXKIDS; i++) {
		if (fork_kid_pid[i] > 0)
			kill(fork_kid_pid[i], SIGKILL);
	}
}

static void inthandlr(void)
{
	intintr++;
}

static void wait_for_parent(void)
{
	while (!intintr)
		usleep(100);
}

static void do_exit(void)
{
	wait_for_parent();
	exit(3);
}

static void do_compute(void)
{
	int i;

	wait_for_parent();

	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;
	for (i = 0; i < 100000; i++) ;

	exit(4);
}

static void do_fork(void)
{
	int fork_pid, wait_pid;
	int status, i;

	wait_for_parent();

	for (i = 0; i < 50; i++) {
		fork_pid = FORK_OR_VFORK();
		if (fork_pid < 0) {
			tst_brkm(TBROK|TERRNO, NULL, "Fork failed");
		}
		if (fork_pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "n", 1) < 0) {
				tst_brkm(TFAIL, NULL,
					 "do_fork self_exec failed");
			}
#else
			do_exit();
#endif
		}

		errno = 0;
		while (((wait_pid = waitpid(fork_pid, &status, 0)) != -1) ||
		       (errno == EINTR)) {
			if (wait_pid == -1)
				continue;

			if (fork_pid != wait_pid) {
				tst_resm(TFAIL, "Didnt get a pid returned "
					 "from waitpid that matches the one "
					 "returned by fork");
				tst_resm(TFAIL, "fork pid = %d, wait pid = "
					 "%d", fork_pid, wait_pid);
				fail = 1;
			}
		}
	}

	exit(4);
}

static void do_sleep(void)
{
	wait_for_parent();
	sleep(1);
	sleep(1);

	exit(4);
}
