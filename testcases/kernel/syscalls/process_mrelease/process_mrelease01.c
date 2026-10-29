// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test verifies that :manpage:`process_mrelease(2)` syscall releases memory
 * from a killed process with pending memory allocations.
 *
 * [Algorithm]
 *
 * - Spawn a child process allocating and dirtying anonymous memory
 * - Wait until the child is ready via checkpoints
 * - Obtain child's pidfd and send SIGKILL to the child
 * - Invoke process_mrelease(pidfd, 0)
 * - If process_mrelease() fails with ESRCH (child terminated too fast),
 *   double the allocation size and retry
 * - Check that process_mrelease() succeeds
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

#define CHUNK (1 * TST_MB)
#define MAX_SIZE_MB (1024 * TST_MB)

static void do_child(size_t size)
{
	void *mem;

	tst_res(TINFO, "Child: allocate %zu MB", size / TST_MB);

	mem = SAFE_MMAP(NULL,
		size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANON,
		-1, 0);

	memset(mem, 0, size);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_MUNMAP(mem, size);
}

static void run(void)
{
	size_t mem_size;
	pid_t pid;
	int pidfd;
	int status;

	for (mem_size = CHUNK; mem_size <= MAX_SIZE_MB; mem_size *= 2) {
		pid = SAFE_FORK();
		if (!pid) {
			do_child(mem_size);
			exit(0);
		}

		TST_CHECKPOINT_WAIT(0);

		pidfd = SAFE_PIDFD_OPEN(pid, 0);

		SAFE_KILL(pid, SIGKILL);

		TEST(tst_syscall(__NR_process_mrelease, pidfd, 0));

		SAFE_WAITPID(pid, &status, 0);
		SAFE_CLOSE(pidfd);

		if (TST_RET == 0) {
			tst_res(TPASS, "process_mrelease() passed with %zu MB",
				mem_size / TST_MB);
			return;
		}

		if (TST_ERR != ESRCH) {
			tst_res(TFAIL | TTERRNO, "process_mrelease() failed");
			return;
		}

		tst_res(TINFO,
			"Child terminated before process_mrelease(). Increasing memory size and retrying");
	}

	tst_res(TFAIL, "All process_mrelease() attempts failed with ESRCH");
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.min_kver = "5.15",
	.needs_checkpoints = 1,
};
