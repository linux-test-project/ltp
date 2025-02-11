// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Author: Madhu T L <madhu.tarikere@wipro.com>
 */

/*\
 * Verify that syslog(2) system call fails with appropriate error number:
 *
 * 1. EINVAL -- invalid type/command
 * 2. EFAULT -- buffer outside program's accessible address space
 * 3. EINVAL -- NULL buffer argument
 * 4. EINVAL -- length argument set to negative value
 * 5. EINVAL -- console level less than 0
 * 6. EINVAL -- console level greater than 8
 * 7. EPERM -- non-root user
 */

#include <errno.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_macros.h"

#define syslog(arg1, arg2, arg3) tst_syscall(__NR_syslog, arg1, arg2, arg3)

static char buf;
static struct passwd *ltpuser;

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
}

static void setup_nonroot(void)
{
	SAFE_SETEGID(ltpuser->pw_gid);
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void cleanup_nonroot(void)
{
	SAFE_SETEUID(0);
}

static struct tcase {
	int type;
	char *buf;
	int len;
	int exp_errno;
	char *desc;
} tcases[] = {
	{100, &buf, 0, EINVAL, "invalid type/command"},
	{2, NULL, 0, EINVAL, "NULL buffer argument"},
	{3, &buf, -1, EINVAL, "negative length argument"},
	{8, &buf, -1, EINVAL, "console level less than 0"},
	{8, &buf, 9, EINVAL, "console level greater than 8"},
	{2, &buf, 0, EPERM, "non-root user"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (n == ARRAY_SIZE(tcases)-1)
		setup_nonroot();

	TST_EXP_FAIL(syslog(tc->type, tc->buf, tc->len), tc->exp_errno,
		"syslog() with %s", tc->desc);

	if (n == ARRAY_SIZE(tcases)-1)
		cleanup_nonroot();
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
};
