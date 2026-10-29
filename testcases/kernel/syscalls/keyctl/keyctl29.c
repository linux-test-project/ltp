// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test ``KEYCTL_PKEY_QUERY`` on public key of :manpage:`keyctl(2)`.
 *
 * ``KEYCTL_PKEY_QUERY`` queries the parameters, operations, and buffer
 * size limits of an asymmetric public or private key.
 *
 * Requires root (CAP_SYS_MODULE) to load the ``x509_key_parser`` module.
 *
 * [Algorithm]
 *
 * - query an RSA-2048 X.509 public key certificate
 * - verify ``key_size`` is 2048 bits
 * - verify ``supported_ops`` contains ``KEYCTL_SUPPORTS_ENCRYPT`` and
 *   ``KEYCTL_SUPPORTS_VERIFY``
 * - verify ``max_enc_size`` is 256 bytes
 * - verify ``max_sig_size`` is 256 bytes
 */

#include "keyctl_common.h"
#include "keyctl_pkey_data.h"
#include "tst_module.h"

static key_serial_t cert_key;
static struct keyctl_pkey_query *query_buf;

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	tst_modprobe("x509_key_parser", NULL);

	cert_key = add_asymmetric_key_or_tconf("cert", rsa2048_cert,
					       sizeof(rsa2048_cert),
					       "CONFIG_X509_CERTIFICATE_PARSER");
}

static void run(void)
{
	memset(query_buf, 0, sizeof(*query_buf));

	TST_EXP_PASS(keyctl(KEYCTL_PKEY_QUERY, (unsigned long)cert_key,
				    0, (unsigned long)"enc=pkcs1",
				    (unsigned long)query_buf));
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU_SILENT(query_buf->key_size, 2048);
	if (!TST_PASS)
		return;

	TST_EXP_EXPR(query_buf->supported_ops & KEYCTL_SUPPORTS_ENCRYPT,
		     "KEYCTL_PKEY_QUERY supports ENCRYPT");

	TST_EXP_EXPR(query_buf->supported_ops & KEYCTL_SUPPORTS_VERIFY,
		     "KEYCTL_PKEY_QUERY supports VERIFY");

	TST_EXP_EQ_LU_SILENT(query_buf->max_enc_size, 256);
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU_SILENT(query_buf->max_sig_size, 256);
	if (!TST_PASS)
		return;

	tst_res(TPASS, "KEYCTL_PKEY_QUERY on X.509 cert returned valid parameters");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "4.20",
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_KEYS=y",
		"CONFIG_ASYMMETRIC_KEY_TYPE=y",
		"CONFIG_X509_CERTIFICATE_PARSER",
		"CONFIG_CRYPTO_RSA",
		"CONFIG_CRYPTO_SHA256",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&query_buf, .size = sizeof(*query_buf)},
		{},
	},
};
