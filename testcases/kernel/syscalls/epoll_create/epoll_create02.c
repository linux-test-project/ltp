// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Author: Xie Ziyao <ziyaoxie@outlook.com>
 */

/*\
 * [Description]
 *
 * Verify that epoll_create returns -1 and set errno to EINVAL if size is not
 * greater than zero.
 */

#include <sys/epoll.h>

#include "tst_test.h"
#include "lapi/epoll.h"
#include "lapi/syscalls.h"

static struct test_case_t {
	int size;
	int exp_err;
} tc[] = {
	{0, EINVAL},
	{-1, EINVAL}
};

static void run(unsigned int n)
{
	TST_EXP_FAIL(tst_syscall(__NR_epoll_create, tc[n].size),
		     tc[n].exp_err, "create(%d)", tc[n].size);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
};
