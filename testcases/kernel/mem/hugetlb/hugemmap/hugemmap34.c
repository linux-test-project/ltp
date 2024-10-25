// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
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

#include "hugetlb.h"
#include <errno.h>

#ifdef __LP64__
#define STACK_ALLOCATION_SIZE	(256*1024*1024)
#else
#define STACK_ALLOCATION_SIZE	(16*1024*1024)
#endif
#define PALIGN(p, a) ((void *)LTP_ALIGN((unsigned long)(p), (a)))
#define MNTPOINT "hugetlbfs/"
static int  fd = -1;
static unsigned long long hpage_size;
static int page_size;


void do_child(void *stop_address)
{
	volatile int *x;

	/* corefile from this process is not interesting and limiting
	 * its size can save a lot of time. '1' is a special value,
	 * that will also abort dumping via pipe, which by default
	 * sets limit to RLIM_INFINITY.
	 */
	tst_no_corefile(1);

	do {
		x = alloca(STACK_ALLOCATION_SIZE);
		*x = 1;
	} while ((void *)x >= stop_address);
}

static void run_test(void)
{
	int pid, status;
	void *stack_address, *mmap_address, *heap_address, *map;

	stack_address = alloca(0);
	heap_address = sbrk(0);

	/*
	 * paranoia: start mapping two hugepages below the start of the stack,
	 * in case the alignment would cause us to map over something if we
	 * only used a gap of one hugepage.
	 */
	mmap_address = PALIGN(stack_address - 2 * hpage_size, hpage_size);
	do {
		map = mmap(mmap_address, hpage_size, PROT_READ|PROT_WRITE,
				MAP_SHARED | MAP_FIXED_NOREPLACE, fd, 0);
		if (map == MAP_FAILED) {
			if (errno == ENOMEM) {
				tst_res(TCONF, "There is no enough memory in the system to do mmap");
				return;
			}
		}
		mmap_address -= hpage_size;
		/*
		 * if we get all the way down to the heap, stop trying
		 */
	} while (mmap_address <= heap_address);
	pid = SAFE_FORK();
	if (pid == 0)
		do_child(mmap_address);

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV)
		tst_res(TPASS, "Child killed by %s as expected", tst_strsig(SIGSEGV));
	else
		tst_res(TFAIL, "Child: %s", tst_strstatus(status));
}

void setup(void)
{
	struct rlimit r;

	page_size = getpagesize();
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
	fd = tst_creat_unlinked(MNTPOINT, 0);
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
