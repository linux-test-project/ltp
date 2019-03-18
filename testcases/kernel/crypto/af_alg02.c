// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

/*
 * Regression test for commit ecaaab564978 ("crypto: salsa20 - fix
 * blkcipher_walk API usage"), or CVE-2017-17805.  This test verifies that an
 * empty message can be encrypted with Salsa20 without crashing the kernel.
 */

#include "tst_test.h"
#include "tst_af_alg.h"

static void run(void)
{
	char buf[16];
	int reqfd = tst_alg_setup_reqfd("skcipher", "salsa20", NULL, 16);

	/* With the bug the kernel crashed here */
	if (read(reqfd, buf, 16) == 0)
		tst_res(TPASS, "Successfully \"encrypted\" an empty message");
	else
		tst_res(TBROK, "read() didn't return 0");
}

static struct tst_test test = {
	.test_all = run,
};
