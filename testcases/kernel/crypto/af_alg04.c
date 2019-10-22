// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

/*
 * Regression test for commit bb2964810233 ("crypto: vmac - separate tfm and
 * request context").  This test verifies that a VMAC transform can be used by
 * multiple concurrent hash requests without crashing the kernel.  Based on the
 * reproducer from the commit message.
 */

#include <sys/wait.h>

#include "tst_test.h"
#include "tst_af_alg.h"

static void run(void)
{
	int algfd, reqfd;
	char buf[256] = { 0 };
	pid_t pid;
	int status;
	int i;

	if (tst_have_alg("hash", "vmac64(aes)"))
		algfd = tst_alg_setup("hash", "vmac64(aes)", NULL, 16);
	else
		algfd = tst_alg_setup("hash", "vmac(aes)", NULL, 16);

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

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "bb2964810233"},
		{}
	}
};
