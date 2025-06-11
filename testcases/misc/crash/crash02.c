// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC <andrea.cervesato@suse.com>
 */

/*\
 * Test the robustness of the system generating random syscalls execution
 * with random data and expecting that the current system is not crashing.
 */

#include <stdlib.h>
#include <limits.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define MAX_SYSCALLS 465

static int *num_errors;
static char *str_num_tries;
static char *str_seed;
static int num_tries = 1000;
static int seed;

static int blacklist[] = {
	__NR_vfork,
	__NR_fork,
	__NR_clone,
	__NR_clone2,
	__NR_clone3,
	__NR_vhangup,		/* terminal logout */
	__NR_pause,		/* sleep indefinitely */
	__NR_read,		/* sleep indefinitely if the first argument is 0 */
	__NR_kill,		/* might kill test */
	__NR_restart_syscall,	/* restart random syscalls */
	__LTP__NR_INVALID_SYSCALL,
};

static long rand_number(void)
{
	long num = 0;

	for (size_t i = 0; i < sizeof(long); i++)
		num |= ((rand() & 0xFFUL) << (i * 8));

	return num;
}

static int in_blacklist(const int sysno)
{
	for (size_t i = 0; i < ARRAY_SIZE(blacklist); i++)
		if (sysno == blacklist[i])
			return 1;

	return 0;
}

static void try_crash(const int num)
{
	long sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6;
	int ret;

	do {
		sysno = rand() % MAX_SYSCALLS;
	} while (in_blacklist(sysno));

	arg0 = rand_number();
	arg1 = rand_number();
	arg2 = rand_number();
	arg3 = rand_number();
	arg4 = rand_number();
	arg5 = rand_number();
	arg6 = rand_number();

	tst_res(TDEBUG,
		"%d: syscall(%ld, %#lx, %#lx, %#lx, %#lx, %#lx, %#lx, %#lx)",
		num, sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6);

	ret = syscall(sysno, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	if (ret == -1) {
		(*num_errors)++;

		tst_res(TDEBUG, "syscall error: %s", strerror(errno));
	}
}

static void run(void)
{
	pid_t pid;
	int status;
	int num_signals = 0;

	*num_errors = 0;

	pid = SAFE_FORK();
	if (!pid) {
		for (int i = 0; i < num_tries; i++)
			try_crash(i);

		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status)) {
		num_signals++;

		tst_res(TDEBUG, "syscall signaled: %s",
			strsignal(WTERMSIG(status)));
	}

	tst_res(TINFO, "Detected errors: %d", *num_errors);
	tst_res(TINFO, "Detected signals: %d", num_signals);

	tst_res(TPASS, "System is still up and running");
}

static void setup(void)
{
	if (tst_parse_int(str_num_tries, &num_tries, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of entries '%s'", str_num_tries);

	if (tst_parse_int(str_seed, &seed, 0, INT_MAX))
		tst_brk(TBROK, "Invalid seed number '%s'", str_num_tries);

	num_errors = SAFE_MMAP(
		NULL, sizeof(int),
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS,
		-1, 0);

	seed = str_seed ? seed : time(NULL);
	srand(seed);

	tst_res(TINFO, "Random seed: %d", seed);
	tst_res(TINFO, "Number of tries: %d", num_tries);

	tst_set_runtime((num_tries / 1000) + 1);
}

static void cleanup(void)
{
	if (num_errors)
		SAFE_MUNMAP(num_errors, sizeof(int));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.options = (struct tst_option []) {
		{"n:", &str_num_tries, "Number of retries (default: 1000)"},
		{"s:", &str_seed, "Initial seed for random generator"},
		{}
	},
};
