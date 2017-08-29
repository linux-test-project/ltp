/*
 * Copyright (c) 2017 Google, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
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
	if (TEST_RETURN < 0)
		tst_brk(TBROK | TTERRNO, "failed to create thread keyring");
	tid_keyring = TEST_RETURN;

	TEST(keyctl(KEYCTL_SET_REQKEY_KEYRING, KEY_REQKEY_DEFL_THREAD_KEYRING));
	if (TEST_RETURN < 0)
		tst_brk(TBROK | TTERRNO, "failed to set reqkey keyring");

	TEST(keyctl(KEYCTL_GET_KEYRING_ID, KEY_SPEC_THREAD_KEYRING, 0));
	if (TEST_RETURN < 0)
		tst_brk(TBROK | TTERRNO, "failed to get thread keyring ID");
	if (TEST_RETURN == tid_keyring)
		tst_res(TPASS, "thread keyring was not leaked");
	else
		tst_res(TFAIL, "thread keyring was leaked!");
}

static struct tst_test test = {
	.test_all = do_test,
};
