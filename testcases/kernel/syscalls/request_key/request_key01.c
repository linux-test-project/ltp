/*
* Copyright (c) 2016 Fujitsu Ltd.
* Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* alone with this program.
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

#include "config.h"
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_KEYUTILS_H
# include <keyutils.h>
#endif

#include "tst_test.h"

#ifdef HAVE_KEYUTILS_H

static int key;

static void verify_request_key(void)
{

	TEST(request_key("keyring", "ltp", NULL, KEY_REQKEY_DEFL_DEFAULT));
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "request_key() failed");
		return;
	}

	if (TEST_RETURN != key)
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

#else

TST_TEST_TCONF("keyutils.h was missing at compilation");

#endif /* HAVE_LINUX_KEYCTL_H */
