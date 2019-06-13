// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) Crackerjack Project., 2007
 *   Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 *   Copyright (c) 2017 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/* Porting from Crackerjack to LTP is done
 * by Masatake YAMATO <yamato@redhat.com>
 *
 * Description:
 * io_destroy(2) fails and returns -EINVAL if ctx is invalid.
 */

#include <errno.h>
#include <string.h>
#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

static void verify_io_destroy(void)
{
	io_context_t ctx;

	memset(&ctx, 0xff, sizeof(ctx));

	TEST(io_destroy(ctx));
	if (TST_RET == 0) {
		tst_res(TFAIL, "io_destroy() succeeded unexpectedly");
		return;
	}

	if (TST_RET == -EINVAL) {
		tst_res(TPASS,
			"io_destroy() failed as expected, returned -EINVAL");
	} else {
		tst_res(TFAIL, "io_destroy() failed unexpectedly, "
			"returned -%s expected -EINVAL",
			tst_strerrno(-TST_RET));
	}
}

static struct tst_test test = {
	.test_all = verify_io_destroy,
};

#else
	TST_TEST_TCONF("test requires libaio and it's development packages");
#endif
