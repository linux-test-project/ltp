/*
 * Copyright (C) 2012  Red Hat, Inc.
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
 *
 * thp03 - Case for spliting unaligned memory.
 *       - System will panic if failed.
 *
 * Modified form a reproducer for
 *          https://patchwork.kernel.org/patch/1358441/
 * Kernel Commit id: 027ef6c87853b0a9df53175063028edb4950d476
 * There was a bug in THP, will crash happened due to the following
 * reason according to developers:
 *
 * most VM places are using pmd_none but a few are still using
 * pmd_present. The meaning is about the same for the pmd. However
 * pmd_present would return the wrong value on PROT_NONE ranges or in
 * case of a non reproducible race with split_huge_page.
 * When the code using pmd_present gets a false negative, the kernel will
 * crash. It's just an annoying DoS with a BUG_ON triggering: no memory
 * corruption and no data corruption (nor userland nor kernel).
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mem.h"
#include "safe_macros.h"
#include "test.h"

char *TCID = "thp03";
int TST_TOTAL = 1;

#ifdef MADV_MERGEABLE

static void thp_test(void);

static long hugepage_size;
static long unaligned_size;
static long page_size;

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		thp_test();
	}
	tst_resm(TPASS, "system didn't crash, pass.");
	cleanup();
	tst_exit();
}

static void thp_test(void)
{
	void *p;

	p = mmap(NULL, unaligned_size, PROT_READ | PROT_WRITE,
		 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (p == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap");

	memset(p, 0x00, unaligned_size);
	if (mprotect(p, unaligned_size, PROT_NONE) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mprotect");

	if (madvise(p + hugepage_size, page_size, MADV_MERGEABLE) == -1) {
		if (errno == EINVAL) {
			tst_brkm(TCONF, cleanup,
			         "MADV_MERGEABLE is not enabled/supported");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup, "madvise");
		}
	}

	switch (fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		exit(0);
	default:
		if (waitpid(-1, NULL, 0) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
	}
}

void setup(void)
{
	if (access(PATH_THP, F_OK) == -1)
		tst_brkm(TCONF, NULL, "THP not enabled in kernel?");

	hugepage_size = read_meminfo("Hugepagesize:") * KB;
	unaligned_size = hugepage_size * 4 - 1;
	page_size = SAFE_SYSCONF(NULL, _SC_PAGESIZE);

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "Kernel doesn't support MADV_MERGEABLE"
		 " or you need to update your glibc-headers");
}
#endif
