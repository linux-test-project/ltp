// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2017 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * Test io_setup invoked via libaio:
 *
 * - io_setup succeeds if both nr_events and ctxp are valid.
 * - io_setup fails and returns -EINVAL if ctxp is not initialized to 0.
 * - io_setup fails and returns -EINVAL if nr_events is invalid.
 * - io_setup fails and returns -EFAULT if ctxp is NULL.
 * - io_setup fails and returns -EAGAIN if nr_events exceeds the limit
 *   1of available events.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

static void verify_failure(unsigned int nr, io_context_t *ctx, int init_val, long exp_err)
{
	if (ctx)
		memset(ctx, init_val, sizeof(*ctx));

	TEST(io_setup(nr, ctx));
	if (TST_RET == 0) {
		tst_res(TFAIL, "io_setup() passed unexpectedly");
		io_destroy(*ctx);
		return;
	}

	if (TST_RET == -exp_err) {
		tst_res(TPASS, "io_setup() failed as expected, returned -%s",
			tst_strerrno(exp_err));
	} else {
		tst_res(TFAIL, "io_setup() failed unexpectedly, returned -%s "
			"expected -%s", tst_strerrno(-TST_RET),
			tst_strerrno(exp_err));
	}
}

static void verify_success(unsigned int nr, io_context_t *ctx, int init_val)
{
	memset(ctx, init_val, sizeof(*ctx));

	TEST(io_setup(nr, ctx));
	if (TST_RET == -ENOSYS)
		tst_brk(TCONF | TRERRNO, "io_setup(): AIO not supported by kernel");
	if (TST_RET != 0) {
		tst_res(TFAIL, "io_setup() failed unexpectedly with %li (%s)",
			TST_RET, tst_strerrno(-TST_RET));
		return;
	}

	tst_res(TPASS, "io_setup() passed as expected");
	io_destroy(*ctx);
}

static void verify_io_setup(void)
{
	io_context_t ctx;
	unsigned int aio_max = 0;

	verify_success(1, &ctx, 0);
	verify_failure(1, &ctx, 1, EINVAL);
	verify_failure(-1, &ctx, 0, EINVAL);
	verify_failure(1, NULL, 0, EFAULT);

	if (!access("/proc/sys/fs/aio-max-nr", F_OK)) {
		SAFE_FILE_SCANF("/proc/sys/fs/aio-max-nr", "%u", &aio_max);
		verify_failure(aio_max + 1, &ctx, 0, EAGAIN);
	} else {
		tst_res(TCONF, "the aio-max-nr file did not exist");
	}
}

static struct tst_test test = {
	.test_all = verify_io_setup,
};

#else
	TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
