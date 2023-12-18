// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2002 Andi Kleen
 * Copyright (C) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * DESCRIPTION
 * Check if gettimeofday is monotonous during 10s
 *
 * - Call gettimeofday() to get a t1 (fist value)
 * - Call it again to get t2, see if t2 < t1, set t2 = t1, repeat for 10 sec
 */

#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_timer.h"
#include "lapi/syscalls.h"

static volatile sig_atomic_t done;

static void breakout(int sig)
{
	done = sig;
}

static void verify_gettimeofday(void)
{
	struct __kernel_old_timeval tv1, tv2;
	unsigned long long cnt = 0;
	int rtime = tst_remaining_runtime();

	done = 0;

	alarm(rtime);

	if (tst_syscall(__NR_gettimeofday, &tv1, NULL))
		tst_brk(TFAIL | TERRNO, "gettimeofday() failed");

	while (!done) {
		if (tst_syscall(__NR_gettimeofday, &tv2, NULL))
			tst_brk(TFAIL | TERRNO, "gettimeofday() failed");

		if (tv2.tv_sec < tv1.tv_sec ||
		    (tv2.tv_sec == tv1.tv_sec && tv2.tv_usec < tv1.tv_usec)) {
			tst_res(TFAIL,
				"Time is going backwards: old %jd.%jd vs new %jd.%jd!",
				(intmax_t) tv1.tv_sec, (intmax_t) tv1.tv_usec,
				(intmax_t) tv2.tv_sec, (intmax_t) tv2.tv_usec);
			return;
		}

		tv1 = tv2;
		cnt++;
	}

	tst_res(TINFO, "gettimeofday() called %llu times", cnt);
	tst_res(TPASS, "gettimeofday() monotonous in %i seconds", rtime);
}

static void setup(void)
{
	SAFE_SIGNAL(SIGALRM, breakout);
}

static struct tst_test test = {
	.setup = setup,
	.max_runtime = 10,
	.test_all = verify_gettimeofday,
};
