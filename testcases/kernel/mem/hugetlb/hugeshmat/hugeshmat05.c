// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2017 Red Hat, Inc.
 *
 *
 * DESCRIPTION
 *	shmget()/shmat() fails to allocate huge pages shared memory segment
 *	with EINVAL if its size is not in the range [ N*HUGE_PAGE_SIZE - 4095,
 *	N*HUGE_PAGE_SIZE ]. This is a problem in the memory segment size round
 *	up algorithm. The requested size is rounded up to PAGE_SIZE (4096), but
 *	if this roundup does not match HUGE_PAGE_SIZE (2Mb) boundary - the
 *	allocation fails.
 *
 *	This bug is present in all RHEL6 versions, but not in RHEL7. It looks
 *	like this was fixed in mainline kernel > v3.3 by the following patches:
 *
 *	091d0d55b286 (shm: fix null pointer deref when userspace specifies
 *		 invalid hugepage size)
 *	af73e4d9506d (hugetlbfs: fix mmap failure in unaligned size request)
 *	42d7395feb56 (mm: support more pagesizes for MAP_HUGETLB/SHM_HUGETLB)
 *	40716e29243d (hugetlbfs: fix alignment of huge page requests)
 *
 * AUTHORS
 *	Vladislav Dronov <vdronov@redhat.com>
 *	Li Wang <liwang@redhat.com>
 *
 */

#include "hugetlb.h"

static long page_size;
static long hpage_size;

#define N 4

void setup(void)
{
	page_size = getpagesize();
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
}

void shm_test(size_t size)
{
	int shmid;
	char *shmaddr;

	shmid = shmget(IPC_PRIVATE, size, 0600 | IPC_CREAT | SHM_HUGETLB);
	if (shmid < 0)
		tst_brk(TBROK | TERRNO, "shmget failed");

	shmaddr = shmat(shmid, 0, 0);
	if (shmaddr == (char *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brk(TFAIL | TERRNO,
			 "Bug: shared memory attach failure.");
	}

	shmaddr[0] = 1;
	tst_res(TINFO, "allocated %zu huge bytes", size);

	if (shmdt((const void *)shmaddr) != 0) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brk(TFAIL | TERRNO, "Detach failure.");
	}

	shmctl(shmid, IPC_RMID, NULL);
}

static void test_hugeshmat(void)
{
	unsigned int i;

	const size_t tst_sizes[] = {
		N * hpage_size - page_size,
		N * hpage_size - page_size - 1,
		hpage_size,
		hpage_size + 1
	};

	for (i = 0; i < ARRAY_SIZE(tst_sizes); ++i)
		shm_test(tst_sizes[i]);

	tst_res(TPASS, "No regression found.");
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.test_all = test_hugeshmat,
	.setup = setup,
	.hugepages = {N+1, TST_NEEDS},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "091d0d55b286c9340201b4ed4470be87fc568228"},
		{"linux-git", "af73e4d9506d3b797509f3c030e7dcd554f7d9c4"},
		{"linux-git", "42d7395feb56f0655cd8b68e06fc6063823449f8"},
		{"linux-git", "40716e29243de46720e5773797791466c28904ec"},
		{}
	}
};
