// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * CHANGES:
 *  2005/01/01: add an hint to a possible solution when test fails
 *              - Ricky Ng-Adam <rngadam@yahoo.com>
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
	{0x19980330, "Test on LINUX_CAPABILITY_VERSION_1"},
	{0x20071026, "Test on LINUX_CAPABILITY_VERSION_2"},
	{0x20080522, "Test on LINUX_CAPABILITY_VERSION_3"},
};

static void verify_capset(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);
	header->version = tc->version;
	header->pid = pid;

	if (tst_syscall(__NR_capget, header, data) == -1) {
		tst_res(TFAIL | TTERRNO, "capget() failed");
		return;
	}

	TEST(tst_syscall(__NR_capset, header, data));
	if (TST_RET == 0)
		tst_res(TPASS, "capset() returned %ld", TST_RET);
	else
		tst_res(TFAIL | TTERRNO, "Test Failed, capset() returned %ld", TST_RET);
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
