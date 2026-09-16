// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Negative and boundary test cases for ``KEYCTL_MOVE`` of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - ``KEYCTL_MOVE_EXCL`` fails with ``EEXIST`` when the destination already
 *   holds a matching key
 * - unknown flag bits are rejected with ``EINVAL``
 * - bogus key or keyring ids fail with ``ENOKEY``
 * - a plain key used as source or destination keyring fails with ``ENOTDIR``
 * - moving a key that is not linked in the source fails with ``ENOENT``
 * - moving a keyring into itself fails with ``EDEADLK`` from the keyring
 *   cycle detection
 * - moving into a keyring without Write permission fails with ``EACCES``
 */

#include "keyctl_common.h"

#define RING_A_DESC	"ltpkeyctl21_a"
#define RING_B_DESC	"ltpkeyctl21_b"
#define RING_C_DESC	"ltpkeyctl21_c"
#define KEY_A_DESC	"ka"
#define KEY_B_DESC	"kb"
#define PAYLOAD		"payload"

static key_serial_t ring_a, ring_b, ring_c, ring_no_write;
static key_serial_t key_a, key_b, key_excl;
static key_serial_t bogus_id = INT32_MAX;

static struct tcase {
	key_serial_t *keyid;
	key_serial_t *from;
	key_serial_t *to;
	unsigned int flags;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{ &key_a, &ring_a, &ring_b, KEYCTL_MOVE_EXCL,
	  EEXIST, "KEYCTL_MOVE_EXCL on existing key" },

	{ &key_a, &ring_a, &ring_b, 0x2,
	  EINVAL, "unknown flag bits" },

	{ &bogus_id, &ring_a, &ring_b, 0,
	  ENOKEY, "bogus key id" },

	{ &key_a, &bogus_id, &ring_b, 0,
	  ENOKEY, "bogus source keyring" },

	{ &key_a, &ring_a, &bogus_id, 0,
	  ENOKEY, "bogus destination keyring" },

	{ &key_a, &key_b, &ring_b, 0,
	  ENOTDIR, "plain key as source keyring" },

	{ &key_a, &ring_a, &key_b, 0,
	  ENOTDIR, "plain key as destination keyring" },

	{ &key_b, &ring_a, &ring_b, 0,
	  ENOENT, "key not linked in the source keyring" },

	{ &ring_c, &ring_b, &ring_c, 0,
	  EDEADLK, "keyring into itself" },

	{ &key_a, &ring_a, &ring_no_write, 0,
	  EACCES, "destination without Write permission" },
};

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	ring_a = SAFE_NEW_RING(RING_A_DESC);
	ring_b = SAFE_NEW_RING(RING_B_DESC);
	ring_c = SAFE_NEW_RING(RING_C_DESC);
	ring_no_write = SAFE_NEW_RING("ltpkeyctl21_nowrite");
	SAFE_KEYCTL(KEYCTL_SETPERM, ring_no_write, KEY_PERM_NO_WRITE, 0, 0);

	key_a = SAFE_NEW_USER_KEY(KEY_A_DESC, PAYLOAD, sizeof(PAYLOAD), ring_a);
	key_b = SAFE_NEW_USER_KEY(KEY_B_DESC, PAYLOAD, sizeof(PAYLOAD), ring_b);
	key_excl = SAFE_NEW_USER_KEY(KEY_A_DESC, PAYLOAD, sizeof(PAYLOAD), ring_b);

	/* Keep key_excl in session keyring */
	SAFE_KEYCTL(KEYCTL_LINK, key_excl, KEY_SPEC_SESSION_KEYRING, 0, 0);

	/* Link ring_c into ring_b for cycle test */
	SAFE_KEYCTL(KEYCTL_LINK, ring_c, ring_b, 0, 0);
}

static void verify_negative(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(keyctl(KEYCTL_MOVE, (unsigned long)*tc->keyid,
			    (unsigned long)*tc->from, (unsigned long)*tc->to,
			    tc->flags),
		     tc->exp_errno,
		     "KEYCTL_MOVE with %s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_negative,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "5.3",
};
