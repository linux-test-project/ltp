/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (C) Cyril Hrubis <chrubis@suse.cz>
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
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
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
	pid_t cpid;

	tst_parse_opts(ac, av, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&do_child, "");
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
			if (self_exec(av[0], "")) {
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

		tst_record_childstatus(NULL, cpid);
	}

	tst_exit();
}

static void do_child(void)
{
	struct timespec timereq = {.tv_sec = 5, .tv_nsec = 9999};
	struct timespec timerem, exp_rem;

	tst_timer_start(CLOCK_MONOTONIC);
	TEST(nanosleep(&timereq, &timerem));
	tst_timer_stop();

	if (tst_timespec_lt(timereq, tst_timer_elapsed())) {
		tst_resm(TFAIL, "nanosleep() slept more than timereq");
		return;
	}

	exp_rem = tst_timespec_diff(timereq, tst_timer_elapsed());

	if (tst_timespec_abs_diff_us(timerem, exp_rem) > USEC_PRECISION) {
		tst_resm(TFAIL,
		         "nanosleep() remaining time %llius, expected %llius, diff %llius",
			 tst_timespec_to_us(timerem), tst_timespec_to_us(exp_rem),
			 tst_timespec_abs_diff_us(timerem, exp_rem));
	} else {
		tst_resm(TPASS,
		         "nanosleep() slept for %llius, remaining time difference %llius",
			 tst_timer_elapsed_us(),
		         tst_timespec_abs_diff_us(timerem, exp_rem));
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	tst_timer_check(CLOCK_MONOTONIC);

	TEST_PAUSE;

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		tst_brkm(TBROK, NULL,
			 "signal() fails to setup signal handler");
	}
}

static void sig_handler(void)
{
}
