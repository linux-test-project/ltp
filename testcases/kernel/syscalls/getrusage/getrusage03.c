// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2011  Red Hat, Inc.
 * Copyright (C) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * Test ru_maxrss behaviors in struct rusage.
 *
 * This test program is backported from upstream commit: 1f10206cf8e9, which
 * fills ru_maxrss value in struct rusage according to rss hiwater mark. To
 * make sure this feature works correctly, a series of tests are executed in
 * this program.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "tst_test.h"
#include "lapi/cpuset.h"
#include "lapi/sched.h"
#include "getrusage03.h"

#define TESTBIN "getrusage03_child"

static struct rusage ru;
static long maxrss_init;
static long lower_allowance;

static const char *const resource[] = {
	TESTBIN,
	NULL,
};

/*
 * Pin the test to the CPU it runs on. Every forked or exec'd child inherits
 * the affinity, so only that CPU can hold RSS counter updates outside the
 * fast percpu_counter read.
 */
static void pin_to_current_cpu(void)
{
	unsigned int cpu;
	cpu_set_t *set;
	size_t size;

	if (getcpu(&cpu, NULL))
		tst_brk(TBROK | TERRNO, "getcpu() failed");

	set = CPU_ALLOC(cpu + 1);
	if (!set)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC() failed");

	size = CPU_ALLOC_SIZE(cpu + 1);
	CPU_ZERO_S(size, set);
	CPU_SET_S(cpu, size, set);
	if (sched_setaffinity(0, size, set))
		tst_brk(TBROK | TERRNO, "sched_setaffinity() failed");

	CPU_FREE(set);
	tst_res(TINFO, "Pinned to CPU %u", cpu);
}

/*
 * The pinned CPU can still hold up to one percpu_counter batch,
 * max(32, 2 * online CPUs) pages, outside the fast RSS read.
 */
static void setup(void)
{
	long online_cpus = tst_ncpus_global();
	long batch = MAX(32L, online_cpus * 2);
	long page_size = SAFE_SYSCONF(_SC_PAGESIZE);
	long batch_kib = batch * page_size / 1024;

	lower_allowance = MAX(20 * 1024L, batch_kib);
	if (lower_allowance >= 102400L)
		tst_brk(TCONF, "Per-CPU RSS allowance is too large: %li KiB",
			lower_allowance);

	tst_res(TINFO, "%li online CPUs, lower RSS allowance %li KiB",
		online_cpus, lower_allowance);

	pin_to_current_cpu();
}

static void check_maxrss(long actual, long expected, const char *name,
			 const char *size)
{
	if (actual >= expected - lower_allowance &&
	    actual <= expected + DELTA_MAX)
		tst_res(TPASS, "%s ~= %s", name, size);
	else
		tst_res(TFAIL, "%s = %li, expected %li", name, actual, expected);
}

static void inherit_fork1(void)
{
	SAFE_GETRUSAGE(RUSAGE_SELF, &ru);
	maxrss_init = ru.ru_maxrss;

	if (!SAFE_FORK()) {
		SAFE_GETRUSAGE(RUSAGE_SELF, &ru);

		if (is_in_delta(maxrss_init - ru.ru_maxrss))
			tst_res(TPASS, "initial.self ~= child.self");
		else
			tst_res(TFAIL, "child.self = %li, expected %li",
				ru.ru_maxrss, maxrss_init);
		exit(0);
	}
	tst_reap_children();
}

static void inherit_fork2(void)
{
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

	check_maxrss(ru.ru_maxrss, 102400, "initial.children", "100MB");

	if (!SAFE_FORK()) {
		SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

		if (!ru.ru_maxrss)
			tst_res(TPASS, "child.children == 0");
		else
			tst_res(TFAIL, "child.children = %li, expected %i",
				ru.ru_maxrss, 0);
		exit(0);
	}
	tst_reap_children();
}

static void grandchild_maxrss(void)
{
	if (!SAFE_FORK())
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "grand_consume", "300", NULL);
	tst_reap_children();
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);

	check_maxrss(ru.ru_maxrss, 307200, "child.children", "300MB");
}

static void zombie(void)
{
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	maxrss_init = ru.ru_maxrss;

	pid_t pid = SAFE_FORK();

	if (!pid)
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "consume", "400", NULL);

	TST_PROCESS_STATE_WAIT(pid, 'Z', 0);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_res(TPASS, "initial.children ~= pre_wait.children");
	else
		tst_res(TFAIL, "pre_wait.children = %li, expected %li",
			ru.ru_maxrss, maxrss_init);

	tst_reap_children();
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	check_maxrss(ru.ru_maxrss, 409600, "post_wait.children", "400MB");
}

static void sig_ign(void)
{
	SAFE_SIGNAL(SIGCHLD, SIG_IGN);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	maxrss_init = ru.ru_maxrss;

	pid_t pid = SAFE_FORK();

	if (!pid)
		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "consume", "500", NULL);

	TST_PROCESS_EXIT_WAIT(pid, 0);
	SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
	if (is_in_delta(ru.ru_maxrss - maxrss_init))
		tst_res(TPASS, "initial.children ~= after_zombie.children");
	else
		tst_res(TFAIL, "after_zombie.children = %li, expected %li",
			ru.ru_maxrss, maxrss_init);

	SAFE_SIGNAL(SIGCHLD, SIG_DFL);
}

static void inherit_exec(void)
{
	if (!SAFE_FORK()) {
		char str_maxrss_self[BUFSIZ], str_maxrss_child[BUFSIZ];

		SAFE_GETRUSAGE(RUSAGE_SELF, &ru);
		sprintf(str_maxrss_self, "%ld", ru.ru_maxrss);
		SAFE_GETRUSAGE(RUSAGE_CHILDREN, &ru);
		sprintf(str_maxrss_child, "%ld", ru.ru_maxrss);

		SAFE_EXECLP("getrusage03_child", "getrusage03_child",
			    "compare", str_maxrss_self, str_maxrss_child, NULL);
	}
	tst_reap_children();
}

void (*testfunc_list[])(void) = {
	inherit_fork1, inherit_fork2, grandchild_maxrss,
	zombie, sig_ign, inherit_exec
};

static void run(unsigned int i)
{
	if (!SAFE_FORK()) {
		if (!SAFE_FORK()) {
			consume_mb(100);
			exit(0);
		}

		SAFE_WAIT(NULL);

		testfunc_list[i]();
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.setup = setup,
	.resource_files = resource,
	.min_mem_avail = 512,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "1f10206cf8e945220f7220a809d8bfc15c21f9a5"},
		{}
	},
	.test = run,
	.tcnt = ARRAY_SIZE(testfunc_list),
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_IPC_LOCK),
		{}
	},
};
