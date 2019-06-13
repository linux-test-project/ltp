// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Test Name: request_key01
 *
 * Description:
 * The testcase checks basic functionality of the request_key(2).
 * request_key(2) asks the kernel to find a key which matches the
 * specified description. If successful, it attaches it to the
 * nominated keyring and returns its serial number.
 *
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static int key;

static void verify_request_key(void)
{

	TEST(request_key("keyring", "ltp", NULL, KEY_REQKEY_DEFL_DEFAULT));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "request_key() failed");
		return;
	}

	if (TST_RET != key)
		tst_res(TFAIL, "serial number mismatched");
	else
		tst_res(TPASS, "request_key() succeed");
}

static void setup(void)
{
	key = add_key("keyring", "ltp", NULL, 0, KEY_SPEC_THREAD_KEYRING);
	if (key == -1)
		tst_brk(TBROK | TERRNO, "add_key() failed");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_request_key,
};
