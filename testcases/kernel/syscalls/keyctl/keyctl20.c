// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_MOVE`` displacement of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * Move a key without ``KEYCTL_MOVE_EXCL`` into a destination keyring that
 * already contains a matching key, verify it displaces the existing key.
 */

#include "keyctl_common.h"

#define RING_A_DESC	"ltpkeyctl20_a"
#define RING_B_DESC	"ltpkeyctl20_b"
#define KEY_DESC	"k"
#define PAYLOAD		"payload"

static key_serial_t ring_a, ring_b;
static key_serial_t key_a, key_excl;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	ring_a = SAFE_NEW_RING(RING_A_DESC);
	ring_b = SAFE_NEW_RING(RING_B_DESC);

	key_a = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD), ring_a);
	key_excl = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD), ring_b);

	/* Keep key_excl in session keyring so displacement does not destroy it */
	SAFE_KEYCTL(KEYCTL_LINK, key_excl, KEY_SPEC_SESSION_KEYRING, 0, 0);
}

static void run(void)
{
	SAFE_KEYCTL(KEYCTL_LINK, key_a, ring_a, 0, 0);
	TST_EXP_PASS(keyctl(KEYCTL_MOVE, key_a, ring_a, ring_b, 0));
	TST_EXP_EQ_LI(keyctl(KEYCTL_SEARCH, ring_b, "user", KEY_DESC, 0), key_a);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "5.3",
};
