// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright 2023 Mike Galbraith <efault-AT-gmx.de> */
/* Copyright 2023 Wei Gao <wegao@suse.com> */
/*\
 *
 * [Description]
 *
 * Thread starvation test. On fauluty kernel the test timeouts.
 *
 * Original reproducer taken from:
 * https://lore.kernel.org/lkml/9fd2c37a05713c206dcbd5866f67ce779f315e9e.camel@gmx.de/
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sched.h>

#include "tst_test.h"
#include "tst_kconfig.h"
#include "tst_safe_clocks.h"
#include "tst_timer.h"

static char *str_loop;
static long loop = 1000000;
static char *str_timeout;
static int timeout;

#define CALLIBRATE_LOOPS 120000000

static int callibrate(void)
{
	int i;
	struct timespec start, stop;
	long long diff;

	for (i = 0; i < CALLIBRATE_LOOPS; i++)
		__asm__ __volatile__ ("" : "+g" (i) : :);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &start);

	for (i = 0; i < CALLIBRATE_LOOPS; i++)
		__asm__ __volatile__ ("" : "+g" (i) : :);

	SAFE_CLOCK_GETTIME(CLOCK_MONOTONIC_RAW, &stop);

	diff = tst_timespec_diff_us(stop, start);

	tst_res(TINFO, "CPU did %i loops in %llius", CALLIBRATE_LOOPS, diff);

	return diff;
}

static int wait_for_pid(pid_t pid)
{
	int status, ret;

again:
	ret = waitpid(pid, &status, 0);
	if (ret == -1) {
		if (errno == EINTR)
			goto again;

		return -1;
	}

	if (WIFSIGNALED(status))
		return 0;

	return -1;
}

static void setup(void)
{
	cpu_set_t mask;
	int cpu = 0;
	long ncpus = tst_ncpus_conf();

	CPU_ZERO(&mask);

	/* Restrict test to a single cpu */
	if (sched_getaffinity(0, sizeof(mask), &mask) < 0)
		tst_brk(TBROK | TERRNO, "sched_getaffinity() failed");

	if (CPU_COUNT(&mask) == 0)
		tst_brk(TBROK, "No cpus available");

	while (CPU_ISSET(cpu, &mask) == 0 && cpu < ncpus)
		cpu++;

	CPU_ZERO(&mask);

	CPU_SET(cpu, &mask);

	tst_res(TINFO, "Setting affinity to CPU %d", cpu);

	if (sched_setaffinity(0, sizeof(mask), &mask) < 0)
		tst_brk(TBROK | TERRNO, "sched_setaffinity() failed");

	if (tst_parse_long(str_loop, &loop, 1, LONG_MAX))
		tst_brk(TBROK, "Invalid number of loop number '%s'", str_loop);

	if (tst_parse_int(str_timeout, &timeout, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of timeout '%s'", str_timeout);
	else
		timeout = callibrate() / 1000;

	if (tst_has_slow_kconfig())
		tst_brk(TCONF, "Skip test due to slow kernel configuration");

	tst_set_max_runtime(timeout);
}

static void handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	if (loop > 0)
		--loop;
}

static void child(void)
{
	pid_t ppid = getppid();

	TST_CHECKPOINT_WAIT(0);

	while (1)
		SAFE_KILL(ppid, SIGUSR1);
}

static void do_test(void)
{
	pid_t child_pid;

	child_pid = SAFE_FORK();

	if (!child_pid)
		child();

	SAFE_SIGNAL(SIGUSR1, handler);
	TST_CHECKPOINT_WAKE(0);

	while (loop)
		sleep(1);

	SAFE_KILL(child_pid, SIGTERM);

	if (!tst_remaining_runtime())
		tst_res(TFAIL, "Scheduler starvation reproduced");
	else
		tst_res(TPASS, "Haven't reproduced scheduler starvation");

	TST_EXP_PASS_SILENT(wait_for_pid(child_pid));
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"l:", &str_loop, "Number of loops (default 2000000)"},
		{"t:", &str_timeout, "Max timeout (default 240s)"},
		{}
	},
	.needs_checkpoints = 1,
};
