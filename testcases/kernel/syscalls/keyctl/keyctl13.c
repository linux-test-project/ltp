// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DESCRIBE`` of :manpage:`keyctl(2)` with NULL buffer and buflen 0.
 *
 * [Algorithm]
 *
 * - describe a key with NULL buffer and ``buflen = 0``, verify the full length
 *   is returned without writing
 */

#include <stdio.h>
#include <unistd.h>

#include "keyctl_common.h"

static key_serial_t key;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY("k", "payload", 7, KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);
}

static void run(void)
{
	char expected[64];
	long expected_len;

	expected_len = snprintf(expected, sizeof(expected),
				"user;%u;%u;%08x;k",
				getuid(), getgid(), KEY_PERM_SET) + 1;

	TST_EXP_VAL(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)NULL, 0, 0),
		    expected_len);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
