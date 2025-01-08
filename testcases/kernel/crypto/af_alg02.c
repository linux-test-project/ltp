// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

/*
 * Regression test for commit ecaaab564978 ("crypto: salsa20 - fix
 * blkcipher_walk API usage"), or CVE-2017-17805.  This test verifies that an
 * empty message can be encrypted with Salsa20 without crashing the kernel.
 *
 * Fix for kernels < 4.14:
 * With kernels missing commit 2d97591ef43d ("crypto: af_alg - consolidation
 * of duplicate code") read() does not return in this situation. The call is
 * now moved to a child thread in order to cancel it in case read() takes an
 * unusual long amount of time.
 */

#include "tst_test.h"
#include "tst_af_alg.h"
#include "tst_safe_pthread.h"
#include <pthread.h>
#include <errno.h>

#define SALSA20_IV_SIZE       8
#define SALSA20_MIN_KEY_SIZE  16
static int volatile completed;

static void *verify_encrypt(void *arg)
{
	const uint8_t iv[SALSA20_IV_SIZE] = { 0 };
	const struct tst_alg_sendmsg_params params = {
		.encrypt = true,
		.iv = iv,
		.ivlen = SALSA20_IV_SIZE,
	};
	char buf[16];
	int reqfd = tst_alg_setup_reqfd("skcipher", "salsa20", NULL,
					SALSA20_MIN_KEY_SIZE);

	/* Send a zero-length message to encrypt */
	tst_alg_sendmsg(reqfd, NULL, 0, &params);

	/*
	 * Read the zero-length encrypted data.
	 * With the bug, the kernel crashed here.
	 */
	TST_CHECKPOINT_WAKE(0);
	if (read(reqfd, buf, 16) == 0)
		tst_res(TPASS, "Successfully \"encrypted\" an empty message");
	else
		tst_res(TFAIL, "read() didn't return 0");

	completed = 1;
	return arg;
}

static void run(void)
{
	pthread_t thr;

	completed = 0;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	SAFE_PTHREAD_CREATE(&thr, NULL, verify_encrypt, NULL);

	TST_CHECKPOINT_WAIT(0);

	while (!completed) {
		if (!tst_remaining_runtime()) {
			pthread_cancel(thr);
			tst_brk(TBROK,
				"Timed out while reading from request socket.");
		}
		usleep(1000);
	}
	pthread_join(thr, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.runtime = 20,
	.needs_checkpoints = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ecaaab564978"},
		{"CVE", "2017-17805"},
		{}
	}
};
