// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 * Copyright (C) 2026 Michael Menasherov <mmenashe@redhat.com>
 */

/*\
 * Check that futex(FUTEX_WAIT) returns EFAULT when:
 *
 * 1) uaddr points to unmapped memory
 * 2) timeout points to unmapped memory
 */
#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex = FUTEX_INITIALIZER;
static void *bad;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static struct testcase {
	const char *desc;
	int bad_uaddr;
	int bad_timeout;
} testcases[] = {
	{
		.desc = "uaddr points to unmapped memory",
		.bad_uaddr = 1,
	},
	{
		.desc = "timeout points to unmapped memory",
		.bad_timeout = 1,
	},
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];
	struct tst_ts ts = tst_ts_from_ms(tv->tstype, 5000);
	futex_t *uaddr = tc->bad_uaddr ? bad : &futex;
	void *timeout = tc->bad_timeout ? bad : tst_ts_get(&ts);

	TST_EXP_FAIL(futex_syscall(tv->fntype, uaddr, FUTEX_WAIT, futex,
				   timeout, NULL, 0, 0), EFAULT, "%s", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	bad = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
