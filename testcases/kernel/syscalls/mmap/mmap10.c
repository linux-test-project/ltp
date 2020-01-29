/*
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

/*
 * mmap/munmap /dev/zero: a common way of malloc()/free() anonymous
 * memory on Solaris.
 *
 * The basic purpose of this is a to test if it is possible to map and
 * unmap /dev/zero, and to read and write the mapping. Being inspired
 * by two bugs in the past, the design of the test was added some
 * variations based on the reproducers for them. It also accept an
 * option to mmap/munmap anonymous pages.
 *
 * One is to trigger panic with transparent hugepage feature that
 * split_huge_page is very strict in checking the rmap walk was
 * perfect. Keep it strict because if page_mapcount isn't stable and
 * just right, the __split_huge_page_refcount that follows the rmap
 * walk could lead to erratic page_count()s for the subpages. The bug
 * in fork lead to the rmap walk finding the parent huge-pmd twice
 * instead of just one, because the anon_vma_chain objects of the
 * child vma still point to the vma->vm_mm of the parent. That trips
 * on the split_huge_page mapcount vs page_mapcount check leading to a
 * BUG_ON.
 *
 * The other bug is mmap() of /dev/zero results in calling map_zero()
 * which on RHEL5 maps the ZERO_PAGE in every PTE within that virtual
 * address range. Since the application which maps a region from 5M to
 * 16M in size is also multi-threaded the subsequent munmap() of
 * /dev/zero results is TLB shootdowns to all other CPUs. When this
 * happens thousands or millions of times the application performance
 * is terrible. The mapping ZERO_PAGE in every pte within that virtual
 * address range was an optimization to make the subsequent pagefault
 * times faster on RHEL5 that has been removed/changed upstream.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "test.h"
#include "config.h"

#define SIZE (5*1024*1024)
#define PATH_KSM "/sys/kernel/mm/ksm/"

char *TCID = "mmap10";
int TST_TOTAL = 1;

static int fd, opt_anon, opt_ksm;
static long ps;
static char *x;

void setup(void);
void cleanup(void);
void mmapzero(void);
void help(void);

static option_t options[] = {
	{"a", &opt_anon, NULL},
	{"s", &opt_ksm, NULL},
	{NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, help);

	if (opt_ksm) {
		if (access(PATH_KSM, F_OK) == -1)
			tst_brkm(TCONF, NULL,
				 "KSM configuration is not enabled");
#ifdef HAVE_DECL_MADV_MERGEABLE
		tst_resm(TINFO, "add to KSM regions.");
#else
		tst_brkm(TCONF, NULL, "MADV_MERGEABLE missing in sys/mman.h");
#endif
	}
	if (opt_anon)
		tst_resm(TINFO, "use anonymous pages.");
	else
		tst_resm(TINFO, "use /dev/zero.");

	setup();

	tst_resm(TINFO, "start tests.");
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		mmapzero();
	}

	cleanup();
	tst_exit();
}

void mmapzero(void)
{
	int n;

	if (opt_anon) {
		x = mmap(NULL, SIZE + SIZE - ps, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	} else {
		if ((fd = open("/dev/zero", O_RDWR, 0666)) < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "open");
		x = mmap(NULL, SIZE + SIZE - ps, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE, fd, 0);
	}
	if (x == MAP_FAILED)
		tst_brkm(TFAIL | TERRNO, cleanup, "mmap");
#ifdef HAVE_DECL_MADV_MERGEABLE
	if (opt_ksm) {
		if (madvise(x, SIZE + SIZE - ps, MADV_MERGEABLE) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "madvise");
	}
#endif
	x[SIZE] = 0;

	switch (n = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		if (munmap(x + SIZE + ps, SIZE - ps - ps) == -1)
			tst_brkm(TFAIL | TERRNO, cleanup, "munmap");
		exit(0);
	default:
		break;
	}

	switch (n = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		if (munmap(x + SIZE + ps, SIZE - ps - ps) == -1)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "subsequent munmap #1");
		exit(0);
	default:
		switch (n = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");
		case 0:
			if (munmap(x + SIZE + ps, SIZE - ps - ps) == -1)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "subsequent munmap #2");
			exit(0);
		default:
			break;
		}
		break;
	}

	if (munmap(x, SIZE + SIZE - ps) == -1)
		tst_resm(TFAIL | TERRNO, "munmap all");

	while (waitpid(-1, &n, WUNTRACED | WCONTINUED) > 0)
		if (WEXITSTATUS(n) != 0)
			tst_resm(TFAIL, "child exit status is %d",
				 WEXITSTATUS(n));
}

void cleanup(void)
{
}

void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	if ((ps = sysconf(_SC_PAGESIZE)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "sysconf(_SC_PAGESIZE)");
}

void help(void)
{
	printf("  -a      Test anonymous pages\n");
	printf("  -s      Add to KSM regions\n");
}
