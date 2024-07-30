// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 * Copyright (c) Linux Test Project, 2018-2024
 */

/*\
 * [Description]
 *
 * Regression test for commit 4dca6ea1d943 ("KEYS: add missing permission check
 * for request_key() destination"), or CVE-2017-17807.  This bug allowed adding
 * a key to a keyring given only Search permission to that keyring, rather than
 * the expected Write permission.
 *
 * We test for the bug by trying to add a negatively instantiated key, since
 * adding a negatively instantiated key using the bug was easy whereas adding a
 * positively instantiated key required exploiting a race condition.
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void do_test(void)
{
	key_serial_t keyid;
	int saved_errno;

	TEST(keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to join new session keyring");

	TEST(keyctl(KEYCTL_SETPERM, KEY_SPEC_SESSION_KEYRING,
		    KEY_POS_SEARCH|KEY_POS_READ|KEY_POS_VIEW));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"failed to set permissions on session keyring");
	}

	TEST(keyctl(KEYCTL_SET_REQKEY_KEYRING,
		    KEY_REQKEY_DEFL_SESSION_KEYRING));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"failed to set request-key default keyring");
	}

	TEST(keyctl(KEYCTL_READ, KEY_SPEC_SESSION_KEYRING,
		    &keyid, sizeof(keyid)));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to read from session keyring");
	if (TST_RET != 0)
		tst_brk(TBROK, "session keyring is not empty");

	TEST(request_key("user", "desc", "callout_info", 0));
	if (TST_RET != -1)
		tst_brk(TBROK, "request_key() unexpectedly succeeded");
	saved_errno = TST_ERR;

	TEST(keyctl(KEYCTL_READ, KEY_SPEC_SESSION_KEYRING,
		    &keyid, sizeof(keyid)));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to read from session keyring");
	if (TST_RET != 0)
		tst_brk(TFAIL, "added key to keyring without permission");

	TST_ERR = saved_errno;
	if (TST_ERR == EACCES) {
		tst_res(TPASS, "request_key() failed with EACCES as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
			"request_key() failed with unexpected error code");
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-17807"},
		{"linux-git", "4dca6ea1d943"},
		{}
	}
};
