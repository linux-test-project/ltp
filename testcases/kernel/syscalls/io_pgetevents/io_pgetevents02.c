// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic io_pgetevents() test to check various failures.
 */
#include "tst_test.h"
#include "lapi/io_pgetevents.h"

#ifdef HAVE_LIBAIO
static sigset_t sigmask;
static struct io_event events[1];
static io_context_t ctx, invalid_ctx = 0;
static int fd, ctx_initialized;

static struct tcase {
	char *name;
	io_context_t *ctx;
	long min_nr;
	long max_nr;
	struct io_event *events;
	struct timespec *timeout;
	sigset_t *sigmask;
	int exp_errno;
} tcases[] = {
	{"invalid ctx", &invalid_ctx, 1, 1, events, NULL, &sigmask, EINVAL},
	{"invalid min_nr", &ctx, -1, 1, events, NULL, &sigmask, EINVAL},
	{"invalid max_nr", &ctx, 1, -1, events, NULL, &sigmask, EINVAL},
	{"invalid events", &ctx, 1, 1, NULL, NULL, &sigmask, EFAULT},
	{"invalid timeout", &ctx, 1, 1, events, (void *)(0xDEAD), &sigmask, EFAULT},
	{"invalid sigmask", &ctx, 1, 1, events, NULL, (void *)(0xDEAD), EFAULT},
};

static void setup(void)
{
	struct iocb cb, *cbs[1];
	char data[4096];
	int ret;

	cbs[0] = &cb;

	sigemptyset(&sigmask);

	fd = SAFE_OPEN("io_pgetevents_file", O_RDWR | O_CREAT, 0644);
	io_prep_pwrite(&cb, fd, data, 4096, 0);

	ret = io_setup(1, &ctx);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "io_setup() failed");

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
	struct tcase *tc = &tcases[n];

	TEST(io_pgetevents(*tc->ctx, tc->min_nr, tc->max_nr, tc->events,
			   tc->timeout, tc->sigmask));

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
	.setup = setup,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
