// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) 2017 Google, Inc.
 *
 * Test that the add_key() syscall correctly handles a NULL payload with nonzero
 * length.  Specifically, it should fail with EFAULT rather than oopsing the
 * kernel with a NULL pointer dereference or failing with EINVAL, as it did
 * before (depending on the key type).  This is a regression test for commit
 * 5649645d725c ("KEYS: fix dereferencing NULL payload with nonzero length").
 *
 * Note that none of the key types that exhibited the NULL pointer dereference
 * are guaranteed to be built into the kernel, so we just test as many as we
 * can, in the hope of catching one.  We also test with the "user" key type for
 * good measure, although it was one of the types that failed with EINVAL rather
 * than dereferencing NULL.
 *
 * This has been assigned CVE-2017-15274.
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

struct tcase {
	const char *type;
	size_t plen;
} tcases[] = {
	/*
	 * The payload length we test for each key type needs to pass initial
	 * validation but is otherwise arbitrary.  Note: the "rxrpc_s" key type
	 * requires a payload of exactly 8 bytes.
	 */
	{ "asymmetric",		64 },
	{ "cifs.idmap",		64 },
	{ "cifs.spnego",	64 },
	{ "pkcs7_test",		64 },
	{ "rxrpc",		64 },
	{ "rxrpc_s",		 8 },
	{ "user",		64 },
	{ "logon",              64 },
	{ "big_key",            64 },
};

static void verify_add_key(unsigned int i)
{
	TEST(add_key(tcases[i].type,
		"abc:def", NULL, tcases[i].plen, KEY_SPEC_PROCESS_KEYRING));

	if (TST_RET != -1) {
		tst_res(TFAIL,
			"add_key() with key type '%s' unexpectedly succeeded",
			tcases[i].type);
		return;
	}

	if (TST_ERR == EFAULT) {
		tst_res(TPASS, "received expected EFAULT with key type '%s'",
			tcases[i].type);
		return;
	}

	if (TST_ERR == ENODEV) {
		tst_res(TCONF, "kernel doesn't support key type '%s'",
			tcases[i].type);
		return;
	}

	/*
	 * It's possible for the "asymmetric" key type to be supported, but with
	 * no asymmetric key parsers registered.  In that case, attempting to
	 * add a key of type asymmetric will fail with EBADMSG.
	 */
	if (TST_ERR == EBADMSG && !strcmp(tcases[i].type, "asymmetric")) {
		tst_res(TCONF, "no asymmetric key parsers are registered");
		return;
	}

	tst_res(TFAIL | TTERRNO, "unexpected error with key type '%s'",
		tcases[i].type);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_add_key,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "5649645d725c"},
		{"CVE", "2017-15274"},
		{}
	}
};
