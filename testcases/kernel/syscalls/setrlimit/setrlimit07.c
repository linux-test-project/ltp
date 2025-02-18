// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Red Hat Inc. All Rights Reserved.
 * Author: Chunfu Wen <chwen@redhat.com>
 */

/*
 * Description:
 * Set CPU time limit64 for a process and check its behavior
 * after reaching CPU time limit64.
 * 1) Process got SIGXCPU after reaching soft limit of CPU time limit64.
 * 2) Process got SIGKILL after reaching hard limit of CPU time limit64.
 *
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <inttypes.h>

#include "tst_test.h"

#include "lapi/syscalls.h"
#include "lapi/abisize.h"

#ifndef HAVE_STRUCT_RLIMIT64
struct rlimit64 {
	uint64_t rlim_cur;
	uint64_t rlim_max;
};
#endif

static int *end;

static void sighandler(int sig)
{
	*end = sig;
}

static void setup(void)
{
	SAFE_SIGNAL(SIGXCPU, sighandler);

	end = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (end)
		SAFE_MUNMAP(end, sizeof(int));
}

static int setrlimit_u64(int resource, const struct rlimit64 *rlim)
{
    return tst_syscall(__NR_prlimit64, 0, resource, rlim, NULL);
}

static void verify_setrlimit64(void)
{
	int status;
	pid_t pid;
	struct rlimit64 rlim;
	rlim.rlim_cur = 1;
	rlim.rlim_max = 2;

	*end = 0;

	pid = SAFE_FORK();
	if (!pid) {
		TEST(setrlimit_u64(RLIMIT_CPU, &rlim));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO,
				"setrlimit_u64(RLIMIT_CPU) failed");
			exit(1);
		}

		alarm(20);

		while (1);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
		return;

	if (WIFSIGNALED(status)) {
		if (WTERMSIG(status) == SIGKILL && *end == SIGXCPU) {
			tst_res(TPASS,
				"Got SIGXCPU then SIGKILL after reaching both limit");
			return;
		}

		if (WTERMSIG(status) == SIGKILL && !*end) {
			tst_res(TFAIL,
				"Got only SIGKILL after reaching both limit");
			return;
		}

		if (WTERMSIG(status) == SIGALRM && *end == SIGXCPU) {
			tst_res(TFAIL,
				"Got only SIGXCPU after reaching both limit");
			return;
		}

		if (WTERMSIG(status) == SIGALRM && !*end) {
			tst_res(TFAIL,
				"Got no signal after reaching both limit");
			return;
		}
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = verify_setrlimit64,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
