// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2012
 * Copyright (c) Linux Test Project, 2012
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Fork two children, one child allocates memory and initializes it;
 * then the other one calls process_vm_readv and reads from the same
 * memory location, it then verifies if process_vm_readv returns
 * correct data.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static uintptr_t *data_ptr;

static void child_alloc(const char *data)
{
	char *foo;

	foo = strdup(data);
	*data_ptr = (uintptr_t)foo;

	tst_res(TINFO, "child 0: memory allocated and initialized");

	/* wake and wait until child_invoke is done reading from our VM */
	TST_CHECKPOINT_WAKE_AND_WAIT(0);
}

static void child_invoke(const char *data, int length, pid_t pid_alloc)
{
	char *lp;
	struct iovec local, remote;

	lp = SAFE_MALLOC(length);
	local.iov_base = lp;
	local.iov_len = length;
	remote.iov_base = (void *)*data_ptr;
	remote.iov_len = length;

	tst_res(TINFO, "child 1: reading string from same memory location");

	TEST(tst_syscall(__NR_process_vm_readv, pid_alloc, &local, 1UL, &remote,
					 1UL, 0UL));

	if (TST_RET != length)
		tst_brk(TBROK, "process_vm_readv: %s", tst_strerrno(-TST_RET));

	if (strncmp(lp, data, length) != 0)
		tst_res(TFAIL, "child 1: expected string: %s, received string: %256s",
				data, lp);
	else
		tst_res(TPASS, "expected string received");
}

static void setup(void)
{
	/* Just a sanity check of the existence of syscall */
	tst_syscall(__NR_process_vm_readv, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	data_ptr = SAFE_MMAP(NULL, sizeof(uintptr_t), PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (data_ptr)
		SAFE_MUNMAP(data_ptr, sizeof(uintptr_t));
}

static void run(void)
{
	const char *data = "test";
	pid_t pid_alloc;
	pid_t pid_invoke;
	int length;
	int status;

	length = strlen(data);

	pid_alloc = SAFE_FORK();
	if (!pid_alloc) {
		child_alloc(data);
		return;
	}

	/* wait until child_alloc has allocated VM */
	TST_CHECKPOINT_WAIT(0);

	pid_invoke = SAFE_FORK();
	if (!pid_invoke) {
		child_invoke(data, length, pid_alloc);
		return;
	}

	/* wait until child_invoke reads from child_alloc's VM */
	SAFE_WAITPID(pid_invoke, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_res(TFAIL, "child 1: %s", tst_strstatus(status));

	/* child_alloc is free to exit now */
	TST_CHECKPOINT_WAKE(0);

	SAFE_WAITPID(pid_alloc, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_res(TFAIL, "child 0: %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
