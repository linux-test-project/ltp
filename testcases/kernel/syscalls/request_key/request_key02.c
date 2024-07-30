// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Fujitsu Ltd.
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Linux Test Project, 2017-2024
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic request_key(2) failure checking. request_key(2) should return -1 and
 * set expected errno:
 *
 * 1. ENOKEY (no matching key was found),
 * 2. EKEYREVOKED (revoked key was found)
 * 3. EKEYEXPIRED (expired key was found)
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

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

	TST_EXP_FAIL2(request_key("keyring", tc->des, NULL, *tc->id),
		      tc->exp_err, "request_key(\"keyring\", %s, NULL, %d)",
		      tc->des, *tc->id);
}

static int init_key(char *name, int cmd)
{
	int n;
	int sec = 1;

	n = add_key("keyring", name, NULL, 0, KEY_SPEC_THREAD_KEYRING);
	if (n == -1)
		tst_brk(TBROK | TERRNO, "add_key() failed");

	if (cmd == KEYCTL_REVOKE) {
		if (keyctl(cmd, n) == -1)
			tst_brk(TBROK | TERRNO,	"failed to revoke a key");
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
