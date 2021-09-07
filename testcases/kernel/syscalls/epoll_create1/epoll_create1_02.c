// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Author: Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Verify that epoll_create1 returns -1 and set errno to EINVAL with an invalid
 * value specified in flags.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

static struct test_case_t {
	int flags;
	int exp_err;
	const char *desc;
} tc[] = {
	{-1, EINVAL, "-1"},
	{EPOLL_CLOEXEC + 1, EINVAL, "EPOLL_CLOEXEC+1"}
};

static void run(unsigned int n)
{
	TST_EXP_FAIL(tst_syscall(__NR_epoll_create1, tc[n].flags),
		     tc[n].exp_err, "epoll_create1(%s)", tc[n].desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
};
