// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 */

/*
 * Regression test for commit 237bbd29f7a0 ("KEYS: prevent creating a different
 * user's keyrings").  The bug allowed any random user to create a keyring named
 * "_uid.$UID" (or "_uid_ses.$UID"), and it would become the user keyring (or
 * user session keyring) for user $UID, provided that it hadn't already been
 * created.
 *
 * This test must be run as root so that it has permission to switch to another
 * user ID and check whether the keyrings are wrong.  However, the underlying
 * bug is actually reachable/exploitable by a non-root user.
 */

#include <errno.h>
#include <pwd.h>
#include <stdio.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static key_serial_t create_keyring(const char *description)
{
	TEST(add_key("keyring", description, NULL, 0,
		     KEY_SPEC_PROCESS_KEYRING));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"unable to create keyring '%s'", description);
	}
	return TST_RET;
}

static key_serial_t get_keyring_id(key_serial_t special_id)
{
	TEST(keyctl(KEYCTL_GET_KEYRING_ID, special_id, 1));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"unable to get ID of keyring %d", special_id);
	}
	return TST_RET;
}

static void do_test(void)
{
	uid_t uid = 1;
	char description[32];
	key_serial_t fake_user_keyring;
	key_serial_t fake_user_session_keyring;

	/*
	 * We need a user to forge the keyrings for.  But the bug is not
	 * reproducible for a UID which already has its keyrings, so find an
	 * unused UID.  Note that it would be better to directly check for the
	 * presence of the UID's keyrings than to search the passwd file.
	 * However, that's not easy to do given that even if we assumed the UID
	 * temporarily to check, KEYCTL_GET_KEYRING_ID for the user and user
	 * session keyrings will create them rather than failing (even if the
	 * 'create' argument is 0).
	 */
	while (getpwuid(uid))
		uid++;

	sprintf(description, "_uid.%u", uid);
	fake_user_keyring = create_keyring(description);
	sprintf(description, "_uid_ses.%u", uid);
	fake_user_session_keyring = create_keyring(description);

	SAFE_SETUID(uid);

	if (fake_user_keyring == get_keyring_id(KEY_SPEC_USER_KEYRING))
		tst_brk(TFAIL, "created user keyring for another user");

	if (fake_user_session_keyring ==
	    get_keyring_id(KEY_SPEC_USER_SESSION_KEYRING))
		tst_brk(TFAIL, "created user session keyring for another user");

	tst_res(TPASS, "expectedly could not create another user's keyrings");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "237bbd29f7a0"},
		{}
	}
};
