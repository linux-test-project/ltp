// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Google LLC
 * Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
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

/*
 * List of possible algorithms to use try (not exhaustive).
 * The algorithm has to be valid (i.e. the drivers must exists
 * and be a valid combination) and it has to be deleteable.
 * To be deletable it cannot be used by someone else.
 * The first algorithm, that fullfils the criteria is used for the test.
 */
static const char * const ALGORITHM_CANDIDATES[] = {
	"hmac(sha1-generic)",
	"hmac(sha224-generic)",
	"hmac(sha256-generic)",
	"hmac(sha384-generic)",
	"hmac(md5-generic)",
	"hmac(sm3-generic)",
	"hmac(sha512-generic)",
	"hmac(rmd160-generic)",
	"hmac(sha3-224-generic)",
	"hmac(sha3-256-generic)",
	"hmac(sha3-384-generic)",
	"hmac(sha3-512-generic)",
	"hmac(streebog256-generic)",
	"hmac(streebog512-generic)"
};

static const char* algorithm = NULL;
static struct tst_netlink_context *ctx;


static void setup(void)
{
	int rc;
	unsigned i;
	struct crypto_user_alg alg;

	ctx = NETLINK_CREATE_CONTEXT(NETLINK_CRYPTO);

	/* find an algorithm, that is not in use */
	for (i = 0; i < ARRAY_SIZE(ALGORITHM_CANDIDATES); ++i) {
		memset(&alg, 0, sizeof(alg));
		strcpy(alg.cru_driver_name, ALGORITHM_CANDIDATES[i]);

		/* try to add it, to see if it is valid */
		rc = tst_crypto_add_alg(ctx, &alg);
		if (rc != 0)
			continue;

		/* it also has to be deletable */
		rc = tst_crypto_del_alg(ctx, &alg, 1000);
		if (rc == 0) {
			algorithm = ALGORITHM_CANDIDATES[i];
			break;
		}
	}

	if (!algorithm)
		tst_brk(TCONF, "No viable algorithm found");
}

static void run(void)
{
	struct crypto_user_alg alg = {};
	pid_t pid;
	int status;

	strcpy(alg.cru_driver_name, algorithm);

	tst_res(TINFO,
		"Starting crypto_user larval deletion test using algorithm %s. May crash buggy kernels.",
		algorithm);

	tst_timer_start(CLOCK_MONOTONIC);

	while (!tst_timer_expired_ms(1000)) {
		pid = SAFE_FORK();

		if (pid == 0) {
			/* Child process: execute CRYPTO_MSG_NEWALG. */
			ctx = NETLINK_CREATE_CONTEXT(NETLINK_CRYPTO);
			for (;;) {
				TEST(tst_crypto_add_alg(ctx, &alg));
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
		TEST(tst_crypto_del_alg(ctx, &alg, 1000));
		if (TST_RET && TST_RET != -ENOENT)
			tst_brk(TBROK | TRERRNO,
				"unexpected error from tst_crypto_del_alg()");
	}

	tst_res(TPASS, "didn't crash");
}

static void cleanup(void)
{
	NETLINK_DESTROY_CONTEXT(ctx);
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
