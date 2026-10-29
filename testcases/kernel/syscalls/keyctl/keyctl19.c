// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_MOVE`` of :manpage:`keyctl(2)` with same source and destination.
 *
 * [Algorithm]
 *
 * - move a key where the source keyring equals the destination keyring
 * - verify it is a successful no-op
 */

#include "keyctl_common.h"

#define RING_DESC	"ltpkeyctl19"
#define KEY_DESC	"k"
#define PAYLOAD		"payload"

static key_serial_t ring;
static key_serial_t key;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	ring = SAFE_NEW_RING(RING_DESC);
	key = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD), ring);
}

static void run(void)
{
	TST_EXP_PASS(keyctl(KEYCTL_MOVE, key, ring, ring, 0));
	TST_EXP_EQ_LI(keyctl(KEYCTL_SEARCH, ring, "user", KEY_DESC, 0), key);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "5.3",
};
