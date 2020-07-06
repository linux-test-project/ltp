// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Block several processes on a mutex, then wake them up.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include "futextest.h"
#include "futex_utils.h"

static futex_t *futex;

static struct test_variants {
	enum futex_fn_type fntype;
	char *desc;
} variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .desc = "syscall time64 with kernel spec"},
#endif
};

static void do_child(void)
{
	struct test_variants *tv = &variants[tst_variant];

	futex_wait(tv->fntype, futex, *futex, NULL, 0);
	exit(0);
}

static void do_wake(int nr_children)
{
	struct test_variants *tv = &variants[tst_variant];
	int res, i, cnt;

	TEST(futex_wake(tv->fntype, futex, nr_children, 0));
	if (TST_RET != nr_children) {
		tst_res(TFAIL | TTERRNO,
		        "futex_wake() woken up %li children, expected %i",
			TST_RET, nr_children);
		return;
	}

	for (cnt = 0, i = 0; i < 100000; i++) {
		while (waitpid(-1, &res, WNOHANG) > 0)
			cnt++;

		if (cnt == nr_children)
			break;

		usleep(100);
	}

	if (cnt != nr_children) {
		tst_res(TFAIL, "reaped only %i childs, expected %i",
		        cnt, nr_children);
	} else {
		tst_res(TPASS, "futex_wake() woken up %i childs", cnt);
	}
}

static void run(void)
{
	struct test_variants *tv = &variants[tst_variant];
	pid_t pids[55];
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pids); i++) {
		pids[i] = SAFE_FORK();
		if (!pids[i])
			do_child();
	}

	for (i = 0; i < ARRAY_SIZE(pids); i++)
		TST_PROCESS_STATE_WAIT(pids[i], 'S', 0);

	for (i = 1; i <= 10; i++)
		do_wake(i);

	TEST(futex_wake(tv->fntype, futex, 1, 0));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
			"futex_wake() woken up %li, none were waiting",
		        TST_RET);
	} else {
		tst_res(TPASS, "futex_wake() woken up 0 children");
	}
}

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	futex = SAFE_MMAP(NULL, sizeof(*futex), PROT_READ | PROT_WRITE,
			  MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	*futex = FUTEX_INITIALIZER;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
	.forks_child = 1,
};
