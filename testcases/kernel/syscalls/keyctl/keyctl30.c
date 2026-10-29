// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_PKEY_QUERY`` on private key of :manpage:`keyctl(2)`.
 *
 * Requires root (CAP_SYS_MODULE) to load the ``pkcs8_key_parser`` module.
 *
 * [Algorithm]
 *
 * - query an RSA-2048 PKCS#8 private key
 * - verify ``key_size`` is 2048 bits
 * - verify ``supported_ops`` contains ``KEYCTL_SUPPORTS_DECRYPT`` and
 *   ``KEYCTL_SUPPORTS_SIGN``
 * - verify ``max_dec_size`` is 256 bytes
 * - verify ``max_sig_size`` is 256 bytes
 */

#include "keyctl_common.h"
#include "keyctl_pkey_data.h"
#include "tst_module.h"

static key_serial_t priv_key;
static struct keyctl_pkey_query *query_buf;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	tst_modprobe("pkcs8_key_parser", NULL);

	priv_key = add_asymmetric_key_or_tconf("priv", rsa2048_pkcs8,
					       sizeof(rsa2048_pkcs8),
					       "CONFIG_PKCS8_PRIVATE_KEY_PARSER");
}

static void run(void)
{
	memset(query_buf, 0, sizeof(*query_buf));

	TST_EXP_PASS(keyctl(KEYCTL_PKEY_QUERY, (unsigned long)priv_key,
				    0, (unsigned long)"enc=pkcs1",
				    (unsigned long)query_buf));
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU_SILENT(query_buf->key_size, 2048);
	if (!TST_PASS)
		return;

	TST_EXP_EXPR(query_buf->supported_ops & KEYCTL_SUPPORTS_DECRYPT,
		     "KEYCTL_PKEY_QUERY supports DECRYPT");

	TST_EXP_EXPR(query_buf->supported_ops & KEYCTL_SUPPORTS_SIGN,
		     "KEYCTL_PKEY_QUERY supports SIGN");

	TST_EXP_EQ_LU_SILENT(query_buf->max_dec_size, 256);
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU_SILENT(query_buf->max_sig_size, 256);
	if (!TST_PASS)
		return;

	tst_res(TPASS, "KEYCTL_PKEY_QUERY on PKCS#8 private key returned valid parameters");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "4.20",
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_KEYS=y",
		"CONFIG_ASYMMETRIC_KEY_TYPE=y",
		"CONFIG_PKCS8_PRIVATE_KEY_PARSER",
		"CONFIG_CRYPTO_RSA",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&query_buf, .size = sizeof(*query_buf)},
		{},
	},
};
