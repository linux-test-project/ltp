// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_GET_SECURITY`` label retrieval of :manpage:`keyctl(2)`.
 *
 * ``KEYCTL_GET_SECURITY`` reads the LSM security label of a key.
 * When no label is set (no LSM enabled or the LSM does not label keys)
 * the operation returns 1 and an empty string.
 *
 * [Algorithm]
 *
 * - read the label of a valid key into a large buffer
 * - verify the return value is at least 1 and an empty string is returned when
 *   no label is set
 */

#include "keyctl_common.h"

#define KEY_DESC	"ltpkeyctl15"
#define PAYLOAD		"payload"
#define BUF_SIZE	256

static key_serial_t key;
static char buf[BUF_SIZE];

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD),
			   KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);
}

static void run(void)
{
	int rc;

	memset(buf, 0, sizeof(buf));

	rc = SAFE_KEYCTL(KEYCTL_GET_SECURITY, key, (unsigned long)buf, sizeof(buf), 0);

	if (rc == 0) {
		tst_res(TFAIL, "empty security label not NUL terminated");
		return;
	}

	if (buf[rc-1] != '\0') {
		tst_res(TFAIL, "security label not NUL terminated");
		return;
	}

	tst_res(TPASS, "Security label was NUL terminated");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
