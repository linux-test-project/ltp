// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2012
 * Copyright (c) Linux Test Project, 2012
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Fork two children, one child mallocs randomly sized trunks of memory
 * and initializes them; the other child calls process_vm_readv with
 * the remote iovecs initialized to the original process memory
 * locations and the local iovecs initialized to randomly sized and
 * allocated local memory locations. The second child then verifies
 * that the data is copied correctly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define MAX_IOVECS 1024

static struct tcase {
	int bufsize;
	int remote_iovecs;
	int local_iovecs;
} testcases[] = {
	{ .bufsize = 1024, .remote_iovecs = 1024, .local_iovecs = 8 },
	{ .bufsize = 1024, .remote_iovecs = 512, .local_iovecs = 16 },
	{ .bufsize = 1024, .remote_iovecs = 256, .local_iovecs = 32 },
	{ .bufsize = 1024, .remote_iovecs = 128, .local_iovecs = 64 },
	{ .bufsize = 1024, .remote_iovecs = 64, .local_iovecs = 128 },
	{ .bufsize = 1024, .remote_iovecs = 32, .local_iovecs = 256 },
	{ .bufsize = 1024, .remote_iovecs = 16, .local_iovecs = 512 },
	{ .bufsize = 1024, .remote_iovecs = 8, .local_iovecs = 1024 },

	{ .bufsize = 131072, .remote_iovecs = 1024, .local_iovecs = 8 },
	{ .bufsize = 131072, .remote_iovecs = 512, .local_iovecs = 16 },
	{ .bufsize = 131072, .remote_iovecs = 256, .local_iovecs = 32 },
	{ .bufsize = 131072, .remote_iovecs = 128, .local_iovecs = 64 },
	{ .bufsize = 131072, .remote_iovecs = 64, .local_iovecs = 128 },
	{ .bufsize = 131072, .remote_iovecs = 32, .local_iovecs = 256 },
	{ .bufsize = 131072, .remote_iovecs = 16, .local_iovecs = 512 },
	{ .bufsize = 131072, .remote_iovecs = 8, .local_iovecs = 1024 },
};

static char **data_ptr;

static void create_data_size(int *arr, int arr_sz, int buffsize)
{
	long bufsz_left;
	int i;

	bufsz_left = buffsize;
	for (i = 0; i < arr_sz - 1; i++) {
		arr[i] = rand() % ((bufsz_left / 2) + 1);
		bufsz_left -= arr[i];
	}

	arr[arr_sz - 1] = bufsz_left;
}

static void child_alloc(const int *sizes, int nr_iovecs)
{
	int i, j;
	long count;

	count = 0;
	for (i = 0; i < nr_iovecs; i++) {
		data_ptr[i] = sizes[i] ? SAFE_MALLOC(sizes[i]) : NULL;

		for (j = 0; j < sizes[i]; j++) {
			data_ptr[i][j] = count % 256;
			count++;
		}
	}

	tst_res(TINFO, "child_alloc: memory allocated and initialized");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);
}

static void child_read(const int *sizes, int local_iovecs, int remote_iovecs,
			 pid_t pid_alloc, int buffsize)
{
	struct iovec local[local_iovecs];
	struct iovec remote[remote_iovecs];
	int i, j;
	int count;
	int nr_error;
	int local_sizes[local_iovecs];
	unsigned char expect, actual;

	for (i = 0; i < remote_iovecs; i++) {
		remote[i].iov_base = (void *)data_ptr[i];
		remote[i].iov_len = sizes[i];
	}

	create_data_size(local_sizes, local_iovecs, buffsize);
	for (i = 0; i < local_iovecs; i++) {
		local[i].iov_base = SAFE_MALLOC(local_sizes[i]);
		local[i].iov_len = local_sizes[i];
	}

	tst_res(TINFO, "child_read: reading string from same memory location");

	TST_EXP_POSITIVE(tst_syscall(__NR_process_vm_readv, pid_alloc, local,
				     local_iovecs, remote, remote_iovecs, 0UL),
			 "process_vm_read()");

	if (TST_RET != buffsize) {
		tst_brk(TBROK, "process_vm_readv: expected %d bytes but got %ld",
			buffsize, TST_RET);
	}

	count = 0;
	nr_error = 0;
	for (i = 0; i < local_iovecs; i++) {
		for (j = 0; j < (int)local[i].iov_len; j++) {
			expect = count % 256;
			actual = ((unsigned char *)local[i].iov_base)[j];
			if (expect != actual)
				nr_error++;

			count++;
		}
	}

	if (nr_error)
		tst_brk(TFAIL, "child_read: %d incorrect bytes received", nr_error);
	else
		tst_res(TPASS, "child_read: all bytes are correctly received");
}

static void setup(void)
{
	tst_syscall(__NR_process_vm_readv, getpid(), NULL, 0UL, NULL, 0UL, 0UL);

	data_ptr = SAFE_MMAP(NULL, sizeof(void *) * MAX_IOVECS, PROT_READ | PROT_WRITE,
			     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (data_ptr)
		SAFE_MUNMAP(data_ptr, sizeof(void *) * MAX_IOVECS);
}

static void run(unsigned int i)
{
	int bufsize = testcases[i].bufsize;
	int remote_iovecs = testcases[i].remote_iovecs;
	int local_iovecs = testcases[i].local_iovecs;
	pid_t pid_alloc;
	pid_t pid_read;
	int status;
	int sizes[remote_iovecs];

	tst_res(TINFO, "bufsize=%d, remote_iovecs=%d, local_iovecs=%d", bufsize,
		remote_iovecs, local_iovecs);

	create_data_size(sizes, remote_iovecs, bufsize);

	pid_alloc = SAFE_FORK();
	if (!pid_alloc) {
		child_alloc(sizes, remote_iovecs);
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	pid_read = SAFE_FORK();
	if (!pid_read) {
		child_read(sizes, local_iovecs, remote_iovecs, pid_alloc, bufsize);
		return;
	}

	SAFE_WAITPID(pid_read, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		tst_res(TFAIL, "child_read: %s", tst_strstatus(status));

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.tcnt = ARRAY_SIZE(testcases),
};
