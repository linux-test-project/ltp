// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DESCRIBE`` of :manpage:`keyctl(2)` with too small buffer.
 *
 * [Algorithm]
 *
 * - describe a key with buffer smaller than full length, verify the buffer
 *   remains untouched and the return value is unchanged
 */

#include "keyctl_common.h"

#define BUF_SIZE	64

static key_serial_t key;
static long desc_len;
static char buf[BUF_SIZE];

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY("k", "payload", 7, KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);

	TEST(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)NULL, 0, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_DESCRIBE failed");

	desc_len = TST_RET;
}

static void run(void)
{
	size_t i;

	memset(buf, 0xAA, sizeof(buf));
	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)buf,
				    desc_len - 1, 0), desc_len);
	if (!TST_PASS)
		return;

	for (i = 0; i < sizeof(buf); i++) {
		if (buf[i] != (char)0xAA) {
			tst_res(TFAIL,
				"too small buffer was written to at offset %zu",
				i);
			return;
		}
	}

	tst_res(TPASS, "too small buffer was left untouched");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
