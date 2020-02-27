// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

/*
 * Regression test for commit 8088d3dd4d7c ("crypto: skcipher - fix crash
 * flushing dcache in error path").  This test verifies the kernel doesn't crash
 * when trying to encrypt a message with size not aligned to the block cipher's
 * block size, and where the destination buffer starts exactly at a page
 * boundary.  Based on the reproducer from the commit message.  Note that this
 * issue only reproduces on certain architectures, such as arm and arm64.
 *
 * On some older kernel without commit 160544075f2a ("crypto: scatterwalk - Hide
 * PageSlab call to optimise away flush_dcache_page") , it doesn't use
 * ARCH_IMPLEMENTS_FLUSH_DCACHE_PAGE macro. It can crash on all architectures.
 * Without skcipher walk interface, it is also a regresstion test for commit
 * 0868def3e410("crypto: blkcipher - fix crash flushing dcache in error path").
 */

#include <errno.h>

#include "tst_test.h"
#include "tst_af_alg.h"

static void run(void)
{
	char buffer[4096] __attribute__((aligned(4096))) = { 0 };
	int reqfd;

	reqfd = tst_alg_setup_reqfd("skcipher", "cbc(aes-generic)", NULL, 16);

	SAFE_WRITE(1, reqfd, buffer, 15);
	/* with the bug, this crashed the kernel on some architectures */
	TEST(read(reqfd, buffer, 15));

	if (TST_RET == 0)
		tst_res(TFAIL, "read() unexpectedly succeeded");
	else if (TST_ERR == EINVAL)
		tst_res(TPASS, "read() expectedly failed with EINVAL");
	else
		tst_res(TFAIL | TTERRNO, "read() failed with unexpected error");

	close(reqfd);
}

static struct tst_test test = {
	.test_all = run,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "8088d3dd4d7c"},
		{"linux-git", "160544075f2a"},
		{"linux-git", "0868def3e410"},
		{}
	}
};
