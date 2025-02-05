// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010  Red Hat, Inc.
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test examines the functionality of mapping and unmapping /dev/zero,
 * which is a common method for allocating anonymous memory in Solaris.
 *
 * The primary objective is to determine whether it is possible to successfully
 * map and unmap /dev/zero, as well as to read from and write to the mapped
 * memory. The design of this test is inspired by two previous bugs,
 * incorporating variations based on their reproducers. Additionally, the test
 * accepts an option to mmap/munmap anonymous pages.
 *
 * One of the bugs aims to trigger a panic related to the transparent hugepage
 * feature. The split_huge_page function is particularly strict in verifying
 * that the reverse mapping (rmap) walk is accurate. This strictness is crucial
 * because if the page_mapcount is not stable or correct, the subsequent
 * __split_huge_page_refcount operation could lead to inconsistent page_count()
 * values for the subpages. A bug related to fork caused the rmap walk to find
 * the parent huge-pmd twice instead of once, due to the anon_vma_chain objects
 * of the child VMA still pointing to the parent's vma->vm_mm. This
 * inconsistency triggers a failure in the split_huge_page mapcount versus
 * page_mapcount check, resulting in a BUG_ON.
 *
 * The second bug involves the mmap() operation on /dev/zero, which invokes
 * map_zero(). On RHEL5, this function maps the ZERO_PAGE into every page table
 * entry (PTE) within the specified virtual address range. When an application
 * maps a region from 5M to 16M and operates in a multi-threaded environment,
 * the subsequent munmap() of /dev/zero leads to TLB shootdowns across all CPUs.
 * When this occurs thousands or millions of times, it severely degrades
 * application performance. The optimization of mapping the ZERO_PAGE in every
 * PTE within that virtual address range was intended to enhance page fault
 * handling times on RHEL5 but has since been modified or removed in upstream
 * versions.
 */

#include "tst_test.h"

#define SIZE (5 * TST_MB)
#define PATH_KSM "/sys/kernel/mm/ksm/"

static size_t page_sz;
static char *memory;

struct tcase {
	int anon;
	int add_ksm;
} tcases[] = {
	{ .anon = 1 },
	{ .add_ksm = 1 },
	{ .anon = 1, .add_ksm = 1 },
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	int fd = -1;

	if (tc->add_ksm) {
		if (access(PATH_KSM, F_OK) == -1)
			tst_brk(TCONF, "KSM configuration is not enabled");
		else
			tst_res(TINFO, "Add to KSM regions");
	}

	if (tc->anon)
		tst_res(TINFO, "Use anonymous pages");
	else
		tst_res(TINFO, "Use /dev/zero device");

	if (tc->anon) {
		memory = SAFE_MMAP(NULL, SIZE + SIZE - page_sz,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	} else {
		fd = SAFE_OPEN("/dev/zero", O_RDWR, 0666);

		memory = SAFE_MMAP(NULL, SIZE + SIZE - page_sz,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE, fd, 0);
	}

	if (tc->add_ksm) {
		if (madvise(memory, SIZE + SIZE - page_sz, MADV_MERGEABLE) == -1)
			tst_brk(TBROK | TERRNO, "madvise error");
	}

	memory[SIZE] = 0;

	for (int i = 0; i < 3; i++) {
		if (!SAFE_FORK()) {
			SAFE_MUNMAP(memory + SIZE + page_sz, SIZE - page_sz * 2);
			exit(0);
		}
	}

	SAFE_MUNMAP(memory, SIZE + SIZE - page_sz);

	tst_reap_children();

	tst_res(TPASS, "All memory has been released");

	if (fd != -1)
		SAFE_CLOSE(fd);
}

static void setup(void)
{
	page_sz = SAFE_SYSCONF(_SC_PAGESIZE);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.forks_child = 1,
	.tcnt = ARRAY_SIZE(tcases),
};
