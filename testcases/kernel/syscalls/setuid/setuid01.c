// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/* DESCRIPTION
 *	This test will verify that setuid(2) syscall basic functionality.
 *	setuid(2) returns a value of 0 and uid has been set successfully
 *	as a normal or super user.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "tst_test.h"
#include "compat_tst_16.h"

static void verify_setuid(void)
{
	uid_t uid;

	/* Set the effective user ID to the current real uid */
	uid = getuid();
	UID16_CHECK(uid, setuid);

	TEST(SETUID(uid));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "setuid(%d) failed", uid);
	else
		tst_res(TPASS, "setuid(%d) successfully", uid);
}

static struct tst_test test = {
	.test_all = verify_setuid,
};
