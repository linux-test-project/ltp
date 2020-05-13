// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic io_pgetevents() test to receive 1 event successfully.
 */
#include "tst_test.h"
#include "lapi/io_pgetevents.h"

#ifdef HAVE_LIBAIO
static int fd;

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	struct io_event events[1];
	struct iocb cb, *cbs[1];
	io_context_t ctx = 0;
	sigset_t sigmask;
	char data[4096];
	int ret, fd;

	cbs[0] = &cb;
	sigemptyset(&sigmask);

	fd = SAFE_OPEN("io_pgetevents_file", O_RDWR | O_CREAT, 0644);
	io_prep_pwrite(&cb, fd, data, 4096, 0);

	ret = io_setup(1, &ctx);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "io_setup() failed");

	ret = io_submit(ctx, 1, cbs);
	if (ret != 1)
		tst_brk(TBROK | TERRNO, "io_submit() failed");

	/* get the reply */
	ret = io_pgetevents(ctx, 1, 1, events, NULL, &sigmask);

	if (ret == 1)
		tst_res(TPASS, "io_pgetevents() works as expected");
	else
		tst_res(TFAIL | TERRNO, "io_pgetevents() failed");

	if (io_destroy(ctx) < 0)
		tst_brk(TBROK | TERRNO, "io_destroy() failed");
}

static struct tst_test test = {
	.min_kver = "4.18",
	.test_all = run,
	.needs_tmpdir = 1,
	.cleanup = cleanup,
};

#else
TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
