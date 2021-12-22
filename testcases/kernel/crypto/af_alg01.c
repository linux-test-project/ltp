// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 * Copyright (c) Linux Test Project, 2019-2021
 */

/*
 * Regression test for commit af3ff8045bbf ("crypto: hmac - require that the
 * underlying hash algorithm is unkeyed"), or CVE-2017-17806.  This test
 * verifies that the hmac template cannot be nested inside itself.
 */

#include <errno.h>
#include <stdio.h>

#include "tst_test.h"
#include "tst_af_alg.h"
#include "lapi/socket.h"

static void test_with_hash_alg(const char *hash_algname)
{
	char hmac_algname[64];
	char key[4096] = { 0 };

	if (!tst_have_alg("hash", hash_algname))
		return;

	sprintf(hmac_algname, "hmac(%s)", hash_algname);
	if (!tst_have_alg("hash", hmac_algname))
		return;

	sprintf(hmac_algname, "hmac(hmac(%s))", hash_algname);
	if (tst_try_alg("hash", hmac_algname) != ENOENT) {
		int algfd;

		tst_res(TFAIL, "instantiated nested hmac algorithm ('%s')!",
			hmac_algname);

		/*
		 * Be extra annoying; with the bug, setting a key on
		 * "hmac(hmac(sha3-256-generic))" crashed the kernel.
		 */
		algfd = tst_alg_setup("hash", hmac_algname, NULL, 0);
		if (setsockopt(algfd, SOL_ALG, ALG_SET_KEY,
			       key, sizeof(key)) == 0) {
			tst_res(TFAIL,
				"set key on nested hmac algorithm ('%s')!",
				hmac_algname);
		}
	} else {
		tst_res(TPASS,
			"couldn't instantiate nested hmac algorithm ('%s')",
			hmac_algname);
	}
}

/* try several different unkeyed hash algorithms */
static const char * const hash_algs[] = {
	"md5", "md5-generic",
	"sha1", "sha1-generic",
	"sha224", "sha224-generic",
	"sha256", "sha256-generic",
	"sha3-256", "sha3-256-generic",
	"sha3-512", "sha3-512-generic",
	"sm3", "sm3-generic",
};

static void do_test(unsigned int i)
{
	test_with_hash_alg(hash_algs[i]);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(hash_algs),
	.tags = (const struct tst_tag[]) {
		{"linux-git", "af3ff8045bbf"},
		{"CVE", "2017-17806"},
		{}
	}
};
