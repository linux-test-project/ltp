// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 */

/*
 * use migrate_pages() and check that address is on correct node
 * 1. process A can migrate its non-shared mem with CAP_SYS_NICE
 * 2. process A can migrate its non-shared mem without CAP_SYS_NICE
 * 3. process A can migrate shared mem only with CAP_SYS_NICE
 * 4. process A can migrate non-shared mem in process B with same effective uid
 * 5. process A can migrate non-shared mem in process B with CAP_SYS_NICE
 */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "numa_helper.h"
#include "migrate_pages_common.h"

/*
 * This is an estimated minimum of free mem required to migrate this
 * process to another node as migrate_pages will fail if there is not
 * enough free space on node. While running this test on x86_64
 * it used ~2048 pages (total VM, not just RSS). Considering ia64 as
 * architecture with largest (non-huge) page size (16k), this limit
 * is set to 2048*16k == 32M.
 */
#define NODE_MIN_FREEMEM (32*1024*1024)

#ifdef HAVE_NUMA_V2

static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;
static int *nodes, nodeA, nodeB;
static int num_nodes;

static void print_mem_stats(pid_t pid, int node)
{
	char s[64];
	long long node_size, freep;

	if (pid == 0)
		pid = getpid();

	tst_res(TINFO, "mem_stats pid: %d, node: %d", pid, node);

	/* dump pid's VM info */
	sprintf(s, "cat /proc/%d/status", pid);
	system(s);
	sprintf(s, "cat /proc/%d/numa_maps", pid);
	system(s);

	/* dump node free mem */
	node_size = numa_node_size64(node, &freep);
	tst_res(TINFO, "Node id: %d, size: %lld, free: %lld",
		 node, node_size, freep);
}

static int migrate_to_node(pid_t pid, int node)
{
	unsigned long nodemask_size, max_node;
	unsigned long *old_nodes, *new_nodes;
	int i;

	tst_res(TINFO, "pid(%d) migrate pid %d to node -> %d",
		 getpid(), pid, node);
	max_node = LTP_ALIGN(get_max_node(), sizeof(unsigned long)*8);
	nodemask_size = max_node / 8;
	old_nodes = SAFE_MALLOC(nodemask_size);
	new_nodes = SAFE_MALLOC(nodemask_size);

	memset(old_nodes, 0, nodemask_size);
	memset(new_nodes, 0, nodemask_size);
	for (i = 0; i < num_nodes; i++)
		set_bit(old_nodes, nodes[i], 1);
	set_bit(new_nodes, node, 1);

	TEST(tst_syscall(__NR_migrate_pages, pid, max_node, old_nodes,
		new_nodes));
	if (TST_RET != 0) {
		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO, "migrate_pages failed "
				 "ret: %ld, ", TST_RET);
			print_mem_stats(pid, node);
		} else {
			tst_res(TINFO, "migrate_pages could not migrate all "
				 "pages, not migrated: %ld", TST_RET);
		}
	}
	free(old_nodes);
	free(new_nodes);
	return TST_RET;
}

static int addr_on_node(void *addr)
{
	int node;
	int ret;

	ret = tst_syscall(__NR_get_mempolicy, &node, NULL, (unsigned long)0,
		      (unsigned long)addr, MPOL_F_NODE | MPOL_F_ADDR);
	if (ret == -1) {
		tst_res(TFAIL | TERRNO,
				"error getting memory policy for page %p", addr);
	}
	return node;
}

static int check_addr_on_node(void *addr, int exp_node)
{
	int node;

	node = addr_on_node(addr);
	if (node == exp_node) {
		tst_res(TPASS, "pid(%d) addr %p is on expected node: %d",
			 getpid(), addr, exp_node);
		return TPASS;
	} else {
		tst_res(TFAIL, "pid(%d) addr %p not on expected node: %d "
			 ", expected %d", getpid(), addr, node, exp_node);
		print_mem_stats(0, exp_node);
		return TFAIL;
	}
}

