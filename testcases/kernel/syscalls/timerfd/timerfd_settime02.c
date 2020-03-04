// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2017-10661
 *
 * Test for race condition vulnerability in timerfd_settime(). Multiple
 * concurrent calls of timerfd_settime() clearing the CANCEL_ON_SET flag may
 * cause memory corruption. Fixed in:
 *
 *  commit 1e38da300e1e395a15048b0af1e5305bd91402f6
 *  Author: Thomas Gleixner <tglx@linutronix.de>
 *  Date:   Tue Jan 31 15:24:03 2017 +0100
 *
 *  timerfd: Protect the might cancel mechanism proper
 */
#include <unistd.h>
#include "tst_safe_timerfd.h"
#include "tst_test.h"
#include "tst_fuzzy_sync.h"
#include "tst_taint.h"

#define TIMERFD_FLAGS "timerfd_settime(TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET)"

#ifndef TFD_TIMER_CANCEL_ON_SET
#define TFD_TIMER_CANCEL_ON_SET (1<<1)
#endif

static int fd = -1;
static struct itimerspec its;
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	tst_taint_init(TST_TAINT_W | TST_TAINT_D);
	fd = SAFE_TIMERFD_CREATE(CLOCK_REALTIME, 0);

	fzsync_pair.exec_loops = 1000000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
	tst_fzsync_pair_cleanup(&fzsync_pair);
}

static int punch_clock(int flags)
{
	return timerfd_settime(fd, flags, &its, NULL);
}

static void *thread_run(void *arg)
{
	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		punch_clock(0);
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		TEST(punch_clock(TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET));

		if (TST_RET == -1)
			tst_brk(TBROK | TTERRNO, TIMERFD_FLAGS " failed");

		if (TST_RET != 0)
			tst_brk(TBROK | TTERRNO, "Invalid " TIMERFD_FLAGS
				" return value");

		tst_fzsync_start_race_a(&fzsync_pair);
		punch_clock(0);
		tst_fzsync_end_race_a(&fzsync_pair);

		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable");
			return;
		}
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "2.6.25",
	.tags = (const struct tst_tag[]) {
		{"linux-git", "1e38da300e1e"},
		{"CVE", "2017-10661"},
		{}
	}
};
