// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 */

/*\
 * [Description]
 * This test case checks whether reboot(2) system call returns appropriate
 * error number for invalid flag parameter or invalid user.
 *
 * [Algorithm]
 * Execute system call with invaid flag parameter and then for invalid user
 * check return value and errno.
 *
 * [Restrictions]
 * For lib4 and lib5 reboot(2) system call is implemented as
 * int reboot(int magic, int magic2, int flag, void *arg); This test case
 * is written for int reboot(int flag); which is implemented under glibc
 * Therefore this testcase may not work under libc4 and libc5 libraries
 */

#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <pwd.h>
#include "tst_test.h"

#define INVALID_PARAMETER 100

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

static struct tcase {
	int flag;
	int exp_errno;
	const char *option_message;
} tcases[] = {
	{INVALID_PARAMETER, EINVAL, "INVALID_PARAMETER"},
	{LINUX_REBOOT_CMD_CAD_ON, EPERM, "LINUX_REBOOT_CMD_CAD_ON"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (n == 0)
		TST_EXP_FAIL(reboot(tc->flag),
			tc->exp_errno, "%s", tc->option_message);
	else {
		ltpuser = SAFE_GETPWNAM(nobody_uid);
		SAFE_SETEUID(ltpuser->pw_uid);

		TST_EXP_FAIL(reboot(tc->flag),
			tc->exp_errno, "%s", tc->option_message);

		SAFE_SETEUID(0);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
