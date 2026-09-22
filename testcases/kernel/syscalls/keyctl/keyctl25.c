// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DH_COMPUTE`` shared secret computation of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - compute DH shared secret using RFC 7919 FFDHE-2048 parameters
 * - verify the 256-byte result matches the expected secret
 */

#include "keyctl_common.h"
#include "keyctl_dh_data.h"

static struct keyctl_dh_params *dh_params;
static unsigned char out_buf[256];

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
	memset(out_buf, 0, sizeof(out_buf));

	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_DH_COMPUTE, (unsigned long)dh_params,
				    (unsigned long)out_buf,
				    sizeof(dh_expected_secret), 0),
			     (long)sizeof(dh_expected_secret));
	if (!TST_PASS)
		return;

	if (memcmp(out_buf, dh_expected_secret, sizeof(dh_expected_secret))) {
		tst_res(TFAIL, "computed secret does not match expected value");
		return;
	}

	tst_res(TPASS, "KEYCTL_DH_COMPUTE computed expected shared secret");
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
