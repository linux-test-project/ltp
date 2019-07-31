// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (C) Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test Description:
 *  Verify that nanosleep() will be successful to suspend the execution
 *  of a process, returns after the receipt of a signal and writes the
 *  remaining sleep time into the structure.
 *
 *  This test also verifies that if the call is interrupted by a signal
 *  handler, nanosleep() returns -1, sets errno to EINTR.
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_macros.h"

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

static void do_child(void)
{
	struct timespec timereq = {.tv_sec = 5, .tv_nsec = 9999};
	struct timespec timerem, exp_rem;

	tst_timer_start(CLOCK_MONOTONIC);
	TEST(nanosleep(&timereq, &timerem));
	tst_timer_stop();

	if (TST_RET != -1) {
		tst_res(TFAIL,
			"nanosleep was not interrupted, returned %ld, expected -1",
			TST_RET);
		return;
	}
	if (TST_ERR != EINTR) {
		tst_res(TFAIL | TTERRNO,
			"nanosleep() failed, expected EINTR, got");
		return;
	}

	tst_res(TPASS, "nanosleep() returned -1, set errno to EINTR");

	if (tst_timespec_lt(timereq, tst_timer_elapsed()))
		tst_brk(TFAIL, "nanosleep() slept more than timereq");

	exp_rem = tst_timespec_diff(timereq, tst_timer_elapsed());

	if (tst_timespec_abs_diff_us(timerem, exp_rem) > USEC_PRECISION) {
		tst_res(TFAIL,
			"nanosleep() remaining time %llius, expected %llius, diff %llius",
			tst_timespec_to_us(timerem), tst_timespec_to_us(exp_rem),
			tst_timespec_abs_diff_us(timerem, exp_rem));
	} else {
		tst_res(TPASS,
			"nanosleep() slept for %llius, remaining time difference %llius",
			tst_timer_elapsed_us(),
			tst_timespec_abs_diff_us(timerem, exp_rem));
	}
}

void run(void)
{
	pid_t cpid;

	cpid = SAFE_FORK();
	if (cpid == 0) {
		do_child();
	} else {
		sleep(1);

		SAFE_KILL(cpid, SIGINT);

		tst_reap_children();
	}
}

static void sig_handler(int si LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	tst_timer_check(CLOCK_MONOTONIC);
	SAFE_SIGNAL(SIGINT, sig_handler);
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.test_all = run,
};
