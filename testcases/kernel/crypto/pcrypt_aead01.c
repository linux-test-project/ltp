/*
 * Copyright (c) 2018 SUSE
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP conversion: Richard Palethorpe <rpalethorpe@suse.com>
 *
 * Originally found by syzkaller:
 * https://groups.google.com/forum/#!topic/syzkaller-bugs/NKn_ivoPOpk
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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
};
