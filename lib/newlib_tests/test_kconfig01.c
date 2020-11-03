// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 *
 * Invalid boolean expression test.
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TPASS, "Test passed!");
}

static const char *kconfigs[] = {
	"CONFIG_EXT4_FS=m | CONFIG_MMU)",
	NULL
};

static struct tst_test test = {
	.test_all = do_test,
	.needs_kconfigs = kconfigs,
};
