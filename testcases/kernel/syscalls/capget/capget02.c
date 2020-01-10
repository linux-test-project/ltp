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

static pid_t unused_pid;
static struct __user_cap_header_struct *header;
static struct __user_cap_data_struct *data, *bad_data;

static struct tcase {
	int version;
	int pid;
	int exp_err;
	int flag;
	char *message;
} tcases[] = {
	{0x20080522, 0, EFAULT, 1, "Test bad address header"},
	{0x20080522, 0, EFAULT, 2, "Test bad address data"},
	{0, 0, EINVAL, 0, "Test bad version"},
	{0x20080522, -1, EINVAL, 0, "Test bad pid"},
	{0x20080522, 1, ESRCH, 0, "Test unused pid"},
};

static void verify_capget(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	header->version = tc->version;
	if (tc->pid == 1)
		header->pid = unused_pid;
	else
		header->pid = tc->pid;

	tst_res(TINFO, "%s", tc->message);

	/*
	 * header must not be NULL. data may be NULL only when the user is
	 * trying to determine the preferred capability version format
	 * supported by the kernel. So use tst_get_bad_addr() to get
	 * this error.
	 */
	TEST(tst_syscall(__NR_capget, tc->flag - 1 ? header : NULL,
				tc->flag - 2 ? data : bad_data));
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
	if (header->version != 0x20080522)
		tst_res(TFAIL, "kernel doesn't return preferred linux"
			" capability version when using bad version");
}

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
	bad_data = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_capget,
	.bufs = (struct tst_buffers []) {
		{&header, .size = sizeof(*header)},
		{&data, .size = 2 * sizeof(*data)},
		{&bad_data, .size = 2 * sizeof(*data)},
		{},
	}
};
