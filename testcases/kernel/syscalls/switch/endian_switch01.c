// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2008
 * Copyright (c) Paul Mackerras, IBM Corp., 2008
 * Copyright (c) 2018-2023 Linux Test Project
 */

/*
 * Test little-endian mode switch system call. Requires a 64-bit
 * processor that supports little-endian mode,such as POWER6.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"

#if defined(__powerpc64__) || defined(__powerpc__)

# ifndef PPC_FEATURE_TRUE_LE
#  define PPC_FEATURE_TRUE_LE              0x00000002
# endif

# ifdef HAVE_GETAUXVAL
#  include <sys/auxv.h>

/*
 * Make minimal call to 0x1ebe. If we get ENOSYS then syscall is not
 * available, likely because of:
 *   commit 727f13616c45 ("powerpc: Disable the fast-endian switch syscall by default")
 * If we get any other outcome, including crashes with various signals,
 * then we assume syscall is available and carry on with the test.
 */
void check_le_switch_supported(void)
{
	int status;

	if (SAFE_FORK() == 0) {
		syscall(0x1ebe);
		exit(errno);
	}

	if (!(getauxval(AT_HWCAP) & PPC_FEATURE_TRUE_LE))
		tst_brk(TCONF, "Processor does not support little-endian mode");

	SAFE_WAIT(&status);
	if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);

		tst_res(TINFO, "check exited with sig %d", sig);
	} else if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);

		tst_res(TINFO, "check exited with %d", rc);
		if (rc == ENOSYS)
			tst_brk(TCONF, "fast endian switch (0x1ebe) N/A");
	}
}

void test_le_switch(void)
{
	int status;

	if (SAFE_FORK() == 0) {
		register int r0 asm("r0") = 0x1ebe;

		asm volatile ("sc; .long 0x02000044"
				: "=&r" (r0)
				: "0"(r0)
				: "cr0", "r9", "r10", "r11", "r12");
		exit(0);
	}

	SAFE_WAIT(&status);
	if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);

		tst_res(TFAIL, "test exited with sig %d", sig);
	} else if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);

		if (rc != 0)
			tst_res(TFAIL, "test exited with %d", rc);
		else
			tst_res(TPASS, "endian_switch() syscall tests passed");
	}
}

static void endian_test(void)
{
	check_le_switch_supported();
	test_le_switch();
}

static struct tst_test test = {
	.test_all = endian_test,
	.forks_child = 1,
};

# else
TST_TEST_TCONF("Toolchain does not have <sys/auxv.h>");
# endif /* HAVE_GETAUXVAL */

#else /* defined (__powerpc64__) || (__powerpc__) */
TST_TEST_TCONF("This system does not support running of switch() syscall");
#endif
