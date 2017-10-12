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
 * Regression test for commit e645016abc80 ("KEYS: fix writing past end of
 * user-supplied buffer in keyring_read()").
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static key_serial_t add_test_key(const char *description)
{
	TEST(add_key("user", description, "payload", 7,
		     KEY_SPEC_PROCESS_KEYRING));
	if (TEST_RETURN < 0)
		tst_brk(TBROK | TTERRNO, "Failed to add test key");
	return TEST_RETURN;
}

static void do_test(void)
{
	key_serial_t key_ids[2];
	key_serial_t key_id_1 = add_test_key("key1");
	key_serial_t key_id_2 = add_test_key("key2");

	memset(key_ids, 0, sizeof(key_ids));
	TEST(keyctl(KEYCTL_READ, KEY_SPEC_PROCESS_KEYRING,
		    (char *)key_ids, sizeof(key_serial_t)));
	if (TEST_RETURN < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_READ failed");

	if (key_ids[1] != 0)
		tst_brk(TFAIL, "KEYCTL_READ overran the buffer");

	if (key_ids[0] == 0)
		tst_brk(TBROK, "KEYCTL_READ didn't read anything");

	if (key_ids[0] != key_id_1 && key_ids[0] != key_id_2)
		tst_brk(TBROK, "KEYCTL_READ didn't return correct key ID");

	if (TEST_RETURN != sizeof(key_serial_t)) {
		tst_brk(TBROK, "KEYCTL_READ returned %ld but expected %zu",
			TEST_RETURN, sizeof(key_serial_t));
	}

	tst_res(TPASS, "KEYCTL_READ didn't overrun the buffer");
}

static struct tst_test test = {
	.test_all = do_test,
};
