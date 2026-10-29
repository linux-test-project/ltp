// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 */

/*\
 * Test for various EINVAL error.
 *
 * - oldfd is equal to newfd without using O_CLOEXEC flag
 * - oldfd is equal to newfd with using O_CLOEXEC flag
 * - flags contain an invalid value
 */

#define _GNU_SOURCE
#define INVALID_FLAG -1

#include <errno.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static int old_fd = 3;
static int new_fd = 5;

static struct tcase {
	int *oldfd;
	int *newfd;
	int flags;
} tcases[] = {
	{&old_fd, &old_fd, O_CLOEXEC},
	{&old_fd, &old_fd, 0},
	{&old_fd, &new_fd, INVALID_FLAG}
};

static void run(unsigned int i)
{
	struct tcase *tc = tcases + i;

	TST_EXP_FAIL2(dup3(*tc->oldfd, *tc->newfd, tc->flags), EINVAL,
		"dup3(%d, %d, %d)", *tc->oldfd, *tc->newfd, tc->flags);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
};
