// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * Basic test for uname(2):
 * Calling uname() succeeded and got correct sysname.
 *
 */

#include <sys/utsname.h>
#include <errno.h>
#include <string.h>
#include "tst_test.h"

static void verify_uname(void)
{
	struct utsname un;

	memset(&un, 0, sizeof(un));

	TST_EXP_PASS(uname(&un));

	if (!TST_PASS)
		return;

	if (strcmp(un.sysname, "Linux")) {
		tst_res(TFAIL, "sysname is not Linux");
		return;
	}

	tst_res(TPASS, "sysname set to Linux");
}

static struct tst_test test = {
	.test_all = verify_uname,
};
