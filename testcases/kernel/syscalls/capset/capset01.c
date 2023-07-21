// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * 2005/01/01: add an hint to a possible solution when test fails
 * Ricky Ng-Adam <rngadam@yahoo.com>
 *
 * Copyright (c) Linux Test Project, 2003-2023
 */

#include <sys/types.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include <linux/capability.h>

static pid_t pid;
static struct __user_cap_header_struct *header;
static struct __user_cap_data_struct *data;
static struct tcase {
	int version;
	char *message;
} tcases[] = {
	{0x19980330, "LINUX_CAPABILITY_VERSION_1"},
	{0x20071026, "LINUX_CAPABILITY_VERSION_2"},
	{0x20080522, "LINUX_CAPABILITY_VERSION_3"},
};

static void verify_capset(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	header->version = tc->version;
	header->pid = pid;

	TEST(tst_syscall(__NR_capget, header, data));
	if (TST_RET == -1)
	      tst_brk(TFAIL | TTERRNO, "capget() failed");

	TST_EXP_PASS(tst_syscall(__NR_capset, header, data),
	             "capset() with %s", tc->message);
}

static void setup(void)
{
	pid = getpid();
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_capset,
	.bufs = (struct tst_buffers []) {
		{&header, .size = sizeof(*header)},
		{&data, .size = 2 * sizeof(*data)},
		{},
	}
};
