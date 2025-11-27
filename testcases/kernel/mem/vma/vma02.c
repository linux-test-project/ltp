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
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "test.h"
#include "safe_macros.h"
#include "numa_helper.h"

char *TCID = "vma02";
int TST_TOTAL = 1;

#ifdef HAVE_NUMA_V2

static unsigned long pagesize;
static int opt_node;
static char *opt_nodestr;
static option_t options[] = {
	{"n:", &opt_node, &opt_nodestr},
	{NULL, NULL, NULL}
};

static void usage(void);

int main(int argc, char **argv)
{
	FILE *fp;
	void *addr, *start, *end, *lastend;
	int node, err, lc;
	char buf[BUFSIZ];
	struct bitmask *nmask = numa_allocate_nodemask();

	pagesize = getpagesize();
	tst_parse_opts(argc, argv, options, usage);

	if (opt_node) {
		node = SAFE_STRTOL(NULL, opt_nodestr, 1, LONG_MAX);
	} else {
		err = get_allowed_nodes(NH_MEMS | NH_MEMS, 1, &node);
		if (err == -3)
			tst_brkm(TCONF, NULL, "requires at least one node.");
		else if (err < 0)
			tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes");
	}
	numa_bitmask_setbit(nmask, node);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		addr = mmap(NULL, pagesize * 3, PROT_WRITE,
			    MAP_ANON | MAP_PRIVATE, 0, 0);
		if (addr == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, NULL, "mmap");

		tst_resm(TINFO, "pid = %d addr = %p", getpid(), addr);
		/* make page populate */
		memset(addr, 0, pagesize * 3);

		/* first mbind */
		err = mbind(addr + pagesize, pagesize, MPOL_BIND, nmask->maskp,
			    nmask->size, MPOL_MF_MOVE_ALL);
		if (err != 0) {
			if (errno != ENOSYS)
				tst_brkm(TBROK | TERRNO, NULL, "mbind1");
			else
				tst_brkm(TCONF, NULL,
					 "mbind syscall not implemented on this system.");
		}

		/* second mbind */
		err = mbind(addr, pagesize * 3, MPOL_DEFAULT, NULL, 0, 0);
		if (err != 0)
			tst_brkm(TBROK | TERRNO, NULL, "mbind2");

		/* /proc/self/maps in the form of
		   "00400000-00406000 r-xp 00000000". */
		fp = SAFE_FOPEN(NULL, "/proc/self/maps", "r");

		while (fgets(buf, BUFSIZ, fp) != NULL) {
			if (sscanf(buf, "%p-%p ", &start, &end) != 2)
				continue;

			if (start == addr) {
				tst_resm(TINFO, "start = %p, end = %p",
					 start, end);
				if (end == addr + pagesize * 3) {
					tst_resm(TPASS, "only 1 VMA.");
					break;
				}

				lastend = end;
				while (fgets(buf, BUFSIZ, fp) != NULL) {
					/* No more VMAs, break */
					if (sscanf(buf, "%p-%p ", &start,
						   &end) != 2)
						break;
					tst_resm(TINFO, "start = %p, end = %p",
						 start, end);

					/* more VMAs found */
					if (start == lastend)
						lastend = end;
					if (end == addr + pagesize * 3) {
						tst_resm(TFAIL,
							 ">1 unmerged VMAs.");
						break;
					}
				}
				if (end != addr + pagesize * 3)
					tst_resm(TFAIL, "no matched VMAs.");
				break;
			}
		}
		fclose(fp);
		if (munmap(addr, pagesize * 3) == -1)
			tst_brkm(TWARN | TERRNO, NULL, "munmap");
	}
	tst_exit();
}

void usage(void)
{
	printf("  -n      Number of NUMA nodes\n");
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
}
#endif
