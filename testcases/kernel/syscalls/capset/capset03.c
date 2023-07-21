// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) Linux Test Project, 2020-2023
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * capset() fails with errno set or EPERM if the new_Inheritable is
 * not a subset of old_Inheritable and old_Permitted without CAP_SETPCAP.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include <linux/capability.h>

#define CAP1 (1 << CAP_KILL)
#define CAP2 (CAP1 | 1 << CAP_NET_RAW)

static struct __user_cap_header_struct *header;
static struct __user_cap_data_struct *data;

static void verify_capset(void)
{
	tst_res(TINFO, "Test bad value data(when pI is not old pP or old pI without CAP_SETPCAP)");
	data[0].inheritable = CAP2;
	TST_EXP_FAIL(tst_syscall(__NR_capset, header, data), EPERM, "capset()");
}

static void setup(void)
{
	header->version = 0x20080522;

	data[0].effective = CAP1;
	data[0].permitted = CAP1;
	data[0].inheritable = CAP1;

	TEST(tst_syscall(__NR_capset, header, data));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "capset data failed");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_capset,
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		{&header, .size = sizeof(*header)},
		{&data, .size = 2 * sizeof(*data)},
		{},
	}
};
