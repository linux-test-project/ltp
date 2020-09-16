// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Block on a futex and wait for wakeup.
 *
 * This tests uses private mutexes with threads.
 */

#include "futextest.h"
#include "futex_utils.h"
#include "tst_safe_pthread.h"

static futex_t futex = FUTEX_INITIALIZER;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .desc = "syscall time64 with kernel spec"},
#endif
};

static void *threaded(void *arg)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	long ret, pid = (long)arg;

	TST_PROCESS_STATE_WAIT(pid, 'S', 0);

	ret = futex_wake(tv->fntype, &futex, 1, FUTEX_PRIVATE_FLAG);
	if (ret != 1)
		tst_res(TFAIL, "futex_wake() returned %li", ret);

	return (void*)ret;
}

static void run(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	long res, pid = getpid();
	pthread_t t;

	SAFE_PTHREAD_CREATE(&t, NULL, threaded, (void*)pid);

	res = futex_wait(tv->fntype, &futex, futex, NULL, FUTEX_PRIVATE_FLAG);
	if (res) {
		tst_res(TFAIL | TERRNO, "futex_wait() failed");
		SAFE_PTHREAD_JOIN(t, NULL);
		return;
	}

	SAFE_PTHREAD_JOIN(t, NULL);
	tst_res(TPASS, "futex_wait() woken up");
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
};
