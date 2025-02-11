// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test io_cancel invoked via syscall(2) with one of pointers set to invalid
 * address and expects it to return EFAULT.
 */

#include <linux/aio_abi.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

static void run(void)
{
	aio_context_t ctx;

	memset(&ctx, 0, sizeof(ctx));
	TST_EXP_FAIL(tst_syscall(__NR_io_cancel, ctx, NULL, NULL), EFAULT);
}

static struct tst_test test = {
	.needs_kconfigs = (const char *[]) {
		"CONFIG_AIO=y",
		NULL
	},
	.test_all = run,
};
