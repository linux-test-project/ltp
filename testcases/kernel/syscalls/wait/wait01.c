// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 */
/*\
 * [Description]
 *
 * Check that if the calling process does not have any unwaited-for children
 * wait() will return ECHILD.
 */

#include <sys/wait.h>
#include <sys/types.h>
#include "tst_test.h"

static void verify_wait(void)
{
	TST_EXP_FAIL2(wait(NULL), ECHILD);
}

static struct tst_test test = {
	.test_all = verify_wait,
};
