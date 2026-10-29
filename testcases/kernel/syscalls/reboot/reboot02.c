// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Linux Test Project, 2009-2021
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 */

/*\
 * Test whether libc wrapper of reboot(2) system call returns appropriate
 * error number for invalid cmd parameter or invalid user.
 */

#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <pwd.h>
#include "tst_test.h"

#define INVALID_CMD 100
#define CMD_DESC(x) .cmd = x, .desc = #x

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

static struct tcase {
	int cmd;
	const char *desc;
	int exp_errno;
} tcases[] = {
	{CMD_DESC(INVALID_CMD), EINVAL},
	{CMD_DESC(LINUX_REBOOT_CMD_CAD_ON), EPERM},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (n == 0)
		TST_EXP_FAIL(reboot(tc->cmd),
			tc->exp_errno, "%s", tc->desc);
	else {
		ltpuser = SAFE_GETPWNAM(nobody_uid);
		SAFE_SETEUID(ltpuser->pw_uid);

		TST_EXP_FAIL(reboot(tc->cmd),
			tc->exp_errno, "%s", tc->desc);

		SAFE_SETEUID(0);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
