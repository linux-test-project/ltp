/*
 * This is a reproducer from mainline commit
 * 9d8cebd4bcd7c3878462fdfda34bbcdeb4df7ef4:
 *
 * "Strangely, current mbind() doesn't merge vma with neighbor vma
 * although it's possible.  Unfortunately, many vma can reduce
 * performance..."
 *
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
#include "test.h"
#include "usctest.h"
#include "config.h"

char *TCID = "mbind01";
int TST_TOTAL = 1;

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
#include <numaif.h>
#include <numa.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static unsigned long pagesize;
static int opt_node;
static char *opt_nodestr;
static option_t options[] = {
	{ "n:", &opt_node,	&opt_nodestr},
	{ NULL, NULL,		NULL}
};

static void usage(void);

int main(int argc, char** argv)
{
	FILE *fp;
	void* addr;
	int node, err, lc;
	struct bitmask *nmask = numa_allocate_nodemask();
	char buf[BUFSIZ], start[BUFSIZ], end[BUFSIZ], string[BUFSIZ];
	char *p, *msg;

	pagesize = getpagesize();
	msg = parse_opts(argc, argv, options, usage);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	if (opt_node) {
		node = atoi(optarg);
		if (node < 1)
			tst_brkm(TBROK, NULL,
				"Number of NUMA nodes cannot be less that 1.");
		numa_bitmask_setbit(nmask, node);
	} else
		numa_bitmask_setbit(nmask, 0);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		addr = mmap(NULL, pagesize*3, PROT_READ|PROT_WRITE,
			MAP_ANON|MAP_PRIVATE, 0, 0);
		if (addr == MAP_FAILED)
			tst_brkm(TBROK|TERRNO, NULL, "mmap");

		tst_resm(TINFO, "pid = %d addr = %p", getpid(), addr);
		/* make page populate */
		memset(addr, 0, pagesize*3);

		/* first mbind */
		err = mbind(addr+pagesize, pagesize, MPOL_BIND, nmask->maskp,
			nmask->size, MPOL_MF_MOVE_ALL);
		if (err != 0)
			tst_brkm(TBROK|TERRNO, NULL, "mbind1");

		/* second mbind */
		err = mbind(addr, pagesize*3, MPOL_DEFAULT, NULL, 0, 0);
		if (err != 0)
			tst_brkm(TBROK|TERRNO, NULL, "mbind2");

		/* /proc/self/maps in the form of
		   "00400000-00406000 r-xp 00000000". */
		sprintf(string, "%p", addr);
		fp = fopen("/proc/self/maps", "r");
		if (fp == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "fopen");

		while (fgets(buf, BUFSIZ, fp) != NULL) {
			/* Find out the 1st VMAs. */
			if (sscanf(buf, "%s-%s ", start, end) != 2)
				continue;
			/* Remove leading 0x. */
			if (strcmp(string + 2, start) != 0)
				continue;
			/* Find out the second VMA. */
			if (fgets(buf, BUFSIZ, fp) == NULL)
				tst_brkm(TBROK|TERRNO, NULL, "fgets");
			p = strtok(buf, "-");
			if (p == NULL || strcmp(p, end) != 0)
				tst_resm(TPASS, "only 1 VMA.");
			else
				tst_resm(TFAIL, "more than 1 VMA.");
		}
		fclose(fp);
		if (munmap(addr, pagesize*3) == -1)
			tst_brkm(TWARN|TERRNO, NULL, "munmap");
	}
	tst_exit();
}

void usage(void)
{
	printf("  -n      Number of NUMA nodes\n");
}

#else /* no NUMA */
int main(void) {
	tst_resm(TCONF, "no NUMA development packages installed.");
	tst_exit();
}
#endif