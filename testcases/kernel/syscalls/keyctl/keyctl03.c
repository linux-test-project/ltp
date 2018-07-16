/*
 * Copyright (c) 2017 Fujitsu Ltd.
 *  Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
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
 * This regression test can crash the buggy kernel,
 * and the bug was fixed in:
 *
 *  commit f05819df10d7b09f6d1eb6f8534a8f68e5a4fe61
 *  Author: David Howells <dhowells@redhat.com>
 *  Date:   Thu Oct 15 17:21:37 2015 +0100
 *
 *  KEYS: Fix crash when attempt to garbage collect
 *        an uninstantiated keyring
 */

#include <errno.h>
#include <sys/types.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void do_test(void)
{
	key_serial_t key;

	key = add_key("user", "ltptestkey", "a", 1, KEY_SPEC_SESSION_KEYRING);
	if (key == -1)
		tst_brk(TBROK, "Failed to add key");

	request_key("keyring", "foo", "bar", KEY_SPEC_THREAD_KEYRING);

	TEST(keyctl(KEYCTL_UNLINK, key, KEY_SPEC_SESSION_KEYRING));
	if (TST_RET)
		tst_res(TFAIL | TTERRNO, "keyctl unlink failed");
	else
		tst_res(TPASS, "Bug not reproduced");
}

static struct tst_test test = {
	.test_all = do_test,
};
