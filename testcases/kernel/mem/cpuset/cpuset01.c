/*
 * Out Of Memory when changing cpuset's mems on NUMA. There was a
 * problem reported upstream that the allocator may see an empty
 * nodemask when changing cpuset's mems.
 * http://lkml.org/lkml/2010/5/4/77
 * http://lkml.org/lkml/2010/5/4/79
 * http://lkml.org/lkml/2010/5/4/80
 * This test is based on the reproducers for the above issue.
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "mem.h"
#include "numa_helper.h"

char *TCID = "cpuset01";
int TST_TOTAL = 1;

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
volatile int end;
static int *nodes;
static int nnodes;
static long ncpus;

static void testcpuset(void);
static void sighandler(int signo LTP_ATTRIBUTE_UNUSED);
static int mem_hog(void);
static int mem_hog_cpuset(int ntasks);
static long count_cpu(void);

int main(int argc, char *argv[])
{

	tst_parse_opts(argc, argv, NULL, NULL);

	ncpus = count_cpu();
	if (get_allowed_nodes_arr(NH_MEMS | NH_CPUS, &nnodes, &nodes) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes_arr");
	if (nnodes <= 1)
		tst_brkm(TCONF, NULL, "requires a NUMA system.");

	setup();
	testcpuset();
	cleanup();
	tst_exit();
}

static void testcpuset(void)
{
	int lc;
	int child, i, status;
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	char mems[BUFSIZ], buf[BUFSIZ];

	read_cpuset_files(CPATH, "cpus", buf);
	write_cpuset_files(CPATH_NEW, "cpus", buf);
	read_cpuset_files(CPATH, "mems", mems);
	write_cpuset_files(CPATH_NEW, "mems", mems);
	SAFE_FILE_PRINTF(cleanup, CPATH_NEW "/tasks", "%d", getpid());

	switch (child = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		for (i = 0; i < nnodes; i++) {
			if (nodes[i] >= MAXNODES)
				continue;
			set_node(nmask, nodes[i]);
		}
		if (set_mempolicy(MPOL_BIND, nmask, MAXNODES) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "set_mempolicy");
		exit(mem_hog_cpuset(ncpus > 1 ? ncpus : 1));
	}
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		snprintf(buf, BUFSIZ, "%d", nodes[0]);
		write_cpuset_files(CPATH_NEW, "mems", buf);
		snprintf(buf, BUFSIZ, "%d", nodes[1]);
		write_cpuset_files(CPATH_NEW, "mems", buf);
	}

	if (waitpid(child, &status, WUNTRACED | WCONTINUED) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
	if (WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child exit status is %d", WEXITSTATUS(status));
}

void setup(void)
{
	tst_require_root();

	if (tst_kvercmp(2, 6, 32) < 0)
		tst_brkm(TCONF, NULL, "2.6.32 or greater kernel required");

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	mount_mem("cpuset", "cpuset", NULL, CPATH, CPATH_NEW);
}

void cleanup(void)
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
		addr = mmap(NULL, pagesize * 10, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (addr == MAP_FAILED) {
			ret = 1;
			tst_resm(TFAIL | TERRNO, "mmap");
			break;
		}
		memset(addr, 0xF7, pagesize * 10);
		munmap(addr, pagesize * 10);
	}
	return ret;
}

static int mem_hog_cpuset(int ntasks)
{
	int i, lc, status, ret = 0;
	struct sigaction sa;
	pid_t *pids;

	if (ntasks <= 0)
		tst_brkm(TBROK | TERRNO, cleanup, "ntasks is small.");
	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sigemptyset");
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "sigaction");

	pids = malloc(sizeof(pid_t) * ntasks);
	if (pids == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc");
	for (i = 0; i < ntasks; i++) {
		switch (pids[i] = fork()) {
		case -1:
			tst_resm(TFAIL | TERRNO, "fork %d", pids[i]);
			ret = 1;
			break;
		case 0:
			ret = mem_hog();
			exit(ret);
		default:
			break;
		}
	}
	for (lc = 0; TEST_LOOPING(lc); lc++) ;
	while (i--) {
		if (kill(pids[i], SIGUSR1) == -1) {
			tst_resm(TFAIL | TERRNO, "kill %d", pids[i]);
			ret = 1;
		}
	}
	while (waitpid(-1, &status, WUNTRACED | WCONTINUED) > 0) {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "child exit status is %d",
					 WEXITSTATUS(status));
				ret = 1;
			}
		} else if (WIFSIGNALED(status)) {
			tst_resm(TFAIL, "child caught signal %d",
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

#else /* no NUMA */
int main(void)
{
	tst_brkm(TCONF, NULL, "no NUMA development packages installed.");
}
#endif
