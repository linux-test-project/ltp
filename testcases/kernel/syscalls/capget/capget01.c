// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *  Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * Description:
 * This case tests capget() syscall whether works well on three versions.
 * Also, it checks the results buffer.
 */
#include <sys/types.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include <linux/capability.h>

static pid_t pid;
static struct __user_cap_header_struct *hdr;
static struct __user_cap_data_struct *data;

static struct tcase {
	int version;
	char *message;
} tcases[] = {
	{0x19980330, "Test on LINUX_CAPABILITY_VERSION_1"},
	{0x20071026, "Test on LINUX_CAPABILITY_VERSION_2"},
	{0x20080522, "Test on LINUX_CAPABILITY_VERSION_3"},
};

static void verify_capget(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);

	hdr->version = tc->version;
	hdr->pid = pid;
	TEST(tst_syscall(__NR_capget, hdr, data));
	if (TST_RET == 0)
		tst_res(TPASS, "capget() returned %ld", TST_RET);
	else
		tst_res(TFAIL | TTERRNO, "Test Failed, capget() returned %ld",
				TST_RET);

	if (data[0].effective & 1 << CAP_NET_RAW)
		tst_res(TFAIL, "capget() gets CAP_NET_RAW unexpectedly in pE");
	else
		tst_res(TPASS, "capget() doesn't get CAP_NET_RAW as expected in PE");
}

static void setup(void)
{
	pid = getpid();
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_capget,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_NET_RAW),
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&hdr, .size = sizeof(*hdr)},
		{&data, .size = 2 * sizeof(*data)},
		{},
	}
};
