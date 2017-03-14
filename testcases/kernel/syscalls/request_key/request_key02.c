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
* Test Name: request_key02
*
* Description:
* 1) request_key(2) fails if no matching key was found.
* 2) request_key(2) fails if A revoked key was found.
* 3) request_key(2) fails if An expired key was found.
*
* Expected Result:
* 1) request_key(2) should return -1 and set errno to ENOKEY.
* 2) request_key(2) should return -1 and set errno to EKEYREVOKED.
* 3) request_key(2) should return -1 and set errno to EKEYEXPIRED.
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

static int key1;
static int key2;
static int key3;

static struct test_case {
	const char *des;
	int exp_err;
	int *id;
} tcases[] = {
	{"ltp1", ENOKEY, &key1},
	{"ltp2", EKEYREVOKED, &key2},
	{"ltp3", EKEYEXPIRED, &key3}
};

static void verify_request_key(unsigned int n)
{
	struct test_case *tc = tcases + n;

	TEST(request_key("keyring", tc->des, NULL, *tc->id));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "request_key() succeed unexpectly");
		return;
	}

	if (TEST_ERRNO == tc->exp_err) {
		tst_res(TPASS | TTERRNO, "request_key() failed expectly");
		return;
	}

	tst_res(TFAIL | TTERRNO, "request_key() failed unexpectly, "
		"expected %s", tst_strerrno(tc->exp_err));
}

static int init_key(char *name, int cmd)
{
	int n;
	int sec = 1;

	n = add_key("keyring", name, NULL, 0, KEY_SPEC_THREAD_KEYRING);
	if (n == -1)
		tst_brk(TBROK | TERRNO, "add_key() failed");

	if (cmd == KEYCTL_REVOKE) {
		if (keyctl(cmd, n) == -1) {
			tst_brk(TBROK | TERRNO,	"failed to revoke a key");
		}
	}

	if (cmd == KEYCTL_SET_TIMEOUT) {
		if (keyctl(cmd, n, sec) == -1) {
			tst_brk(TBROK | TERRNO,
				"failed to set timeout for a key");
		}

		sleep(sec + 1);
	}

	return n;
}

static void setup(void)
{
	key1 = KEY_REQKEY_DEFL_DEFAULT;
	key2 = init_key("ltp2", KEYCTL_REVOKE);
	key3 = init_key("ltp3", KEYCTL_SET_TIMEOUT);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_request_key,
};

#else

TST_TEST_TCONF("keyutils.h was missing at compilation");

#endif /* HAVE_LINUX_KEYCTL_H */
