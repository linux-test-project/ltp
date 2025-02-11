// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 * Copyright (c) Linux Test Project, 2017-2024
 */

/*\
 * Regression test for commit c9f838d104fe ("KEYS: fix
 * keyctl_set_reqkey_keyring() to not leak thread keyrings"), a.k.a.
 * CVE-2017-7472.  This bug could be used to exhaust kernel memory, though it
 * would take a while to do that and it would grind the test suite to a halt.
 * Instead we do a quick check for whether the existing thread keyring is
 * replaced when the default request-key destination is set to the thread
 * keyring.  It shouldn't be, but before the fix it was (and the old thread
 * keyring was leaked).
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void do_test(void)
{
	key_serial_t tid_keyring;

	TEST(keyctl(KEYCTL_GET_KEYRING_ID, KEY_SPEC_THREAD_KEYRING, 1));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to create thread keyring");
	tid_keyring = TST_RET;

	TEST(keyctl(KEYCTL_SET_REQKEY_KEYRING, KEY_REQKEY_DEFL_THREAD_KEYRING));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to set reqkey keyring");

	TEST(keyctl(KEYCTL_GET_KEYRING_ID, KEY_SPEC_THREAD_KEYRING, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to get thread keyring ID");
	if (TST_RET == tid_keyring)
		tst_res(TPASS, "thread keyring was not leaked");
	else
		tst_res(TFAIL, "thread keyring was leaked!");
}

static struct tst_test test = {
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-7472"},
		{"linux-git", "c9f838d104fe"},
		{}
	}
};
