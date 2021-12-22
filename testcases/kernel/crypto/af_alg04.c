// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 * Copyright (c) Linux Test Project, 2019-2021
 */

/*
 * Regression test for commit bb2964810233 ("crypto: vmac - separate tfm and
 * request context").  This test verifies that a VMAC transform can be used by
 * multiple concurrent hash requests without crashing the kernel.  Based on the
 * reproducer from the commit message.
 */

#include <stdio.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_af_alg.h"

static void test_with_symm_enc_algs(const char *symm_enc_algname)
{
	int algfd, reqfd;
	char buf[256] = { 0 };
	char vmac_algname[64];
	pid_t pid;
	int status;
	int i;

	sprintf(vmac_algname, "vmac64(%s)", symm_enc_algname);
	if (!tst_have_alg("hash", vmac_algname)) {
		sprintf(vmac_algname, "vmac(%s)", symm_enc_algname);
		if (!tst_have_alg("hash", vmac_algname))
			return;
	}
	algfd = tst_alg_setup("hash", vmac_algname, NULL, 16);

	tst_res(TINFO, "Starting vmac hashing test.  May crash buggy kernels.");

	pid = SAFE_FORK();

	reqfd = tst_alg_accept(algfd);

	for (i = 0; i < 500000; i++)
		SAFE_WRITE(1, reqfd, buf, sizeof(buf));

	close(reqfd);

	if (pid != 0) {
		SAFE_WAIT(&status);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			tst_res(TPASS, "didn't crash");
		else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL)
			tst_res(TFAIL, "crashed");
		else
			tst_brk(TBROK, "child %s", tst_strstatus(status));

		close(algfd);
	}
}

/* try several different symmetric encryption algorithms */
static const char * const symm_enc_algs[] = {
	"aes",
	"sm4",
	"sm4-generic",
};

static void do_test(unsigned int i)
{
	test_with_symm_enc_algs(symm_enc_algs[i]);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(symm_enc_algs),
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "bb2964810233"},
		{}
	}
};
