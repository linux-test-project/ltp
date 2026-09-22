// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DH_COMPUTE`` size query of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - verify calling ``KEYCTL_DH_COMPUTE`` with ``buflen = 0`` returns required
 *   buffer size (256) without writing
 */

#include "keyctl_common.h"
#include "keyctl_dh_data.h"

static struct keyctl_dh_params *dh_params;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	dh_params->private = SAFE_NEW_USER_KEY("dh_priv", dh_priv, sizeof(dh_priv),
				       KEY_SPEC_PROCESS_KEYRING);
	dh_params->prime = SAFE_NEW_USER_KEY("dh_prime", dh_prime, sizeof(dh_prime),
					KEY_SPEC_PROCESS_KEYRING);
	dh_params->base = SAFE_NEW_USER_KEY("dh_base", dh_base, sizeof(dh_base),
				       KEY_SPEC_PROCESS_KEYRING);
}

static void run(void)
{
	TST_EXP_VAL(keyctl(KEYCTL_DH_COMPUTE, (unsigned long)dh_params,
			   (unsigned long)NULL, 0, 0),
		    (long)sizeof(dh_expected_secret));
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "4.7",
	.needs_kconfigs = (const char *[]) {
		"CONFIG_KEYS=y",
		"CONFIG_KEY_DH_OPERATIONS=y",
		"CONFIG_CRYPTO_DH",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&dh_params, .size = sizeof(*dh_params)},
		{},
	},
};
