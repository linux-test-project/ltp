// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_RESTRICT_KEYRING`` reject-all of :manpage:`keyctl(2)`,
 * added in Linux 4.12.
 *
 * [Algorithm]
 *
 * - restrict a keyring with ``NULL`` type and ``NULL`` restriction (reject all links)
 * - verify subsequent links and add_key fail with ``EPERM``
 */

#include "keyctl_common.h"

#define PAYLOAD		"payload"

static key_serial_t ring_reject;
static key_serial_t user_key;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	ring_reject = SAFE_NEW_RING("ltpkeyctl22_reject");
	user_key = SAFE_NEW_USER_KEY("k", PAYLOAD, sizeof(PAYLOAD),
				KEY_SPEC_PROCESS_KEYRING);
}

static void run(void)
{
	TEST(keyctl(KEYCTL_RESTRICT_KEYRING, ring_reject, 0, 0, 0));
	if (TST_RET == 0)
		tst_res(TPASS, "KEYCTL_RESTRICT_KEYRING with NULL type and restriction passed");
	else if (TST_RET == -1 && TST_ERR == EEXIST)
		tst_res(TPASS, "KEYCTL_RESTRICT_KEYRING reject-all already active");
	else
		tst_res(TFAIL | TTERRNO, "KEYCTL_RESTRICT_KEYRING reject-all failed");

	TST_EXP_FAIL(keyctl(KEYCTL_LINK, user_key, ring_reject, 0, 0), EPERM,
		     "KEYCTL_LINK on reject-all restricted keyring");

	TST_EXP_FAIL2(add_key("user", "k_rej", PAYLOAD, sizeof(PAYLOAD),
			      ring_reject), EPERM,
		      "add_key on reject-all restricted keyring");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "4.12",
	.iterations = 2,
};
