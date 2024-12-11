// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2008 David Gibson, IBM Corporation.
 * Author: David Gibson
 */

/*\
 * [Description]
 *
 * This checks copy-on-write semantics, specifically the semantics of a
 * MAP_PRIVATE mapping across a fork().  Some versions of the powerpc
 * kernel had a bug in huge_ptep_set_wrprotect() which would fail to
 * flush the hash table after setting the write protect bit in the parent's
 * page tables, thus allowing the parent to pollute the child's mapping.
 *
 */

#include <sys/wait.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "hugetlb.h"

#define C1		0x1234ABCD
#define C2		~0x1234ABCD
#define C3		0xfeef5678
#define MNTPOINT "hugetlbfs/"
static int  fd = -1;
static long hpage_size;

static void run_test(void)
{
	volatile unsigned int *p;
	int parent_readback;
	pid_t pid;

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	*p = C1;

	pid = SAFE_FORK();
	if (pid != 0) {
		*p = C2;
		TST_CHECKPOINT_WAKE_AND_WAIT(0);
		parent_readback = *p;
		TST_CHECKPOINT_WAKE(0);
	} else {
		unsigned int child_readback = 0;

		TST_CHECKPOINT_WAIT(0);
		child_readback = *p;
		*p = C3;
		TST_CHECKPOINT_WAKE_AND_WAIT(0);
		TST_EXP_EXPR(child_readback == C1, "0x%x == child_readback (0x%x)",
				C1, child_readback);
		exit(0);
	}
	tst_reap_children();
	TST_EXP_EXPR(parent_readback == C2, "0x%x == parent_readback (0x%x)",
			C2, parent_readback);

	SAFE_MUNMAP((void *)p, hpage_size);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "86df86424939"},
		{}
	},
	.needs_root = 1,
	.needs_checkpoints = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
