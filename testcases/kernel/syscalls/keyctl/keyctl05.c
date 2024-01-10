// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 */

/*
 * Regression test for commit 63a0b0509e70 ("KEYS: fix freeing uninitialized
 * memory in key_update()").  Try to reproduce the crash in two different ways:
 *
 * 1. Try to update a key of a type that has a ->preparse() method but not an
 *    ->update() method.  Examples are the "asymmetric" and "dns_resolver" key
 *    types.  It crashes reliably for the "asymmetric" key type, since the
 *    "asymmetric" key type's ->free_preparse() method will dereference a
 *    pointer in the uninitialized memory, whereas other key types often just
 *    free a pointer which tends be NULL in practice, depending on how the stack
 *    is laid out.  However, to actually be able to add an "asymmetric" key, we
 *    need a specially-formatted payload and several kernel config options.  We
 *    do try it, but for completeness we also try the "dns_resolver" key type
 *    (though that's not guaranteed to be available either).
 *
 * 2. Race keyctl_update() with another task removing write permission from the
 *    key using keyctl_setperm().  This can cause a crash with almost any key
 *    type.  "user" is a good one to try, since it's always available if
 *    keyrings are supported at all.  However, depending on how the stack is
 *    laid out the crash may not actually occur.
 */

#include <errno.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

/*
 * A valid payload for the "asymmetric" key type.  This is an x509 certificate
 * in DER format, generated using:
 *
 *	openssl req -x509 -newkey rsa:512 -batch -nodes -outform der \
 *		| ~/linux/scripts/bin2c
 */
static const char x509_cert[] =
	"\x30\x82\x01\xd3\x30\x82\x01\x7d\xa0\x03\x02\x01\x02\x02\x09\x00"
	"\x92\x2a\x76\xff\x0c\x00\xfb\x9a\x30\x0d\x06\x09\x2a\x86\x48\x86"
	"\xf7\x0d\x01\x01\x0b\x05\x00\x30\x45\x31\x0b\x30\x09\x06\x03\x55"
	"\x04\x06\x13\x02\x41\x55\x31\x13\x30\x11\x06\x03\x55\x04\x08\x0c"
	"\x0a\x53\x6f\x6d\x65\x2d\x53\x74\x61\x74\x65\x31\x21\x30\x1f\x06"
	"\x03\x55\x04\x0a\x0c\x18\x49\x6e\x74\x65\x72\x6e\x65\x74\x20\x57"
	"\x69\x64\x67\x69\x74\x73\x20\x50\x74\x79\x20\x4c\x74\x64\x30\x1e"
	"\x17\x0d\x31\x37\x30\x37\x32\x38\x32\x31\x34\x31\x33\x34\x5a\x17"
	"\x0d\x31\x37\x30\x38\x32\x37\x32\x31\x34\x31\x33\x34\x5a\x30\x45"
	"\x31\x0b\x30\x09\x06\x03\x55\x04\x06\x13\x02\x41\x55\x31\x13\x30"
	"\x11\x06\x03\x55\x04\x08\x0c\x0a\x53\x6f\x6d\x65\x2d\x53\x74\x61"
	"\x74\x65\x31\x21\x30\x1f\x06\x03\x55\x04\x0a\x0c\x18\x49\x6e\x74"
	"\x65\x72\x6e\x65\x74\x20\x57\x69\x64\x67\x69\x74\x73\x20\x50\x74"
	"\x79\x20\x4c\x74\x64\x30\x5c\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7"
	"\x0d\x01\x01\x01\x05\x00\x03\x4b\x00\x30\x48\x02\x41\x00\xde\x0b"
	"\x1c\x24\xe2\x0d\xf8\x17\xf2\xc3\x6f\xc9\x72\x3e\x9d\xb0\x2d\x47"
	"\xe4\xc4\x85\x87\xed\xde\x06\xe3\xf3\xe9\x4c\x35\x6c\xe4\xcb\x0e"
	"\x44\x28\x23\x66\x76\xec\x4e\xdf\x10\x93\x92\x1e\x52\xfb\xdf\x5c"
	"\x08\xe7\x24\x04\x66\xe3\x06\x05\x27\x56\xfb\x3e\x91\x31\x02\x03"
	"\x01\x00\x01\xa3\x50\x30\x4e\x30\x1d\x06\x03\x55\x1d\x0e\x04\x16"
	"\x04\x14\x6f\x39\x3a\x46\xdf\x29\x63\xde\x54\x7b\x6c\x31\x06\xd0"
	"\x9f\x36\x16\xfb\x9c\xbf\x30\x1f\x06\x03\x55\x1d\x23\x04\x18\x30"
	"\x16\x80\x14\x6f\x39\x3a\x46\xdf\x29\x63\xde\x54\x7b\x6c\x31\x06"
	"\xd0\x9f\x36\x16\xfb\x9c\xbf\x30\x0c\x06\x03\x55\x1d\x13\x04\x05"
	"\x30\x03\x01\x01\xff\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01"
	"\x01\x0b\x05\x00\x03\x41\x00\x73\xf0\x4b\x62\x56\xed\xf0\x8b\x7e"
	"\xc4\x75\x78\x98\xa2\x7a\x6e\x75\x1f\xde\x9b\xa0\xbe\x1a\x1f\x86"
	"\x44\x13\xcd\x45\x06\x7f\x86\xde\xf6\x36\x4e\xb6\x15\xfa\xf5\xb0"
	"\x34\xd2\x5e\x0b\xb3\x2c\x03\x5a\x5a\x28\x97\x5e\x7b\xdf\x63\x75"
	"\x83\x8d\x69\xda\xd6\x59\xbd"
	;

	static int fips_enabled;

