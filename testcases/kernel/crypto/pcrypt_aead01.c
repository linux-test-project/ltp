// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 SUSE
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP conversion: Richard Palethorpe <rpalethorpe@suse.com>
 *
 * Originally found by syzkaller:
 * https://groups.google.com/forum/#!topic/syzkaller-bugs/NKn_ivoPOpk
 *
 * Test for CVE-2017-5754 - pcrypt mishandles freeing instances.
 *
 * The test works by adding and then removing pcrypt-AEAD instances.
 * See commit d76c68109f37 crypto: pcrypt - fix freeing pcrypt instances.
 *
 * If the bug is present then this will probably crash the kernel, but also
 * sometimes the test simply times out.
 */

#include <errno.h>
#include <time.h>

#include "tst_test.h"
#include "tst_safe_net.h"
#include "tst_taint.h"
#include "tst_crypto.h"

#define ATTEMPTS 10000

static struct tst_crypto_session ses = TST_CRYPTO_SESSION_INIT;

void setup(void)
{
	tst_crypto_open(&ses);
}

void run(void)
{
	int i;
	struct crypto_user_alg a = {
		.cru_driver_name = "pcrypt(authenc(hmac(sha256-generic),cbc(aes-generic)))",
		.cru_type = CRYPTO_ALG_TYPE_AEAD,
		.cru_mask = CRYPTO_ALG_TYPE_MASK,
	};

	for (i = 0; i < ATTEMPTS; ++i) {
		TEST(tst_crypto_add_alg(&ses, &a));
		if (TST_RET && TST_RET == -ENOENT) {
			tst_brk(TCONF | TRERRNO,
				"pcrypt, hmac, sha256, cbc or aes not supported");
		}
		if (TST_RET && TST_RET != -EEXIST)
			tst_brk(TBROK | TRERRNO, "add_alg");

		TEST(tst_crypto_del_alg(&ses, &a));
		if (TST_RET)
			tst_brk(TBROK | TRERRNO, "del_alg");

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Time limit reached, stopping at "
				"%d iterations", i);
			break;
		}
	}

	tst_res(TPASS, "Nothing bad appears to have happened");
}

void cleanup(void)
{
	tst_crypto_close(&ses);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.max_runtime = 300,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d76c68109f37"},
		{"CVE", "2017-5754"},
		{}
	}
};
