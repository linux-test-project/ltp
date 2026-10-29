// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_GET_SECURITY`` truncated copy of :manpage:`keyctl(2)`.
 *
 * ``KEYCTL_GET_SECURITY`` returns the full length of the label, but unlike
 * ``KEYCTL_DESCRIBE`` the kernel performs a truncated copy when the user
 * buffer is too small.
 *
 * [Algorithm]
 *
 * Verify a one byte buffer receives exactly one byte while the full length is
 * still returned.
 */

#include "keyctl_common.h"

#define KEY_DESC	"ltpkeyctl16"
#define PAYLOAD		"payload"
#define BUF_SIZE	128

static key_serial_t key;
static long sec_len;
static char buf[BUF_SIZE];

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD),
			   KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);

	TEST(keyctl(KEYCTL_GET_SECURITY, key, (unsigned long)NULL, 0, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_GET_SECURITY failed");

	sec_len = TST_RET;
}

static void run(void)
{
	size_t i;

	memset(buf, 0xAA, sizeof(buf));
	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_GET_SECURITY, key,
				    (unsigned long)buf, 1, 0), sec_len);
	if (!TST_PASS)
		return;

	if (buf[0] == (char)0xAA) {
		tst_res(TFAIL, "nothing was copied into the buffer");
		return;
	}

	for (i = 1; i < sizeof(buf); i++) {
		if (buf[i] != (char)0xAA) {
			tst_res(TFAIL, "copy overran the buffer at offset %zu",
				i);
			return;
		}
	}

	tst_res(TPASS, "exactly one byte copied, full length %ld returned",
		sec_len);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