static void test_migrate_current_process(int node1, int node2, int cap_sys_nice)
{
	char *private, *shared;
	int ret;
	pid_t child;

	/* parent can migrate its non-shared memory */
	tst_res(TINFO, "current_process, cap_sys_nice: %d", cap_sys_nice);
	private =  SAFE_MMAP(NULL, getpagesize(), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	private[0] = 0;
	tst_res(TINFO, "private anonymous: %p", private);

	migrate_to_node(0, node2);
	check_addr_on_node(private, node2);
	migrate_to_node(0, node1);
	check_addr_on_node(private, node1);
	SAFE_MUNMAP(private, getpagesize());

	/* parent can migrate shared memory with CAP_SYS_NICE */
	shared = SAFE_MMAP(NULL, getpagesize(), PROT_READ | PROT_WRITE,
		      MAP_ANONYMOUS | MAP_SHARED, 0, 0);
	shared[0] = 1;
	tst_res(TINFO, "shared anonymous: %p", shared);
	migrate_to_node(0, node2);
	check_addr_on_node(shared, node2);

	/* shared mem is on node2, try to migrate in child to node1 */
	fflush(stdout);
	child = SAFE_FORK();
	if (child == 0) {
		tst_res(TINFO, "child shared anonymous, cap_sys_nice: %d",
			 cap_sys_nice);
		private =  SAFE_MMAP(NULL, getpagesize(),
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		private[0] = 1;
		shared[0] = 1;
		if (!cap_sys_nice)
			SAFE_SETEUID(ltpuser->pw_uid);

		migrate_to_node(0, node1);
		/* child can migrate non-shared memory */
		ret = check_addr_on_node(private, node1);

		exit(ret);
	}

	SAFE_WAITPID(child, NULL, 0);
	if (cap_sys_nice)
		/* child can migrate shared memory only
		 * with CAP_SYS_NICE */
		check_addr_on_node(shared, node1);
	else
		check_addr_on_node(shared, node2);
	SAFE_MUNMAP(shared, getpagesize());
}

static void test_migrate_other_process(int node1, int node2, int cap_sys_nice)
{
	char *private;
	int ret;
	pid_t child1, child2;

	tst_res(TINFO, "other_process, cap_sys_nice: %d", cap_sys_nice);

	fflush(stdout);
	child1 = SAFE_FORK();
	if (child1 == 0) {
		private =  SAFE_MMAP(NULL, getpagesize(),
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		private[0] = 0;

		/* make sure we are on node1 */
		migrate_to_node(0, node1);
		check_addr_on_node(private, node1);

		SAFE_SETUID(ltpuser->pw_uid);

		/* commit_creds() will clear dumpable, restore it */
		if (prctl(PR_SET_DUMPABLE, 1))
			tst_brk(TBROK | TERRNO, "prctl");

		/* signal child2 it's OK to migrate child1 and wait */
		TST_CHECKPOINT_WAKE(0);
		TST_CHECKPOINT_WAIT(1);

		/* child2 can migrate child1 process if it's privileged */
		/* child2 can migrate child1 process if it has same uid */
		ret = check_addr_on_node(private, node2);

		exit(ret);
	}

	fflush(stdout);
	child2 = SAFE_FORK();
	if (child2 == 0) {
		if (!cap_sys_nice)
			SAFE_SETUID(ltpuser->pw_uid);

		/* wait until child1 is ready on node1, then migrate and
		 * signal to check current node */
		TST_CHECKPOINT_WAIT(0);
		migrate_to_node(child1, node2);
		TST_CHECKPOINT_WAKE(1);

		exit(TPASS);
	}

	SAFE_WAITPID(child1, NULL, 0);
	SAFE_WAITPID(child2, NULL, 0);
}

static void run(void)
{
	test_migrate_current_process(nodeA, nodeB, 1);
	test_migrate_current_process(nodeA, nodeB, 0);
	test_migrate_other_process(nodeA, nodeB, 1);
	test_migrate_other_process(nodeA, nodeB, 0);
}

static void setup(void)
{
	int ret, i, j;
	int pagesize = getpagesize();
	void *p;

	tst_syscall(__NR_migrate_pages, 0, 0, NULL, NULL);

	if (numa_available() == -1)
		tst_brk(TCONF, "NUMA not available");

	ret = get_allowed_nodes_arr(NH_MEMS, &num_nodes, &nodes);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "get_allowed_nodes(): %d", ret);

	if (num_nodes < 2)
		tst_brk(TCONF, "at least 2 allowed NUMA nodes"
			 " are required");
	else if (tst_kvercmp(2, 6, 18) < 0)
		tst_brk(TCONF, "2.6.18 or greater kernel required");

	/*
	 * find 2 nodes, which can hold NODE_MIN_FREEMEM bytes
	 * The reason is that:
	 * 1. migrate_pages() is expected to succeed
	 * 2. this test avoids hitting:
	 *    Bug 870326 - migrate_pages() reports success, but pages are
	 *                 not moved to desired node
	 *    https://bugzilla.redhat.com/show_bug.cgi?id=870326
	 */
	nodeA = nodeB = -1;
	for (i = 0; i < num_nodes; i++) {
		p = numa_alloc_onnode(NODE_MIN_FREEMEM, nodes[i]);
		if (p == NULL)
			break;
		memset(p, 0xff, NODE_MIN_FREEMEM);

		j = 0;
		while (j < NODE_MIN_FREEMEM) {
			if (addr_on_node(p + j) != nodes[i])
				break;
			j += pagesize;
		}
		numa_free(p, NODE_MIN_FREEMEM);

		if (j >= NODE_MIN_FREEMEM) {
			if (nodeA == -1)
				nodeA = nodes[i];
			else if (nodeB == -1)
				nodeB = nodes[i];
			else
				break;
		}
	}

	if (nodeA == -1 || nodeB == -1)
		tst_brk(TCONF, "at least 2 NUMA nodes with "
			 "free mem > %d are needed", NODE_MIN_FREEMEM);
	tst_res(TINFO, "Using nodes: %d %d", nodeA, nodeB);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brk(TBROK | TERRNO, "getpwnam failed");
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.test_all = run,
	.setup = setup,
	.save_restore = (const struct tst_path_val[]) {
		{"?/proc/sys/kernel/numa_balancing", "0"},
		{}
	},
};
#else
TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
