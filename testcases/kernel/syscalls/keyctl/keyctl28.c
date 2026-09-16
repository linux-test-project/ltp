// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Negative and boundary test cases for ``KEYCTL_DH_COMPUTE`` of :manpage:`keyctl(2)`.
 *
 * [Algorithm]
 *
 * - verify ``EOVERFLOW`` when buffer is smaller than secret size (255 < 256)
 * - verify ``ENOKEY`` when prime key id does not exist
 * - verify ``EOPNOTSUPP`` when prime key is not of "user" type
 * - verify ``EINVAL`` when buffer is ``NULL`` with non-zero buflen
 * - verify ``EINVAL`` when KDF ``__spare`` field contains non-zero data
 * - verify ``EMSGSIZE`` when KDF requested output length exceeds 1024
 * - verify ``EMSGSIZE`` when KDF otherinfo length exceeds 64
 * - verify ``ENOENT`` when KDF hash algorithm name is unknown
 */

#include "keyctl_common.h"
#include "keyctl_dh_data.h"

static struct keyctl_dh_params *dh_params;
static struct keyctl_dh_params *dh_bogus_prime;
static struct keyctl_dh_params *dh_nonuser_prime;

static struct keyctl_kdf_params *kdf_valid;
static struct keyctl_kdf_params *kdf_spare_nonzero;
static struct keyctl_kdf_params *kdf_oi_toolarge;
static struct keyctl_kdf_params *kdf_unknown_hash;

static unsigned char out_buf[1025];

static struct tcase {
	struct keyctl_dh_params **params;
	void *buffer;
	size_t buflen;
	struct keyctl_kdf_params **kdf;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{ &dh_params, out_buf, 255, NULL,
	  EOVERFLOW, "buffer smaller than secret size (255 < 256)" },

	{ &dh_bogus_prime, out_buf, 256, NULL,
	  ENOKEY, "bogus prime key ID" },

	{ &dh_nonuser_prime, out_buf, 256, NULL,
	  EOPNOTSUPP, "non-user key as prime parameter" },

	{ &dh_params, NULL, 256, NULL,
	  EINVAL, "NULL buffer with non-zero buflen" },

	{ &dh_params, out_buf, 32, &kdf_spare_nonzero,
	  EINVAL, "KDF non-zero __spare field" },

	{ &dh_params, out_buf, 1025, &kdf_valid,
	  EMSGSIZE, "KDF output length > 1024" },

	{ &dh_params, out_buf, 32, &kdf_oi_toolarge,
	  EMSGSIZE, "KDF otherinfolen > 64" },

	{ &dh_params, out_buf, 32, &kdf_unknown_hash,
	  ENOENT, "unknown KDF hash algorithm name" },
};

static void setup(void)
{
	key_serial_t key_priv, key_prime, key_base;

	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key_priv = SAFE_NEW_USER_KEY("dh_priv", dh_priv, sizeof(dh_priv),
				KEY_SPEC_PROCESS_KEYRING);
	key_prime = SAFE_NEW_USER_KEY("dh_prime", dh_prime, sizeof(dh_prime),
				 KEY_SPEC_PROCESS_KEYRING);
	key_base = SAFE_NEW_USER_KEY("dh_base", dh_base, sizeof(dh_base),
				KEY_SPEC_PROCESS_KEYRING);

	dh_params->priv = key_priv;
	dh_params->prime = key_prime;
	dh_params->base = key_base;

	*dh_bogus_prime = *dh_params;
	dh_bogus_prime->prime = INT32_MAX;

	*dh_nonuser_prime = *dh_params;
	dh_nonuser_prime->prime = KEY_SPEC_PROCESS_KEYRING;

	kdf_valid->hashname = "sha256";
	kdf_valid->otherinfo = "info";
	kdf_valid->otherinfolen = 4;

	*kdf_spare_nonzero = *kdf_valid;
	kdf_spare_nonzero->__spare[0] = 1;

	*kdf_oi_toolarge = *kdf_valid;
	kdf_oi_toolarge->otherinfolen = 65;

	*kdf_unknown_hash = *kdf_valid;
	kdf_unknown_hash->hashname = "sha9000";
}

static void verify_negative(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct keyctl_dh_params *p = *tc->params;
	struct keyctl_kdf_params *kdf = tc->kdf ? *tc->kdf : NULL;

	TST_EXP_FAIL2(keyctl(KEYCTL_DH_COMPUTE, (unsigned long)p,
			     (unsigned long)tc->buffer, tc->buflen,
			     (unsigned long)kdf),
		      tc->exp_errno,
		      "KEYCTL_DH_COMPUTE with %s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_negative,
	.tcnt = ARRAY_SIZE(tcases),
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
		{&dh_bogus_prime, .size = sizeof(*dh_bogus_prime)},
		{&dh_nonuser_prime, .size = sizeof(*dh_nonuser_prime)},
		{&kdf_valid, .size = sizeof(*kdf_valid)},
		{&kdf_spare_nonzero, .size = sizeof(*kdf_spare_nonzero)},
		{&kdf_oi_toolarge, .size = sizeof(*kdf_oi_toolarge)},
		{&kdf_unknown_hash, .size = sizeof(*kdf_unknown_hash)},
		{},
	},
};
