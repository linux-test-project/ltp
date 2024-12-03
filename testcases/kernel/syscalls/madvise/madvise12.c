// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that MADV_GUARD_INSTALL is causing SIGSEGV when someone is accessing
 * memory advised with it.
 *
 * This is a test for feature implemented in
 * 662df3e5c376 ("mm: madvise: implement lightweight guard page mechanism")
 *
 * [Algorithm]
 *
 * - allocate a certain amount of memory
 * - advise memory with MADV_GUARD_INSTALL
 * - access to memory from within a child and verify it gets killed by SIGSEGV
 * - release memory with MADV_GUARD_REMOVE
 * - verify that memory has not been modified before child got killed
 * - modify memory within a new child
 * - verify that memory is accessable and child was not killed by SIGSEGV
 */

#include "tst_test.h"
#include "lapi/mmap.h"

#define MAP_SIZE (8 * TST_KB)

static char *addr;

static void run(void)
{
	pid_t pid;
	int status;

	memset(addr, 0, MAP_SIZE);

	TST_EXP_PASS(madvise(addr, MAP_SIZE, MADV_GUARD_INSTALL));

	pid = SAFE_FORK();
	if (!pid) {
		tst_res(TINFO, "Modifying memory content");

		memset(addr, 'a', MAP_SIZE);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		tst_res(TPASS, "Child ended by SIGSEGV as expected");
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));

	TST_EXP_PASS(madvise(addr, MAP_SIZE, MADV_GUARD_REMOVE));

	for (int i = 0; i < MAP_SIZE; i++) {
		if (addr[i] == 'a') {
			tst_res(TFAIL, "Memory content has been modified");
			return;
		}
	}

	tst_res(TPASS, "Memory content didn't change");

	pid = SAFE_FORK();
	if (!pid) {
		tst_res(TINFO, "Modifying memory content");

		memset(addr, 'b', MAP_SIZE);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (!WIFSIGNALED(status))
		tst_res(TPASS, "Child ended without being signaled");
	else
		tst_res(TFAIL, "Child ended with %s", tst_strstatus(status));
}

static void setup(void)
{
	addr = SAFE_MMAP(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
}

static void cleanup(void)
{
	if (addr)
		SAFE_MUNMAP(addr, MAP_SIZE);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.min_kver = "6.13",
};
