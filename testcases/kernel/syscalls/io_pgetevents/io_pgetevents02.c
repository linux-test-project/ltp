// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic io_pgetevents() test to check various failures.
 */
#include "time64_variants.h"
#include "tst_test.h"
#include "tst_timer.h"
#include "lapi/io_pgetevents.h"

#ifdef HAVE_LIBAIO
static sigset_t sigmask;
static struct io_event events[1];
static io_context_t ctx, invalid_ctx = 0;
static int fd, ctx_initialized;

static struct tst_ts to;
static void *bad_addr;

static struct tcase {
	char *name;
	io_context_t *ctx;
	long min_nr;
	long max_nr;
	struct io_event *events;
	struct tst_ts *timeout;
	sigset_t *sigmask;
	int exp_errno;
} tcases[] = {
	{"invalid ctx", &invalid_ctx, 1, 1, events, &to, &sigmask, EINVAL},
	{"invalid min_nr", &ctx, -1, 1, events, &to, &sigmask, EINVAL},
	{"invalid max_nr", &ctx, 1, -1, events, &to, &sigmask, EINVAL},
	{"invalid events", &ctx, 1, 1, NULL, &to, &sigmask, EFAULT},
	{"invalid timeout", &ctx, 1, 1, events, NULL, &sigmask, EFAULT},
	{"invalid sigmask", &ctx, 1, 1, events, &to, NULL, EFAULT},
};

static struct time64_variants variants[] = {
#if (__NR_io_pgetevents != __LTP__NR_INVALID_SYSCALL)
	{ .io_pgetevents = sys_io_pgetevents, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_io_pgetevents_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .io_pgetevents = sys_io_pgetevents_time64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct iocb cb, *cbs[1];
	char data[4096];
	int ret;

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	bad_addr = tst_get_bad_addr(NULL);
	to = tst_ts_from_ns(tv->ts_type, 10000);

	cbs[0] = &cb;

	sigemptyset(&sigmask);

	fd = SAFE_OPEN("io_pgetevents_file", O_RDWR | O_CREAT, 0644);
	io_prep_pwrite(&cb, fd, data, 4096, 0);

	TEST(io_setup(1, &ctx));
	if (TST_RET == -ENOSYS)
		tst_brk(TCONF | TRERRNO, "io_setup(): AIO not supported by kernel");
	if (TST_RET < 0)
		tst_brk(TBROK | TRERRNO, "io_setup() failed");

	ctx_initialized = 1;

	ret = io_submit(ctx, 1, cbs);
	if (ret != 1)
		tst_brk(TBROK | TERRNO, "io_submit() failed");
}

static void cleanup(void)
{
	if (ctx_initialized) {
		if (io_destroy(ctx) < 0)
			tst_res(TWARN | TERRNO, "io_destroy() failed");
	}

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void run(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct tcase *tc = &tcases[n];
	void *const to = tc->timeout ? tst_ts_get(tc->timeout) : bad_addr;
	sigset_t *const sigmask = tc->sigmask ? tc->sigmask : bad_addr;

	TEST(tv->io_pgetevents(*tc->ctx, tc->min_nr, tc->max_nr, tc->events, to,
			       sigmask));

	if (TST_RET == 1) {
		tst_res(TFAIL, "%s: io_pgetevents() passed unexpectedly",
			tc->name);
		return;
	}

	if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: io_pgetevents() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
		return;
	}

	tst_res(TPASS | TTERRNO, "%s: io_pgetevents() failed as expected",
		tc->name);
}

static struct tst_test test = {
	.min_kver = "4.18",
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