static void new_session_keyring(void)
{
	TEST(keyctl(KEYCTL_JOIN_SESSION_KEYRING, NULL));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "failed to join new session keyring");
}

static void test_update_nonupdatable(const char *type,
				     const void *payload, size_t plen)
{
	key_serial_t keyid;

	new_session_keyring();

	int is_asymmetric = !strcmp(type, "asymmetric");

	TEST(add_key(type, "desc", payload, plen, KEY_SPEC_SESSION_KEYRING));
	if (TST_RET < 0) {
		if (TST_ERR == EINVAL && is_asymmetric && fips_enabled) {
			tst_res(TCONF, "key size not allowed in FIPS mode");
			return;
		}
		if (TST_ERR == ENODEV) {
			tst_res(TCONF, "kernel doesn't support key type '%s'",
				type);
			return;
		}
		if (TST_ERR == EBADMSG && is_asymmetric) {
			tst_res(TCONF, "kernel is missing x509 cert parser "
				"(CONFIG_X509_CERTIFICATE_PARSER)");
			return;
		}
		if (TST_ERR == ENOENT && is_asymmetric) {
			tst_res(TCONF, "kernel is missing crypto algorithms "
				"needed to parse x509 cert (CONFIG_CRYPTO_RSA "
				"and/or CONFIG_CRYPTO_SHA256)");
			return;
		}
		tst_res(TFAIL | TTERRNO, "unexpected error adding '%s' key",
			type);
		return;
	}
	keyid = TST_RET;

	/*
	 * Non-updatable keys don't start with write permission, so we must
	 * explicitly grant it.
	 */
	TEST(keyctl(KEYCTL_SETPERM, keyid, KEY_POS_ALL));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"failed to grant write permission to '%s' key", type);
		return;
	}

	tst_res(TINFO, "Try to update the '%s' key...", type);
	TEST(keyctl(KEYCTL_UPDATE, keyid, payload, plen));
	if (TST_RET == 0) {
		tst_res(TFAIL,
			"updating '%s' key unexpectedly succeeded", type);
		return;
	}
	if (TST_ERR != EOPNOTSUPP) {
		tst_res(TFAIL | TTERRNO,
			"updating '%s' key unexpectedly failed", type);
		return;
	}
	tst_res(TPASS, "updating '%s' key expectedly failed with EOPNOTSUPP",
		type);
}

/*
 * Try to update a key, racing with removing write permission.
 * This may crash buggy kernels.
 */
static void test_update_setperm_race(void)
{
	static const char payload[] = "payload";
	key_serial_t keyid;
	int i;

	new_session_keyring();

	TEST(add_key("user", "desc", payload, sizeof(payload),
		KEY_SPEC_SESSION_KEYRING));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "failed to add 'user' key");
		return;
	}
	keyid = TST_RET;

	if (SAFE_FORK() == 0) {
		uint32_t perm = KEY_POS_ALL;

		for (i = 0; i < 10000; i++) {
			perm ^= KEY_POS_WRITE;
			TEST(keyctl(KEYCTL_SETPERM, keyid, perm));
			if (TST_RET != 0)
				tst_brk(TBROK | TTERRNO, "setperm failed");
		}
		exit(0);
	}

	tst_res(TINFO, "Try to update the 'user' key...");
	for (i = 0; i < 10000; i++) {
		TEST(keyctl(KEYCTL_UPDATE, keyid, payload, sizeof(payload)));
		if (TST_RET != 0 && TST_ERR != EACCES) {
			tst_res(TFAIL | TTERRNO, "failed to update 'user' key");
			return;
		}
	}
	tst_reap_children();
	tst_res(TPASS, "didn't crash while racing to update 'user' key");
}

static void setup(void)
{
	fips_enabled = tst_fips_enabled();
}

static void do_test(unsigned int i)
{
	/*
	 * We need to pass check in dns_resolver_preparse(),
	 * give it dummy server list request.
	 * From v6.7-rc8 commit 1997b3cb4217b09e49659b634c94da47f0340409:
	 * the incoming data for add_key syscall should be larger than 6 bytes,
	 * because struct dns_server_list_v1_header without body is 6 bytes.
	 */
	static char dns_res_payload[] = { 0x00, 0x00, 0x01, 0xff, 0x00, \
					  0x00, 0x00 };

	switch (i) {
	case 0:
		test_update_nonupdatable("asymmetric",
					 x509_cert, sizeof(x509_cert));
		break;
	case 1:
		test_update_nonupdatable("dns_resolver", dns_res_payload,
			sizeof(dns_res_payload));
		break;
	case 2:
		test_update_setperm_race();
		break;
	}
}

static struct tst_test test = {
	.tcnt = 3,
	.setup = setup,
	.test = do_test,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "63a0b0509e70"},
		{}
	}
};
