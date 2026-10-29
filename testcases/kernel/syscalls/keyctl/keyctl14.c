// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Negative test cases for ``KEYCTL_DESCRIBE`` of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - describe with bogus key id fails with ``ENOKEY``
 * - describe without View permission fails with ``EACCES``
 */

#include "keyctl_common.h"

#define BUF_SIZE	64

static key_serial_t key_no_view;
static key_serial_t bogus_id = INT32_MAX;
static char buf[BUF_SIZE];

static struct tcase {
	key_serial_t *keyid;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{ &bogus_id, ENOKEY, "bogus key id" },
	{ &key_no_view, EACCES, "key without View permission" },
};

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key_no_view = SAFE_NEW_USER_KEY("k_no_view", "payload", 7, KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key_no_view, KEY_PERM_NO_VIEW, 0, 0);
}

static void verify_negative(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL2(keyctl(KEYCTL_DESCRIBE, (unsigned long)*tc->keyid,
			     (unsigned long)buf, sizeof(buf), 0),
		      tc->exp_errno,
		      "KEYCTL_DESCRIBE with %s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_negative,
	.tcnt = ARRAY_SIZE(tcases),
};
