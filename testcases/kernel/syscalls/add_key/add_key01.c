// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Porting from Crackerjack to LTP is done by
 * Manas Kumar Nayak maknayak@in.ibm.com>
 *
 * Basic test for the add_key() syscall.
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void verify_add_key(void)
{
	TEST(add_key("keyring", "wjkey", NULL, 0, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "add_key call failed");
	else
		tst_res(TPASS, "add_key call succeeded");
}

static struct tst_test test = {
	.test_all = verify_add_key,
};
