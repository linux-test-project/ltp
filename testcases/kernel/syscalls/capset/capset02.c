// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 * Copyright (c) Linux Test Project, 2003-2023
 */

/*\
 * Verify that, capset(2) fails and sets errno to
 *
 * - EFAULT if an invalid address is given for header.
 * - EFAULT if an invalid address is given for data.
 * - EINVAL if an invalid value is given for header->version.
 * - EPERM if the new_Effective is not a subset of the new_Permitted.
 * - EPERM if the new_Permitted is not a subset of the old_Permitted.
 * - EPERM if the new_Inheritable is not a subset of the
 *   old_Inheritable and bounding set.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include <linux/capability.h>

#define CAP1 (1 << CAP_NET_RAW | 1 << CAP_CHOWN  | 1 << CAP_SETPCAP)
#define CAP2 (CAP1 | 1 << CAP_KILL)

static struct __user_cap_header_struct *header;
static struct __user_cap_data_struct *data;

static void *bad_addr;

static struct tcase {
	int version;
	int pid;
	int effective;
	int permitted;
	int inheritable;
	int exp_err;
	int flag;
	char *message;
} tcases[] = {
	{0x20080522, 0, CAP1, CAP1, CAP1, EFAULT, 1, "bad address header"},
	{0x20080522, 0, CAP1, CAP1, CAP1, EFAULT, 2, "bad address data"},
	{0, 0, CAP1, CAP1, CAP1, EINVAL, 0, "bad version"},
	{0x20080522, 0, CAP2, CAP1, CAP1, EPERM, 0, "bad value data(when pE is not in pP)"},
	{0x20080522, 0, CAP1, CAP2, CAP1, EPERM, 0, "bad value data(when pP is not in old pP)"},
	{0x20080522, 0, CAP1, CAP1, CAP2, EPERM, 0, "bad value data(when pI is not in bounding set or old pI)"},
};

static void verify_capset(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	header->version = tc->version;
	header->pid = tc->pid;

	data->effective = tc->effective;
	data->permitted = tc->permitted;
	data->inheritable = tc->inheritable;

	TST_EXP_FAIL(tst_syscall(__NR_capset, tc->flag - 1 ? header : bad_addr,
	                         tc->flag - 2 ? data : bad_addr),
	             tc->exp_err, "capset() with %s", tc->message);
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
	header->version = 0x20080522;
	data->effective = CAP1;
	data->permitted = CAP1;
	data->inheritable = CAP1;

	TEST(tst_syscall(__NR_capset, header, data));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "capset data failed");

	TEST(prctl(PR_CAPBSET_DROP, CAP_KILL));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "drop CAP_KILL failed");

	bad_addr = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_capset,
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		{&header, .size = sizeof(*header)},
		{&data, .size = 2 * sizeof(*data)},
		{},
	}
};
