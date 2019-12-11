// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * Tests basic error handling of the capget syscall.
 * 1) capget() fails with errno set to EFAULT if an invalid address
 * is given for header.
 * 2) capget() fails with errno set to EFAULT if an invalid address
 * is given for data
 * 3) capget() fails with errno set to EINVAL if an invalid value
 * is given for header->version
 * 4) capget() fails with errno set to EINVAL if header->pid < 0
 * 5) capget() fails with errno set to ESRCH if the process with
 *  pid, header->pid does not exist.
 */

#include <sys/types.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include <linux/capability.h>

static struct __user_cap_header_struct header, bad_version_header, bad_pid_header, unused_pid_header;
static struct __user_cap_data_struct data;

static struct tcase {
	cap_user_header_t headerp;
	cap_user_data_t datap;
	int exp_err;
	int reset_flag;
	char *message;
} tcases[] = {
	{NULL, &data, EFAULT, 0, "Test bad address header"},
	{&header, NULL, EFAULT, 0, "Test bad address data"},
	{&bad_version_header, &data, EINVAL, 1, "Test bad version"},
	{&bad_pid_header, &data, EINVAL, 0, "Test bad pid"},
	{&unused_pid_header, &data, ESRCH, 0, "Test unused pid"},
};

static void verify_capget(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);
	TEST(tst_syscall(__NR_capget, tc->headerp, tc->datap));
	if (TST_RET == 0) {
		tst_res(TFAIL, "capget() succeed unexpectedly");
		return;
	}
	if (TST_ERR == tc->exp_err)
		tst_res(TPASS | TTERRNO, "capget() failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "capget() expected %s got ",
			tst_strerrno(tc->exp_err));

	/*
	 * When an unsupported version value is specified, it will
	 * return the kernel preferred value of _LINUX_CAPABILITY_VERSION_?.
	 * Since linux 2.6.26, version 3 is default. We use it.
	 */
	if (tc->reset_flag) {
		if (tc->headerp->version == 0x20080522)
			tc->headerp->version = 0;
		else
			tst_res(TFAIL, "kernel doesn't return preferred linux"
				" capability version when using bad version");
	}
}

static void setup(void)
{
	unsigned int i, pid;

	pid = getpid();

	header.version = 0x19980330;
	header.pid = pid;

	bad_version_header.version = 0;
	bad_version_header.pid = pid;

	bad_pid_header.version = 0x19980330;
	bad_pid_header.pid = -1;

	unused_pid_header.version = 0x19980330;
	unused_pid_header.pid = tst_get_unused_pid();

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (!tcases[i].headerp)
			tcases[i].headerp = tst_get_bad_addr(NULL);
		if (!tcases[i].datap)
			tcases[i].datap = tst_get_bad_addr(NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_capget,
};
