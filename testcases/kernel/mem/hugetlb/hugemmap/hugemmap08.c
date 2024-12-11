// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * Some kernel versions after hugepage demand allocation was added used a
 * dubious heuristic to check if there was enough hugepage space available
 * for a given mapping.  The number of not-already-instantiated pages in
 * the mapping was compared against the total hugepage free pool. It was
 * very easy to confuse this heuristic into overcommitting by allocating
 * hugepage memory in chunks, each less than the total available pool size
 * but together more than available.  This would generally lead to OOM
 * SIGKILLs of one process or another when it tried to instantiate pages
 * beyond the available pool.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
#define WITH_OVERCOMMIT 0
#define WITHOUT_OVERCOMMIT 1

static long hpage_size;
static int huge_fd = -1;

static void test_chunk_overcommit(void)
{
	unsigned long totpages, chunk1, chunk2;
	void *p, *q;
	pid_t child;
	int status;

	totpages = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);

	chunk1 = (totpages / 2) + 1;
	chunk2 = totpages - chunk1 + 1;

	tst_res(TINFO, "Free: %ld hugepages available: "
	       "chunk1=%ld chunk2=%ld", totpages, chunk1, chunk2);

	p = SAFE_MMAP(NULL, chunk1*hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 huge_fd, 0);

	q = mmap(NULL, chunk2*hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 huge_fd, chunk1*hpage_size);
	if (q == MAP_FAILED) {
		if (errno != ENOMEM) {
			tst_res(TFAIL | TERRNO, "mmap() chunk2");
			goto cleanup1;
		} else {
			tst_res(TPASS, "Successful without overcommit pages");
			goto cleanup1;
		}
	}

	tst_res(TINFO, "Looks like we've overcommitted, testing...");
	/* Looks like we're overcommited, but we need to confirm that
	 * this is bad.  We touch it all in a child process because an
	 * overcommit will generally lead to a SIGKILL which we can't
	 * handle, of course.
	 */
	child = SAFE_FORK();

	if (child == 0) {
		memset(p, 0, chunk1*hpage_size);
		memset(q, 0, chunk2*hpage_size);
		exit(0);
	}

	SAFE_WAITPID(child, &status, 0);

	if (WIFSIGNALED(status)) {
		tst_res(TFAIL, "Killed by signal '%s' due to overcommit",
		     tst_strsig(WTERMSIG(status)));
		goto cleanup2;
	}

	tst_res(TPASS, "Successful with overcommit pages");

cleanup2:
	SAFE_MUNMAP(q, chunk2*hpage_size);

cleanup1:
	SAFE_MUNMAP(p, chunk1*hpage_size);
	SAFE_FTRUNCATE(huge_fd, 0);
}

static void run_test(unsigned int test_type)
{
	switch (test_type) {
	case WITHOUT_OVERCOMMIT:
		tst_res(TINFO, "Without overcommit testing...");
		SAFE_FILE_PRINTF(PATH_OC_HPAGES, "%d", 0);
		break;
	case WITH_OVERCOMMIT:
		tst_res(TINFO, "With overcommit testing...");
		SAFE_FILE_PRINTF(PATH_OC_HPAGES, "%d", 2);
		break;
	}
	test_chunk_overcommit();
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
	huge_fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	SAFE_CLOSE(huge_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.forks_child = 1,
	.save_restore = (const struct tst_path_val[]) {
		{PATH_OC_HPAGES, NULL, TST_SR_TCONF},
		{}
	},
	.tcnt = 2,
	.setup = setup,
	.cleanup = cleanup,
	.test = run_test,
	.hugepages = {3, TST_NEEDS},
};
