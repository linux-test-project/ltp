// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 */

/*\
 * Out Of Memory when changing cpuset's mems on NUMA.
 *
 * Regression test based on the reproducers posted in:
 * https://lore.kernel.org/lkml/4BDFFCC4.5000106@cn.fujitsu.com/
 *
 * Fixed in kernel v2.6.35:
 *
 * - 708c1bbc9d0c ("mempolicy: restructure rebinding-mempolicy functions")
 * - c0ff7453bb5c ("cpuset,mm: fix no node to alloc memory when changing cpuset's mems")
 */

#include "config.h"
#include <stdio.h>
#include <sys/wait.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif

#include "tst_test.h"
#include "numa_helper.h"

#ifdef HAVE_NUMA_V2

volatile int end;
static int *nodes;
static int nnodes;
static long ncpus;

static void sighandler(int signo LTP_ATTRIBUTE_UNUSED);
static int mem_hog(void);
static int mem_hog_cpuset(int ntasks);
static long count_cpu(void);

static void test_cpuset(void)
{
	int child, i;
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	char buf[BUFSIZ];

	SAFE_CG_READ(tst_cg, "cpuset.cpus", buf, sizeof(buf));
	SAFE_CG_PRINT(tst_cg, "cpuset.cpus", buf);
	SAFE_CG_READ(tst_cg, "cpuset.mems", buf, sizeof(buf));
	SAFE_CG_PRINT(tst_cg, "cpuset.mems", buf);

	child = SAFE_FORK();
	if (child == 0) {
		for (i = 0; i < nnodes; i++) {
			if (nodes[i] >= MAXNODES)
				continue;
			set_node(nmask, nodes[i]);
		}
		if (set_mempolicy(MPOL_BIND, nmask, MAXNODES) == -1)
			tst_brk(TBROK | TERRNO, "set_mempolicy");
		exit(mem_hog_cpuset(ncpus > 1 ? ncpus : 1));
	}

	SAFE_CG_PRINTF(tst_cg, "cpuset.mems", "%d", nodes[0]);
	SAFE_CG_PRINTF(tst_cg, "cpuset.mems", "%d", nodes[1]);

	tst_reap_children();

	tst_res(TPASS, "cpuset test pass");
}

static void setup(void)
{
	ncpus = count_cpu();
	if (get_allowed_nodes_arr(NH_MEMS | NH_CPUS, &nnodes, &nodes) < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes_arr");
	if (nnodes <= 1)
		tst_brk(TCONF, "requires a NUMA system.");

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());
}

static void sighandler(int signo LTP_ATTRIBUTE_UNUSED)
{
	end = 1;
}

static int mem_hog(void)
{
	long pagesize;
	unsigned long *addr;
	int ret = 0;

	pagesize = getpagesize();
	while (!end) {
		addr = SAFE_MMAP(NULL, pagesize * 10, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		memset(addr, 0xF7, pagesize * 10);
		SAFE_MUNMAP(addr, pagesize * 10);
	}
	return ret;
}

static int mem_hog_cpuset(int ntasks)
{
	int i, status, ret = 0;
	struct sigaction sa;
	pid_t *pids;

	if (ntasks <= 0)
		tst_brk(TBROK | TERRNO, "ntasks is small.");
	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		tst_brk(TBROK | TERRNO, "sigemptyset");
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		tst_brk(TBROK | TERRNO, "sigaction");

	pids = SAFE_MALLOC(sizeof(pid_t) * ntasks);
	for (i = 0; i < ntasks; i++) {
		switch (pids[i] = fork()) {
		case -1:
			tst_res(TFAIL | TERRNO, "fork %d", pids[i]);
			ret = 1;
			break;
		case 0:
			ret = mem_hog();
			exit(ret);
		default:
			break;
		}
	}

	while (i--) {
		if (kill(pids[i], SIGUSR1) == -1) {
			tst_res(TFAIL | TERRNO, "kill %d", pids[i]);
			ret = 1;
		}
	}
	while (waitpid(-1, &status, WUNTRACED | WCONTINUED) > 0) {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) != 0) {
				tst_res(TFAIL, "child exit status is %d",
					 WEXITSTATUS(status));
				ret = 1;
			}
		} else if (WIFSIGNALED(status)) {
			tst_res(TFAIL, "child caught signal %d",
				 WTERMSIG(status));
			ret = 1;
		}
	}
	return ret;
}

static long count_cpu(void)
{
	int ncpus = 0;

	while (tst_path_exists(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	return ncpus;
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.test_all = test_cpuset,
	.needs_cgroup_ctrls = (const char *const []){ "cpuset", NULL },
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
