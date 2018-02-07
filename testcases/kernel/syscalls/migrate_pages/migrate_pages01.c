/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
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

/*
 * errno tests for migrate_pages() syscall
 */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"
#include "numa_helper.h"
#include "migrate_pages_common.h"

char *TCID = "migrate_pages01";
int TST_TOTAL = 1;

option_t options[] = {
	{NULL, NULL, NULL}
};

#ifdef HAVE_NUMA_V2

static unsigned long *sane_old_nodes;
static unsigned long *sane_new_nodes;
static int sane_nodemask_size;
static int sane_max_node;

static void setup(void);
static void cleanup(void);

static void test_sane_nodes(void)
{
	tst_resm(TINFO, "test_empty_mask");
	TEST(ltp_syscall(__NR_migrate_pages, 0, sane_max_node,
		     sane_old_nodes, sane_new_nodes));
	check_ret(0);
}

static void test_invalid_pid(void)
{
	pid_t invalid_pid = -1;

	tst_resm(TINFO, "test_invalid_pid -1");
	TEST(ltp_syscall(__NR_migrate_pages, invalid_pid, sane_max_node,
		     sane_old_nodes, sane_new_nodes));
	check_ret(-1);
	check_errno(ESRCH);

	tst_resm(TINFO, "test_invalid_pid unused pid");
	invalid_pid = tst_get_unused_pid(cleanup);
	TEST(ltp_syscall(__NR_migrate_pages, invalid_pid, sane_max_node,
		     sane_old_nodes, sane_new_nodes));
	check_ret(-1);
	check_errno(ESRCH);
}

static void test_invalid_masksize(void)
{
	tst_resm(TINFO, "test_invalid_masksize");
	TEST(ltp_syscall(__NR_migrate_pages, 0, -1, sane_old_nodes,
		     sane_new_nodes));
	check_ret(-1);
	check_errno(EINVAL);
}

static void test_invalid_mem(void)
{
	unsigned long *p;

	tst_resm(TINFO, "test_invalid_mem -1");
	TEST(ltp_syscall(__NR_migrate_pages, 0, sane_max_node, -1, -1));
	check_ret(-1);
	check_errno(EFAULT);

	tst_resm(TINFO, "test_invalid_mem invalid prot");
	p = mmap(NULL, getpagesize(), PROT_NONE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");
	TEST(ltp_syscall(__NR_migrate_pages, 0, sane_max_node, p, p));
	check_ret(-1);
	check_errno(EFAULT);

	SAFE_MUNMAP(cleanup, p, getpagesize());
	tst_resm(TINFO, "test_invalid_mem unmmaped");
	TEST(ltp_syscall(__NR_migrate_pages, 0, sane_max_node, p, p));
	check_ret(-1);
	check_errno(EFAULT);
}

static void test_invalid_nodes(void)
{
	int *nodes;
	int num_nodes, ret, i;
	int invalid_node = 0;
	unsigned long *old_nodes, *new_nodes;

	tst_resm(TINFO, "test_invalid_nodes");
	ret = get_allowed_nodes_arr(NH_MEMS, &num_nodes, &nodes);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "get_allowed_nodes_arr: %d", ret);

	/* get first node which is not in nodes */
	for (i = 0; i < num_nodes; i++, invalid_node++)
		if (invalid_node != nodes[i])
			break;
	if (invalid_node < sane_max_node) {
		old_nodes = SAFE_MALLOC(NULL, sane_nodemask_size);
		new_nodes = SAFE_MALLOC(NULL, sane_nodemask_size);
		memcpy(old_nodes, sane_old_nodes, sane_nodemask_size);
		memset(new_nodes, 0, sane_nodemask_size);
		set_bit(new_nodes, invalid_node, 1);

		TEST(ltp_syscall(__NR_migrate_pages, 0, sane_max_node,
			     old_nodes, new_nodes));
		check_ret(-1);
		check_errno(EINVAL);
		free(old_nodes);
		free(new_nodes);
	} else {
		tst_resm(TCONF, "All possible nodes are present");
	}

	free(nodes);
}

static void test_invalid_perm(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	int status;
	pid_t child_pid;
	pid_t parent_pid;
	int ret = 0;

	tst_resm(TINFO, "test_invalid_perm");
	parent_pid = getpid();
	fflush(stdout);
	child_pid = fork();
	switch (child_pid) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
		break;
	case 0:
		ltpuser = getpwnam(nobody_uid);
		if (ltpuser == NULL)
			tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");
		SAFE_SETUID(NULL, ltpuser->pw_uid);
		TEST(ltp_syscall(__NR_migrate_pages, parent_pid,
			     sane_max_node, sane_old_nodes, sane_new_nodes));
		ret |= check_ret(-1);
		ret |= check_errno(EPERM);
		exit(ret);
	default:
		SAFE_WAITPID(cleanup, child_pid, &status, 0);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child returns %d", status);
	}
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, NULL);

	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_sane_nodes();
		test_invalid_pid();
		test_invalid_masksize();
		test_invalid_mem();
		test_invalid_nodes();
		test_invalid_perm();
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	int node, ret;

	tst_require_root();
	TEST(ltp_syscall(__NR_migrate_pages, 0, 0, NULL, NULL));

	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brkm(TCONF, NULL, "requires NUMA with at least 1 node");

	ret = get_allowed_nodes(NH_MEMS, 1, &node);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes_arr: %d",
			 ret);

	sane_max_node = LTP_ALIGN(get_max_node(), sizeof(unsigned long)*8);
	sane_nodemask_size = sane_max_node / 8;
	sane_old_nodes = SAFE_MALLOC(NULL, sane_nodemask_size);
	sane_new_nodes = SAFE_MALLOC(NULL, sane_nodemask_size);
	memset(sane_old_nodes, 0, sane_nodemask_size);
	memset(sane_new_nodes, 0, sane_nodemask_size);

	set_bit(sane_old_nodes, node, 1);
	set_bit(sane_new_nodes, node, 1);

	TEST_PAUSE;
}

static void cleanup(void)
{
	free(sane_old_nodes);
	free(sane_new_nodes);
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
}
#endif
