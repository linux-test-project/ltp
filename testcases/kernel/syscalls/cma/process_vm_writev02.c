// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2012
 * Copyright (c) Linux Test Project, 2012
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Fork two children, the first one allocates a chunk of memory and the
 * other one call process_vm_writev to write known data into the first
 * child. Then first child verifies that the data is as expected.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static uintptr_t *data_ptr;
static char *str_buffsize;
static int bufsize = 100000;

static void child_alloc_and_verify(int buffsize)
{
	char foo[buffsize];
	int i;
	int err;

	tst_res(TINFO, "child 0: allocate memory");

	memset(foo, 'a', buffsize);
	*data_ptr = (uintptr_t)foo;

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	err = 0;
	for (i = 0; i < buffsize; i++)
		if (foo[i] != 'w')
			err++;

	if (err)
		tst_res(TFAIL, "child 0: found %d differences from expected data", err);
	else
		tst_res(TPASS, "child 0: read back expected data");
}

static void child_write(int buffsize, pid_t pid_alloc)
{
	char lp[buffsize];
	struct iovec local, remote;

	tst_res(TINFO, "child 1: write to the same memory location");

	memset(lp, 'w', buffsize);

	local.iov_base = lp;
	local.iov_len = buffsize;
	remote.iov_base = (void *)*data_ptr;
	remote.iov_len = buffsize;

	TST_EXP_POSITIVE(tst_syscall(__NR_process_vm_writev, pid_alloc, &local,
				     1UL, &remote, 1UL, 0UL));

	if (TST_RET != buffsize) {
		tst_brk(TBROK, "process_vm_writev: expected %d bytes but got %ld",
			buffsize, TST_RET);
	}
}

static void setup(void)
{
	tst_syscall(__NR_process_vm_writev, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	if (tst_parse_int(str_buffsize, &bufsize, 1, INT_MAX))
		tst_brk(TBROK, "Invalid buffer size '%s'", str_buffsize);

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
	pid_t pid_alloc;
	pid_t pid_write;
	int status;

	pid_alloc = SAFE_FORK();
	if (!pid_alloc) {
		child_alloc_and_verify(bufsize);
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	pid_write = SAFE_FORK();
	if (!pid_write) {
		child_write(bufsize, pid_alloc);
		return;
	}

	SAFE_WAITPID(pid_write, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_res(TFAIL, "write child: %s", tst_strstatus(status));

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.options =
		(struct tst_option[]){
			{ "s:", &str_buffsize, "Total buffer size (default 100000)" },
			{},
		},
};
