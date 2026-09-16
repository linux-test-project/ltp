// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DESCRIBE`` of :manpage:`keyctl(2)` with exact buffer size.
 *
 * [Algorithm]
 *
 * Describe a key into a buffer sized exactly to the full string length,
 * verify full string copy including terminating NUL byte.
 */

#include "keyctl_common.h"

static key_serial_t key;
static long desc_len;
static char *buf;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY("k", "payload", 7, KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);

	TEST(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)NULL, 0, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_DESCRIBE failed");

	desc_len = TST_RET;
	buf = SAFE_MALLOC(desc_len);
}

static void cleanup(void)
{
	free(buf);
}

static void run(void)
{
	memset(buf, 0xff, desc_len);
	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)buf,
				    desc_len, 0), desc_len);
	if (!TST_PASS)
		return;

	if (buf[desc_len - 1] != '\0') {
		tst_res(TFAIL, "description is not NUL terminated");
		return;
	}

	tst_res(TPASS, "full description including NUL fits exact %ld byte buffer",
		desc_len);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
