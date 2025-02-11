// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) 2017 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2009-2024
 * Ported by Manas Kumar Nayak maknayak@in.ibm.com>
 * Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

/*\
 * Tests the keyctl(2) syscall.
 *
 * Manipulate the kernel's key management facility.
 */

#include <errno.h>
#include <stdint.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void do_test(void)
{
	key_serial_t key;

	TEST(keyctl(KEYCTL_GET_KEYRING_ID, KEY_SPEC_USER_SESSION_KEYRING));
	if (TST_RET != -1)
		tst_res(TPASS, "KEYCTL_GET_KEYRING_ID succeeded");
	else
		tst_res(TFAIL | TTERRNO, "KEYCTL_GET_KEYRING_ID failed");

	for (key = INT32_MAX; key > INT32_MIN; key--) {
		TEST(keyctl(KEYCTL_READ, key));
		if (TST_RET == -1 && TST_ERR == ENOKEY)
			break;
	}

	TEST(keyctl(KEYCTL_REVOKE, key));
	if (TST_RET != -1) {
		tst_res(TFAIL, "KEYCTL_REVOKE succeeded unexpectedly");
		return;
	}

	if (TST_ERR != ENOKEY) {
		tst_res(TFAIL | TTERRNO, "KEYCTL_REVOKE failed unexpectedly");
		return;
	}

	tst_res(TPASS | TTERRNO, "KEYCTL_REVOKE failed as expected");
}

static struct tst_test test = {
	.test_all = do_test,
};
