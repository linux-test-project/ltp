/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * Out Of Memory when changing cpuset's mems on NUMA. There was a
 * problem reported upstream that the allocator may see an empty
 * nodemask when changing cpuset's mems.
 * http://lkml.org/lkml/2010/5/4/77
 * http://lkml.org/lkml/2010/5/4/79
 * http://lkml.org/lkml/2010/5/4/80
 * This test is based on the reproducers for the above issue.
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

#include "mem.h"
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
	int child, i, status;
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	char mems[BUFSIZ], buf[BUFSIZ];

	read_cpuset_files(CPATH, "cpus", buf);
	write_cpuset_files(CPATH_NEW, "cpus", buf);
	read_cpuset_files(CPATH, "mems", mems);
	write_cpuset_files(CPATH_NEW, "mems", mems);
	SAFE_FILE_PRINTF(CPATH_NEW "/tasks", "%d", getpid());

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

	snprintf(buf, BUFSIZ, "%d", nodes[0]);
	write_cpuset_files(CPATH_NEW, "mems", buf);
	snprintf(buf, BUFSIZ, "%d", nodes[1]);
	write_cpuset_files(CPATH_NEW, "mems", buf);

	SAFE_WAITPID(child, &status, WUNTRACED | WCONTINUED);
	if (WEXITSTATUS(status) != 0) {
		tst_res(TFAIL, "child exit status is %d", WEXITSTATUS(status));
		return;
	}

	tst_res(TPASS, "cpuset test pass");
}

static void setup(void)
{
	mount_mem("cpuset", "cpuset", NULL, CPATH, CPATH_NEW);
	ncpus = count_cpu();
	if (get_allowed_nodes_arr(NH_MEMS | NH_CPUS, &nnodes, &nodes) < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes_arr");
	if (nnodes <= 1)
		tst_brk(TCONF, "requires a NUMA system.");
}

static void cleanup(void)
{
	umount_mem(CPATH, CPATH_NEW);
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

	while (path_exist(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	return ncpus;
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_cpuset,
	.min_kver = "2.6.32",
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
