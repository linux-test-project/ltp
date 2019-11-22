// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
 * Author:	Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 * Ported to new library:
 * 07/2019	Christian Amann <camann@suse.com>
 */
/*
 *
 * Basic test for timer_create(2):
 *
 *	Creates a timer for each available clock using the
 *	following notification types:
 *	1) SIGEV_NONE
 *	2) SIGEV_SIGNAL
 *	3) SIGEV_THREAD
 *	4) SIGEV_THREAD_ID
 *	5) NULL
 *
 * This is also regression test for commit:
 * f18ddc13af98 ("alarmtimer: Use EOPNOTSUPP instead of ENOTSUPP")
 */

#include <signal.h>
#include <time.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/common_timers.h"

static struct notif_type {
	int		sigev_signo;
	int		sigev_notify;
	char		*message;
} types[] = {
	{SIGALRM, SIGEV_NONE, "SIGEV_NONE"},
	{SIGALRM, SIGEV_SIGNAL, "SIGEV_SIGNAL"},
	{SIGALRM, SIGEV_THREAD, "SIGEV_THREAD"},
	{SIGALRM, SIGEV_THREAD_ID, "SIGEV_THREAD_ID"},
	{0, 0, "NULL"},
};

static void run(unsigned int n)
{
	unsigned int i;
	struct sigevent evp;
	struct notif_type *nt = &types[n];
	kernel_timer_t created_timer_id;

	tst_res(TINFO, "Testing notification type: %s", nt->message);

	memset(&evp, 0, sizeof(evp));

	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

		evp.sigev_value  = (union sigval) 0;
		evp.sigev_signo  = nt->sigev_signo;
		evp.sigev_notify = nt->sigev_notify;

		if (clock == CLOCK_PROCESS_CPUTIME_ID ||
			clock == CLOCK_THREAD_CPUTIME_ID) {
			/* (PROCESS_CPUTIME_ID &
			 *  THREAD_CPUTIME_ID)
			 * is not supported on kernel versions
			 * lower than 2.6.12
			 */
			if (!have_cputime_timers())
				continue;
		}
		if (clock == CLOCK_MONOTONIC_RAW)
			continue;

		if (nt->sigev_notify == SIGEV_THREAD_ID)
			evp._sigev_un._tid = getpid();

		TEST(tst_syscall(__NR_timer_create, clock,
				nt->sigev_notify ? &evp : NULL,
				&created_timer_id));

		if (TST_RET != 0) {
			if (possibly_unsupported(clock) &&
			    (TST_ERR == EINVAL || TST_ERR == ENOTSUP)) {
				tst_res(TCONF | TTERRNO, "%s unsupported",
					get_clock_str(clock));
			} else {
				tst_res(TFAIL | TTERRNO,
					"Failed to create timer for %s",
					get_clock_str(clock));
			}
			continue;
		}

		tst_res(TPASS, "Timer successfully created for %s",
					get_clock_str(clock));

		TEST(tst_syscall(__NR_timer_delete, created_timer_id));
		if (TST_RET != 0) {
			tst_res(TFAIL | TTERRNO, "Failed to delete timer %s",
				get_clock_str(clock));
		}
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(types),
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f18ddc13af98"},
		{}
	}
};
