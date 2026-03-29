// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Red Hat, Inc.
 * Check that futex(FUTEX_WAIT) returns EINTR when interrupted by a signal.
 * A child process blocks on futex_wait() with a long timeout. The parent
 * waits for the child to enter sleep state, then sends SIGUSR1 to it.
 * The child verifies it received EINTR and exits accordingly.
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "futextest.h"

static futex_t *futex;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

/* We need a handler so SIGUSR1 is caught instead of killing the process.
 * The empty body is needed, just receiving the signal is enough to
 * interrupt futex_wait() and make it return into EINTR -1 status.
 */
static void sigusr1_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void do_child(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct sigaction sa;
	struct tst_ts timeout;
	int res;

	/* Set up the signal handler for SIGUSR1 */
	sa.sa_handler = sigusr1_handler;
	sa.sa_flags = 0;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	/* Create a timeout for 5 sec for this variant.
	 * if no one wakes the child before 5 sec, futex_wait() will return
	 * on its own with ETIMEDOUT and will not wait any longer
	 */
	timeout = tst_ts_from_ms(tv->tstype, 5000);
	res = futex_wait(tv->fntype, futex, *futex, &timeout, 0);

	if (res != -1) {
		tst_res(TFAIL, "futex_wait() should have failed with EINTR but returned success instead");
		exit(1);
	}
	if (errno != EINTR) {
		tst_res(TFAIL | TERRNO, "futex_wait() expected EINTR but got something else, errno");
		exit(1);
	}
	tst_res(TPASS | TERRNO, "futex_wait() returned EINTR as expected");
	exit(0);
}

static void run(void)
{
	pid_t child;
	int status;

	child = SAFE_FORK();

	if (child == 0) {
		do_child();
	}
	/* Wait until child is sleeping before sending signal */
	TST_PROCESS_STATE_WAIT(child, 'S', 0);
	/* Wake up the child so it will return EINTR -1 status */
	SAFE_KILL(child, SIGUSR1);
	SAFE_WAITPID(child, &status, 0);
	/* Check if the child finished everything as it should */
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		tst_res(TFAIL, "child exited abnormally");
	}
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	/* Futex needs to be in a shared memory so the parent and the child can access into it */
	futex = SAFE_MMAP(NULL, sizeof(*futex), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	*futex = FUTEX_INITIALIZER;
}

static void cleanup(void)
{
	if (futex) {
		SAFE_MUNMAP((void *)futex, sizeof(*futex));
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
	.forks_child = 1,
};
