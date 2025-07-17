// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that changing the value of the CLOCK_REALTIME clock via
 * clock_settime() shall have no effect on a thread that is blocked on
 * absolute/relative clock_nanosleep().
 */

#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "time64_variants.h"

#define SEC_TO_US(x)     (x * 1000 * 1000)

#define CHILD_SLEEP_US SEC_TO_US(5)
#define PARENT_SLEEP_S 2
#define DELTA_US SEC_TO_US(1)

static struct tst_ts *sleep_child;

static struct time64_variants variants[] = {
	{
		.clock_nanosleep = libc_clock_nanosleep,
		.ts_type = TST_LIBC_TIMESPEC,
		.desc = "vDSO or syscall with libc spec"
	},

#if (__NR_clock_nanosleep != __LTP__NR_INVALID_SYSCALL)
	{
		.clock_nanosleep = sys_clock_nanosleep,
		.ts_type = TST_KERN_OLD_TIMESPEC,
		.desc = "syscall with old kernel spec"
	},
#endif

#if (__NR_clock_nanosleep_time64 != __LTP__NR_INVALID_SYSCALL)
	{
		.clock_nanosleep = sys_clock_nanosleep64,
		.ts_type = TST_KERN_TIMESPEC,
		.desc = "syscall time64 with kernel spec"
	},
#endif
};

static void child_nanosleep(struct time64_variants *tv, const int flags)
{
	long long delta;
	struct timespec begin, end;

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &begin);

	if (flags & TIMER_ABSTIME) {
		tst_res(TINFO, "Using absolute time sleep");

		tst_ts_set_sec(sleep_child, begin.tv_sec);
		tst_ts_set_nsec(sleep_child, begin.tv_nsec);

		*sleep_child = tst_ts_add_us(*sleep_child, CHILD_SLEEP_US);
	} else {
		tst_res(TINFO, "Using relative time sleep");

		*sleep_child = tst_ts_from_us(sleep_child->type, CHILD_SLEEP_US);
	}

	TEST(tv->clock_nanosleep(CLOCK_REALTIME, flags, tst_ts_get(sleep_child), NULL));
	if (TST_RET)
		tst_brk(TBROK | TERRNO, "clock_nanosleep() error");

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC, &end);

	if (tst_timespec_lt(end, begin)) {
		tst_res(TFAIL, "clock_settime() didn't sleep enough. "
			"begin: %lld ms >= end: %lld ms",
			tst_timespec_to_ms(begin),
			tst_timespec_to_ms(end));
		return;
	}

	delta = tst_timespec_abs_diff_us(begin, end);
	if (!(flags & TIMER_ABSTIME))
		delta -= CHILD_SLEEP_US;

	if (delta > DELTA_US) {
		tst_res(TFAIL, "parent clock_settime() affected child sleep. "
			"begin: %lld ms, end: %lld ms",
			tst_timespec_to_ms(begin),
			tst_timespec_to_ms(end));
		return;
	}

	tst_res(TPASS, "parent clock_settime() didn't affect child sleep "
		"(delta time: %lld us)", delta);
}

static void run(unsigned int tc_index)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct timespec begin;
	struct timespec sleep_parent = {
		.tv_sec = PARENT_SLEEP_S,
	};

	if (!SAFE_FORK()) {
		child_nanosleep(tv, tc_index ? TIMER_ABSTIME : 0);
		exit(0);
	}

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME, &begin);
	SAFE_CLOCK_NANOSLEEP(CLOCK_REALTIME, 0, &sleep_parent, NULL);
	SAFE_CLOCK_SETTIME(CLOCK_REALTIME, &begin);
}

static void setup(void)
{
	sleep_child->type = variants[tst_variant].ts_type;

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = 2,
	.needs_root = 1,
	.forks_child = 1,
	.restore_wallclock = 1,
	.test_variants = ARRAY_SIZE(variants),
	.bufs = (struct tst_buffers []) {
		{&sleep_child, .size = sizeof(struct tst_ts)},
		{},
	}
};
