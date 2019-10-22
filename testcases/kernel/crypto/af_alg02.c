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

static void *verify_encrypt(void *arg)
{
	char buf[16];
	int reqfd = tst_alg_setup_reqfd("skcipher", "salsa20", NULL, 16);

	TST_CHECKPOINT_WAKE(0);

	/* With the bug the kernel crashed here */
	if (read(reqfd, buf, 16) == 0)
		tst_res(TPASS, "Successfully \"encrypted\" an empty message");
	else
		tst_res(TFAIL, "read() didn't return 0");
	return arg;
}

static void run(void)
{
	pthread_t thr;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	SAFE_PTHREAD_CREATE(&thr, NULL, verify_encrypt, NULL);

	TST_CHECKPOINT_WAIT(0);

	while (pthread_kill(thr, 0) != ESRCH) {
		if (tst_timeout_remaining() <= 10) {
			pthread_cancel(thr);
			tst_brk(TBROK,
				"Timed out while reading from request socket.");
		}
		usleep(1000);
	}
}

static struct tst_test test = {
	.test_all = run,
	.timeout = 20,
	.needs_checkpoints = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ecaaab564978"},
		{"CVE", "2017-17805"},
		{}
	}
};
