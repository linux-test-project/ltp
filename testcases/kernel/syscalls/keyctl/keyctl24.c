// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Negative test cases for ``KEYCTL_RESTRICT_KEYRING`` of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - restrict with ``NULL`` type and non-NULL restriction fails with ``EINVAL``
 * - restrict with non-NULL type and ``NULL`` restriction fails with ``EINVAL``
 * - restrict on a non-keyring key fails with ``ENOTDIR``
 * - restrict with unknown key type fails with ``ENOKEY``
 * - restrict with key type having no restriction lookup fails with ``ENOENT``
 * - restrict with asymmetric type and invalid restriction string fails with
 *   ``EINVAL``
 * - restrict with asymmetric type and bogus key serial fails with ``ENOKEY``
 * - restrict an already restricted keyring fails with ``EEXIST``
 * - restrict with self-referencing key_or_keyring chain fails with ``EDEADLK``
 * - restrict without Setattr permission fails with ``EACCES``
 */

#include <stdio.h>

#include "keyctl_common.h"

static key_serial_t ring_reject, ring_no_setattr;
static key_serial_t user_key;
static int asym_supported;

/*
 * A tcase with .id = NULL asks verify_negative() to allocate a fresh
 * unrestricted keyring for the case and unlink it afterwards, so that a
 * false-positive result cannot restrict a ring shared with later cases.
 * .self_cycle asks for a "key_or_keyring:<fresh>:chain" restriction string
 * pointing back at that fresh ring, needed for the EDEADLK check.
 */
static struct tcase {
	key_serial_t *id;
	int self_cycle;
	const char *type;
	const char *restriction;
	int exp_errno;
	int needs_asym;
	const char *desc;
} tcases[] = {
	{
		.restriction = "builtin_trusted",
		.exp_errno = EINVAL,
		.desc = "NULL type and non-NULL restriction",
	},
	{
		.type = "asymmetric",
		.exp_errno = EINVAL,
		.desc = "non-NULL type and NULL restriction",
	},
	{
		.id = &user_key,
		.exp_errno = ENOTDIR,
		.desc = "non-keyring key",
	},
	{
		.type = "nosuchtype",
		.restriction = "builtin_trusted",
		.exp_errno = ENOKEY,
		.desc = "unknown key type",
	},
	{
		.type = "user",
		.restriction = "builtin_trusted",
		.exp_errno = ENOENT,
		.desc = "key type having no lookup_restriction",
	},
	{
		.type = "asymmetric",
		.restriction = "bogus",
		.exp_errno = EINVAL,
		.needs_asym = 1,
		.desc = "asymmetric with invalid restriction string",
	},
	{
		.type = "asymmetric",
		.restriction = "key_or_keyring:2147483647",
		.exp_errno = ENOKEY,
		.needs_asym = 1,
		.desc = "asymmetric with bogus key serial",
	},
	{
		.id = &ring_reject,
		.exp_errno = EEXIST,
		.desc = "already restricted keyring",
	},
	{
		.self_cycle = 1,
		.type = "asymmetric",
		.exp_errno = EDEADLK,
		.needs_asym = 1,
		.desc = "self-referencing key_or_keyring chain",
	},
	{
		.id = &ring_no_setattr,
		.exp_errno = EACCES,
		.desc = "keyring without Setattr permission",
	},
};

static void setup(void)
{
	key_serial_t probe_ring;

	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	ring_reject = SAFE_NEW_RING("ltpkeyctl24_reject");
	ring_no_setattr = SAFE_NEW_RING("ltpkeyctl24_no_setattr");
	SAFE_KEYCTL(KEYCTL_SETPERM, ring_no_setattr, KEY_PERM_NO_SETATTR, 0, 0);

	/* Permanently restrict ring_reject with reject-all for EEXIST test */
	SAFE_KEYCTL(KEYCTL_RESTRICT_KEYRING, ring_reject, 0, 0, 0);

	user_key = SAFE_NEW_USER_KEY("k", "payload", 7, KEY_SPEC_PROCESS_KEYRING);

	/* Probe asymmetric support on a throwaway ring so we cannot poison
	 * any ring reused later by the tcase table.
	 */
	probe_ring = SAFE_NEW_RING("ltpkeyctl24_probe");
	asym_supported = is_asym_supported(probe_ring);
	keyctl(KEYCTL_UNLINK, probe_ring, KEY_SPEC_PROCESS_KEYRING, 0, 0);
}

static void verify_negative(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	key_serial_t id, fresh_ring = 0;
	const char *restriction = tc->restriction;
	char cycle_buf[64];

	if (tc->needs_asym && !asym_supported) {
		tst_res(TCONF, "asymmetric key type not supported");
		return;
	}

	if (tc->id) {
		id = *tc->id;
	} else {
		fresh_ring = SAFE_NEW_RING("ltpkeyctl24_fresh");
		id = fresh_ring;
		if (tc->self_cycle) {
			snprintf(cycle_buf, sizeof(cycle_buf),
				 "key_or_keyring:%d:chain", fresh_ring);
			restriction = cycle_buf;
		}
	}

	TST_EXP_FAIL(keyctl(KEYCTL_RESTRICT_KEYRING, (unsigned long)id,
			    (unsigned long)tc->type,
			    (unsigned long)restriction, 0),
		     tc->exp_errno,
		     "KEYCTL_RESTRICT_KEYRING with %s", tc->desc);

	if (fresh_ring)
		keyctl(KEYCTL_UNLINK, fresh_ring, KEY_SPEC_PROCESS_KEYRING,
		       0, 0);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_negative,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "4.12",
};
