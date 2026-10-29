// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test io_getevents invoked via syscall(2) with invalid ctx and expects it to
 * return EINVAL.
 */

#include <linux/aio_abi.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

static void run(void)
{
	aio_context_t ctx;

	memset(&ctx, 0, sizeof(ctx));
	TST_EXP_FAIL2(tst_syscall(__NR_io_getevents, ctx, 0, 0, NULL, NULL), EINVAL,
		"io_getevents syscall with invalid ctx");
}

static struct tst_test test = {
	.needs_kconfigs = (const char *[]) {
		"CONFIG_AIO=y",
		NULL
	},
	.test_all = run,
};
