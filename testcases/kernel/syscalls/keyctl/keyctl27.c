// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_DH_COMPUTE`` KDF key derivation of :manpage:`keyctl(2)`.
 *
 * The KDF extension to ``KEYCTL_DH_COMPUTE`` (SP800-56A) landed in Linux 4.12.
 *
 * [Algorithm]
 *
 * - derive key with SP800-56A KDF (SHA-256 + otherinfo)
 * - verify derived key matches expected 32-byte value
 */

#include "keyctl_common.h"
#include "keyctl_dh_data.h"

#define KDF_OTHERINFO	"LTP-KDF-TEST-INFO"

static struct keyctl_dh_params *dh_params;
static struct keyctl_kdf_params *kdf_params;
static unsigned char out_buf[32];

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
	memset(kdf_params, 0, sizeof(*kdf_params));
	kdf_params->hashname = "sha256";
	kdf_params->otherinfo = (char *)KDF_OTHERINFO;
	kdf_params->otherinfolen = strlen(KDF_OTHERINFO);

	memset(out_buf, 0, sizeof(out_buf));
	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_DH_COMPUTE, (unsigned long)dh_params,
				    (unsigned long)out_buf,
				    sizeof(dh_kdf_expected),
				    (unsigned long)kdf_params),
			     (long)sizeof(dh_kdf_expected));
	if (!TST_PASS)
		return;

	if (memcmp(out_buf, dh_kdf_expected, sizeof(dh_kdf_expected))) {
		tst_res(TFAIL, "derived KDF key does not match expected value");
		return;
	}

	tst_res(TPASS, "KEYCTL_DH_COMPUTE with KDF derived expected key");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "4.12",
	.needs_kconfigs = (const char *[]) {
		"CONFIG_KEYS=y",
		"CONFIG_KEY_DH_OPERATIONS=y",
		"CONFIG_CRYPTO_DH",
		"CONFIG_CRYPTO_SHA256",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&dh_params, .size = sizeof(*dh_params)},
		{&kdf_params, .size = sizeof(*kdf_params)},
		{},
	},
};
