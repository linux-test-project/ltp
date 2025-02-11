// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation
 * Author: David Gibson & Adam Litke
 */

/*\
 * On PowerPC, the address space is divided into segments.  These segments can
 * contain either huge pages or normal pages, but not both.  All segments are
 * initially set up to map normal pages.  When a huge page mapping is created
 * within a set of empty segments, they are "enabled" for huge pages at that
 * time.  Once enabled for huge pages, they can not be used again for normal
 * pages for the remaining lifetime of the process.
 *
 * If the segment immediately preceeding the segment containing the stack is
 * converted to huge pages and the stack is made to grow into the this
 * preceeding segment, some kernels may attempt to map normal pages into the
 * huge page-only segment -- resulting in bugs.
 */

#define _GNU_SOURCE
#include "lapi/mmap.h"
#include "hugetlb.h"
#include <errno.h>
#include <inttypes.h>
#include <sched.h>

#ifdef __LP64__
#define STACK_ALLOCATION_SIZE	(256*1024*1024)
#else
#define STACK_ALLOCATION_SIZE	(16*1024*1024)
#endif
#define MNTPOINT "hugetlbfs/"
#define PATH_HUGEPAGE "/sys/kernel/mm/hugepages"

#define STACKS_MAX 64

static int  fd = -1;
static uintptr_t hpage_size, stacks[STACKS_MAX];
static int stacks_num;
static void *hpage_addr, *stack_addr;
static void **shared_area;

int do_child(void *stop_address)
{
	volatile char *x;

	/* corefile from this process is not interesting and limiting
	 * its size can save a lot of time. '1' is a special value,
	 * that will also abort dumping via pipe, which by default
	 * sets limit to RLIM_INFINITY.
	 */
	tst_no_corefile(1);
	tst_res(TINFO, "Child process starting with top of stack at %p", &x);

	do {
		x = alloca(STACK_ALLOCATION_SIZE);
		*shared_area = (void *)x;
		*x = 1;
	} while ((void *)x >= stop_address);
	exit(0);
}

static void run_test(void)
{
	int pid, status;

	pid = ltp_clone(CLONE_VM | CLONE_VFORK | SIGCHLD, do_child,
		hpage_addr, hpage_size, stack_addr);
	if (pid == 0)
		do_child(hpage_addr);

	SAFE_WAITPID(pid, &status, 0);
	tst_res(TINFO, "Child process extended stack up to %p, hasn't reached %p",
		*shared_area, *shared_area - STACK_ALLOCATION_SIZE);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		tst_res(TPASS, "Child killed by %s as expected", tst_strsig(SIGSEGV));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));
}

/* Return start address of next mapping or 0 */
static uintptr_t  get_next_mapping_start(uintptr_t addr)
{
	FILE *fp = fopen("/proc/self/maps", "r");

	if (fp == NULL)
		tst_brk(TBROK | TERRNO, "Failed to open /proc/self/maps.");

	while (!feof(fp)) {
		uintptr_t start, end;
		int ret;

		ret = fscanf(fp, "%"PRIxPTR"-%"PRIxPTR" %*[^\n]\n", &start, &end);
		if (ret != 2) {
			fclose(fp);
			tst_brk(TBROK | TERRNO, "Couldn't parse /proc/self/maps line.");
		}

		if (start > addr) {
			fclose(fp);
			return start;
		}
	}
	fclose(fp);
	return 0;
}

static int is_addr_in_stacks(uintptr_t addr)
{
	int i;

	for (i = 0; i < stacks_num; i++) {
		if (addr == stacks[i])
			return 1;
	}
	return 0;
}

void setup(void)
{
	struct rlimit r;
	int i;

	hpage_size = tst_get_hugepage_size();
	/*
	 * Setting the stack size to unlimited.
	 */
	r.rlim_cur = RLIM_INFINITY;
	r.rlim_max = RLIM_INFINITY;
	SAFE_SETRLIMIT(RLIMIT_STACK, &r);
	SAFE_GETRLIMIT(RLIMIT_STACK, &r);
	if (r.rlim_cur != RLIM_INFINITY)
		tst_brk(TCONF, "Stack rlimit must be 'unlimited'");

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	/* shared memory to pass a (void *) from child process */
	shared_area = SAFE_MMAP(0, getpagesize(), PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	/*
	 * We search for potential stack addresses by looking at
	 * places where kernel would map next huge page and occupying that
	 * address as potential stack. When huge page lands in such place
	 * that next mapping is one of our stack mappings, we use those
	 * two for the test. We try to map huge pages in child process so that
	 * slices in parent are not affected.
	 */
	tst_res(TINFO, "searching for huge page and child stack placement");
	for (i = 0; i < STACKS_MAX; i++) {
		uintptr_t next_start;
		int pid, status;

		pid = SAFE_FORK();
		if (pid == 0) {
			*shared_area = SAFE_MMAP(0, hpage_size, PROT_READ|PROT_WRITE,
				MAP_PRIVATE, fd, 0);
			SAFE_MUNMAP(*shared_area, hpage_size);
			exit(0);
		}
		SAFE_WAITPID(pid, &status, 0);
		if (status != 0)
			tst_brk(TFAIL, "Child: %s", tst_strstatus(status));

		next_start = get_next_mapping_start((uintptr_t)*shared_area);
		if (is_addr_in_stacks(next_start)) {
			stack_addr = (void *)next_start;
			hpage_addr = *shared_area;
			break;
		}

		tst_res(TINFO, "potential stack at address %p", *shared_area);
		stacks[stacks_num++] = (uintptr_t) SAFE_MMAP(*shared_area, hpage_size,
			PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE|MAP_GROWSDOWN, -1, 0);
	}

	if (!stack_addr)
		tst_brk(TCONF, "failed to find good place for huge page and stack");

	hpage_addr = mmap(hpage_addr, hpage_size, PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_FIXED, fd, 0);
	if (hpage_addr == MAP_FAILED) {
		if (errno == ENOMEM)
			tst_brk(TCONF, "Not enough memory.");
		tst_brk(TBROK|TERRNO, "mmap failed");
	}
	tst_res(TINFO, "stack = %p-%p, hugepage = %p-%p", stack_addr, stack_addr+hpage_size,
			hpage_addr, hpage_addr+hpage_size);
}

void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "0d59a01bc461"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {1, TST_NEEDS},
	.forks_child = 1,
	.supported_archs = (const char *const []){"ppc", "ppc64", NULL},
};
