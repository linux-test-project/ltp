// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic io_pgetevents() test to receive 1 event successfully.
 */
#include "time64_variants.h"
#include "tst_test.h"
#include "tst_timer.h"
#include "lapi/io_pgetevents.h"

#ifdef HAVE_LIBAIO
static int fd;

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
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static void run(void)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct io_event events[1];
	struct iocb cb, *cbs[1];
	io_context_t ctx = 0;
	struct tst_ts to = tst_ts_from_ns(tv->ts_type, 10000);
	sigset_t sigmask;
	char data[4096];
	int ret;

	cbs[0] = &cb;
	sigemptyset(&sigmask);

	fd = SAFE_OPEN("io_pgetevents_file", O_RDWR | O_CREAT, 0644);
	io_prep_pwrite(&cb, fd, data, 4096, 0);

	TEST(io_setup(1, &ctx));
	if (TST_RET == -ENOSYS)
		tst_brk(TCONF | TRERRNO, "io_setup(): AIO not supported by kernel");
	if (TST_RET < 0)
		tst_brk(TBROK | TRERRNO, "io_setup() failed");

	ret = io_submit(ctx, 1, cbs);
	if (ret != 1)
		tst_brk(TBROK | TERRNO, "io_submit() failed");

	/* get the reply */
	ret = tv->io_pgetevents(ctx, 1, 1, events, tst_ts_get(&to), &sigmask);

	if (ret == 1)
		tst_res(TPASS, "io_pgetevents() works as expected");
	else
		tst_res(TFAIL | TERRNO, "io_pgetevents() failed");

	if (io_destroy(ctx) < 0)
		tst_brk(TBROK | TERRNO, "io_destroy() failed");

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.min_kver = "4.18",
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
	.needs_tmpdir = 1,
	.setup = setup,
};

#else
TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
