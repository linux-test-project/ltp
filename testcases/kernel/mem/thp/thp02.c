/*
 * Copyright (C) 2011  Red Hat, Inc.
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
 * thp02 - detect mremap bug when THP is enabled.
 *
 * There was a bug in mremap THP support, sometimes crash happened
 * due to the following reason according to developers:
 *
 * "alloc_new_pmd was forcing the allocation of a pte before calling
 * move_huge_page and that resulted in a VM_BUG_ON in move_huge_page
 * because the pmd wasn't zero."
 *
 * There are 4 cases to test this bug:
 *
 * 1) old_addr hpage aligned, old_end not hpage aligned, new_addr
 *    hpage aligned;
 * 2) old_addr hpage aligned, old_end not hpage aligned, new_addr not
 *    hpage aligned;
 * 3) old_addr not hpage aligned, old_end hpage aligned, new_addr
 *    hpage aligned;
 * 4) old_addr not hpage aligned, old_end hpage aligned, new_addr not
 *    hpage aligned.
 *
 */

#define _GNU_SOURCE
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"

char *TCID = "thp02";
int TST_TOTAL = 1;

#ifdef HAVE_MREMAP_FIXED
static int ps;
static long hps, size;
static void *p, *p2, *p3, *p4;

static void do_mremap(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		do_mremap();
	}
	tst_resm(TPASS, "Still alive.");

	cleanup();
	tst_exit();

}

static void do_mremap(void)
{
	int i;
	void *old_addr, *new_addr;

	for (i = 0; i < 4; i++) {
		if (posix_memalign(&p, hps, size))
			tst_brkm(TBROK | TERRNO, cleanup, "memalign p");
		if (posix_memalign(&p2, hps, size))
			tst_brkm(TBROK | TERRNO, cleanup, "memalign p2");
		if (posix_memalign(&p3, hps, size))
			tst_brkm(TBROK | TERRNO, cleanup, "memalign p3");

		memset(p, 0xff, size);
		memset(p2, 0xff, size);
		memset(p3, 0x77, size);

		/*
		 * Will try to do the following 4 mremaps cases:
		 *   mremap(p, size-ps, size-ps, flag, p3);
		 *   mremap(p, size-ps, size-ps, flag, p3+ps);
		 *   mremap(p+ps, size-ps, size-ps, flag, p3);
		 *   mremap(p+ps, size-ps, size-ps, flag, p3+ps);
		 */
		old_addr = p + ps * (i >> 1);
		new_addr = p3 + ps * (i & 1);
		tst_resm(TINFO, "mremap %p to %p", old_addr, new_addr);

		p4 = mremap(old_addr, size - ps, size - ps,
			    MREMAP_FIXED | MREMAP_MAYMOVE, new_addr);
		if (p4 == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup, "mremap");
		if (memcmp(p4, p2, size - ps))
			tst_brkm(TBROK, cleanup, "mremap bug");
	}
}

void setup(void)
{
	if (access(PATH_THP, F_OK) == -1)
		tst_brkm(TCONF, NULL, "THP not enabled in kernel?");

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	ps = sysconf(_SC_PAGESIZE);
	hps = read_meminfo("Hugepagesize:") * 1024;
	size = hps * 4;
}

void cleanup(void)
{
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "MREMAP_FIXED not present in <sys/mman.h>");
}
#endif /* HAVE_MREMAP_FIXED */
