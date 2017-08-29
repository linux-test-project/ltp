/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) 2017 Fujitsu Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Description: This tests the keyctl() syscall
 *		Manipulate the kernel's key management facility
 *
 * Ported by Manas Kumar Nayak maknayak@in.ibm.com>
 * Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 */

#include <errno.h>
#include <stdint.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void do_test(void)
{
	key_serial_t key;

	TEST(keyctl(KEYCTL_GET_KEYRING_ID, KEY_SPEC_USER_SESSION_KEYRING));
	if (TEST_RETURN != -1)
		tst_res(TPASS, "KEYCTL_GET_KEYRING_ID succeeded");
	else
		tst_res(TFAIL | TTERRNO, "KEYCTL_GET_KEYRING_ID failed");

	for (key = INT32_MAX; key > INT32_MIN; key--) {
		TEST(keyctl(KEYCTL_READ, key));
		if (TEST_RETURN == -1 && TEST_ERRNO == ENOKEY)
			break;
	}

	TEST(keyctl(KEYCTL_REVOKE, key));
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "KEYCTL_REVOKE succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO != ENOKEY) {
		tst_res(TFAIL | TTERRNO, "KEYCTL_REVOKE failed unexpectedly");
		return;
	}

	tst_res(TPASS | TTERRNO, "KEYCTL_REVOKE failed as expected");
}

static struct tst_test test = {
	.test_all = do_test,
};
