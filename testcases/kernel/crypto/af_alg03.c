// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 * Copyright (c) Linux Test Project, 2019-2021
 */

/*
 * Regression test for commit e57121d08c38 ("crypto: chacha20poly1305 - validate
 * the digest size").  This test verifies that the rfc7539 template can't be
 * instantiated with a hash algorithm whose digest size is not 16 bytes.
 */

#include "tst_test.h"
#include "tst_af_alg.h"

static void run(void)
{
	tst_require_alg("aead", "rfc7539(chacha20,poly1305)");
	tst_require_alg("hash", "sha256");

	if (tst_try_alg("aead", "rfc7539(chacha20,sha256)") != ENOENT) {
		tst_res(TFAIL,
			"instantiated rfc7539 template with wrong digest size");
	} else {
		tst_res(TPASS,
			"couldn't instantiate rfc7539 template with wrong digest size");
	}
}

static struct tst_test test = {
	.test_all = run,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e57121d08c38"},
		{}
	}
};
