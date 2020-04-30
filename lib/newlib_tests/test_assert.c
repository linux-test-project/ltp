// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

#include <stdio.h>
#include "tst_test.h"

static void do_test(void)
{
	TST_ASSERT_INT("/proc/self/oom_score", 0);
	TST_ASSERT_STR("/proc/self/comm", "test_assert");
	TST_ASSERT_FILE_INT("/proc/self/io", "read_bytes:", 0);
	TST_ASSERT_FILE_STR("/proc/self/status1", "State", "unexpected");
}

static struct tst_test test = {
	.test_all = do_test,
};
