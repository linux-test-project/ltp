// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Negative test cases for ``KEYCTL_GET_SECURITY`` of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - get_security with bogus key id fails with ``ENOKEY``
 * - get_security without View permission fails with ``ENOKEY`` (the kernel
 *   falls back to looking for an auth token and returns its ``ENOKEY``)
 */

#include "keyctl_common.h"

#define BUF_SIZE	128

static key_serial_t key_no_view;
static key_serial_t bogus_id = INT32_MAX;
static char buf[BUF_SIZE];

static struct tcase {
	key_serial_t *keyid;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{ &bogus_id, ENOKEY, "bogus key id" },
	{ &key_no_view, ENOKEY, "key without View permission" },
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

	TST_EXP_FAIL2(keyctl(KEYCTL_GET_SECURITY, (unsigned long)*tc->keyid,
			     (unsigned long)buf, sizeof(buf), 0),
		      tc->exp_errno,
		      "KEYCTL_GET_SECURITY with %s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_negative,
	.tcnt = ARRAY_SIZE(tcases),
};
