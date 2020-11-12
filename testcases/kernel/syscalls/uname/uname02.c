// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Basic test for uname():
 * Calling uname() with invalid buf got EFAULT.
 *
 */

#include <errno.h>
#include <sys/utsname.h>
#include "tst_test.h"

static void *bad_addr;

static void verify_uname(void)
{
	TEST(uname(bad_addr));
	if (TST_RET == 0) {
		tst_res(TFAIL, "uname() succeed when failure expected");
		return;
	}

	if (TST_RET != -1) {
		tst_res(TFAIL, "Invalid uname() return value %ld", TST_RET);
		return;
	}

	if (TST_ERR == EFAULT)
		tst_res(TPASS, "uname() got EFAULT as expected");
	else
		tst_res(TFAIL | TTERRNO, "uname() failed unexpectedly");

}

static void setup(void)
{
	bad_addr = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.test_all = verify_uname,
	.setup = setup,
};
