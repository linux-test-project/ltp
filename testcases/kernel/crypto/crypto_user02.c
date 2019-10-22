// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

/*
 * Regression test for kernel commit 21d4120ec6f5 ("crypto: user - prevent
 * operating on larval algorithms").  See the commit message for a detailed
 * explanation of the problem.  Basically, this test tries to cause a NULL
 * pointer dereference in the kernel by abusing the CRYPTO_MSG_DELALG message in
 * the NETLINK_CRYPTO interface to try to delete a "larval" algorithm, which is
 * a kernel-internal marker for an algorithm which has been registered but isn't
 * ready yet (e.g., hasn't completed the in-kernel crypto self-tests yet).
 *
 * CRYPTO_MSG_NEWALG will create such a larval algorithm.  However, it waits
 * (killably) for the larval to mature before returning, and it holds a lock
 * that prevents CRYPTO_MSG_DELALG from running.  To get around this, this test
 * sends a fatal signal to the process executing CRYPTO_MSG_NEWALG.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_crypto.h"
#include "tst_timer.h"

static struct tst_crypto_session ses = TST_CRYPTO_SESSION_INIT;

static void setup(void)
{
	tst_crypto_open(&ses);
}

static void run(void)
{
	struct crypto_user_alg alg = {
		/*
		 * Any algorithm instantiated from a template can do here, but
		 * choose something that's commonly available.
		 */
		.cru_driver_name = "hmac(sha256-generic)",
	};
	pid_t pid;
	int status;

	/* Check whether the algorithm is supported before continuing. */
	TEST(tst_crypto_add_alg(&ses, &alg));
	if (TST_RET != 0 && TST_RET != -EEXIST) {
		if (TST_RET == -ENOENT)
			tst_brk(TCONF, "%s not supported", alg.cru_driver_name);

		tst_brk(TBROK | TRERRNO,
			"unexpected error checking for algorithm support");
	}

	tst_res(TINFO,
		"Starting crypto_user larval deletion test.  May crash buggy kernels.");

	tst_timer_start(CLOCK_MONOTONIC);

	while (!tst_timer_expired_ms(1000)) {
		pid = SAFE_FORK();

		if (pid == 0) {
			/* Child process: execute CRYPTO_MSG_NEWALG. */
			tst_crypto_open(&ses);
			for (;;) {
				TEST(tst_crypto_add_alg(&ses, &alg));
				if (TST_RET && TST_RET != -EEXIST)
					tst_brk(TBROK | TRERRNO,
						"unexpected error from tst_crypto_add_alg()");
			}
		}

		/*
		 * Parent process: kill the child process (hopefully while it's
		 * executing CRYPTO_MSG_NEWALG) and execute CRYPTO_MSG_DELALG.
		 * Buggy kernels sometimes dereferenced a NULL pointer during
		 * CRYPTO_MSG_DELALG here.
		 */
		usleep(rand() % 5000);
		kill(pid, SIGKILL);
		SAFE_WAIT(&status);
		if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL)
			tst_brk(TBROK, "child %s", tst_strstatus(status));
		TEST(tst_crypto_del_alg(&ses, &alg));
		if (TST_RET && TST_RET != -ENOENT)
			tst_brk(TBROK | TRERRNO,
				"unexpected error from tst_crypto_del_alg()");
	}

	tst_res(TPASS, "didn't crash");
}

static void cleanup(void)
{
	tst_crypto_close(&ses);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "21d4120ec6f5"},
		{}
	}
};
