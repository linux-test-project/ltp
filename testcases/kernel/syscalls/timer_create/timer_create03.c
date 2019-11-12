// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 *
 * Author:	Christian Amann <camann@suse.com>
 */
/*
 * Regression test for CVE-2017-18344:
 *
 * In kernels prior to 4.14.8 sigevent.sigev_notify is not
 * properly verified when calling timer_create(2) with the
 * field being set to (SIGEV_SIGNAL | SIGEV_THREAD_ID).
 * This can be used to read arbitrary kernel memory.
 *
 * For more info see: https://nvd.nist.gov/vuln/detail/CVE-2017-18344
 * or commit: cef31d9af908
 *
 * This test uses an unused number instead of SIGEV_THREAD_ID to check
 * if this field gets verified correctly.
 */

#include <errno.h>
#include <signal.h>
#include <time.h>
#include "tst_test.h"
#include "lapi/common_timers.h"

#define RANDOM_UNUSED_NUMBER (54321)

static void run(void)
{
	struct sigevent evp;
	clock_t clock = CLOCK_MONOTONIC;
	kernel_timer_t created_timer_id;

	memset(&evp, 0, sizeof(evp));

	evp.sigev_signo  = SIGALRM;
	evp.sigev_notify = SIGEV_SIGNAL | RANDOM_UNUSED_NUMBER;
	evp._sigev_un._tid = getpid();

	TEST(tst_syscall(__NR_timer_create, clock, &evp, &created_timer_id));

	if (TST_RET != 0) {
		if (TST_ERR == EINVAL) {
			tst_res(TPASS | TTERRNO,
					"timer_create() failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO,
					"timer_create() unexpectedly failed");
		}
		return;
	}

	tst_res(TFAIL,
		"timer_create() succeeded for invalid notification type");

	TEST(tst_syscall(__NR_timer_delete, created_timer_id));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "Failed to delete timer %s",
			get_clock_str(clock));
	}
}

static struct tst_test test = {
	.test_all = run,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-18344"},
		{"linux-git", "cef31d9af908"},
		{}
	}
};
